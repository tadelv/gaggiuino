/* 09:32 15/03/2023 - change triggering comment */
#include "pump.h"
#include "pindef.h"
#include <PSM.h>
#include "utils.h"
#include "internal_watchdog.h"

PSM pump(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 1, 6);

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

  if (formula)
  {
    if (currentState.smoothedPressure <= 0.f)
    {
      return getClicksPerSecondForFlow(flowRestriction > 0 ? flowRestriction : (flowPerClickAtZeroBar * maxPumpClicksPerSecond),
                                       currentState.smoothedPressure) /
             (float)maxPumpClicksPerSecond;
    }
    LOG_DEBUG("current flow: %f", currentState.smoothedPumpFlow);
    LOG_DEBUG("current press: %f", currentState.smoothedPressure);
    float resistance = currentState.smoothedPressure / currentState.smoothedPumpFlow;
    LOG_DEBUG("resistance: %f", resistance);
    float targetFlow = targetPressure / resistance;
    LOG_DEBUG("target flow: %f", targetFlow);
    float pressureDiff = targetPressure - currentState.smoothedPressure;
    float kFactor = fabs(pressureDiff) / 10; // 0.8;
    #if 0
    if (pressureDiff < 5 || currentState.pressureChangeSpeed > 0.3)
    {
      // decrease factor
      kFactor = fabs(pressureDiff) / 10;//(10 * currentState.pressureChangeSpeed > 1 ? (currentState.pressureChangeSpeed * 2) : 1);
    }
    #endif
    float suggestedFlow = targetFlow + kFactor * ((targetPressure * 0.9f) - currentState.smoothedPressure);
    LOG_DEBUG("suggested flow: %f", suggestedFlow);
    float suggestedPumpPct = getClicksPerSecondForFlow(suggestedFlow, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;
    LOG_DEBUG("suggested pct: %f", suggestedPumpPct);

    return fminf(1.f, fmaxf(suggestedPumpPct, 0));
  }
  float diff = targetPressure - currentState.smoothedPressure;
  float maxPumpPct = flowRestriction <= 0.f ? 1.f : getClicksPerSecondForFlow(flowRestriction, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;
  float pumpPctToMaintainFlow = getClicksPerSecondForFlow(currentState.smoothedPumpFlow, currentState.smoothedPressure) / (float)maxPumpClicksPerSecond;

  if (diff > 2.f) {
    return fminf(maxPumpPct, 0.25f + 0.2f * diff);
  }

  if (diff > 0.f) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.95f + 0.1f + 0.2f * diff);
  }

  if (currentState.pressureChangeSpeed < 0) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.2f);
  }

  return 0;
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
void setPumpPressure(const float targetPressure, const float flowRestriction, const SensorState& currentState) {
  float pumpPct = getPumpPct(targetPressure, flowRestriction, currentState);
  setPumpToRawValue((uint8_t)(pumpPct * PUMP_RANGE));
}

void setPumpOff(void) {
  pump.set(0);
}

void setPumpFullOn(void) {
  pump.set(PUMP_RANGE);
}

void setPumpToRawValue(const uint8_t val) {
  pump.set(val);
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
float getPumpFlowPerClick(const float pressure) {
  float fpc = 0.f;
  const int degree = 6; //pressureInefficiencyCoefficient.size() - 1;

  for (int i = 0; i <= degree; ++i) {
    fpc += pressureInefficiencyCoefficient[i] * (float)std::pow(pressure, degree - i);
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
    setPumpToRawValue(pumpPct * PUMP_RANGE);
  }
}
