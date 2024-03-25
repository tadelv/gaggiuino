#include <Arduino.h>
#include "task_config.h"
#include "filesystem/filesystem.h"
#include "stm_comms/stm_comms.h"
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
// #include "scales/ble_scales.h"
#include "persistence/persistence.h"
#include "state/state.h"
#include "./log/log.h"
#include "ui/esp_ui.h"
#include <mutex>

std::mutex lvgl_mutex = std::mutex();

/*
void loadEverything(void *params) {
  wifiSetup();
  webServerSetup();

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  uiGoToHomeScreen();

  vTaskDelete(NULL);
}
*/
void stmLog(const char *logData) {
  LOG_INFO(logData);
}

void setup()
{
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  persistence::init();
  state::init();
  stmCommsInit(Serial);
  // stmCommsSetLogCallback(stmLog);
  /*
====== HEAD

  lvgl_mutex.lock();
  uiInit();
  lvgl_mutex.unlock();
  xTaskCreateUniversal(loadEverything, "preLoad", configMINIMAL_STACK_SIZE + 2048, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
=======
*/
wifiSetup();
webServerSetup();
// blescales::init();
vTaskDelete(NULL); // Delete own task by passing NULL
}

static bool isBrewing = false;

void loop() {
  /*
====== HEAD
  // vTaskDelete(NULL);     //Delete own task by passing NULL(task handle can also be used)
  lvgl_mutex.lock();
  uiHandleLoop();
  lvgl_mutex.unlock();
  if (millis() % 1000 == 0) {
    SensorStateSnapshot sensorData {0};
    sensorData.temperature = random(90, 100);
    sensorData.waterLvl = 33; //random(15, 85);
    sensorData.weight = 0;
    sensorData.brewActive = isBrewing;
    onSensorStateSnapshotReceived(sensorData);
  }
=======
*/
  vTaskDelete(NULL);     //Delete own task by passing NULL
}

// ------------------------------------------------------------------------
// ----------------------- Handle STM callbacks ---------------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(const SensorStateSnapshot& sensorData) {
  LOG_INFO("received state");
  wsSendSensorStateSnapshotToClients(sensorData);
  // lvgl_mutex.lock();
  // uiHandleStateSnapshot(sensorData);
  // lvgl_mutex.unlock();
}
void onShotSnapshotReceived(const ShotSnapshot& shotData) {
  LOG_INFO("received shot");
  wsSendShotSnapshotToClients(shotData);
  // lvgl_mutex.lock();
  // uiHandleShotSnapshot(shotData);
  // lvgl_mutex.unlock();
}

void onSystemStateReceived(const SystemState& systemState) {
  state::updateSystemState(systemState);
}
void onScalesTareReceived() {
  LOG_INFO("STM sent tare command");
  blescales::tare();
}
void onGaggiaSettingsRequested() {
  LOG_INFO("STM request active settings");
  stmCommsSendGaggiaSettings(state::getSettings());
}
void onProfileRequested() {
  LOG_INFO("STM request active profile");
  stmCommsSendProfile(state::getActiveProfile());
}
void onNotification(const Notification& notification) {
  wsSendNotification(notification);
}
void onDescalingProgressReceived(const DescalingProgress& progress) {
  wsSendDescalingProgress(progress);
}

// ------------------------------------------------------------------------
// ------------------ Handle state updated callbacks ----------------------
// ------------------------------------------------------------------------
void state::onActiveProfileUpdated(const Profile& profile) {
  LOG_INFO("active profile updated");
  stmCommsSendProfile(state::getActiveProfile());
  wsSendActiveProfileUpdated();
}
void state::onAllSettingsUpdated(const GaggiaSettings& settings) {
  LOG_INFO("All settings updated");
  stmCommsSendGaggiaSettings(state::getSettings());
  wsSendSettingsUpdated();
}
void state::onBrewSettingsUpdated(const BrewSettings& settings) {
  stmCommsSendBrewSettings(settings);
}
void state::onBoilerSettingsUpdated(const BoilerSettings& settings) {
  stmCommsSendBoilerSettings(settings);
}
void state::onLedSettingsUpdated(const LedSettings& settings) {
  stmCommsSendLedSettings(settings);
}
void state::onSystemSettingsUpdated(const SystemSettings& settings) {
  stmCommsSendSystemSettings(settings);
}
void state::onScalesSettingsUpdated(const ScalesSettings& settings) {
  stmCommsSendScalesSettings(settings);
}
void state::onSystemStateUpdated(const SystemState& systemState) {
  wsSendSystemStateToClients(systemState);
}
void state::onUpdateSystemStateCommandSubmitted(const UpdateSystemStateComand& command) {
  stmCommsSendUpdateSystemState(command);
}
void state::onConnectedBleScalesUpdated(const blescales::Scales& scales) {
  if (scales.address.length() == 0) {
    stmCommsSendScaleDisconnected();
  }
  wsSendConnectedBleScalesUpdated(scales);
}

// ------------------------------------------------------------------------
// -------------------- Handle ble scales callbacks -----------------------
// ------------------------------------------------------------------------
// void blescales::onWeightReceived(float weight) {
//   stmCommsSendWeight(weight);
// }
