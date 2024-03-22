#include <esp32_smartdisplay.h>
#include "src/task_config.h"
#include "esp_ui.h"
#include "ui/ui.h"
#include "src/server/websocket/websocket.h"


void resetBrewGraph();
void updateBrewGraphWithSnapshot(const ShotSnapshot &snapshot);

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
  lv_chart_series_t *series = lv_chart_get_series_next(ui_currentProfileGraph, NULL);
  while (series != NULL) {
    lv_chart_remove_series(ui_currentProfileGraph, series);
    series = lv_chart_get_series_next(ui_currentProfileGraph, NULL);
  }

  lv_chart_series_t *flowSeries = lv_chart_add_series(ui_currentProfileGraph, lv_palette_main(LV_PALETTE_YELLOW), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t *pressureSeries = lv_chart_add_series(ui_currentProfileGraph, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_all_value(ui_currentProfileGraph, pressureSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_currentProfileGraph, flowSeries, LV_CHART_POINT_NONE);

  int pointsCount = profile.phaseCount() * 2;

  lv_chart_set_point_count(ui_currentProfileGraph, pointsCount);

  for (int i = 0; i < profile.phases.size(); i++) {
    Phase phase = profile.phases[i];
    uint16_t flowTargets[2];
    getFlowTargets(phase, flowTargets);
    uint16_t pressureTargets[2];
    getPressureTargets(phase, pressureTargets);
    lv_chart_set_next_value(ui_currentProfileGraph, flowSeries, flowTargets[0]);
    lv_chart_set_next_value(ui_currentProfileGraph, flowSeries, flowTargets[1]);
    lv_chart_set_next_value(ui_currentProfileGraph, pressureSeries, pressureTargets[0]);
    lv_chart_set_next_value(ui_currentProfileGraph, pressureSeries, pressureTargets[1]);
  }
}

void uiHandleCurrentProfileChange(NamedProfile profile)
{
  lv_label_set_text(ui_profileNameLabel, profile.name);
  drawProfileChart(profile.profile);
}

void uiHandleStateSnapshot(const SensorStateSnapshot &state)
{
  if (lv_scr_act() == ui_BrewingScreen)
  {
    return;
  }
  if (state.brewActive) {
    lv_scr_load(ui_BrewingScreen);
    resetBrewGraph();
    return;
  }

  char tempChar[6];
  snprintf(tempChar, 6, "%.1f", state.temperature);
  lv_label_set_text(ui_tempLabel, tempChar);
  lv_arc_set_value(ui_tempGauge, (int)state.temperature);
  char wtrLvlChar[5];
  snprintf(wtrLvlChar, 5, "%d%%", state.waterLvl);
  lv_label_set_text(ui_waterLabel, wtrLvlChar);
  lv_arc_set_value(ui_waterGauge, state.waterLvl);
  char weightChar[7];
  snprintf(weightChar, 7, "%.1fg", state.weight);
  lv_label_set_text(ui_weightLabel, weightChar);
}

void uiHandleShotSnapshot(const ShotSnapshot &snapshot)
{
  if (lv_scr_act() != ui_BrewingScreen) {
    log_w("received shot snapshot while not in brewing screen");
    return; // FIXME: what should happen here?
  }
  updateBrewGraphWithSnapshot(snapshot);
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

void tareButtonTapped(lv_event_t *e)
{
  log_d("tare");
}

// Brew Graph Drawing

static lv_chart_series_t *pressureSeries;
static lv_chart_series_t *tempSeries;
static lv_chart_series_t *weightSeries;
static lv_chart_series_t *weightPerSecSeries;
static lv_chart_series_t *flowSeries;
static unsigned long brewGraphPointCount;

void resetBrewGraph() {
  lv_chart_series_t *series = lv_chart_get_series_next(ui_BrewGraph, NULL);
  while (series != NULL)
  {
    lv_chart_remove_series(ui_BrewGraph, series);
    series = lv_chart_get_series_next(ui_BrewGraph, NULL);
  }

  lv_chart_set_update_mode(ui_BrewGraph, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_chart_set_type(ui_BrewGraph, LV_CHART_TYPE_LINE);

  pressureSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  tempSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
  weightSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_CHART_AXIS_SECONDARY_Y);
  weightPerSecSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_PURPLE), LV_CHART_AXIS_PRIMARY_Y);
  flowSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_YELLOW), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_all_value(ui_BrewGraph, pressureSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_BrewGraph, tempSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_BrewGraph, weightSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_BrewGraph, weightPerSecSeries, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(ui_BrewGraph, flowSeries, LV_CHART_POINT_NONE);

  brewGraphPointCount = 450;
  lv_chart_set_point_count(ui_BrewGraph, brewGraphPointCount);
}

