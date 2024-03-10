#ifndef ESP_UI_H
#define ESP_UI_H

#include "sensors_state.h"
#include "profiling_phases.h"

struct UI {
  public:
  static void init();
  static void handleLoop();
  static void handleStateSnapshot(const SensorStateSnapshot &state);
  static void handleShotSnapshot(const ShotSnapshot &snapshot);
};

#endif //ESP_UI_H