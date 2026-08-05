#ifndef BMI088_H
#define BMI088_H

#include <stdint.h>

void spi_config(void);
void bmi088_write(void);
void bmi088_read_accel(int16_t *x, int16_t *y, int16_t *z);
void bmi088_read_gyro(int16_t *x, int16_t *y, int16_t *z);

#endif