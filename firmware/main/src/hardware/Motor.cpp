#include "hardware/Motor.hpp"
#include <algorithm>
#include <cmath>

// 10-bit resolution -> duty is 0..1023. More bits = finer speed steps,
// but max resolution shrinks as PWM frequency rises (clock / freq / 2^bits).
static constexpr ledc_timer_bit_t kResolution = LEDC_TIMER_10_BIT;
static constexpr uint32_t kMaxDuty = (1u << 10) - 1;
static constexpr ledc_mode_t kMode = LEDC_LOW_SPEED_MODE;  // the only mode on the S3
static constexpr ledc_timer_t kTimer = LEDC_TIMER_0;

Motor::Motor(gpio_num_t en, gpio_num_t in1, gpio_num_t in2,
             ledc_channel_t channel, bool inverted)
    : en_(en), in1_(in1), in2_(in2), channel_(channel), inverted_(inverted) {}

esp_err_t Motor::initTimer(uint32_t freq_hz) {
    ledc_timer_config_t t = {};
    t.speed_mode = kMode;
    t.duty_resolution = kResolution;
    t.timer_num = kTimer;
    t.freq_hz = freq_hz;
    t.clk_cfg = LEDC_AUTO_CLK;
    return ledc_timer_config(&t);
}

esp_err_t Motor::begin() {
    // Direction pins: plain digital outputs.
    gpio_config_t io = {};
    io.pin_bit_mask = (1ULL << in1_) | (1ULL << in2_);
    io.mode = GPIO_MODE_OUTPUT;
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) return err;

    // Enable pin: a PWM channel attached to the shared timer.
    ledc_channel_config_t c = {};
    c.gpio_num = en_;
    c.speed_mode = kMode;
    c.channel = channel_;
    c.timer_sel = kTimer;
    c.duty = 0;          // start stopped: never let a motor twitch at boot
    c.hpoint = 0;
    err = ledc_channel_config(&c);
    if (err != ESP_OK) return err;

    coast();
    return ESP_OK;
}

void Motor::set(float power) {
    if (std::isnan(power)) power = 0.0f;               // bad input -> stop
    power = std::clamp(power, -1.0f, 1.0f);
    power_ = power;
    if (inverted_) power = -power;

    // H-bridge truth table: which side of the motor gets battery+.
    gpio_set_level(in1_, power > 0.0f);
    gpio_set_level(in2_, power < 0.0f);
    writeDuty(std::fabs(power));
}

void Motor::coast() {
    power_ = 0.0f;
    gpio_set_level(in1_, 0);
    gpio_set_level(in2_, 0);
    writeDuty(0.0f);
}

void Motor::brake() {
    power_ = 0.0f;
    gpio_set_level(in1_, 1);
    gpio_set_level(in2_, 1);
    writeDuty(1.0f);   // EN high so the bridge actually shorts the motor
}

void Motor::writeDuty(float fraction) {
    uint32_t duty = static_cast<uint32_t>(fraction * kMaxDuty + 0.5f);
    ledc_set_duty(kMode, channel_, duty);   // stage the new value...
    ledc_update_duty(kMode, channel_);      // ...then latch it into hardware
}