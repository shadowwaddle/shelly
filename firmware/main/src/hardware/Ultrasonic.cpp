#include "hardware/Ultrasonic.hpp"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include <cmath>
#include <algorithm>

Ultrasonic::Ultrasonic (gpio_num_t trig, gpio_num_t echo)
    : trig_(trig), echo_(echo) {};

esp_err_t Ultrasonic::begin() {
    // Trig: we drive it -> output
    gpio_config_t out = {};
    out.pin_bit_mask = 1ULL << trig_;
    out.mode = GPIO_MODE_OUTPUT;
    esp_err_t err = gpio_config(&out);
    if (err != ESP_OK) return err;

    // Echo: the sensor drives it -> input
    gpio_config_t in = {};
    in.pin_bit_mask = 1ULL << echo_;
    in.mode = GPIO_MODE_INPUT;
    err = gpio_config(&in);
    if (err != ESP_OK) return err;

    gpio_set_level(trig_, 0);   // start with Trig LOW
    return ESP_OK;
}

// 30 ms of round trip ≈ 5 m: beyond the HC-SR04's ~4 m range.
static constexpr int64_t kTimeoutUs = 30000;
// Speed of sound: 0.0343 cm/µs, divided by 2 for the round trip.
static constexpr float kCmPerUs = 0.0343f / 2.0f;

std::optional<float> Ultrasonic::readCm() {
    // 1) Send the 10 µs trigger pulse.
    gpio_set_level(trig_, 0);
    esp_rom_delay_us(2);
    gpio_set_level(trig_, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_, 0);

    // 2) Wait for Echo to go HIGH (the sensor has sent its pings).
    int64_t waitStart = esp_timer_get_time();
    while (gpio_get_level(echo_) == 0) {
        if (esp_timer_get_time() - waitStart > kTimeoutUs) return std::nullopt;
    }

    // 3) Time how long Echo stays HIGH (the sound's round trip).
    int64_t pulseStart = esp_timer_get_time();
    while (gpio_get_level(echo_) == 1) {
        if (esp_timer_get_time() - pulseStart > kTimeoutUs) return std::nullopt;
    }
    int64_t pulseUs = esp_timer_get_time() - pulseStart;

    // 4) Convert time to distance.
    return pulseUs * kCmPerUs;
}


