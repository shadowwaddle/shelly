// main.cpp: Milestone 1: drive forward slowly, stop at obstacles.
#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "config.hpp"
#include "Chassis.hpp"
#include "hal/ledc_types.h"
#include "hardware/Motor.hpp"
#include "hardware/Ultrasonic.hpp"
#include "hardware/IMU.hpp"
#include "hardware/Camera.hpp"

static const char* TAG = "main";

// ---------- Tunables ----------
constexpr float kCruiseSpeed    = 0.3f;
constexpr float kStopDistanceCm = 20.0f;
constexpr float kNoReading      = -1.0f;   // mailbox value meaning "no valid distance"

// ---------- Hardware objects ----------
static Motor leftMotor(config::kLeftEn, config::kLeftIn1, config::kLeftIn2, LEDC_CHANNEL_0);
static Motor rightMotor(config::kRightEn, config::kRightIn1, config::kRightIn2, LEDC_CHANNEL_1);
static Chassis chassis(leftMotor, rightMotor, 0.3f);
static Ultrasonic ultrasonic(config::kUltrasonicTrig, config::kUltrasonicEcho);
static Camera camera;
static IMU imu;

// ---------- Shared data ----------
static QueueHandle_t distanceMailbox = nullptr;

// ---------- Setup helpers ----------
static i2c_master_bus_handle_t createI2cBus() {
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port          = I2C_NUM_0;
    cfg.sda_io_num        = config::kImuSda;
    cfg.scl_io_num        = config::kImuScl;
    cfg.clk_source        = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
    return bus;
}

// ---------- Tasks ----------
static void sensorTask(void*) {
    while (true) {
        auto distance = ultrasonic.readCm();
        float cm = distance.value_or(kNoReading);
        xQueueOverwrite(distanceMailbox, &cm);
        vTaskDelay(pdMS_TO_TICKS(100));   // HC-SR04 needs ~60 ms between pings

    }
}

static void controlTask(void*) {
    TickType_t last = xTaskGetTickCount();
    while (true) {
        float cm = kNoReading;
        xQueuePeek(distanceMailbox, &cm, 0);
        float speed = kCruiseSpeed;
        if (cm < 0.0f || cm < kStopDistanceCm) {
            speed = 0.0f;
        }
        chassis.drive(speed, 0.0f);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(10));   
    }
}

// ---------- Entry point ----------
extern "C" void app_main() {
    i2c_master_bus_handle_t bus = createI2cBus();

    ESP_ERROR_CHECK(Motor::initTimer(config::kMotorPwmHz));
    ESP_ERROR_CHECK(chassis.begin());
    chassis.stop();
    ESP_LOGI(TAG, "Motors ready");

    ESP_ERROR_CHECK(imu.begin(bus));
    ESP_ERROR_CHECK(imu.calibrate());
    ESP_LOGI(TAG, "IMU ready");


    ESP_ERROR_CHECK(ultrasonic.begin());
    ESP_LOGI(TAG, "Sonar ready");

    distanceMailbox = xQueueCreate(1, sizeof(float));
    if (distanceMailbox == nullptr) {
        ESP_LOGE(TAG, "Failed to create mailbox");
        return;
    }

    xTaskCreatePinnedToCore(sensorTask,  "sensor",  4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask, "control", 4096, nullptr, 5, nullptr, 1);
    ESP_LOGI(TAG, "Robot running");
    // app_main may return: the tasks keep running.
}