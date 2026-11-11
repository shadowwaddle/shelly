#pragma once
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c_master.h"
#include <optional>

class IMU {
public:
   explicit IMU (uint8_t address = 0x68);

    esp_err_t begin(i2c_master_bus_handle_t bus);
    esp_err_t calibrate();
    
    std::optional<float> readRawYawRate();
    std::optional<float> readYawRate();

private:
    esp_err_t writeReg(uint8_t reg, uint8_t value);
    esp_err_t readReg(uint8_t reg, uint8_t* out, size_t len);

    uint8_t address_;
    i2c_master_dev_handle_t dev_ = nullptr;
    float bias_ = 0.0f;
};