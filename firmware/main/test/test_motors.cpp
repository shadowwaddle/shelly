// Bench test: motors + chassis. LiPo on, WHEELS OFF THE GROUND.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config.hpp"
#include "Chassis.hpp"
#include "hardware/Motor.hpp"

static const char* TAG = "test_motors";

static Motor   left (config::kLeftEn,  config::kLeftIn1,  config::kLeftIn2,  LEDC_CHANNEL_0);
static Motor   right(config::kRightEn, config::kRightIn1, config::kRightIn2, LEDC_CHANNEL_1);
static Chassis chassis(left, right, 0.0f);   // deadband 0: we're measuring it

static void delayMs(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

// Step 1: one motor at a time -> checks wiring and which way is "forward".
static void spinTest(Motor& m, const char* name) {
    ESP_LOGI(TAG, "%s forward 50%%", name);
    m.set(0.5f);  delayMs(1500);
    m.coast();    delayMs(500);
    ESP_LOGI(TAG, "%s reverse 50%%", name);
    m.set(-0.5f); delayMs(1500);
    m.coast();    delayMs(1000);
}

// Step 2: slow ramp -> write down the % where the wheel first moves (deadband).
static void deadbandRamp(Motor& m, const char* name) {
    ESP_LOGI(TAG, "%s ramp: note the %% where it starts turning", name);
    for (int pct = 0; pct <= 60; pct += 2) {
        m.set(pct / 100.0f);
        ESP_LOGI(TAG, "  %s %d%%", name, pct);
        delayMs(500);
    }
    m.coast(); delayMs(1000);
}

extern "C" void app_main() {
    ESP_ERROR_CHECK(Motor::initTimer(config::kMotorPwmHz));
    ESP_ERROR_CHECK(chassis.begin());   // begins both motors
    chassis.stop();

    for (int s = 3; s > 0; --s) {
        ESP_LOGW(TAG, "Wheels OFF the ground! Starting in %d...", s);
        delayMs(1000);
    }

    while (true) {
        spinTest(left, "LEFT");
        spinTest(right, "RIGHT");
        deadbandRamp(left, "LEFT");
        deadbandRamp(right, "RIGHT");

        // Step 3: chassis mixing
        ESP_LOGI(TAG, "chassis: straight (both forward)");
        chassis.drive(0.5f, 0.0f); delayMs(2000);
        ESP_LOGI(TAG, "chassis: turn right (left fwd, right back)");
        chassis.drive(0.0f, 0.5f); delayMs(2000);
        chassis.stop();

        ESP_LOGI(TAG, "Cycle done. Repeating in 5 s.");
        delayMs(5000);
    }
}