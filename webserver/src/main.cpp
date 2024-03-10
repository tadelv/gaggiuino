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
  // smartdisplay_lcd_set_backlight();
  // smartdisplay_led_set_rgb(true, false, true);

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

static lv_chart_series_t *pressureSeries;
static lv_chart_series_t *tempSeries;

void drawGraphTask(void *params) {
  uint32_t startTime = millis();
  auto screen = lv_scr_act();
  uint32_t pressVal = 0;
  uint32_t tempVal = 900;
  while (screen == ui_BrewingScreen) {
    if (pressVal < 60) {
      pressVal += 1;
    }
    tempVal += lv_rand(-1, 1);
    lv_chart_set_next_value(ui_BrewGraph, pressureSeries, pressVal);
    lv_chart_set_next_value(ui_BrewGraph, tempSeries, tempVal);
    uint16_t count = lv_chart_get_point_count(ui_BrewGraph);
    // if (count > 100) { // figure out how to scale horizontally
      // lv_chart_set_zoom_x(ui_BrewGraph, 255 / count * 255);
      // lv_chart_set_point_count(ui_BrewGraph, count);
    // }
    // lv_chart_refresh(ui_BrewGraph);

    uint32_t elapsed = millis() - startTime;
    uint32_t secs = elapsed / 1000u;
    uint32_t mins = secs / 60u;
    char time[6];
    snprintf(time, 6, "%02u:%02u", mins, secs);
    lv_label_set_text(ui_timeLabel, time);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    screen = lv_scr_act();
  }
  log_i("done with graph");
  vTaskDelete(NULL);
}

void brewingScreenAppear(lv_event_t *e) 
{
  lv_chart_set_update_mode(ui_BrewGraph, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_chart_set_type(ui_BrewGraph, LV_CHART_TYPE_LINE);

  lv_chart_series_t *ser = NULL;
  do
  {
    ser = lv_chart_get_series_next(ui_BrewGraph, NULL);
    if (ser != NULL) {
      lv_chart_remove_series(ui_BrewGraph, ser);
    }
  } while (ser != NULL);

  pressureSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  tempSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
  lv_chart_set_all_value(ui_BrewGraph, pressureSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_BrewGraph, tempSeries, LV_CHART_POINT_NONE);
  // lv_chart_set_x_start_point(ui_BrewGraph, pressureSeries, 0);
  // lv_chart_set_x_start_point(ui_BrewGraph, tempSeries, 0);

  lv_chart_set_point_count(ui_BrewGraph, 450);
  xTaskCreateUniversal(drawGraphTask, "drawGraph", configMINIMAL_STACK_SIZE + 2048, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
}