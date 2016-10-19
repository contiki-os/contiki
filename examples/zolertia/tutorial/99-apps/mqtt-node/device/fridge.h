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
#ifndef FRIDGE_H_
#define FRIDGE_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
enum {
  FRIDGE_SENSOR_TEMP = 0,
  FRIDGE_SENSOR_HUMD,
};
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t fridge_sensors_data_event;
extern process_event_t fridge_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t fridge_sensors;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_TEMP    "temperature"
#define DEFAULT_PUBLISH_EVENT_HUMD    "humidity"
#define DEFAULT_PUBLISH_ALARM_TEMP    "alarm_temperature"
#define DEFAULT_PUBLISH_ALARM_HUMD    "alarm_humidity"

/* SUBSCRIBE strings */
#define DEFAULT_SUBSCRIBE_CFG_TEMPTHR "temperature_thresh"
#define DEFAULT_SUBSCRIBE_CFG_HUMDTHR "humidity_thresh"

/* Minimum and maximum values for the SHT25 sensor */
#define DEFAULT_SHT25_TEMP_MIN        (-2000)
#define DEFAULT_SHT25_TEMP_MAX        12000
#define DEFAULT_SHT25_HUMD_MIN        0
#define DEFAULT_SHT25_HUMD_MAX        10000

/* Default sensor state and thresholds
 * We are only checking for values over a given threshold (over), to avoid
 * checking for values below, we use the minimum sensor value
 */
#define DEFAULT_TEMP_THRESH           3000
#define DEFAULT_HUMD_THRESH           8000
#define DEFAULT_TEMP_THRESL           DEFAULT_SHT25_TEMP_MIN
#define DEFAULT_HUMD_THRESL           DEFAULT_SHT25_HUMD_MIN
/*---------------------------------------------------------------------------*/
#endif /* FRIDGE_H_ */
/** @} */

