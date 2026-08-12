#ifndef BMI088_H
#define BMI088_H

#include <stdint.h>
#include "esp_err.h"

// Initializes the SPI bus, both BMI088 sub-devices (accel + gyro),
// and configures sensor ranges/ODR. Returns ESP_OK on success, or
// ESP_FAIL if the chip IDs read back don't match the datasheet values
// (usually means a wiring problem).
esp_err_t bmi088_init(void);

void bmi088_read_accel(int16_t *x, int16_t *y, int16_t *z);
void bmi088_read_gyro(int16_t *x, int16_t *y, int16_t *z);

#endif // BMI088_H
