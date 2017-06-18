/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-pm10-sensor Analog PM10 sensor
 * @{
 * \file
 *         GP2Y1010AU0F PM10 sensor driver
 * \author
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#ifndef PM10_SENSOR_H_
#define PM10_SENSOR_H_
/* -------------------------------------------------------------------------- */
#define PM10_ERROR 	                 (-1)
#define PM10_SUCCESS                 0
#define PM10_SENSOR                  "PM10 Sensor"
#define PM10_SENSOR_PULSE_DELAY      280
#define PM10_EXTERNAL_VREF           5000
#define PM10_EXTERNAL_VREF_CROSSVAL  3000
/* -------------------------------------------------------------------------- */
#ifdef PM10_SENSOR_CONF_CTRL_PIN
#define PM10_SENSOR_CTRL_PIN         PM10_SENSOR_CONF_CTRL_PIN
#else
#define PM10_SENSOR_CTRL_PIN         7
#endif
#ifdef PM10_SENSOR_CONF_CTRL_PORT
#define PM10_SENSOR_CTRL_PORT        PM10_SENSOR_CONF_CTRL_PORT
#else
#define PM10_SENSOR_CTRL_PORT        GPIO_A_NUM
#endif
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor pm10;
/* -------------------------------------------------------------------------- */
#endif /* ifndef PM10_SENSOR_H_ */
/**
 * @}
 * @}
 */
