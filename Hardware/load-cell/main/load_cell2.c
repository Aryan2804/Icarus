#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hx711.h"

static const char *TAG = "HX711";

#define LOADCELL_DOUT_PIN   GPIO_NUM_14
#define LOADCELL_SCK_PIN    GPIO_NUM_4

// From your calibration fit: raw = 725.5 * weight(g) + offset
#define SCALE_FACTOR   728.69f   // counts per gram

static hx711_t dev = {
    .dout = LOADCELL_DOUT_PIN,
    .pd_sck = LOADCELL_SCK_PIN,
    .gain = HX711_GAIN_A_128,
};

void app_main(void)
{
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
        int32_t offset = 3198.66;
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
                ESP_LOGI(TAG, "raw = %ld, offset-corrected = %ld, weight = %.2f g",
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
