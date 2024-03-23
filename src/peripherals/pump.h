/* 09:32 15/03/2023 - change triggering comment */
#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "sensors_state.h"

#define ZC_MODE    FALLING

constexpr uint8_t PUMP_RANGE = 250; // Push to 250 for less rounding

void pumpInit(const int powerLineFrequency, const float pumpFlowAtZero);
void setPumpPressure(const float targetPressure, const float flowRestriction, const SensorState &currentState);
void setPumpOff(void);
void setPumpFullOn(void);
void setPumpToPercentage(const float percentage); // 0.0 - 1.0
long  getAndResetClickCounter(void);
int getCPS(void);
void pumpPhaseShift(void);
void pumpStopAfter(const uint8_t val);
float getPumpFlow(const float cps, const float pressure);
float getPumpFlowPerClick(const float pressure);
float getClicksPerSecondForFlow(const float flow, const float pressure);
void setPumpFlow(const float targetFlow, const float pressureRestriction, const SensorState &currentState);

void resetController();
void setControllerParams(float *kp, float *ki, float *kd);
void getControllerParams(float *kp, float *ki, float *kd);
#endif
