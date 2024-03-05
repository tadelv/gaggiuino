#include "PIDController.h"

PIDController::PIDController(float kp, float ki, float kd)
    : kp_(kp), ki_(ki), kd_(kd), prev_error_(0), integral_(0)
{
}

float PIDController::calculate(float setpoint, float process_variable, float dt)
{

  float error = setpoint - process_variable;

  // Proportional term
  float proportional = kp_ * error;

  // Integral term
  integral_ = integral_ + (error * dt);

  // Derivative term
  float derivative = (error - prev_error_) / dt;
  prev_error_ = error;

  // Calculate the control output
  float control_output = (kp_ * error)  + (ki_ * integral_) + (kd_ * derivative);

  return control_output;
}

void PIDController::reset() {
  prev_error_ = 0;
  integral_ = 0;
}
