#include <stdio.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define BMI088_PIN_SCK        GPIO_NUM_11   
#define BMI088_PIN_MOSI       GPIO_NUM_10   
#define BMI088_PIN_CS_ACCEL   GPIO_NUM_9    
#define BMI088_PIN_MISO       GPIO_NUM_12   
#define BMI088_PIN_CS_GYRO    GPIO_NUM_8         


spi_device_handle_t slave1, slave2; 

void spi_config (void){
    spi_bus_config_t cnfg = {
        .sclk_io_num = BMI088_PIN_SCK,
        .mosi_io_num = BMI088_PIN_MOSI,
        .miso_io_num = BMI088_PIN_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    spi_bus_initialize(SPI2_HOST, &cfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg1 = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = BMI088_PIN_CS_ACCEL,
        .queue_size = 1,
        .cs_ena_pretrans = 1,
    };
    spi_bus_add_device(SPI2_HOST, &devcfg1, &slave1);

    spi_device_interface_config_t devcfg2 = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = BMI088_PIN_CS_GYRO,
        .queue_size = 1,
        .cs_ena_pretrans = 1,
    };
    spi_bus_add_device(SPI2_HOST, &devcfg2, &slave2);
}


