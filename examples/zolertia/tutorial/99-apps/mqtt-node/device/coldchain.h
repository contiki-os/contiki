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
#ifndef COLDCHAIN_H_
#define COLDCHAIN_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
enum {
  COLDCHAIN_SENSOR_TEMP = 0,
  COLDCHAIN_SENSOR_HUMD,
  COLDCHAIN_SENSOR_LGHT
};
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t coldchain_sensors_data_event;
extern process_event_t coldchain_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t coldchain_sensors;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_TEMP    "temperature"
#define DEFAULT_PUBLISH_EVENT_HUMD    "humidity"
#define DEFAULT_PUBLISH_EVENT_LGTH    "light"
#define DEFAULT_PUBLISH_ALARM_TEMP    "alarm_temperature"
#define DEFAULT_PUBLISH_ALARM_HUMD    "alarm_humidity"
#define DEFAULT_PUBLISH_ALARM_LGTH    "alarm_light"

/* SUBSCRIBE strings */
#define DEFAULT_SUBSCRIBE_CFG_TEMPTHR "temperature_thresh"
#define DEFAULT_SUBSCRIBE_CFG_HUMDTHR "humidity_thresh"
#define DEFAULT_SUBSCRIBE_CFG_LGTHTHR "light_thresh"

/* Minimum and maximum values for the DHT22 sensor */
#define DEFAULT_TEMP_MIN              (-200)
#define DEFAULT_TEMP_MAX              1200
#define DEFAULT_HUMD_MIN              0
#define DEFAULT_HUMD_MAX              1000
#define DEFAULT_LGTH_MIN              0
#define DEFAULT_LGTH_MAX              32000

/* Default sensor state and thresholds
 * We are only checking for values over a given threshold (over), to avoid
 * checking for values below, we use the minimum sensor value
 */
#define DEFAULT_TEMP_THRESH           300
#define DEFAULT_HUMD_THRESH           800
#define DEFAULT_LGTH_THRESH           1000

#define DEFAULT_TEMP_THRESL           0
#define DEFAULT_HUMD_THRESL           0
#define DEFAULT_LGTH_THRESL           0

/*---------------------------------------------------------------------------*/
#endif /* COLDCHAIN_H_ */
/** @} */

