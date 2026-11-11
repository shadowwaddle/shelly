#include "hardware/IMU.hpp"     // brings in i2c_master.h, <optional>, <cstdint>
#include <algorithm>            // std::min, std::max
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"      // vTaskDelay
#include "esp_check.h"          // ESP_RETURN_ON_ERROR
#include "esp_log.h"            // ESP_LOGI

namespace {
    constexpr uint8_t REG_CONFIG      = 0x1A;
    constexpr uint8_t REG_GYRO_CONFIG = 0x1B;
    constexpr uint8_t REG_GYRO_ZOUT_H = 0x47;
    constexpr uint8_t REG_PWR_MGMT_1  = 0x6B;
    constexpr uint8_t REG_WHO_AM_I    = 0x75;

    // Values
    constexpr uint8_t WHO_AM_I_VALUE  = 0x68;
    constexpr uint8_t PWR_WAKE        = 0x00;
    constexpr uint8_t DLPF_44HZ       = 0x03;
    constexpr uint8_t GYRO_RANGE_500  = 0x01 << 3;   // range bits sit at positions 4..3
    
    constexpr float   LSB_PER_DPS     = 65.5f;       // must match the range above
    constexpr int     kTimeoutMs      = 50;
    constexpr int kCalibSamples = 200;
    constexpr float kMaxStillSpread = 2.0f;
    
    constexpr const char* TAG = "IMU";
}


IMU::IMU (uint8_t address)
    : address_(address) {}

esp_err_t IMU::begin(i2c_master_bus_handle_t bus) {
    // 1) Register on the bus
    i2c_device_config_t cfg = {};
    cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    cfg.device_address  = address_;
    cfg.scl_speed_hz    = 100000;
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &cfg, &dev_), TAG, "add device failed");

    // 2) Is it an MPU6050?
    uint8_t id = 0;
    ESP_RETURN_ON_ERROR(readReg(REG_WHO_AM_I, &id, 1), TAG, "no response (check wiring)");
    if (id != WHO_AM_I_VALUE) return ESP_ERR_NOT_FOUND;

    // 3) Wake it (it powers up asleep)
    ESP_RETURN_ON_ERROR(writeReg(REG_PWR_MGMT_1, PWR_WAKE), TAG, "wake failed");
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4) Filter + range
    ESP_RETURN_ON_ERROR(writeReg(REG_CONFIG, DLPF_44HZ), TAG, "config failed");
    ESP_RETURN_ON_ERROR(writeReg(REG_GYRO_CONFIG, GYRO_RANGE_500), TAG, "gyro config failed");

    return ESP_OK;
}

// Private: straight from the chip, no bias correction.
std::optional<float> IMU::readRawYawRate() {
    uint8_t b[2];
    if (readReg(REG_GYRO_ZOUT_H, b, 2) != ESP_OK) return std::nullopt;
    int16_t raw = static_cast<int16_t>((b[0] << 8) | b[1]);   // MPU6050: high byte first
    return raw / LSB_PER_DPS;
}

// Public: what the rest of the robot uses.
std::optional<float> IMU::readYawRate() {
    auto raw = readRawYawRate();
    if (!raw) return std::nullopt;
    return *raw - bias_;
}

// Public: call once at startup, with the car completely still.
esp_err_t IMU::calibrate() {
    float sum = 0.0f;
    float lo = 1e9f, hi = -1e9f;   // track the spread to detect movement

    for (int i = 0; i < kCalibSamples; ++i) {
        auto rate = readRawYawRate();
        if (!rate) return ESP_FAIL;          // I2C error mid-calibration

        sum += *rate;
        lo = std::min(lo, *rate);
        hi = std::max(hi, *rate);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (hi - lo > kMaxStillSpread) return ESP_ERR_INVALID_STATE;   // car moved: try again

    bias_ = sum / kCalibSamples;
    ESP_LOGI(TAG, "gyro bias = %.3f deg/s", bias_);
    return ESP_OK;
}

// Helper methods for reading and writing
esp_err_t IMU::writeReg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_transmit(dev_, data, sizeof(data), kTimeoutMs);
};



esp_err_t IMU::readReg(uint8_t reg, uint8_t* out, size_t len) {
    return i2c_master_transmit_receive(dev_, &reg, 1, out, len, kTimeoutMs);
};