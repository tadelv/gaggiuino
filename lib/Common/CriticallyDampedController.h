#include <Arduino.h>
#ifndef CRITICALLY_DAMPED_CONTROLLER_H
#define CRITICALLY_DAMPED_CONTROLLER_H

class CriticallyDampedController
{
public:
  CriticallyDampedController(float kp, float kd);
  float calculate(float setpoint, float process_variable);
  // void reset();

private:
  float kp_;         // Proportional gain
  float kd_;         // Derivative gain
  float prev_error_; // Previous error
  uint32_t prev_time_; // Previous measurement time
};

#endif // CRITICALLY_DAMPED_CONTROLLER_H