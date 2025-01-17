/* 09:32 15/03/2023 - change triggering comment */
#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "pindef.h"
#include "peripherals.h"
#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif


static inline void pinInit(void) {
#if defined(LEGO_VALVE_RELAY)
  pinMode(valvePin, OUTPUT_OPEN_DRAIN);
#else
  pinMode(valvePin, OUTPUT);
#endif
  pinMode(relayPin, OUTPUT);
#ifdef steamValveRelayPin
  pinMode(steamValveRelayPin, OUTPUT);
#endif
#ifdef steamBoilerRelayPin
  pinMode(steamBoilerRelayPin, OUTPUT);
#endif
  pinMode(brewPin, INPUT_PULLUP);
#ifndef ASCASO
  pinMode(steamPin, INPUT_PULLUP);
#endif
#ifdef waterPin
  pinMode(waterPin, INPUT_PULLUP);
#endif
}

// Actuating the heater element
static inline void setBoilerOn(void) {
  digitalWrite(relayPin, HIGH);  // boilerPin -> HIGH
}

static inline void setBoilerOff(void) {
  digitalWrite(relayPin, LOW);  // boilerPin -> LOW
}

static inline void setSteamValveRelayOn(void) {
#ifdef steamValveRelayPin
  digitalWrite(steamValveRelayPin, HIGH);  // steamValveRelayPin -> HIGH
#endif
}

static inline void setSteamValveRelayOff(void) {
#ifdef steamValveRelayPin
  digitalWrite(steamValveRelayPin, LOW);  // steamValveRelayPin -> LOW
#endif
}

static inline void setSteamBoilerRelayOn(void) {
#ifdef steamBoilerRelayPin
  digitalWrite(steamBoilerRelayPin, HIGH);  // steamBoilerRelayPin -> HIGH
#endif
}

static inline void setSteamBoilerRelayOff(void) {
#ifdef steamBoilerRelayPin
  digitalWrite(steamBoilerRelayPin, LOW);  // steamBoilerRelayPin -> LOW
#endif
}

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
static inline bool brewState(void) {
#ifdef ASCASO_MOMENTARY_SWITCH
  static bool brewState = false;
  static uint32_t time = millis();
  uint32_t currentMillis = millis();
  uint8_t newButtonState = digitalRead(brewPin);
  if (newButtonState == LOW &&
    time < currentMillis - 1 * 1000) {
    brewState = !brewState;
    time = currentMillis;
  }
  return brewState;
#else
  return digitalRead(brewPin) == LOW; // pin will be low when switch is ON.
#endif
}

// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is ON.
static inline bool steamState(void) {
#ifdef steamPin
  return digitalRead(steamPin) == LOW; // pin will be low when switch is ON.
#else
  return false;
#endif
}

static inline bool waterPinState(void) {
#ifdef waterPin
  return digitalRead(waterPin) == LOW; // pin will be low when switch is ON.
#else
  return false;
#endif
}

static inline void openValve(void) {
#if defined LEGO_VALVE_RELAY
  digitalWrite(valvePin, LOW);
#else
  digitalWrite(valvePin, HIGH);
#endif
}

static inline void closeValve(void) {
#if defined LEGO_VALVE_RELAY
  digitalWrite(valvePin, HIGH);
#else
  digitalWrite(valvePin, LOW);
#endif
}

#endif
