/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
 */

#ifndef ADC_SENSOR_H_
#define ADC_SENSOR_H_

#include "lib/sensors.h"
#include "contiki-conf.h"
#include "lpc17xx_pinsel.h"

//Conversion rate of the ADC module in Hz,
//it should be lower than 200000(200kHz)
#define ADC_CONV_RATE 200000
//VDD reference value used for the ADC
#define VDD 3.293
//Max value of an ADC 12b register
#define ADC_MAX 4095
//Macros for converting the ADC values
//to float values in SI units
#define VOUT_ADC_TO_FLOAT(x)    28*(x)*VDD/(ADC_MAX*3)
#define VIN_ADC_TO_FLOAT(x)     28*(x)*VDD/(ADC_MAX*3)
#define VREF_ADC_TO_FLOAT(x)    28*(x)*VDD/(ADC_MAX*3)
#define VMAX_ADC_TO_FLOAT(x)    28*(x)*VDD/(ADC_MAX*3)
#define IIN_ADC_TO_FLOAT(x)     2*(x)*VDD/(ADC_MAX*0.151)
#define IOUT_ADC_TO_FLOAT(x)    2*(x)*VDD/(ADC_MAX*0.151)
#define IL_ADC_TO_FLOAT(x)      2*(x)*VDD/(ADC_MAX*0.151)
#define IMAX_ADC_TO_FLOAT(x)    2*(x)*VDD/(ADC_MAX*0.151)

//Macros to convert float values in SI units(V, A)
//into values of the ADC range(0x000-0xFFF)
#define VOUT_FLOAT_TO_ADC(x)    (int)((3*(x)*ADC_MAX)/(28*VDD))
#define VIN_FLOAT_TO_ADC(x)     (int)((3*(x)*ADC_MAX)/(28*VDD))
#define VREF_FLOAT_TO_ADC(x)    (int)((3*(x)*ADC_MAX)/(28*VDD))
#define VMAX_FLOAT_TO_ADC(x)    (int)((3*(x)*ADC_MAX)/(28*VDD))
#define IIN_FLOAT_TO_ADC(x)     (int)((ADC_MAX*0.151*(x))/(2*VDD))
#define IOUT_FLOAT_TO_ADC(x)    (int)((ADC_MAX*0.151*(x))/(2*VDD))
#define IL_FLOAT_TO_ADC(x)      (int)((ADC_MAX*0.151*(x))/(2*VDD))
#define IMAX_FLOAT_TO_ADC(x)    (int)((ADC_MAX*0.151*(x))/(2*VDD))


/* Ports and pins of the ADC inputs */
//P0.23=ADC0 --> OUTPUT VOLTAGE
#define VOUT_ADC_CHANNEL        ADC_CHANNEL_0
#define VOUT_PORT               PINSEL_PORT_0
#define VOUT_PIN                PINSEL_PIN_23
#define VOUT_PINFUNC            PINSEL_FUNC_1
//P0.24=ADC1 --> INPUT VOLTAGE
#define VIN_ADC_CHANNEL         ADC_CHANNEL_1
#define VIN_PORT                PINSEL_PORT_0
#define VIN_PIN                 PINSEL_PIN_24
#define VIN_PINFUNC             PINSEL_FUNC_1
//P0.25=ADC2 --> CURRENT SENSOR VZCR
#define VZCR_ADC_CHANNEL        ADC_CHANNEL_2
#define VZCR_PORT               PINSEL_PORT_0
#define VZCR_PIN                PINSEL_PIN_25
#define VZCR_PINFUNC             PINSEL_FUNC_1
//P0.26=ADC3 --> CURRENT SENSOR VIOUT
#define VIOUT_ADC_CHANNEL       ADC_CHANNEL_3
#define VIOUT_PORT              PINSEL_PORT_0
#define VIOUT_PIN               PINSEL_PIN_26
#define VIOUT_PINFUNC            PINSEL_FUNC_1

#define SVECTOR_SENSOR "State vector"

extern const struct sensors_sensor svector_sensor;

#define SVECTOR_SENSOR_ACTIVATE() svector_sensor.configure(SENSORS_ACTIVE, 1)

//Type values for the sensor.value() function
typedef enum{
  SVECTOR_SENSOR_VOUT,
  SVECTOR_SENSOR_VIN,
  SVECTOR_SENSOR_IL,
  SVECTOR_SENSOR_IIN,
  SVECTOR_SENSOR_IOUT
}SVECTOR_SENSOR_TYPES;

float dc_converter_get_svector_parameter(int paramType);


#endif /* ADC_SENSOR_H_ */
