#ifndef __MODELS_H__
#define __MODELS_H__

/* Define model text */
#ifdef MODEL_N100
#define SENSINODE_MODEL "N100 Module"
#endif
#ifdef MODEL_N600
#define SENSINODE_MODEL "N600 NanoRouter USB"
#endif
#ifdef MODEL_N601
#define SENSINODE_MODEL "N601 NanoRouter USB"
#endif
#ifdef MODEL_N710
#define SENSINODE_MODEL "N710 NanoSensor"
#endif
#ifdef MODEL_N711
#define SENSINODE_MODEL "N711 NanoSensor"
#endif
#ifdef MODEL_N740
#define SENSINODE_MODEL "N740 NanoSensor"
#endif

#ifndef SENSINODE_MODEL
#define SENSINODE_MODEL "N100 Module"
#endif

#ifndef FLASH_SIZE
#define FLASH_SIZE "F128"
#endif

/* 
 * N740 has a serial-parallel chip onboard
 * Defines and functions to control it
 */
#ifdef MODEL_N740
#include "dev/n740.h"

#else
/* All other models use these LED pins */
#define LED1_PIN 	P0_4
#define LED2_PIN	P0_5
#endif

#ifdef MODEL_N711
#define BUTTON1_PIN		P0_6
#define BUTTON2_PIN		P0_7
#endif

/* Sensor pins */

#ifdef MODEL_N711
#define LIGHT_PIN		P0_0
#define TEMP_PIN		P0_1
#endif

/* Model-Specific startup functions */
void model_init();
void model_uart_intr_en();
#endif /* __MODELS_H__ */
