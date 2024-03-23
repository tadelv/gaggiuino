/* 09:32 15/03/2023 - change triggering comment */
#ifndef PINDEF_H
#define PINDEF_H

#ifdef RUNNING_TESTS
#include "mock.h"
#endif

// STM32F4 pins definitions
#define thermoDO      PB4
#define thermoDI      PA7 // not used
#define thermoCS      PA6
#define thermoCLK     PA5

#define zcPin         PA0
#define brewPin       PC14
#define relayPin      PA15
#define dimmerPin     PA1
#if defined(ASCASO)
// #define steamPin      PB15
#else
#define steamPin      PC15
#endif
#define valvePin      PC13
#if defined(SINGLE_BOARD) && !defined(ASCASO)
#define waterPin      PB15
#elif defined(ASCASO)
#define waterPin      PC15
#else
#define waterPin      PA12
#endif

#ifdef PCBV2
// PCB V2
#define steamValveRelayPin PB12
#define steamBoilerRelayPin PB13
#endif

#define HX711_sck_1   PB0
#define HX711_dout_1  PB8
#define HX711_dout_2  PB9

#define USART_LCD     Serial2 // [Active - FULL DUPLEX DATA TRANSFER] - PA2(TX) & PA3(RX)
#define USART_ESP     Serial1 // [INACTIVE - SEE BELOW]  PA9(TX) & PA10(RX)
#define USART_DEBUG   Serial  // USB-CDC (Takes PA8,PA9,PA10,PA11)

#endif
