#ifndef STM_COMMS_H
#define STM_COMMS_H

#include "mcu_comms.h"
#include "gaggia_settings.h"
#include "system_state.h"
#include "notification_message.h"

void stmCommsInit(HardwareSerial& serial);
void stmCommsReadData();
void stmCommsSendWeight(float weight);
void stmCommsSendScaleDisconnected();
void stmCommsSendGaggiaSettings(const GaggiaSettings& settings);
void stmCommsSendProfile(const Profile& profile);
void stmCommsSendBrewSettings(const BrewSettings& settings);
void stmCommsSendBoilerSettings(const BoilerSettings& settings);
void stmCommsSendLedSettings(const LedSettings& settings);
void stmCommsSendSystemSettings(const SystemSettings& settings);
void stmCommsSendScalesSettings(const ScalesSettings& settings);
void stmCommsSendUpdateSystemState(const UpdateSystemStateComand& state);

// To be defined elsewhere
void onSensorStateSnapshotReceived(const SensorStateSnapshot& snapshot);
void onShotSnapshotReceived(const ShotSnapshot& snapshot);
void onScalesTareReceived();
void onGaggiaSettingsRequested();
void onProfileRequested();
void onSystemStateReceived(const SystemState& systemState);
void onNotification(const Notification& notification);
void onDescalingProgressReceived(const DescalingProgress& progress);

#endif
