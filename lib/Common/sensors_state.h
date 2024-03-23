/* 09:32 15/03/2023 - change triggering comment */
#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  bool brewSwitchState;
  bool steamSwitchState;
  bool hotWaterSwitchState;
  float temperature;          // °C
  /* calculated water temperature as wanted but not guaranteed
  due to boiler having a hard limit of 4ml/s heat capacity */
  float waterTemperature;     // °C
  float pressure;             // bar
  float pressureChangeSpeed;  // bar/s
  float pumpFlow;             // ml/s
  float pumpFlowChangeSpeed;  // ml/s^2
  float waterPumped;
  float weightFlow;
  float weight;
  float shotWeight;
  float smoothedPressure;
  float smoothedPumpFlow;
  float smoothedWeightFlow;
  float consideredFlow;
  float pumpClicks;
  uint16_t waterLvl;
  bool tofReady;
  // Pump PID
  uint32_t lastPumpCalcTime;
  float powerLineFrequency;
};

struct SensorStateSnapshot {
  bool brewActive;
  bool steamActive;
  bool hotWaterSwitchState;
  float temperature;
  float waterTemperature;
  float pressure;
  float pumpFlow;
  float weightFlow;
  float weight;
  uint16_t waterLevel;
};

struct ShotSnapshot {
  uint32_t timeInShot;
  float pressure;
  float pumpFlow;
  float weightFlow;
  float temperature;
  float shotWeight;
  float waterPumped;

  float targetTemperature;
  float targetPumpFlow;
  float targetPressure;
};

#endif
