// Bench test: IMU (MPU6050). USB power is enough. Keep the board STILL at boot.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c_master.h"
#include "config.hpp"
#include "hardware/IMU.hpp"

static const char* TAG = "test_imu";
static IMU imu;

extern "C" void app_main() {
    // 1) Bus (same settings as main.cpp)
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port          = I2C_NUM_0;
    cfg.sda_io_num        = config::kImuSda;
    cfg.scl_io_num        = config::kImuScl;
    cfg.clk_source        = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true;
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));

    // 2) Is anything answering? Tests the WIRING before any IMU code runs.
    esp_err_t probe = i2c_master_probe(bus, 0x68, 50);
    ESP_LOGI(TAG, "probe 0x68: %s", esp_err_to_name(probe));
    if (probe != ESP_OK) {
        ESP_LOGE(TAG, "No device at 0x68. Check VCC, GND, SDA=%d, SCL=%d.",
                 config::kImuSda, config::kImuScl);
        if (i2c_master_probe(bus, 0x69, 50) == ESP_OK)
            ESP_LOGW(TAG, "Found a device at 0x69 instead: AD0 is pulled HIGH.");
        return;
    }

    // 3) Driver setup + calibration
    ESP_ERROR_CHECK(imu.begin(bus));
    ESP_LOGI(TAG, "Calibrating, keep still...");
    ESP_ERROR_CHECK(imu.calibrate());

    // 4) Print rate and an integrated heading (preview of HeadingEstimator).
    //    Rotate the board by hand ~90 degrees and check the heading.
    float heading = 0.0f;
    int64_t last = esp_timer_get_time();
    while (true) {
        auto rate = imu.readYawRate();
        int64_t now = esp_timer_get_time();
        float dt = (now - last) / 1e6f;   // microseconds -> seconds
        last = now;

        if (rate) {
            heading += *rate * dt;
            ESP_LOGI(TAG, "rate %7.2f deg/s   heading %7.1f deg", *rate, heading);
        } else {
            ESP_LOGW(TAG, "read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}