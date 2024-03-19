#ifndef ESP_UI_H
#define ESP_UI_H

#include "sensors_state.h"
#include "profiling_phases.h"
#include <src/filesystem/filesystem.h>

void uiInit();
void uiHandleLoop();
void uiHandleCurrentProfileChange(NamedProfile profile);
void uiHandleStateSnapshot(const SensorStateSnapshot &state);
void uiHandleShotSnapshot(const ShotSnapshot &snapshot);
void uiSetActiveProfileName(const char *name);

#endif //ESP_UI_H