/* 09:32 15/03/2023 - change triggering comment */
#include "descale.h"
#include "just_do_coffee.h"
#include "../peripherals/internal_watchdog.h"
#include "../peripherals/esp_comms.h"

const int DESCALE_MAX_CYCLES = 100;

DescalingState descalingState = DescalingState::IDLE;

short flushCounter = 0;
uint8_t counter = 0;
uint32_t descalingTimer = 0;
uint32_t descalingStartTime = 0;
uint32_t descalingStopTime = 0;
int descalingCycle = 0;

void sendDescalingProgressToEsp();

void deScale(const GaggiaSettings& gaggiaSettings, const SensorState& currentState) {
  if (currentState.waterLvl <= 4u) {
    setPumpOff();
    closeValve();
    setSteamValveRelayOff();
    espCommsSendNotification(Notification::info("Fill tank with fresh water to clean the system!"));
    return;
  }

  switch (descalingState) {
  case DescalingState::IDLE: // Waiting for fuckfest to begin
    if (currentState.brewSwitchState) {
      openValve();
      setSteamValveRelayOn();
      descalingState = DescalingState::PHASE1;
      descalingCycle = 0;
      descalingTimer = millis();
      descalingStartTime = millis();
      descalingStopTime = 0;
    }
    break;
  case DescalingState::PHASE1: // Slowly penetrating that scale
    descalingState = currentState.brewSwitchState ? descalingState : DescalingState::FINISHED;
    setPumpToPercentage(0.1f);
    if (millis() - descalingTimer > DESCALE_PHASE1_EVERY) {
      descalingCycle += 1;
      if (descalingCycle < DESCALE_MAX_CYCLES) {
        descalingTimer = millis();
        descalingState = DescalingState::PHASE2;
      }
      else {
        descalingState = DescalingState::FINISHED;
      }
    }
    break;
  case DescalingState::PHASE2: // Softening the f outta that scale
    descalingState = currentState.brewSwitchState ? descalingState : DescalingState::FINISHED;
    setPumpOff();
    if (millis() - descalingTimer > DESCALE_PHASE2_EVERY) {
      descalingTimer = millis();
      descalingCycle += 1;
      descalingState = DescalingState::PHASE3;
    }
    break;
  case DescalingState::PHASE3: // Fucking up that scale big time
    descalingState = currentState.brewSwitchState ? descalingState : DescalingState::FINISHED;
    setPumpToPercentage(0.3f);
    if (millis() - descalingTimer > DESCALE_PHASE3_EVERY) {
      solenoidBeat();
      descalingCycle += 1;
      if (descalingCycle < DESCALE_MAX_CYCLES) {
        descalingTimer = millis();
        descalingState = DescalingState::PHASE1;
      }
      else {
        descalingState = DescalingState::FINISHED;
      }
    }
    break;
  case DescalingState::FINISHED: // Scale successufuly fucked
    if (descalingStopTime == 0) {
      descalingStopTime = millis();
      espCommsSendNotification(Notification::info("Descale finished!"));
    }
    setPumpOff();
    closeValve();
    setSteamValveRelayOff();
    descalingState = currentState.brewSwitchState ? DescalingState::FINISHED : DescalingState::IDLE;
    break;
  }

  sendDescalingProgressToEsp();
  justDoCoffee(gaggiaSettings, currentState, 9.f, false);
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
  openValve();
  setPumpFullOn();
}

void flushDeactivated(void) {
  closeValve();
  setPumpOff();
}

void flushPhases(void) {
  static uint32_t timer = millis();
  static uint32_t phaseDuration = 7000;
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

void sendDescalingProgressToEsp()
{
  static uint32_t espUpdateTimer = 0;
  uint32_t now = millis();

  if (now - espUpdateTimer > 1000)
  {
    espUpdateTimer = now;
    uint32_t time = 0;
    uint8_t progress = 0;
    if (descalingState != DescalingState::IDLE)
    {
      time = descalingStopTime == 0 ? now - descalingStartTime : descalingStopTime - descalingStartTime;
      progress = (uint8_t)(100 * descalingCycle / DESCALE_MAX_CYCLES);
    }

    espCommsSendDescaleProgress({.state = descalingState,
                                 .time = time,
                                 .progess = progress});
  }
}
