#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bmi088.h"
#include "pid.h"

static const char *TAG = "FLIGHT_CTRL";

// ---------------- PWM / motor config ----------------
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          4000
#define MOTORS                  4

static const int MOTOR_GPIO[MOTORS] = {13, 25, 26, 33};
static const ledc_channel_t LEDC_CHANNEL[MOTORS] = {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3
};

#define THROTTLE_MIN_US    1000.0f
#define THROTTLE_MAX_US    2000.0f
#define HOVER_THROTTLE_US  1450.0f   

#define STAGE1_END   3.0f
#define STAGE2_END   (STAGE1_END + 10.0f)
#define STAGE3_END   (STAGE2_END + 3.0f)

static int64_t mission_start_us = 0;
static bool armed = false;

// ---------------- PID configuration ----------------
static throttle_config tcfg = { .launch_end = 1350.0f, .flight_start = 1500.0f };

static pid_gains roll_gains = {
    .launch_kp = 18.0f, .launch_kd = 0.5f,
    .flight_kp = 8.0f,  .flight_kd = 0.2f,
    .ki = 1.5f, .max_i_output = 20.0f, .max_output = 100.0f
};
static pid_gains pitch_gains = {
    .launch_kp = 18.0f, .launch_kd = 0.5f,
    .flight_kp = 8.0f,  .flight_kd = 0.2f,
    .ki = 1.5f, .max_i_output = 20.0f, .max_output = 100.0f
};

#define YAW_OUTPUT_CONSTANT 0.0f  // yaw axis is set to zero

static pid_state roll_state, pitch_state;


static void pwm_config(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_CLK_SRC,
    };
    ledc_timer_config(&ledc_timer);

    for (int i = 0; i < MOTORS; i++) {
        ledc_channel_config_t ledc_channel = {
            .speed_mode = LEDC_MODE,
            .channel    = LEDC_CHANNEL[i],
            .timer_sel  = LEDC_TIMER,
            .gpio_num   = MOTOR_GPIO[i],
            .duty       = 0,
            .hpoint     = 0,
        };
        ledc_channel_config(&ledc_channel);
    }
}

static void uart_config_init(void)
{
    uart_config_t uart_cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_0, &uart_cfg);
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
}


static void uart_poll_commands(void)
{
    uint8_t ch;
    int len = uart_read_bytes(UART_NUM_0, &ch, 1, 0);
    if (len > 0) {
        if (ch == 'a' || ch == 'A') {
            armed = true;
            mission_start_us = esp_timer_get_time();
            ESP_LOGW(TAG, "ARMED - motors live");
        } else if (ch == 'd' || ch == 'D') {
            armed = false;
            ESP_LOGW(TAG, "DISARMED");
        }
    }
}

// duty_percent: 0-100
static void pwm_set_speed(int motor_no, float duty_percent)
{
    duty_percent = clamp(duty_percent, 0.0f, 100.0f);
    double max_duty = (double)((1 << LEDC_DUTY_RES) - 1);
    uint32_t duty = (uint32_t)((duty_percent / 100.0f) * max_duty);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL[motor_no], duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL[motor_no]);
}

static float us_to_duty_percent(float us)
{
    us = clamp(us, THROTTLE_MIN_US, THROTTLE_MAX_US);
    return (us - THROTTLE_MIN_US) / (THROTTLE_MAX_US - THROTTLE_MIN_US) * 100.0f;
}


static float flight_path(void)
{
    float elapsed_s = (esp_timer_get_time() - mission_start_us) / 1e6f;

    if (elapsed_s < STAGE1_END) {
        float t = elapsed_s / STAGE1_END;
        return THROTTLE_MIN_US + t * (HOVER_THROTTLE_US - THROTTLE_MIN_US);
    } else if (elapsed_s < STAGE2_END) {
        return HOVER_THROTTLE_US;
    } else if (elapsed_s < STAGE3_END) {
        float t = (elapsed_s - STAGE2_END) / (STAGE3_END - STAGE2_END);
        return HOVER_THROTTLE_US - t * (HOVER_THROTTLE_US - THROTTLE_MIN_US);
    }
    armed = false; // mission complete - cut motors automatically
    return THROTTLE_MIN_US;
}

