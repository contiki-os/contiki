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
#ifndef WATERING_H_
#define WATERING_H_
/*---------------------------------------------------------------------------*/
#include "mqtt-sensors.h"
/*---------------------------------------------------------------------------*/
enum {
  WATERING_SENSOR_SOIL = 0,
};

enum {
  WATERING_COMMAND_VALVE = 0,
};
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t watering_sensors_data_event;
extern process_event_t watering_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t watering_sensors;
/*---------------------------------------------------------------------------*/
extern command_values_t watering_commands;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_SOIL    "soil_moisture"
#define DEFAULT_PUBLISH_ALARM_SOIL    "alarm_soil_moist"

/* SUBSCRIBE strings */
#define DEFAULT_SUBSCRIBE_CFG_SOILTHR "soil_moist_thresl"

/* Minimum and maximum values for the sensor */
#define DEFAULT_SOIL_MOIST_MIN        0
#define DEFAULT_SOIL_MOIST_MAX        100

/* Default sensor state and thresholds
 * We only care to check for the soil moisture value below a level
 */
#define DEFAULT_SOIL_THRESH           DEFAULT_SOIL_MOIST_MAX
#define DEFAULT_SOIL_THRESL           30

/* Command string: open an electrovalve for a given period */
#define DEFAULT_COMMAND_EVENT_VALVE   "electrovalve"

#define ELECTROVALVE_ON_INTERVAL      ((CLOCK_SECOND)*2)
/*---------------------------------------------------------------------------*/
#endif /* WATERING_H_ */
/** @} */

