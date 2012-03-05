/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
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
 */

/**
 * \file
 *         Defines for the sensors on the various Sensinode models.
 *
 *         Sensors will be off by default, unless turned on explicitly
 *         in contiki-conf.h
 *
 *         If you enable sensors generating interrupts, make sure you include
 *         this file in the file containing main().
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef SENSINODE_SENSORS_H_
#define SENSINODE_SENSORS_H_

#include "cc2430_sfr.h"
#include "contiki-conf.h"
#include "dev/models.h"
#include "lib/sensors.h"

void sensinode_sensors_activate();
void sensinode_sensors_deactivate();

/* ADC Sensor Types */
#define ADC_SENSOR "ADC"

#define ADC_SENSOR_TYPE_TEMP    0
#define ADC_SENSOR_TYPE_ACC_X   1
#define ADC_SENSOR_TYPE_ACC_Y   2
#define ADC_SENSOR_TYPE_ACC_Z   3
#define ADC_SENSOR_TYPE_VDD     4
#define ADC_SENSOR_TYPE_LIGHT   5
#define ADC_SENSOR_TYPE_BATTERY 6

/* Defines to help us control Acc and Ill individually */
#define  ADC_VAL_NONE       0x00
#define  ADC_VAL_ALL        0x01
#define  ADC_VAL_LIGHT_ON   0x04
#define  ADC_VAL_ACC_ON     0x08
#define  ADC_VAL_ACC_GSEL   0x10

#ifdef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_ON ADC_SENSOR_CONF_ON
#endif /* ADC_SENSOR_CONF_ON */

#if ADC_SENSOR_ON
extern const struct sensors_sensor adc_sensor;
#endif /* ADC_SENSOR_ON */

/*
 * Accelerometer. Available on N740 only.
 * This is a Freescale Semiconductor MMA7340L (3 axis, 3/11g)
 * X: P0_5
 * Y: P0_6
 * Z: P0_7
 */
#ifdef MODEL_N740
#ifdef ACC_SENSOR_CONF_ON
#define ACC_SENSOR_ON ACC_SENSOR_CONF_ON
#endif /* ACC_SENSOR_CONF_ON */

/* Accelerometer g-Select - Define for +/-11g, +/-3g Otherwise */
#if ACC_SENSOR_ON
#ifdef ACC_SENSOR_CONF_GSEL
#define ACC_SENSOR_GSEL ACC_SENSOR_CONF_GSEL
#endif /* ACC_SENSOR_CONF_GSEL */
#endif /* ACC_SENSOR_ON */
#endif /* MODEL_N740 */

/*
 * Buttons
 * N740: P1_0, P0_4
 * N711: P0_6, P0_7
 * N710: Unknown. This will mainly influence which ISRs to declare here
 */
#if defined(MODEL_N740) || defined(MODEL_N711)
#ifdef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_ON BUTTON_SENSOR_CONF_ON
#endif /* BUTTON_SENSOR_CONF_ON */
#endif /* defined(MODEL_FOO) */

#define BUTTON_1_SENSOR "Button 1"
#define BUTTON_2_SENSOR "Button 2"
#define BUTTON_SENSOR   BUTTON_1_SENSOR

#if BUTTON_SENSOR_ON
extern const struct sensors_sensor button_1_sensor;
extern const struct sensors_sensor button_2_sensor;
#define button_sensor button_1_sensor

/* Port 0 ISR needed for both models */
void port_0_ISR(void) __interrupt (P0INT_VECTOR);

/* Only declare the Port 1 ISR for N740 */
#ifdef MODEL_N740
void port_1_ISR(void) __interrupt (P1INT_VECTOR);
#endif /* MODEL_N740 */
#endif /* BUTTON_SENSOR_ON */

/*
 * Light - N710, N711, N740
 * N740: P0_0
 * N711: P0_0
 * N710: P?_?
 */
#if defined(MODEL_N740) || defined(MODEL_N711) || defined(MODEL_N710)
#ifdef LIGHT_SENSOR_CONF_ON
#define LIGHT_SENSOR_ON LIGHT_SENSOR_CONF_ON
#endif /* LIGHT_SENSOR_CONF_ON */
#endif /* defined(MODEL_FOO) */

/*
 * Battery - N711, N740, (N710 likely)
 * N740: P0_1
 * Unknown for other models
 */
#if defined(MODEL_N740) || defined(MODEL_N711) || defined(MODEL_N710)
#ifdef BATTERY_SENSOR_CONF_ON
#define BATTERY_SENSOR_ON BATTERY_SENSOR_CONF_ON
#endif /* BATTERY_SENSOR_CONF_ON */
#endif /* defined(MODEL_FOO) */

/* Temperature - Available on all cc2430 devices */
#ifdef TEMP_SENSOR_CONF_ON
#define TEMP_SENSOR_ON TEMP_SENSOR_CONF_ON
#endif /* TEMP_SENSOR_CONF_ON */

/* Supply Voltage - Available on all cc2430 devices*/
#ifdef VDD_SENSOR_CONF_ON
#define VDD_SENSOR_ON VDD_SENSOR_CONF_ON
#endif /* VDD_SENSOR_CONF_ON */

#endif /* SENSINODE_SENSORS_H_ */
