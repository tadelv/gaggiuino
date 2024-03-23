/* 09:32 15/03/2023 - change triggering comment */
#include "just_do_coffee.h"

extern unsigned long steamTime;
// inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }
inline static float TEMP_DELTA(float d, const SensorState &currentState) {
  return (
    d * (currentState.pumpFlow < 1.f
      ? currentState.pumpFlow / 7.f
      : currentState.pumpFlow / 5.f
    )
  );
}

void justDoCoffee(const GaggiaSettings &settings, const SensorState &currentState, float waterTemperature, const bool brewActive) {
  float brewTempSetPoint = waterTemperature + settings.boiler.offsetTemp;
  float sensorTemperature = currentState.temperature + settings.boiler.offsetTemp;

  if (brewActive) { //if brewState == true
    if(sensorTemperature <= brewTempSetPoint - 5.f) {
      setBoilerOn();
    } else {
      float deltaOffset = 0.f;
      if (settings.brew.brewDeltaState) {
        float tempDelta = TEMP_DELTA(brewTempSetPoint, currentState);
        float BREW_TEMP_DELTA = mapRange(sensorTemperature, brewTempSetPoint, brewTempSetPoint + tempDelta, tempDelta, 0, 0);
        deltaOffset = constrain(BREW_TEMP_DELTA, 0, tempDelta);
      }
      if (sensorTemperature <= brewTempSetPoint + deltaOffset) {
        pulseHeaters(settings.boiler.hpwr, settings.boiler.mainDivider, settings.boiler.brewDivider, brewActive);
      } else {
        setBoilerOff();
      }
    }
  } else { //if brewState == false
    if (sensorTemperature <= ((float)brewTempSetPoint - 10.f)) {
      setBoilerOn();
    } else {
      int HPWR_LOW = settings.boiler.hpwr / settings.boiler.mainDivider;
      // Calculating the boiler heating power range based on the below input values
      int HPWR_OUT = mapRange(sensorTemperature, brewTempSetPoint - 10, brewTempSetPoint, settings.boiler.hpwr, HPWR_LOW, 0);
      HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, settings.boiler.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR

      if (sensorTemperature <= ((float)brewTempSetPoint - 5.f)) {
        pulseHeaters(HPWR_OUT, 1, settings.boiler.mainDivider, brewActive);
      } else if (sensorTemperature < ((float)brewTempSetPoint)) {
        pulseHeaters(HPWR_OUT,  settings.boiler.brewDivider, settings.boiler.brewDivider, brewActive);
      } else {
        setBoilerOff();
      }
    }
  }
  if (brewActive || !currentState.brewSwitchState) { // keep steam boiler supply valve open while steaming/descale only
    setSteamValveRelayOff();
  }
  setSteamBoilerRelayOff();
}

void pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive) {
  static uint32_t heaterWave;
  static bool heaterState;
  if (!heaterState && ((millis() - heaterWave) > (pulseLength * factor_1))) {
    brewActive ? setBoilerOff() : setBoilerOn();
    heaterState=!heaterState;
    heaterWave=millis();
  } else if (heaterState && ((millis() - heaterWave) > (pulseLength / factor_2))) {
    brewActive ? setBoilerOn() : setBoilerOff();
    heaterState=!heaterState;
    heaterWave=millis();
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################
void steamCtrl(const GaggiaSettings& settings, const SensorState& currentState, SystemState& systemState) {
  // steam temp control, needs to be aggressive to keep steam pressure acceptable
  float steamTempSetPoint = settings.boiler.steamSetPoint + settings.boiler.offsetTemp;
  float sensorTemperature = currentState.temperature + settings.boiler.offsetTemp;

  if (currentState.smoothedPressure > steamThreshold_ || sensorTemperature > steamTempSetPoint) {
    setBoilerOff();
    setSteamBoilerRelayOff();
    setSteamValveRelayOff();
    setPumpOff();
  } else {
    if (sensorTemperature < steamTempSetPoint) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
    setSteamValveRelayOn();
    setSteamBoilerRelayOn();
    #ifndef DREAM_STEAM_DISABLED // disabled for bigger boilers which have no  need of adding water during steaming
      if (currentState.smoothedPressure < activeSteamPressure_) {
        float pumpPercent = mapRange(currentState.smoothedPressure, 0.8f, 2.f, 0.08f, 0.03f, 2);
        pumpPercent = constrain(pumpPercent, 0.03f, 0.08f);
        setPumpToPercentage(pumpPercent);
      } else {
        setPumpOff();
      }
    #endif
  }

  /*In case steam is forgotten ON for more than 15 min*/
  if (currentState.smoothedPressure > passiveSteamPressure_) {
    systemState.isSteamForgottenON = millis() - steamTime >= STEAM_TIMEOUT;
  } else steamTime = millis();
}

/*Water mode and all that*/
void hotWaterMode(const SensorState &currentState) {
  closeValve();
  setPumpToPercentage(0.8f);
  setBoilerOn();
  if (currentState.temperature < MAX_WATER_TEMP) setBoilerOn();
  else setBoilerOff();
}
