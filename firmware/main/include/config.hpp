#pragma once
#include <cstdint>
#include "driver/gpio.h"

// Every pin and tuning constant lives here, so rewiring = editing one file.
namespace config {

// ---- Motors (L298N) ----
// Left motor -> channel A
constexpr gpio_num_t kLeftEn   = GPIO_NUM_4;    // ENA (PWM, remove the jumper cap)
constexpr gpio_num_t kLeftIn1  = GPIO_NUM_5;    // IN1
constexpr gpio_num_t kLeftIn2  = GPIO_NUM_6;    // IN2
// Right motor -> channel B
constexpr gpio_num_t kRightEn  = GPIO_NUM_7;    // ENB (PWM, remove the jumper cap)
constexpr gpio_num_t kRightIn1 = GPIO_NUM_15;   // IN3
constexpr gpio_num_t kRightIn2 = GPIO_NUM_16;   // IN4

// The L298N uses slow bipolar transistors; above ~5 kHz it switches poorly
// and heats up. 1 kHz is a safe default (you may hear a faint whine).
constexpr uint32_t kMotorPwmHz = 1000;

// ---- Ultrasonic (HC-SR04) ----
constexpr gpio_num_t kUltrasonicTrig = GPIO_NUM_21;
constexpr gpio_num_t kUltrasonicEcho = GPIO_NUM_47;   // via 1k/2k divider (5 V -> 3.3 V)

// ---- IMU (MPU6050, I2C) ----
constexpr gpio_num_t kImuSda = GPIO_NUM_41;
constexpr gpio_num_t kImuScl = GPIO_NUM_42;

// ---- Battery sense (later) ----
constexpr gpio_num_t kBatterySense = GPIO_NUM_1;      // ADC1: works with Wi-Fi on

// ---- Camera (OV2640, DVP) ----
constexpr gpio_num_t kCamXclk  = GPIO_NUM_10;
constexpr gpio_num_t kCamPclk  = GPIO_NUM_11;
constexpr gpio_num_t kCamVsync = GPIO_NUM_38;
constexpr gpio_num_t kCamHref  = GPIO_NUM_48;
constexpr gpio_num_t kCamD0    = GPIO_NUM_8;
constexpr gpio_num_t kCamD1    = GPIO_NUM_9;
constexpr gpio_num_t kCamD2    = GPIO_NUM_14;
constexpr gpio_num_t kCamD3    = GPIO_NUM_2;
constexpr gpio_num_t kCamD4    = GPIO_NUM_39;
constexpr gpio_num_t kCamD5    = GPIO_NUM_40;
constexpr gpio_num_t kCamD6    = GPIO_NUM_NC;   // TBD
constexpr gpio_num_t kCamD7    = GPIO_NUM_NC;   // TBD
constexpr gpio_num_t kCamSda   = GPIO_NUM_NC;   // TBD (SCCB)
constexpr gpio_num_t kCamScl   = GPIO_NUM_NC;   // TBD (SCCB)
// PWDN / RESET not used (-1 in camera_config_t). PWDN must be LOW for the camera to run.

// ---- Fixed on the UCSD board (don't reuse) ----
constexpr gpio_num_t kLed1 = GPIO_NUM_17;
constexpr gpio_num_t kLed2 = GPIO_NUM_18;
constexpr gpio_num_t kSw1  = GPIO_NUM_12;
constexpr gpio_num_t kSw2  = GPIO_NUM_13;

} // namespace config