    #include <stdio.h>
    #include "driver/gpio.h"
    #include "driver/ledc.h"
    #include "sdkconfig.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #define LEDC_TIMER              LEDC_TIMER_0
    #define LEDC_MODE               LEDC_LOW_SPEED_MODE
    #define LEDC_DUTY_RES           LEDC_TIMER_13_BIT 
    #define LEDC_DUTY               (4096) 
    #define LEDC_CLK_SRC            LEDC_AUTO_CLK
    #define LEDC_FREQUENCY          (4000)
    #define MOTORS                  (4)
    
    static const int MOTOR_GPIO[MOTORS] = {25, 26, 32, 33};
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
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
        for(int i =0; i < MOTORS; i++){
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL[i],
            .timer_sel      = LEDC_TIMER,
            .gpio_num       = MOTOR_GPIO[i],
            .duty           = 0, 
            .hpoint         = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }
    }

    void pwm_set_speed (int motor_no ,float duty_percent){
        double max_duty = (1 << LEDC_DUTY_RES)-1;
        double duty = (double)((duty_percent/100.0f)*max_duty);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL[motor_no], duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL[motor_no]);
    }
    void app_main (void){
        // int a;
        //    printf("Enter desired speed: "); //testing you:)
        //       scanf("%d", &a); 

        pwm_config();
        pwm_set_speed(0,15);
        pwm_set_speed(1,40);
        pwm_set_speed(2,75);
        pwm_set_speed(3,70);
    
}

    

