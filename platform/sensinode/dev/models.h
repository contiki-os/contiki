#ifndef __MODELS_H__
#define __MODELS_H__

/* Define model text */
#ifdef MODEL_N100
#define SENSINODE_MODEL "N100 Module (CC2431-F128)"
#endif
#ifdef MODEL_N600
#define SENSINODE_MODEL "N600 NanoRouter USB (CC2430-F128)"
#endif
#ifdef MODEL_N601
#define SENSINODE_MODEL "N601 NanoRouter USB (CC2431-F128)"
#endif
#ifdef MODEL_N710
#define SENSINODE_MODEL "N710 NanoSensor (CC2430-F128)"
#endif
#ifdef MODEL_N711
#define SENSINODE_MODEL "N711 NanoSensor (CC2431-F128)"
#endif

#ifndef SENSINODE_MODEL
#define MODEL_N100
#define SENSINODE_MODEL "Sensinode N100 (CC2431-F128)"
#endif

/* All current models use these LED pins */
#define LED1_PIN 	P0_4
#define LED2_PIN	P0_5

/* Buttons */

#ifdef MODEL_N711
#define BUTTON1_PIN		P0_6
#define BUTTON2_PIN		P0_7
#endif

/* Sensor pins */

#ifdef MODEL_N711
#define LIGHT_PIN		P0_0
#define TEMP_PIN		P0_1
#endif

#endif /* __MODELS_H__ */
