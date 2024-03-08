#include <Arduino.h>
#include "task_config.h"
#include "filesystem/filesystem.h"
#include "stm_comms/stm_comms.h"
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket/websocket.h"
#include "scales/ble_scales.h"
#include "./log/log.h"
#include "ui/ui.h"
#include <esp32_smartdisplay.h>

void setup() {
  LOG_INIT();
  REMOTE_LOG_INIT([](std::string message) {wsSendLog(message);});
  initFS();
  stmCommsInit(Serial1);
  wifiSetup();
  webServerSetup();
  // bleScalesInit();
  // lcdInit();
  // vTaskDelete(NULL);     //Delete own task by passing NULL(task handle can also be used)
  smartdisplay_init();
  auto disp = lv_disp_get_default();
  lv_disp_set_rotation(disp, LV_DISP_ROT_270);

// touch_calibration_data;
  ui_init();
}

void loop() {
  // vTaskDelete(NULL);     //Delete own task by passing NULL(task handle can also be used)
  lv_timer_handler(); /* let the GUI do its work */
}

// ------------------------------------------------------------------------
// ---------------- Handle STM communication messages ---------------------
// ------------------------------------------------------------------------
void onSensorStateSnapshotReceived(SensorStateSnapshot& sensorData) {
  wsSendSensorStateSnapshotToClients(sensorData);
}

void onShotSnapshotReceived(ShotSnapshot& shotData) {
  wsSendShotSnapshotToClients(shotData);
}

void onScalesTareReceived() {
  // bleScalesTare();
}

void sendFlushAction(lv_event_t *e)
{
  // handleFlushButton();
  log_i("handle flush");
}