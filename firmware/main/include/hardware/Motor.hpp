#pragma once
#include "driver/gpio.h"
#include "driver/ledc.h"

// One DC motor on one L298N channel.
// Knows nothing about FreeRTOS, PID, or the other motor: it only turns a
// number into pin states. That keeps it reusable and easy to reason about.
class Motor {
public:
    Motor(gpio_num_t en, gpio_num_t in1, gpio_num_t in2,
          ledc_channel_t channel, bool inverted = false);

    // Call once before set(). Configures the direction pins and PWM channel.
    esp_err_t begin();

    // power: -1.0 (full reverse) .. 0 .. +1.0 (full forward)
    void set(float power);

    void coast();   // IN1=IN2=LOW : motor spins down freely
    void brake();   // IN1=IN2=HIGH: motor terminals shorted, stops fast

    float power() const { return power_; }

    // Shared by all motors: one hardware timer sets the PWM frequency.
    static esp_err_t initTimer(uint32_t freq_hz);

private:
    void writeDuty(float fraction);   // 0..1

    gpio_num_t en_, in1_, in2_;
    ledc_channel_t channel_;
    bool inverted_;     // flip if a motor is mounted mirrored
    float power_ = 0.0f;
};