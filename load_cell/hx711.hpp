#pragma once
#include "driver/gpio.h"

class LoadCell{
    private:
    long OFFSET = 0;
    float SCALE = 1;
    gpio_num_t sckPin;
    gpio_num_t doutPin;

    public:
    void begin(gpio_num_t sck_pin, gpio_num_t dout_pin);
    void tare();
    void setScale();
    float getUnits();
    long readRaw();
};