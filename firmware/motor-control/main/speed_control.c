#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (4000)
#define MOTORS                  4
#define TAG                     "ESP"

static const int MOTOR_GPIO[MOTORS] = {38, 39, 40, 41};
static const ledc_channel_t LEDC_CHANNEL[MOTORS] = {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3
};
static const char LABELS[MOTORS] = {'X', 'Y', 'Z', 'W'};

/* ---------------------------------------------------------------
   USB console init — required because this board runs native USB
   (D+/D- straight into the S3), not a USB-UART bridge chip.
   Uses the IDF v5.x usb_serial_jtag_vfs API (the old
   esp_vfs_usb_serial_jtag.h calls are deprecated/renamed).
   --------------------------------------------------------------- */
static void usb_console_init(void)
{
    usb_serial_jtag_driver_config_t usb_cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_cfg));

    usb_serial_jtag_vfs_use_driver();

    // Blocking reads/writes (fixes "always reads 0" issue)
    fcntl(fileno(stdin), F_SETFL, 0);
    fcntl(fileno(stdout), F_SETFL, 0);

    // Most terminals send \r on Enter, not \n
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
}

static esp_err_t pwm_config(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_CLK_SRC,
    };
    esp_err_t err = ledc_timer_config(&timer_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed: %s", esp_err_to_name(err));
        return err;
    }

    for (int i = 0; i < MOTORS; i++) {
        ledc_channel_config_t ch_cfg = {
            .speed_mode = LEDC_MODE,
            .channel    = LEDC_CHANNEL[i],
            .timer_sel  = LEDC_TIMER,
            .gpio_num   = MOTOR_GPIO[i],
            .duty       = 0,
            .hpoint     = 0,
        };
        err = ledc_channel_config(&ch_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ledc_channel_config failed for motor %d: %s", i, esp_err_to_name(err));
            return err;
        }
    }
    return ESP_OK;
}

static void pwm_set_speed(int motor_no, int duty_percent)
{
    if (duty_percent < 0)   duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;

    const uint32_t max_duty = (1U << LEDC_DUTY_RES) - 1;
    uint32_t duty = (uint32_t)(((uint64_t)duty_percent * max_duty) / 100);

    esp_err_t err = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL[motor_no], duty);
    if (err == ESP_OK) err = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL[motor_no]);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set duty for motor %d: %s", motor_no, esp_err_to_name(err));
    }
}

/* Returns -1 if no valid integer was entered (blank line / non-numeric) */
static int uart_read_int(void)
{
    char buf[32] = {0};
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return -1;
    }
    buf[strcspn(buf, "\r\n")] = 0;

    if (buf[0] == '\0') return -1;
    for (char *p = buf; *p; p++) {
        if (*p < '0' || *p > '9') return -1;
    }
    return atoi(buf);
}

void app_main(void)
{
    usb_console_init();

    if (pwm_config() != ESP_OK) {
        ESP_LOGE(TAG, "PWM init failed, halting.");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("Motor speed console ready.\n");
    fflush(stdout);

    while (1) {
        int speeds[MOTORS];

        for (int i = 0; i < MOTORS; i++) {
            int val = -1;
            while (val < 0) {
                printf("Enter desired speed for Motor %c (0-100): ", LABELS[i]);
                fflush(stdout);
                val = uart_read_int();
                if (val < 0) {
                    printf("Invalid input, try again.\n");
                    fflush(stdout);
                }
            }
            speeds[i] = val;
            printf("Received: %d\n", val);
            fflush(stdout);
        }

        for (int i = 0; i < MOTORS; i++) {
            pwm_set_speed(i, speeds[i]);
        }

        printf("All motor speeds updated! Starting next loop\n\n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}