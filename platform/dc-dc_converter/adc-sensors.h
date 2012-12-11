/*
 * adc-sensor.h
 *
 *  Created on: Dec 5, 2012
 *      Author: cazulu
 */

#ifndef ADC_SENSOR_H_
#define ADC_SENSOR_H_

#include "lib/sensors.h"
#include "contiki-conf.h"
#include "lpc17xx_pinsel.h"

//Conversion rate of the ADC module in Hz,
//it should be lower than 200000(200kHz)
#define ADC_CONV_RATE 200000

/* Ports and pins of the ADC inputs */
//P0.23=ADC0 --> OUTPUT VOLTAGE
#define VOUT_ADC_CHANNEL        ADC_CHANNEL_0
#define VOUT_PORT               PINSEL_PORT_0
#define VOUT_PIN                PINSEL_PIN_23
//P0.24=ADC1 --> INPUT VOLTAGE
#define VIN_ADC_CHANNEL         ADC_CHANNEL_1
#define VIN_PORT                PINSEL_PORT_0
#define VIN_PIN                 PINSEL_PIN_24
//P0.25=ADC2 --> CURRENT SENSOR VZCR
#define VZCR_ADC_CHANNEL        ADC_CHANNEL_2
#define VZCR_PORT               PINSEL_PORT_0
#define VZCR_PIN                PINSEL_PIN_25
//P0.26=ADC3 --> CURRENT SENSOR VIOUT
#define VIOUT_ADC_CHANNEL       ADC_CHANNEL_3
#define VIOUT_PORT              PINSEL_PORT_0
#define VIOUT_PIN               PINSEL_PIN_26


#define SVECTOR_SENSOR "State vector"

extern const struct sensors_sensor svector_sensor;

#define SVECTOR_SENSOR_ACTIVATE() svector_sensor.configure(SENSORS_ACTIVE, 1)

//Type values for the sensor.value() function
typedef enum{
  SVECTOR_SENSOR_VOUT,
  SVECTOR_SENSOR_VIN,
  SVECTOR_SENSOR_IL
}SVECTOR_SENSOR_TYPES;


#endif /* ADC_SENSOR_H_ */
