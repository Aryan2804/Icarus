#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "hx711.h" 
#include "esp_timer.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT 
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (4000)
#define MOTOR_GPIO              (25)
#define LEDC_CHANNEL            LEDC_CHANNEL_0

#define LOADCELL_DOUT_PIN       GPIO_NUM_14
#define LOADCELL_SCK_PIN        GPIO_NUM_4
#define SCALE_FACTOR            728.69f

static const char *TAG = "HX711";
static volatile int g_current_speed = 0;

static hx711_t dev = {
    .dout = LOADCELL_DOUT_PIN,
    .pd_sck = LOADCELL_SCK_PIN,
    .gain = HX711_GAIN_A_128,
};

void load_cell(void *pvParameters){
    esp_err_t err = hx711_init(&dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HX711 not found (err %d)", err);
    }

    // Tare 
    bool ready = false;
    do {
        hx711_is_ready(&dev, &ready);
        vTaskDelay(pdMS_TO_TICKS(100));
    } while (!ready);

    ESP_LOGI(TAG, "Tare... remove any weights from the scale.");
    vTaskDelay(pdMS_TO_TICKS(3000));
// 3199.66;
        int32_t offset = 3197.765;
    if (hx711_read_average(&dev, 20, &offset) != ESP_OK) {
        ESP_LOGE(TAG, "Tare read failed");
    }
    ESP_LOGI(TAG, "Tare done... offset = %ld", (long)offset);

    //  measurement loop
    while (1) {
        hx711_is_ready(&dev, &ready);

        if (ready) {
            int32_t raw = 0;
            if (hx711_read_average(&dev, 10, &raw) == ESP_OK) {
                float weight_g = (float)(raw - offset) / SCALE_FACTOR;
                int64_t time_ms = esp_timer_get_time() / 1000;

                // CSV row: time_ms,speed_percent,weight_g
                printf("%lld,%d,%.2f\n", (long long)time_ms, g_current_speed, weight_g);
                fflush(stdout);
                // ESP_LOGI(TAG, "raw = %ld, offset-corrected = %ld, weight = %.2f g",
                           (long)raw, (long)(raw - offset), weight_g);
            } else {
                ESP_LOGE(TAG, "Read failed");
            }
        } else {
            ESP_LOGW(TAG, "HX711 not ready.");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void pwm_config (void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  
        .clk_cfg          = LEDC_CLK_SRC,
    };
    ledc_timer_config(&ledc_timer);

    
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .gpio_num       = MOTOR_GPIO,
        .duty           = 0, 
        .hpoint         = 0,
    };
    ledc_channel_config(&ledc_channel);
    

    const uart_port_t uart_num = UART_NUM_0;

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };    
    uart_param_config(uart_num, &uart_config);
    // Added 1024 RX buffer, 0 TX buffer
    uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
}

void pwm_set_speed (float duty_percent){
    double max_duty = (1 << LEDC_DUTY_RES) - 1;
    double duty = (double)((duty_percent / 100.0f) * max_duty);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
}

int uart_read_int() {
    char input[32] = {0};
    int idx = 0;
    uint8_t ch;

    while (idx < sizeof(input) - 1) {
        int len = uart_read_bytes(UART_NUM_0, &ch, 1, pdMS_TO_TICKS(50));
        if (len > 0) {
            // Check for carriage return or newline
            if (ch == '\n' || ch == '\r') {
                if (idx > 0) break; // Only break if we actually typed something
                else continue;
            }
            input[idx++] = ch;
            uart_write_bytes(UART_NUM_0, (const char *)&ch, 1);  // Echo character back
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to CPU
    }

    input[idx] = '\0';
    return atoi(input);
}

void speed_ramp (){
    // int max_speed = 100;

    for(int i=0; i <= 100; i++){
        pwm_set_speed(i);
        g_current_speed = i;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelay(pdMS_TO_TICKS(10000));

    for(int i=100; i >= 0; i--){
        pwm_set_speed(i);
        g_current_speed = i;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
     g_current_speed = 0;

}

void motor_control_task(void *pvParameters){
    while (1) {
        int speed;

        printf("\nEnter desired maximum speed for Motor (0-100): ");
        fflush(stdout);

        // speed = uart_read_int();
        // printf("\nReceived: %d", speed);
        // fflush(stdout);

        speed_ramp();

        printf("\nMotor ramp complete! Starting next loop\n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main (void){
    pwm_config();

    printf("time_ms,speed_percent,weight_g\n");
    fflush(stdout);

    xTaskCreate(load_cell, "load_cell_task", 4096, NULL, 5, NULL);
    xTaskCreate(motor_control_task, "motor_control_task", 4096, NULL, 5, NULL);
    
    // while (1) {
    //     int speed;

    //     printf("\nEnter desired maximum speed for Motor (0-100): ");
    //     fflush(stdout);

    //     speed = uart_read_int();
    //     printf("\nReceived: %d", speed);
    //     fflush(stdout);

    //     speed_ramp(speed);

    //     printf("\nMotor ramp complete! Starting next loop\n");
    //     fflush(stdout);
    //     vTaskDelay(pdMS_TO_TICKS(500));
    // }
    
}