#include <Arduino.h>
#include "debug_comms.h"
#ifndef UNIT_TEST
#include "pindef.h"
#else 
#define USART_DEBUG Serial
#endif



debug_callbacks_t debug_callbacks = debug_callbacks_t();

void debugInit()
{
  // debug_callbacks.pumpControllerData = dummy;
}

void parsePSetCommand(const char *cmd, float &kp, float &ki, float &kd)
{
  char *kpEnd;
  kp = strtof(cmd, &kpEnd);

  if (*kpEnd != ',')
    return; // Invalid command format

  char *kiEnd;
  ki = strtof(kpEnd + 1, &kiEnd);

  if (*kiEnd != ',')
    return; // Invalid command format

  kd = strtof(kiEnd + 1, NULL);
}

void readDebugCommand()
{
  if (USART_DEBUG.available() < 1)
    return;

  char cmd[64]; // Adjust the size based on your maximum expected command length
  USART_DEBUG.readBytesUntil('\n', cmd, sizeof(cmd));
  cmd[sizeof(cmd) - 1] = '\0'; // Ensure null-termination
  switch (cmd[0])
  {
  case 'p':
    if (strcmp(cmd, "pc") == 0 && debug_callbacks.pumpControllerData != NULL)
    {
      debug_callbacks.pumpControllerData();
    }
    else if (strncmp(cmd, "pset", 4) == 0 && debug_callbacks.pumpControllerSet != NULL)
    {
      float kp, ki, kd;
      parsePSetCommand(cmd + 4, kp, ki, kd);
      debug_callbacks.pumpControllerSet(kp, ki, kd);
    }
    break;

  case 's':
    if (strcmp(cmd, "status") == 0 && debug_callbacks.systemStatus != NULL)
    {
      debug_callbacks.systemStatus();
    }
    break;

    // Add more cases as needed

  default:
    // Handle unknown command
    break;
  }
}
