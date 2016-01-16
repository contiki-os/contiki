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
 * \defgroup zoul-weather-meter-sensor Sparkfun's weather meter
 * @{
 *
 * \file
 *         Weather meter header file
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/* -------------------------------------------------------------------------- */
#ifndef WEATHER_METER_H_
#define WEATHER_METER_H_
/* -------------------------------------------------------------------------- */
/**
 * \name Weather meter sensor return and operation values
 * @{
 */
#define WEATHER_METER_ANEMOMETER 0x00
#define WEATHER_METER_RAIN_GAUGE 0x01
#define WEATHER_METER_WIND_VANE  0x02

#define WEATHER_METER_ACTIVE     SENSORS_ACTIVE
#define WEATHER_METER_SUCCESS    0
#define WEATHER_METER_ERROR      (-1)

#define WEATHER_METER_ANEMOMETER_RADIUS  65  /**< 65.2 mm pin to cup centre */
#define WEATHER_METER_AUX_CAL            377 /**< 2*pi*60 (376.992 rounded) */
#define WEATHER_METER_AUX_ANGULAR        (WEATHER_METER_ANEMOMETER_RADIUS * \
                                         WEATHER_METER_AUX_CAL)
#define WEATHER_METER_AUX_RAIN_MM        2794 /**< 0.2794mm per tick */

#ifdef WEATHER_METER_RAIN_CONF_RETURN
#define WEATHER_METER_RAIN_RETURN_TICKS  WEATHER_METER_RAIN_CONF_RETURN
#else
#define WEATHER_METER_RAIN_RETURN_TICKS  1
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Anemometer and rain gauge sensor interrupt callback macro
 * @{
 */
#define WEATHER_METER_REGISTER_ANEMOMETER_INT(ptr) anemometer_int_callback = ptr;
#define WEATHER_METER_REGISTER_RAIN_GAUGE_INT(ptr) rain_gauge_int_callback = ptr;
extern void (*anemometer_int_callback)(uint8_t value);
extern void (*rain_gauge_int_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Weather meter's anemometer default pin, port and interrupt vector
 * @{
 */
#ifdef WEATHER_METER_CONF_ANEMOMETER_PIN
#define ANEMOMETER_SENSOR_PIN   WEATHER_METER_CONF_ANEMOMETER_PIN
#else
#define ANEMOMETER_SENSOR_PIN   1
#endif
#ifdef WEATHER_METER_CONF_ANEMOMETER_PORT
#define ANEMOMETER_SENSOR_PORT  WEATHER_METER_CONF_ANEMOMETER_PORT
#else
#define ANEMOMETER_SENSOR_PORT  GPIO_D_NUM
#endif
#ifdef WEATHER_METER_CONF_ANEMOMETER_VECTOR
#define ANEMOMETER_SENSOR_VECTOR  WEATHER_METER_CONF_ANEMOMETER_VECTOR
#else
#define ANEMOMETER_SENSOR_VECTOR  NVIC_INT_GPIO_PORT_D
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Weather meter's rain gauge default pin, port and interrupt vector
 * @{
 */
#ifdef WEATHER_METER_CONF_RAIN_GAUGE_PIN
#define RAIN_GAUGE_SENSOR_PIN   WEATHER_METER_CONF_RAIN_GAUGE_PIN
#else
#define RAIN_GAUGE_SENSOR_PIN   2
#endif
#ifdef WEATHER_METER_CONF_RAIN_GAUGE_PORT
#define RAIN_GAUGE_SENSOR_PORT  WEATHER_METER_CONF_RAIN_GAUGE_PORT
#else
#define RAIN_GAUGE_SENSOR_PORT  GPIO_D_NUM
#endif
#ifdef WEATHER_METER_CONF_RAIN_GAUGE_VECTOR
#define RAIN_GAUGE_SENSOR_VECTOR  WEATHER_METER_CONF_RAIN_GAUGE_VECTOR
#else
#define RAIN_GAUGE_SENSOR_VECTOR  NVIC_INT_GPIO_PORT_D
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Weather meter's wind vane default ADCx pin (see board.h)
 * @{
 */
#ifdef WEATHER_METER_CONF_RAIN_WIND_VANE_ADC
#define WIND_VANE_ADC   WEATHER_METER_CONF_RAIN_WIND_VANE_ADC
#else
#define WIND_VANE_ADC   ZOUL_SENSORS_ADC3
#endif
/** @} */
/* -------------------------------------------------------------------------- */
#define WEATHER_METER_SENSOR "Sparkfun weather meter"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor weather_meter;
/* -------------------------------------------------------------------------- */
#endif /* ifndef WEATHER_METER_H_ */
/**
 * @}
 * @}
 */
