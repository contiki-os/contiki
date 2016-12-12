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
#ifndef FLOWERPOT_H_
#define FLOWERPOT_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
enum {
  FLOWERPOT_SENSOR_TEMP = 0,
  FLOWERPOT_SENSOR_HUMD,
  FLOWERPOT_SENSOR_SOIL,
};
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t flowerpot_sensors_data_event;
extern process_event_t flowerpot_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t flowerpot_sensors;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_TEMP    "temperature"
#define DEFAULT_PUBLISH_EVENT_HUMD    "humidity"
#define DEFAULT_PUBLISH_EVENT_SOIL    "soil"

#define DEFAULT_PUBLISH_ALARM_TEMP    "alarm_temperature"
#define DEFAULT_PUBLISH_ALARM_HUMD    "alarm_humidity"
#define DEFAULT_PUBLISH_ALARM_SOIL    "alarm_soil"

/* SUBSCRIBE strings */
#define DEFAULT_SUBSCRIBE_CMD_TEMPTHR "temperature_thresh"
#define DEFAULT_SUBSCRIBE_CMD_HUMDTHR "humidity_thresh"
#define DEFAULT_SUBSCRIBE_CMD_SOILTHR "soil_thresh"

/* Minimum and maximum values for the sensors */
#define DEFAULT_SHT25_TEMP_MIN        (-2000)
#define DEFAULT_SHT25_TEMP_MAX        12000
#define DEFAULT_SHT25_HUMD_MIN        0
#define DEFAULT_SHT25_HUMD_MAX        10000
#define DEFAULT_GROVE_SOIL_MIN        0
#define DEFAULT_GROVE_SOIL_MAX        3300

/* Default sensor state and thresholds (only check for values above the
 * threshold) */
#define DEFAULT_TEMP_THRESH           DEFAULT_SHT25_TEMP_MAX
#define DEFAULT_HUMD_THRESH           DEFAULT_SHT25_HUMD_MAX
#define DEFAULT_SOIL_THRESH           DEFAULT_GROVE_SOIL_MAX

#define DEFAULT_TEMP_THRESL           DEFAULT_SHT25_TEMP_MIN
#define DEFAULT_HUMD_THRESL           DEFAULT_SHT25_HUMD_MIN
#define DEFAULT_SOIL_THRESL           DEFAULT_GROVE_SOIL_MIN
/*---------------------------------------------------------------------------*/
#endif /* FLOWERPOT_H_ */
/** @} */

