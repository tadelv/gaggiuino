/* 09:32 15/03/2023 - change triggering comment */
#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include "utils.h"
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "gaggia_settings.h"
#include "sensors_state.h"
#include "system_state.h"
#include <Arduino.h>


const uint32_t STEAM_TIMEOUT = 900000UL; // steam mode timeout
const float MAX_WATER_TEMP = 105.f; // max temp allowed in hot water mode
const float steamThreshold_ = 11.f; // max allowed pressure before heaters are disabled
const float activeSteamPressure_ = 2.f; // DreamSteam enabling threshold.
const float passiveSteamPressure_ = 3.f; // DreamSteam disabling threshold.

enum class HEATING {
  MODE_brew,
  MODE_steam,
  MODE_hotWater
};

void justDoCoffee(const GaggiaSettings &gaggiaSettings, const SensorState &currentState, float waterTemperature, const bool brewActive);
void pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive);
void steamCtrl(const GaggiaSettings &runningCfg, const SensorState &currentState, SystemState& systemState);
void hotWaterMode(const SensorState &currentState);

#endif

