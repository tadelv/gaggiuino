/* 09:32 15/03/2023 - change triggering comment */
#include "pump.h"
#include "pindef.h"
#include <PSM.h>
#include "utils.h"
#include "internal_watchdog.h"
#include "log.h"
#include "PIDController.h"
#undef round
#include "math.h"

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 1, 6);
PIDController controller(
  0.7f, 
  0.01f,
  0.3f
);


float flowPerClickAtZeroBar = 0.216f;
int maxPumpClicksPerSecond = 50;
float fpc_multiplier = 1.2f;
//estimated through CP4SP datasheet
constexpr std::array<float, 7> pressureInefficiencyCoefficient{ {
  0.002854508806889097f,
  -0.13472880734783618f,
  2.476926292402373f,
  -22.330673515198136f,
  101.80888820175406f,
  -231.10822510824374f,
  520.0000000000166f
} };

// Initialising some pump specific specs, mainly:
// - max pump clicks(dependant on region power grid spec)
// - pump clicks at 0 pressure in the system
void pumpInit(const int powerLineFrequency, const float pumpFlowAtZero) {
  // pump.freq = powerLineFrequency;
  maxPumpClicksPerSecond = powerLineFrequency;
  flowPerClickAtZeroBar = pumpFlowAtZero;
  fpc_multiplier = 60.f / (float)maxPumpClicksPerSecond;
}

// Function that returns the percentage of clicks the pump makes in it's current phase
inline float getPumpPct(const float targetPressure, const float flowRestriction, const SensorState& currentState) {
  if (targetPressure == 0.f) {
    return 0.f;
  }

  LOG_DEBUG("current flow: %f", currentState.smoothedPumpFlow);
  LOG_DEBUG("current press: %f", currentState.smoothedPressure);
  LOG_DEBUG("target pressure: %f", targetPressure);
  float resistance = currentState.smoothedPressure / currentState.smoothedPumpFlow;
  LOG_DEBUG("resistance: %f", resistance);
  uint32_t currentMillis = millis();
  uint32_t delta_t = currentMillis - currentState.lastPumpCalcTime;
  LOG_DEBUG("delta t: %u", delta_t);
  float control_output = controller.calculate(targetPressure, currentState.smoothedPressure, delta_t / 1000.f);
  LOG_DEBUG("control output: %f", control_output);
  float targetFlow = fmaxf(currentState.smoothedPumpFlow + control_output, 0);
  LOG_DEBUG("target flow: %f", targetFlow);

  float suggestedFlow = flowRestriction > 0 ? fminf(targetFlow, flowRestriction) : targetFlow;
  LOG_DEBUG("suggested flow: %f", suggestedFlow);
  float suggestedPumpPct = getClicksPerSecondForFlow(suggestedFlow, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;
  LOG_DEBUG("suggested pct: %f", suggestedPumpPct);

  return fminf(1.f, fmaxf(suggestedPumpPct, 0));
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
void setPumpPressure(const float targetPressure, const float flowRestriction, const SensorState& currentState) {
  float pumpPct = getPumpPct(targetPressure, flowRestriction, currentState);
  setPumpToPercentage(pumpPct);
}

void setPumpOff(void) {
  pump.set(0);
}

void setPumpFullOn(void) {
  pump.set(PUMP_RANGE);
}

void setPumpToPercentage(float pct) {
    pump.set((uint8_t) std::round(pct * PUMP_RANGE));
}

void pumpStopAfter(const uint8_t val) {
  pump.stopAfter(val);
}

long getAndResetClickCounter(void) {
  long counter = pump.getCounter();
  pump.resetCounter();
  return counter;
}

int getCPS(void) {
  watchdogReload();
  unsigned int cps = pump.cps();
  watchdogReload();
  if (cps > 80u) {
    pump.setDivider(2);
    pump.initTimer(cps > 110u ? 60u : 50u, TIM9);
  }
  else {
    pump.initTimer(cps > 55u ? 60u : 50u, TIM9);
  }
  return cps;
}

void pumpPhaseShift(void) {
  pump.shiftDividerCounter();
}

// Models the flow per click, follows a compromise between the schematic and recorded findings
// plotted: https://www.desmos.com/calculator/eqynzclagu
float getPumpFlowPerClick(const float pressure)
{
  float fpc = 0.f;
  float livepressure = pressure;
  if (livepressure <= 0.f ) livepressure = 0.1f;
  const int degree = 6; //pressureInefficiencyCoefficient.size() - 1;

  for (int i = 0; i <= degree; ++i) {
    fpc += pressureInefficiencyCoefficient[i] * (float)std::pow(livepressure, degree - i);
  }

  return fpc / 60 / maxPumpClicksPerSecond;
}

// Follows the schematic from https://www.cemegroup.com/solenoid-pump/e5-60 modified to per-click
float getPumpFlow(const float cps, const float pressure) {
  return cps * getPumpFlowPerClick(pressure);
}

// Currently there is no compensation for pressure measured at the puck, resulting in incorrect estimates
float getClicksPerSecondForFlow(const float flow, const float pressure) {
  if (flow == 0.f) return 0;
  float flowPerClick = getPumpFlowPerClick(pressure);
  float cps = flow / flowPerClick;
  return fminf(cps, (float)maxPumpClicksPerSecond);
}

// Calculates pump percentage for the requested flow and updates the pump raw value
void setPumpFlow(const float targetFlow, const float pressureRestriction, const SensorState& currentState) {
  // If a pressure restriction exists then the we go into pressure profile with a flowRestriction
  // which is equivalent but will achieve smoother pressure management
  if (pressureRestriction > 0.f && currentState.smoothedPressure > pressureRestriction * 0.5f) {
    setPumpPressure(pressureRestriction, targetFlow, currentState);
  }
  else {
    float pumpPct = getClicksPerSecondForFlow(targetFlow, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;
    setPumpToPercentage(pumpPct);
  }
}

void resetController() {
 controller.reset(); 
}

void setControllerParams(float *kp, float *ki, float *kd)
{
  if (kp != NULL) {
    controller.setKp(*kp);
  }
  if (ki != NULL) {
    controller.setKi(*ki);
  }
  if (kd != NULL) {
    controller.setKd(*kd);
  }
}

void getControllerParams(float *kp, float *ki, float *kd)
{
  controller.getParams(kp, ki, kd);
}