void updateBrewGraphWithSnapshot(const ShotSnapshot &snapshot) {
  // brewGraphPointCount += 1;
  // lv_chart_set_point_count(ui_BrewGraph, brewGraphPointCount);
  lv_chart_set_next_value(ui_BrewGraph, pressureSeries, snapshot.pressure * 10);
  lv_chart_set_next_value(ui_BrewGraph, tempSeries, snapshot.temperature * 10);
  lv_chart_set_next_value(ui_BrewGraph, weightSeries, snapshot.shotWeight * 10);
  lv_chart_set_next_value(ui_BrewGraph, weightPerSecSeries, snapshot.weightFlow * 10);
  lv_chart_set_next_value(ui_BrewGraph, flowSeries, snapshot.pumpFlow * 10);

  uint32_t elapsed = snapshot.timeInShot;
  uint32_t secs = (elapsed / 1000u) % 60;
  uint32_t mins = (secs / 60u);
  char time[6];
  snprintf(time, 6, "%02u:%02u", mins, secs);
  lv_label_set_text(ui_timeLabel, time);

  char pressure[5];
  snprintf(pressure, 5, "%.1f", snapshot.pressure);
  lv_label_set_text(ui_pValueLabel, pressure);
  char flow[5];
  snprintf(flow, 5, "%.1f", snapshot.pumpFlow);
  lv_label_set_text(ui_fValueLabel, flow);
  char weight[7];
  snprintf(weight, 7, "%.1fg", snapshot.shotWeight);
  lv_label_set_text(ui_wValueLabel, weight);
  char weightPerSec[6];
  snprintf(weightPerSec, 6, "%.1f", snapshot.weightFlow);
  lv_label_set_text(ui_wPsValueLabel, weightPerSec);
  char temperature[6];
  snprintf(temperature, 6, "%.1f", snapshot.temperature);
  lv_label_set_text(ui_tValueLabel, temperature);
}

void onShotSnapshotReceived(ShotSnapshot &shotData);
void toggleBrewState();

void drawGraphTask(void *params)
{
  uint32_t startTime = millis();
  auto screen = lv_scr_act();
  uint32_t pressVal = 0;
  uint32_t tempVal = 900;
  float weight = 0;
  while (screen == ui_BrewingScreen)
  {
    if (pressVal < 60)
    {
      pressVal += 1;
    }
    tempVal += lv_rand(-1, 1);
    // lv_chart_set_next_value(ui_BrewGraph, pressureSeries, pressVal);
    // lv_chart_set_next_value(ui_BrewGraph, tempSeries, tempVal);
    // uint16_t count = lv_chart_get_point_count(ui_BrewGraph);
    // if (count > 100) { // figure out how to scale horizontally
    // lv_chart_set_zoom_x(ui_BrewGraph, 255 / count * 255);
    // lv_chart_set_point_count(ui_BrewGraph, count);
    // }
    // lv_chart_refresh(ui_BrewGraph);

    uint32_t elapsed = millis() - startTime;

    if (elapsed > 50000) {
      toggleBrewState();
      break;
    }
    // uint32_t secs = elapsed / 1000u;
    // uint32_t mins = (secs / 60u) % 60;
    // char time[6];
    // snprintf(time, 6, "%02u:%02u", mins, secs);
    // lv_label_set_text(ui_timeLabel, time);

    ShotSnapshot snapshot = {0};
    snapshot.pressure = pressVal / (float)10;
    snapshot.temperature = tempVal / (float)10;
    snapshot.pumpFlow = 3.2;
    snapshot.weightFlow = 1.5;
    if (weight < 90) {
      weight += lv_rand(0, 2) / 10.f;
    }
    snapshot.shotWeight = weight;
    snapshot.timeInShot = elapsed;
    onShotSnapshotReceived(snapshot);

    vTaskDelay(150 / portTICK_PERIOD_MS);
    screen = lv_scr_act();
  }
  log_i("done with graph");
  vTaskDelete(NULL);
}

void brewingScreenAppear(lv_event_t *e)
{
  log_i("brew screen appear");
  // lv_chart_set_update_mode(ui_BrewGraph, LV_CHART_UPDATE_MODE_CIRCULAR);
  // lv_chart_set_type(ui_BrewGraph, LV_CHART_TYPE_LINE);

  // lv_chart_series_t *ser = NULL;
  // do
  // {
  //   ser = lv_chart_get_series_next(ui_BrewGraph, NULL);
  //   if (ser != NULL)
  //   {
  //     lv_chart_remove_series(ui_BrewGraph, ser);
  //   }
  // } while (ser != NULL);

  // pressureSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);
  // tempSeries = lv_chart_add_series(ui_BrewGraph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
  // lv_chart_set_all_value(ui_BrewGraph, pressureSeries, LV_CHART_POINT_NONE);
  // lv_chart_set_all_value(ui_BrewGraph, tempSeries, LV_CHART_POINT_NONE);
  // lv_chart_set_x_start_point(ui_BrewGraph, pressureSeries, 0);
  // lv_chart_set_x_start_point(ui_BrewGraph, tempSeries, 0);

  // lv_chart_set_point_count(ui_BrewGraph, 450);
  xTaskCreateUniversal(drawGraphTask, "drawGraph", configMINIMAL_STACK_SIZE + 2048, NULL, PRIORITY_BLE_SCALES_MAINTAINANCE, NULL, CORE_BLE_SCALES_MAINTAINANCE);
}

void toggleBrewState(lv_event_t *e) {
  toggleBrewState();
}
void brewGraphCloseButtonTapped(lv_event_t *e) {
  toggleBrewState();
  lv_scr_load(ui_HomeScreen);
}