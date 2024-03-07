#include <Arduino.h>
#include "debug_comms.h"
// #include "pindef.h"

debug_callbacks_t debug_callbacks = debug_callbacks_t();

void debugInit()
{
  // debug_callbacks.pumpControllerData = dummy;
}

void readDebugCommand()
{
  if (Serial.available() > 0) {
    String cmd = Serial.readString();
    if (
      cmd == "pc" &&
      debug_callbacks.pumpControllerData != NULL
      ) {
      debug_callbacks.pumpControllerData();
    } else if (
      cmd.startsWith("pset ") &&
      debug_callbacks.pumpControllerSet != NULL
    ) {
      String kpParams = cmd.substring(5);
      int idx = kpParams.indexOf(",");
      String substr = kpParams.substring(0, idx);
      float kp = substr.toFloat();
      String kiParams = kpParams.substring(idx + 1);
      idx = kiParams.indexOf(",");
      float ki = kiParams.substring(0, idx).toFloat();
      String kdParams = kiParams.substring(idx + 1);
      float kd = kdParams.toFloat();
      debug_callbacks.pumpControllerSet(kp, ki, kd);
    }
  }
}