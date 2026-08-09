#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"   
#include "esp_timer.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT 
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (4000)
#define MOTORS                  (4)
#define STAGE1_END              3.0f
#define STAGE2_END              (STAGE1_END + 10.0f)   
#define STAGE3_END              (STAGE2_END + 3.0f) 

static const int MOTOR_GPIO[MOTORS] = {13, 25, 26, 33};
static const ledc_channel_t LEDC_CHANNEL[MOTORS] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const int Throttle = {0};

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

    for(int i = 0; i < MOTORS; i++){
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL[i],
            .timer_sel      = LEDC_TIMER,
            .gpio_num       = MOTOR_GPIO[i],
            .duty           = 0, 
            .hpoint         = 0,
        };
        ledc_channel_config(&ledc_channel);
    }

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

void pwm_set_speed (int motor_no, float duty_percent){
    double max_duty = (1 << LEDC_DUTY_RES) - 1;
    double duty = (double)((duty_percent / 100.0f) * max_duty);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL[motor_no], duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL[motor_no]);
}

static int64_t mission_start_us = 0;

void mission_clock_start(void)
{
    mission_start_us = esp_timer_get_time();
}

void flight_path(){
    float elapsed_s = (esp_timer_get_time() - mission_start_us) / 1e6f;

    if (elapsed_s < STAGE1_END) {
        Throttle = 0; 
    }else if (elapsed_s < STAGE2_END) {
        Throttle = 0;
    }else if (elapsed_s < STAGE3_END) {
        Throttle = 0;
    }
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

void app_main (void){
    pwm_config();
    mission_clock_start();
    while (1) {

        flight_path();
        // PID PART

        int speeds[MOTORS] = {0};
        
        speeds[0] = Throttle - roll_output - pitch_output;
        speeds[1] = Throttle + roll_output - pitch_output;
        speeds[2] = Throttle - roll_output + pitch_output;
        speeds[3] = Throttle + roll_output + pitch_output;

        for(int i = 0; i< MOTORS; i++){
            pwm_set_speed(i, speeds[i]);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

    


