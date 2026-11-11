#pragma once
#include "hardware/Motor.hpp"

class Chassis {
public:
    Chassis(Motor& left, Motor& right, float deadband = 0.0f);

    esp_err_t begin();
    void drive(float speed, float turn);
    void stop();

private:
    float applyDeadband(float power) const;

    Motor& left_;
    Motor& right_;
    float deadband_;
};