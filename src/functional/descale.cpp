/* 09:32 15/03/2023 - change triggering comment */
#include "descale.h"
#include "just_do_coffee.h"
#include "../peripherals/internal_watchdog.h"
#include "../lcd/lcd.h"

DescalingState descalingState = DescalingState::IDLE;

short flushCounter = 0;
uint8_t counter = 0;
unsigned long descalingTimer = 0;
int descalingCycle = 0;

void deScale(eepromValues_t &runningCfg, const SensorState &currentState) {
  switch (descalingState) {
    case DescalingState::IDLE: // Waiting for fuckfest to begin
      if (currentState.brewSwitchState) {
        ACTIVE_PROFILE(runningCfg).setpoint = 9;
        openValve();
        setSteamValveRelayOn();
        descalingState = DescalingState::DESCALING_PHASE1;
        descalingCycle = 0;
        descalingTimer = millis();
      }
      break;
    case DescalingState::DESCALING_PHASE1: // Slowly penetrating that scale
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(10);
      if (millis() - descalingTimer > DESCALE_PHASE1_EVERY) {
        lcdSetDescaleCycle(descalingCycle++);
        if (descalingCycle < 100) {
          descalingTimer = millis();
          descalingState = DescalingState::DESCALING_PHASE2;
        } else {
          descalingState = DescalingState::FINISHED;
        }
      }
      break;
    case DescalingState::DESCALING_PHASE2: // Softening the f outta that scale
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpOff();
      if (millis() - descalingTimer > DESCALE_PHASE2_EVERY) {
        descalingTimer = millis();
        lcdSetDescaleCycle(descalingCycle++);
        descalingState = DescalingState::DESCALING_PHASE3;
      }
      break;
    case DescalingState::DESCALING_PHASE3: // Fucking up that scale big time
      currentState.brewSwitchState ? descalingState : descalingState = DescalingState::FINISHED;
      setPumpToRawValue(30);
      if (millis() - descalingTimer > DESCALE_PHASE3_EVERY) {
        solenoidBeat();
        lcdSetDescaleCycle(descalingCycle++);
        if (descalingCycle < 100) {
          descalingTimer = millis();
          descalingState = DescalingState::DESCALING_PHASE1;
        } else {
          descalingState = DescalingState::FINISHED;
        }
      }
      break;
    case DescalingState::FINISHED: // Scale successufuly fucked
      setPumpOff();
      closeValve();
      setSteamValveRelayOff();
      currentState.brewSwitchState ? descalingState = DescalingState::FINISHED : descalingState = DescalingState::IDLE;
      if (millis() - descalingTimer > 1000) {
        lcdBrewTimerStop();
        lcdShowPopup("FINISHED");
        descalingTimer = millis();
      }
      break;
  }
  justDoCoffee(runningCfg, currentState, false);
}

void solenoidBeat() {
  setPumpFullOn();
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  delay(200);
  closeValve();
  delay(1000);
  watchdogReload();
  openValve();
  setPumpOff();
}

void backFlush(const SensorState &currentState) {
#ifndef ASCASO_FLUSH
  static unsigned long backflushTimer = millis();
  unsigned long elapsedTime = millis() - backflushTimer;
#else  
  static bool flushReady = false;
#endif

  if (currentState.brewSwitchState) {
    if (flushCounter >= 11) {
      flushDeactivated();
      return;
    }
#ifdef ASCASO_FLUSH
    if (currentState.smoothedPressure > 9.f) {
      flushReady = true;
    }
    if (flushReady) {
      flushPhases();
      return;
    }
    flushActivated(); 
#else
    else if (
      elapsedTime > 7000UL && currentState.smoothedPressure > 5.f
      ) {
      flushPhases();
    }
    else flushActivated();
#endif
  } else {
    flushDeactivated();
    flushCounter = 0;
#ifndef ASCASO_FLUSH
    backflushTimer = millis();
#else
    flushReady = false;
#endif
  }
}


void flushActivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      openValve();
  #endif
  setPumpFullOn();
}

void flushDeactivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      closeValve();
  #endif
  setPumpOff();
}

void flushPhases(void) {
  static long timer = millis();
  static int phaseDuration = 7000;
  if (flushCounter > 10) {
    flushDeactivated();
    timer = millis();
    return;
  }
  switch (flushCounter % 2) {
    case 1:
      openValve();
      setPumpFullOn();
      break;
    case 0:
      closeValve();
      setPumpOff();
      break;
  }
  if (millis() - timer >= phaseDuration) {
    flushCounter++;
    timer = millis();
  }
}