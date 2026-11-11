#include "Chassis.hpp"
#include <algorithm>
#include <cmath>

Chassis::Chassis(Motor& left, Motor& right, float deadband)
    : left_(left), right_(right), deadband_(deadband) {}

esp_err_t Chassis::begin() {
    esp_err_t err = left_.begin();
    if (err != ESP_OK) return err;
    return right_.begin();
}

void Chassis::drive(float speed, float turn) {
    // 1) Mix: turning right = left wheel faster, right wheel slower.
    float left  = speed + turn;
    float right = speed - turn;

    // 2) Normalize: if a wheel exceeds 1, scale BOTH down equally.
    //    Clipping only one would change the ratio, and so the turn radius.
    float biggest = std::max(std::fabs(left), std::fabs(right));
    if (biggest > 1.0f) {
        left  /= biggest;
        right /= biggest;
    }

    // 3) Compensate for deadband, then send to the motors.
    left_.set(applyDeadband(left));
    right_.set(applyDeadband(right));
}

void Chassis::stop() {
    left_.coast();
    right_.coast();
}

float Chassis::applyDeadband(float power) const {
    if (std::fabs(power) < 0.01f) return 0.0f;   // tiny command = stop
    // Map (0, 1] onto [deadband, 1], keeping the sign.
    float magnitude = deadband_ + (1.0f - deadband_) * std::fabs(power);
    return std::copysign(magnitude, power);
}