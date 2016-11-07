/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
 */
/*---------------------------------------------------------------------------*/
#ifndef REMOTE_H_
#define REMOTE_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
enum {
  REMOTE_SENSOR_TEMP = 0,
  REMOTE_SENSOR_BATT,
  REMOTE_SENSOR_BUTN,
  REMOTE_SENSOR_ADC1,
  REMOTE_SENSOR_ADC3,
};
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t remote_sensors_data_event;
extern process_event_t remote_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t remote_sensors;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_TEMP    "core_temp"
#define DEFAULT_PUBLISH_EVENT_BATT    "battery"
#define DEFAULT_PUBLISH_ALARM_BUTN    "button_pres"
#define DEFAULT_PUBLISH_EVENT_ADC1    "ADC1"
#define DEFAULT_PUBLISH_EVENT_ADC3    "ADC3"

/* Minimum and maximum values for the sensors */
#define DEFAULT_CC2538_TEMP_MIN       0
#define DEFAULT_CC2538_TEMP_MAX       32666
#define DEFAULT_CC2538_BATT_MIN       0
#define DEFAULT_CC2538_BATT_MAX       5000
#define DEFAULT_CC2538_ADC1_MIN       0
#define DEFAULT_CC2538_ADC1_MAX       5000
#define DEFAULT_CC2538_ADC3_MIN       0
#define DEFAULT_CC2538_ADC3_MAX       5000
#define DEFAULT_CC2538_BUTN_MIN       0
#define DEFAULT_CC2538_BUTN_MAX       32666

/* Default sensor state and thresholds (not checking for alarms) */
#define DEFAULT_TEMP_THRESH           DEFAULT_CC2538_TEMP_MAX
#define DEFAULT_BATT_THRESH           DEFAULT_CC2538_BATT_MAX
#define DEFAULT_ADC1_THRESH           DEFAULT_CC2538_ADC1_MAX
#define DEFAULT_ADC3_THRESH           DEFAULT_CC2538_ADC3_MAX

#define DEFAULT_TEMP_THRESL           DEFAULT_CC2538_TEMP_MIN
#define DEFAULT_BATT_THRESL           DEFAULT_CC2538_BATT_MIN
#define DEFAULT_ADC1_THRESL           DEFAULT_CC2538_ADC1_MIN
#define DEFAULT_ADC3_THRESL           DEFAULT_CC2538_ADC3_MIN

/* We post alarms directly to the platform process, so we avoid having the value
 * being checked by the mqtt-sensors process
 */
#define DEFAULT_BUTN_THRESH           DEFAULT_CC2538_BUTN_MAX
#define DEFAULT_BUTN_THRESL           DEFAULT_CC2538_BUTN_MIN

/* Use a lower alarm threshold as the user button is more interactive than
 * others type of alarms
 */
#define DEFAULT_CONF_ALARM_TIME       3
/*---------------------------------------------------------------------------*/
#endif /* REMOTE_H_ */
/** @} */

