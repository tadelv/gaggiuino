#include "CriticallyDampedController.h"

CriticallyDampedController::CriticallyDampedController(float kp, float kd)
    : kp_(kp), kd_(kd), prev_error_(0) {
      prev_time_ = 0;
    }

float CriticallyDampedController::calculate(float setpoint, float process_variable)
{

  uint32_t currentMillis = millis();
  uint32_t elapsed = currentMillis - prev_time_;
  prev_time_ = currentMillis;

  // workaround because I don't want to drag around time as
  // state currently
  if (elapsed > 1000U) {
    elapsed = 1000;
  }

  float dt = elapsed / 1000;

  float error = setpoint - process_variable;

  // Derivative term
  float derivative = (error - prev_error_) / dt;
  prev_error_ = error;

  // Calculate the control output using PD control
  float control_output = kp_ * error + kd_ * derivative;

  return control_output;
}
