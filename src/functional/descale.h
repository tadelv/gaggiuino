/* 09:32 15/03/2023 - change triggering comment */
#ifndef DESCALE_H
#define DESCALE_H

#include <Arduino.h>
#include "sensors_state.h"
#include "system_state.h"
#include "gaggia_settings.h"

const unsigned long DESCALE_PHASE1_EVERY = 30000UL; //30000 // short pump pulses during descale
const unsigned long DESCALE_PHASE2_EVERY = 60000UL; //60000 // long pause for scale softening
const unsigned long DESCALE_PHASE3_EVERY = 10000UL; //10000 // short burst for descale efficiency

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(const GaggiaSettings &gaggiaSettings, const SensorState &currentState);
void solenoidBeat(void);
void backFlush(const SensorState &currentState);
void flushActivated(void);
void flushDeactivated(void);
void flushPhases(void);
#endif
