/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 * \defgroup zoul-dht22 DHT22 temperature and humidity sensor
 *
 * Driver for the DHT22 temperature and humidity sensor
 * @{
 *
 * \file
 * Header file for the DHT22 temperature and humidity sensor
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#ifndef DHT22_H_
#define DHT22_H_
/* -------------------------------------------------------------------------- */
/**
 * \name DHT22 default pin and port
 * @{
 */
#ifdef DHT22_CONF_PIN
#define DHT22_PIN        DHT22_CONF_PIN
#else
#define DHT22_PIN        5
#endif
#ifdef DHT22_CONF_PORT
#define DHT22_PORT       DHT22_CONF_PORT
#else
#define DHT22_PORT       GPIO_A_NUM
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name DHT22 available commands
 * @{
 */
#define DHT22_READ_TEMP         0x01
#define DHT22_READ_HUM          0x02
#define DHT22_READ_ALL          0x03
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name DHT22 return types
 * @{
 */
#define DHT22_ERROR             (-1)
#define DHT22_SUCCESS           0x00
#define DHT22_BUSY              0xFF
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name DHT22 constants
 * @{
 */
#define DHT22_BUFFER            5    /**< Buffer to store the samples         */
#define DHT22_COUNT             8    /**< Minimum ticks to detect a "1" bit   */
#define DHT22_MAX_TIMMING       85   /**< Maximum ticks in a single operation */
#define DHT22_READING_DELAY     1                                 /**< 1 us   */
#define DHT22_READY_TIME        20                                /**< 40 us  */
#define DHT22_START_TIME        (RTIMER_SECOND / 50)              /**< 20 ms  */
#define DHT22_AWAKE_TIME        (RTIMER_SECOND / 4)               /**< 250 ms */
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name DHT22 auxiliary functions
 * @{
 */
int dht22_read_all(int *temperature, int *humidity);
/** @} */
/* -------------------------------------------------------------------------- */
#define DHT22_SENSOR "DHT22 sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor dht22;
/* -------------------------------------------------------------------------- */
#endif /* DHT22_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
