#include <Arduino.h>
#include "task_config.h"
#include "filesystem/filesystem.h"
#include "stm_comms/stm_comms.h"
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#include "scales/ble_scales.h"
#include "./log/log.h"
#include "ui/esp_ui.h"

void setup() {
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  stmCommsInit(Serial1);
  wifiSetup();
  webServerSetup();
  UI::init();

  listFS();
  fsDeleteProfile("test");
  auto profiles = fsGetProfiles();
  for (NamedProfile pr : profiles)
  {
    LOG_INFO("have: %s, phases: %u", pr.name, pr.profile.phaseCount());
    fsDeleteProfile(pr.name);
  }

  Profile profile;
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_PRESSURE, Transition(0.f, 10.f, TransitionCurve::EASE_IN_OUT, 1000), -1, PhaseStopConditions{.time = 1000}});
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_FLOW, Transition(10.f, 5.f, TransitionCurve::LINEAR), 2.f, PhaseStopConditions{.weight = 10.f}});
  profile.phases.push_back({PHASE_TYPE::PHASE_TYPE_FLOW, Transition(10.f, 5.f, TransitionCurve::LINEAR), 3.f, PhaseStopConditions{.pressureAbove = 2.f}});

  NamedProfile pr;
  strcpy(pr.name, "test");
  pr.profile = profile;
  fsSaveProfile(pr);
}

void loop() {
  // vTaskDelete(NULL);     //Delete own task by passing NULL(task handle can also be used)
  UI::handleLoop();
}

// ------------------------------------------------------------------------
// ---------------- Handle STM communication messages ---------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(SensorStateSnapshot& sensorData) {
  wsSendSensorStateSnapshotToClients(sensorData);
  UI::handleStateSnapshot(sensorData);
}

void onShotSnapshotReceived(ShotSnapshot& shotData) {
  wsSendShotSnapshotToClients(shotData);
  UI::handleShotSnapshot(shotData);
}

void onScalesTareReceived() {
  // bleScalesTare();
}