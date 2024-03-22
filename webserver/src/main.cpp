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
#include <mutex>

void handleWsSetDefaultProfile(NamedProfile profile);

std::mutex lvgl_mutex = std::mutex();

void loadEverything(void *params) {
  wifiSetup();
  webServerSetup();
  auto storedProfiles = fsGetProfiles();
  if (!storedProfiles.empty())
  {
    NamedProfile profile = storedProfiles[0];
    handleWsSetDefaultProfile(profile);
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  uiGoToHomeScreen();

  vTaskDelete(NULL);
}

void setup()
{
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  stmCommsInit(Serial1);

  lvgl_mutex.lock();
  uiInit();
  lvgl_mutex.unlock();
  xTaskCreateUniversal(loadEverything, "preLoad", configMINIMAL_STACK_SIZE + 2048, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
}

static bool isBrewing = false;

void loop() {
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
}

// ------------------------------------------------------------------------
// ---------------- Handle STM communication messages ---------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(SensorStateSnapshot& sensorData) {
  wsSendSensorStateSnapshotToClients(sensorData);
  lvgl_mutex.lock();
  uiHandleStateSnapshot(sensorData);
  lvgl_mutex.unlock();
}

void onShotSnapshotReceived(ShotSnapshot& shotData) {
  wsSendShotSnapshotToClients(shotData);
  lvgl_mutex.lock();
  uiHandleShotSnapshot(shotData);
  lvgl_mutex.unlock();
}

void stmResponseReceived(McuCommsResponse response) {
  LOG_INFO("received response %s for %d", response.result == McuCommsResponseResult::MCUC_OK ? "ok" : "error", response.type);
}

void handleWsSetDefaultProfile(NamedProfile profile) {
  LOG_INFO("Setting default profile to: %s", profile.name);
  stmCommsSendProfile(profile.profile);
  lvgl_mutex.lock();
  uiHandleCurrentProfileChange(profile);
  lvgl_mutex.unlock();
}

void toggleBrewState() {
  isBrewing = !isBrewing;
}