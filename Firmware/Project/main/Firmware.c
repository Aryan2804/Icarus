#include <stdio.h>
#include "bmi.h"
#include "attitude.h"
#include "motor_control.h"

// void app_main(void)
// {

// }


void app_main(void)
{
    spi_config();
    bmi088_writel();
    pid_init(&roll_state);
    pid_init(&pitch_state);

    while (1) {


          int16_t ax, ay, az, gx, gy, gz;
        bmi088_read_accel(&ax, &ay, &az);
        bmi088_read_gyro(&gx, &gy, &gz);

        ESP_LOGI(TAG, "ACC: x=%d y=%d z=%d | GYRO: x=%d y=%d z=%d", ax, ay, az, gx, gy, gz);

        vTaskDelay(pdMS_TO_TICKS(1));
       
        static int64_t last_time_us = 0;
        int64_t now_us = esp_timer_get_time();
        float dt = (last_time_us == 0) ? 0.001f : (now_us - last_time_us) / 1e6f;
        last_time_us = now_us;

        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        bmi088_read_accel(&ax_raw, &ay_raw, &az_raw);
        bmi088_read_gyro(&gx_raw, &gy_raw, &gz_raw);

        float gx_dps = (float)gx_raw / 16.384f;
        float gy_dps = (float)gy_raw / 16.384f;
        float gz_dps = (float)gz_raw / 16.384f;

        float ax_g = (float)ax_raw / 5460.0f;
        float ay_g = (float)ay_raw / 5460.0f;
        float az_g = (float)az_raw / 5460.0f;

        float accel_roll  = atan2f(ay_g, az_g) * 180.0f / (float)M_PI;
        float accel_pitch = atan2f(-ax_g, sqrtf(ay_g*ay_g + az_g*az_g)) * 180.0f / (float)M_PI;

        static float current_roll = 0.0f;
        static float current_pitch = 0.0f;

        const float alpha = 0.98f;
        current_roll  = alpha * (current_roll  + gx_dps * dt) + (1.0f - alpha) * accel_roll;
        current_pitch = alpha * (current_pitch + gy_dps * dt) + (1.0f - alpha) * accel_pitch;

        float roll_rate  = gx_dps;
        float pitch_rate = gy_dps;

        float throttle      = 0.0f;  // raw throttle channel, same units as tcfg (e.g. µs pulse width)
        float desired_roll  = 0.0f;  // stick input mapped to a target roll angle (deg)
        float desired_pitch = 0.0f;  // stick input mapped to a target pitch angle (deg)

        // ---- PID ----
        float gain_blend;
        int enable_i;
        pid_compute_gain_blend(&tcfg, throttle, &gain_blend, &enable_i);

        float roll_output = pid_update(&roll_state, &roll_gains,
                                        desired_roll, current_roll, roll_rate,
                                        dt, gain_blend, enable_i);

        float pitch_output = pid_update(&pitch_state, &pitch_gains,
                                         desired_pitch, current_pitch, pitch_rate,
                                         dt, gain_blend, enable_i);

        float yaw_output = YAW_OUTPUT_CONSTANT;  // unused in mixer since yaw is fixed

        

    }
}