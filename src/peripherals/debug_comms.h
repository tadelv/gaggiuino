#ifndef DEBUG_COMMS
#define DEBUG_COMMS

struct debug_callbacks_t {
  void (*pumpControllerData)();
  void (*pumpControllerSet)(float, float, float);
};

void debugInit();
void readDebugCommand();

#endif