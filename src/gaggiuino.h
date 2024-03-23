/* 09:32 15/03/2023 - change triggering comment */
#ifndef GAGGIUINO_H
#define GAGGIUINO_H

#include <Arduino.h>
#include <SimpleKalmanFilter.h>

#include "log.h"
#include "peripherals/internal_watchdog.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/scales.h"
#include "peripherals/peripherals.h"
#include "peripherals/thermocouple.h"
#include "sensors_state.h"
#include "system_state.h"
#include "functional/descale.h"
#include "functional/just_do_coffee.h"
#include "functional/predictive_weight.h"
#include "functional/shot_profiler.h"
#include "profiling_phases.h"
#include "peripherals/esp_comms.h"
#include "peripherals/led.h"
#include "peripherals/tof.h"
#include "peripherals/debug_comms.h"

// Define some const values
#if defined SINGLE_BOARD
#define GET_KTYPE_READ_EVERY    70 // max31855 amp module data read interval not recommended to be changed to lower than 70 (ms)
#else
#define GET_KTYPE_READ_EVERY    250 // max6675 amp module data read interval not recommended to be changed to lower than 250 (ms)
#endif
#define GET_PRESSURE_READ_EVERY 10 // Pressure refresh interval (ms)
#define GET_SCALES_READ_EVERY   200 // Scales refresh interval (ms)
#define GET_SCALES_ACCIDENTAL   2000u // Accidental touches or placing cup on scales post brew activation timeout
#define REFRESH_ESP_DATA_EVERY  100 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      200 // Flow refresh interval (ms)
#define HEALTHCHECK_EVERY       30000 // System checks happen every 30sec
#define BOILER_FILL_START_TIME  3000UL // Boiler fill start time - 3 sec since system init.
#define BOILER_FILL_TIMEOUT     8000UL // Boiler fill timeout - 8sec since system init.
#define BOILER_FILL_SKIP_TEMP   85.f // Boiler fill skip temperature threshold
#define SYS_PRESSURE_IDLE       0.7f // System pressure threshold at idle
#define MIN_WATER_LVL           10u // Min allowable tank water lvl

// If not defined in the extra_defines.ini use default range
#if not defined(TOF_START) || not defined(TOF_END)
#define TOF_START 6u
#define TOF_END 130u
#endif

const uint16_t tofStartValue = TOF_START; // Tof offset when tank is full
const uint16_t tofEndValue = TOF_END; // Tof offset when tank is nearly empty
const float weightRateThreshold = 9.f; // The rate of weigh random change(aka accidental scales touching)
const float weightIncreaseThreshold = 40.f; // Accounting for placing a cup on the scales after initiating brew

//Timers
unsigned long systemHealthTimer;
unsigned long pageRefreshTimer;
unsigned long pressureTimer;
unsigned long brewingTimer;
unsigned long thermoTimer;
unsigned long scalesTimer;
unsigned long scalesTimeout;
unsigned long flowTimer;
unsigned long steamTime;

//scales vars
Measurements weightMeasurements(4);

// brew detection vars
bool brewActive = false;
bool nonBrewModeActive = false;

//PP&PI variables
int preInfusionFinishedPhaseIdx = 3;
bool homeScreenScalesEnabled = false;

// Other util vars
float previousSmoothedPressure;
float previousSmoothedPumpFlow;

static void sysHealthCheck(float pressureThreshold);

#endif
