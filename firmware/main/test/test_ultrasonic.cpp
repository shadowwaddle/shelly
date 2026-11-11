// Bench test: ultrasonic (HC-SR04). Needs 5 V on VCC and the Echo divider.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config.hpp"
#include "hardware/Ultrasonic.hpp"

static const char* TAG = "test_sonar";
static Ultrasonic sonar(config::kUltrasonicTrig, config::kUltrasonicEcho);

extern "C" void app_main() {
    ESP_ERROR_CHECK(sonar.begin());
    ESP_LOGI(TAG, "Point at a wall and move closer/farther. Compare with a ruler.");

    int reads = 0, misses = 0;
    while (true) {
        auto d = sonar.readCm();
        ++reads;
        if (d) {
            ESP_LOGI(TAG, "%6.1f cm", *d);
        } else {
            ++misses;
            ESP_LOGW(TAG, "no echo (%d of %d missed)", misses, reads);
        }
        vTaskDelay(pdMS_TO_TICKS(100));   // >60 ms between pings
    }
}