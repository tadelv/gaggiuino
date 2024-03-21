#include <esp32_smartdisplay.h>
#include "src/task_config.h"
#include "esp_ui.h"
#include "ui/ui.h"
#include "src/server/websocket/websocket.h"

void uiInit()
{
  smartdisplay_init();
  auto disp = lv_disp_get_default();
  lv_disp_set_rotation(disp, LV_DISP_ROT_270);
  ui_init();
}

void uiHandleLoop()
{
  lv_timer_handler(); /* let the GUI do its work */
}

void getFlowTargets(Phase phase, uint16_t *targets)
{
  if (phase.type == PHASE_TYPE::PHASE_TYPE_FLOW)
  {
    targets[0] = phase.target.start * 10;
    targets[1] = phase.target.end > 0 ? phase.target.end * 10 : targets[0];
  }
  else
  {
    targets[0] = targets[1] = phase.restriction * 10;
  }
}

void getPressureTargets(Phase phase, uint16_t *targets)
{
  if (phase.type == PHASE_TYPE::PHASE_TYPE_PRESSURE)
  {
    targets[0] = phase.target.start * 10;
    targets[1] = phase.target.end > 0 ? phase.target.end * 10 : targets[0];
  }
  else
  {
    targets[0] = targets[1] = phase.restriction * 10;
  }
}

void drawProfileChart(Profile profile) {
  log_i("inside");
  lv_chart_series_t *series = lv_chart_get_series_next(ui_currentProfileGraph, NULL);
  log_i("next");
  while (series != NULL) {
    lv_chart_remove_series(ui_currentProfileGraph, series);
    series = lv_chart_get_series_next(ui_currentProfileGraph, NULL);
  }
  log_i("remove");

  lv_chart_series_t *flowSeries = lv_chart_add_series(ui_currentProfileGraph, lv_palette_main(LV_PALETTE_YELLOW), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t *pressureSeries = lv_chart_add_series(ui_currentProfileGraph, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_all_value(ui_currentProfileGraph, pressureSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_currentProfileGraph, flowSeries, LV_CHART_POINT_NONE);

  log_i("create");
  int pointsCount = profile.phaseCount() * 2;

  lv_chart_set_point_count(ui_currentProfileGraph, pointsCount);
  log_i("set count");

  for (int i = 0; i < profile.phases.size(); i++) {
    Phase phase = profile.phases[i];
    log_i("t: %.1f, %.1f, r: %.1f", phase.target.start, phase.target.end, phase.restriction);
    uint16_t flowTargets[2];
    getFlowTargets(phase, flowTargets);
    log_i("f vals: %d, %d", flowTargets[0], flowTargets[1]);
    uint16_t pressureTargets[2];
    getPressureTargets(phase, pressureTargets);
    log_i("p vals: %d, %d", pressureTargets[0], pressureTargets[1]);
    lv_chart_set_next_value(ui_currentProfileGraph, flowSeries, flowTargets[0]);
    lv_chart_set_next_value(ui_currentProfileGraph, flowSeries, flowTargets[1]);
    lv_chart_set_next_value(ui_currentProfileGraph, pressureSeries, pressureTargets[0]);
    lv_chart_set_next_value(ui_currentProfileGraph, pressureSeries, pressureTargets[1]);
  }
  log_i("draw");
}

void uiHandleCurrentProfileChange(NamedProfile profile)
{
  lv_label_set_text(ui_profileNameLabel, profile.name);
  drawProfileChart(profile.profile);
}

void uiHandleStateSnapshot(const SensorStateSnapshot &state)
{
  char tempChar[4];
  sprintf(tempChar, "%.1f", state.temperature);
  lv_label_set_text(ui_tempLabel, tempChar);
  lv_arc_set_value(ui_tempGauge, (int)state.temperature);
  char wtrLvlChar[5];
  sprintf(wtrLvlChar, "%d%%", state.waterLvl);
  lv_label_set_text(ui_waterLabel, wtrLvlChar);
  lv_arc_set_value(ui_waterGauge, state.waterLvl);
  char weightChar[5];
  sprintf(weightChar, "%.1fg", state.weight);
  lv_label_set_text(ui_weightLabel, weightChar);
}

void uiHandleShotSnapshot(const ShotSnapshot &snapshot)
{
}

void uiGoToHomeScreen()
{
  lv_scr_load(ui_HomeScreen);
}

void sendFlushAction(lv_event_t *e)
{
  // handleFlushButton();
  log_i("handle flush");
  SensorStateSnapshot snapshot = {0};
  snapshot.brewActive = false;
  snapshot.steamActive = false;
  snapshot.scalesPresent = true;
  snapshot.temperature = 94.2;
  snapshot.waterLvl = 90;
  snapshot.pressure = 6.2;
  snapshot.pumpFlow = 1.2;
  snapshot.weightFlow = 0.5;
  snapshot.weight = 200.4;
  wsSendSensorStateSnapshotToClients(snapshot);
}

static lv_chart_series_t *pressureSeries;
static lv_chart_series_t *tempSeries;

void drawGraphTask(void *params)
{
  uint32_t startTime = millis();
  auto screen = lv_scr_act();
  uint32_t pressVal = 0;
  uint32_t tempVal = 900;
  while (screen == ui_BrewingScreen)
  {
    if (pressVal < 60)
    {
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

    ShotSnapshot snapshot = {0};
    snapshot.pressure = pressVal / (float)10;
    snapshot.temperature = tempVal / (float)10;
    snapshot.pumpFlow = 3.2;
    snapshot.timeInShot = elapsed;
    wsSendShotSnapshotToClients(snapshot);

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
    if (ser != NULL)
    {
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

void tareButtonTapped(lv_event_t *e)
{
  log_d("tare");
}