#include <string.h>
#include "bmi088.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "BMI088";

#define BMI088_PIN_SCK        GPIO_NUM_11
#define BMI088_PIN_MOSI       GPIO_NUM_10
#define BMI088_PIN_CS_ACCEL   GPIO_NUM_9
#define BMI088_PIN_MISO       GPIO_NUM_12
#define BMI088_PIN_CS_GYRO    GPIO_NUM_8

#define ACC_CHIP_ID            0x00
#define ACC_X_LSB              0x12   // burst read start for accel data (X,Y,Z = 6 bytes)
#define ACC_RANGE               0x41
#define ACC_CONF                0x40
#define ACC_PWR_CONF             0x7C
#define ACC_PWR_CTRL             0x7D

#define GYRO_CHIP_ID            0x00
#define GYRO_RATE_X_LSB         0x02  // burst read start for gyro data (X,Y,Z = 6 bytes)
#define GYRO_RANGE               0x0F
#define GYRO_BANDWIDTH            0x10

static spi_device_handle_t accel_spi, gyro_spi;

static esp_err_t gyro_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    uint8_t tx[9] = {0};
    uint8_t rx[9] = {0};
    tx[0] = reg_addr | 0x80;                                   // bit7 = 1 -> read

    spi_transaction_t t = {0};
    t.length = (1 + len) * 8;                                  // 1 addr byte + len data bytes
    t.tx_buffer = tx;
    t.rx_buffer = rx;

    esp_err_t ret = spi_device_transmit(gyro_spi, &t);
    if (ret == ESP_OK) {
        memcpy(data, &rx[1], len);
    }
    return ret;
}

static esp_err_t accel_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    uint8_t tx[10] = {0};
    uint8_t rx[10] = {0};
    tx[0] = reg_addr | 0x80;

    spi_transaction_t t = {0};
    t.length = (2 + len) * 8;                                  // accel needs 1 addr + 1 dummy byte
    t.tx_buffer = tx;
    t.rx_buffer = rx;

    esp_err_t ret = spi_device_transmit(accel_spi, &t);
    if (ret == ESP_OK) {
        memcpy(data, &rx[2], len);
    }
    return ret;
}

static esp_err_t reg_write(spi_device_handle_t dev, uint8_t reg_addr, uint8_t value)
{
    uint8_t tx[2] = { (uint8_t)(reg_addr & 0x7F), value };     // bit7=0 -> write
    spi_transaction_t t = {0};
    t.length = 16;
    t.tx_buffer = tx;
    return spi_device_transmit(dev, &t);
}

esp_err_t bmi088_init(void)
{
    spi_bus_config_t bus_cfg = {
        .sclk_io_num = BMI088_PIN_SCK,
        .mosi_io_num = BMI088_PIN_MOSI,
        .miso_io_num = BMI088_PIN_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t accel_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = BMI088_PIN_CS_ACCEL,
        .queue_size = 1,
        .cs_ena_pretrans = 1,
    };
    ret = spi_bus_add_device(SPI2_HOST, &accel_cfg, &accel_spi);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t gyro_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = BMI088_PIN_CS_GYRO,
        .queue_size = 1,
        .cs_ena_pretrans = 1,
    };
    ret = spi_bus_add_device(SPI2_HOST, &gyro_cfg, &gyro_spi);
    if (ret != ESP_OK) return ret;

    // A throwaway read is required right after power-on to switch the
    // accelerometer out of I2C mode and into SPI mode (BMI088 datasheet).
    uint8_t dummy;
    accel_read(ACC_CHIP_ID, &dummy, 1);
    vTaskDelay(pdMS_TO_TICKS(1));

    reg_write(accel_spi, ACC_PWR_CONF, 0x00);   // 0x00 = exit suspend mode
    vTaskDelay(pdMS_TO_TICKS(1));
    reg_write(accel_spi, ACC_PWR_CTRL, 0x04);   // 0x04 = accelerometer on
    vTaskDelay(pdMS_TO_TICKS(1));

    reg_write(accel_spi, ACC_RANGE, 0x01);      // +/-6g
    reg_write(accel_spi, ACC_CONF, 0xAB);       // normal filter, ODR = 800Hz

    reg_write(gyro_spi, GYRO_RANGE, 0x00);      // +/-2000 deg/s
    reg_write(gyro_spi, GYRO_BANDWIDTH, 0x02);  // ODR=1000Hz, BW=116Hz

    uint8_t acc_id = 0, gyro_id = 0;
    accel_read(ACC_CHIP_ID, &acc_id, 1);
    gyro_read(GYRO_CHIP_ID, &gyro_id, 1);
    ESP_LOGI(TAG, "ACC_CHIP_ID = 0x%02X (expect 0x1E), GYRO_CHIP_ID = 0x%02X (expect 0x0F)",
             acc_id, gyro_id);

    if (acc_id != 0x1E || gyro_id != 0x0F) {
        ESP_LOGE(TAG, "IMU not detected correctly - check wiring/power before flying.");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void bmi088_read_accel(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t raw[6];
    accel_read(ACC_X_LSB, raw, 6);
    *x = (int16_t)((raw[1] << 8) | raw[0]);
    *y = (int16_t)((raw[3] << 8) | raw[2]);
    *z = (int16_t)((raw[5] << 8) | raw[4]);
}

void bmi088_read_gyro(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t raw[6];
    gyro_read(GYRO_RATE_X_LSB, raw, 6);
    *x = (int16_t)((raw[1] << 8) | raw[0]);
    *y = (int16_t)((raw[3] << 8) | raw[2]);
    *z = (int16_t)((raw[5] << 8) | raw[4]);
}
