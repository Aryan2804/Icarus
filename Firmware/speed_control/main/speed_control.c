#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"   

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT 
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (4000)
#define MOTORS                  (4)

static const int MOTOR_GPIO[MOTORS] = {13, 25, 26, 33};
static const ledc_channel_t LEDC_CHANNEL[MOTORS] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};

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
    
    while (1) {
        int speeds[MOTORS] = {0};
        char labels[MOTORS] = {'X', 'Y', 'Z', 'W'};

        for (int i = 0; i < MOTORS; i++) {
            // Added \n and explicit fflush to force the terminal to show text immediately
            printf("\nEnter desired speed for Motor %c (0-100): ", labels[i]);
            fflush(stdout); 
            
            speeds[i] = uart_read_int();
            printf("\nReceived: %d", speeds[i]);
            fflush(stdout);
        }

        // Apply speeds to all motors
        for (int i = 0; i < MOTORS; i++) {
            pwm_set_speed(i, speeds[i]);
        }
        
        printf("\n All motor speeds updated! Starting next loop \n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

    


