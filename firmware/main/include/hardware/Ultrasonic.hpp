#pragma once
#include <optional>
#include "driver/gpio.h"

class Ultrasonic {
public:
    Ultrasonic (gpio_num_t trig, gpio_num_t echo);
    esp_err_t begin();

    // Distance in cm, or std::nulopt if nothing echoed back
    std::optional<float> readCm();

private:
    gpio_num_t trig_;
    gpio_num_t echo_;
};