void app_main(void)
{
    pwm_config();
    uart_config_init();

    if (bmi088_init() != ESP_OK) {
        ESP_LOGE(TAG, "IMU init failed - halting rather than flying blind.");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    pid_init(&roll_state);
    pid_init(&pitch_state);

    ESP_LOGW(TAG, "Send 'a' over UART to arm, 'd' to disarm. Motors stay OFF until armed.");

    float current_roll = 0.0f, current_pitch = 0.0f;
    int64_t last_time_us = 0;
    int64_t last_log_us = 0;

    while (1) {
        int64_t now_us = esp_timer_get_time();
        float dt = (last_time_us == 0) ? 0.001f : (now_us - last_time_us) / 1e6f;
        last_time_us = now_us;

        uart_poll_commands();

        // ---- Read IMU ----
        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        bmi088_read_accel(&ax_raw, &ay_raw, &az_raw);
        bmi088_read_gyro(&gx_raw, &gy_raw, &gz_raw);

        // +/-2000 dps range -> 16.384 LSB/(deg/s)
        float gx_dps = (float)gx_raw / 16.384f;
        float gy_dps = (float)gy_raw / 16.384f;

        // +/-6g range -> 5460 LSB/g
        float ax_g = (float)ax_raw / 5460.0f;
        float ay_g = (float)ay_raw / 5460.0f;
        float az_g = (float)az_raw / 5460.0f;

        float accel_roll  = atan2f(ay_g, az_g) * 180.0f / (float)M_PI;
        float accel_pitch = atan2f(-ax_g, sqrtf(ay_g * ay_g + az_g * az_g)) * 180.0f / (float)M_PI;

        // ---- Complementary filter: fuse gyro (fast, drifts) with accel (slow, noisy) ----
        const float alpha = 0.98f;
        current_roll  = alpha * (current_roll  + gx_dps * dt) + (1.0f - alpha) * accel_roll;
        current_pitch = alpha * (current_pitch + gy_dps * dt) + (1.0f - alpha) * accel_pitch;

        float roll_rate  = gx_dps;
        float pitch_rate = gy_dps;
        float throttle_us   = armed ? flight_path() : THROTTLE_MIN_US;
        float desired_roll  = 0.0f;
        float desired_pitch = 0.0f;

        // PID
        float gain_blend;
        int enable_i;
        pid_compute_gain_blend(&tcfg, throttle_us, &gain_blend, &enable_i);

        float roll_output = pid_update(&roll_state, &roll_gains,
                                        desired_roll, current_roll, roll_rate,
                                        dt, gain_blend, enable_i);
        float pitch_output = pid_update(&pitch_state, &pitch_gains,
                                         desired_pitch, current_pitch, pitch_rate,
                                         dt, gain_blend, enable_i);
        float yaw_output = YAW_OUTPUT_CONSTANT;
        (void)yaw_output; // reserved until a yaw controller exists

        // ---- Motor mixing (X-frame: 0=front-left,1=front-right,2=rear-left,3=rear-right) ----
        // roll/pitch outputs are in the same microsecond scale as throttle_us, so they add directly.
        float mix_us[MOTORS];
        mix_us[0] = throttle_us - roll_output - pitch_output;
        mix_us[1] = throttle_us + roll_output - pitch_output;
        mix_us[2] = throttle_us - roll_output + pitch_output;
        mix_us[3] = throttle_us + roll_output + pitch_output;

        for (int i = 0; i < MOTORS; i++) {
            float duty = armed ? us_to_duty_percent(mix_us[i]) : 0.0f;
            pwm_set_speed(i, duty);
        }

        // ---- Telemetry, throttled to ~10Hz so logging doesn't eat into the 1kHz loop ----
        if (now_us - last_log_us > 100000) {
            ESP_LOGI(TAG, "armed=%d roll=%.1f pitch=%.1f thr_us=%.0f",
                     armed, current_roll, current_pitch, throttle_us);
            last_log_us = now_us;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
