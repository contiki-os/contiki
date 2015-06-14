/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup remote-sensors
 * @{
 *
 * \defgroup remote-tsl2563-sensor Re-Mote TSL2563 Sensor
 *
 * Driver for the Re-Mote TSL2563 sensor
 *
 * The TSL2563 driver returns the converted light value value in lux
 * @{
 *
 * \file
 * Header file for the Re-Mote external TSL2563 Sensor Driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_
#include <stdio.h>
#include "lib/sensors.h"
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name TSL2563 digital Light sensor
 * @{
 */
/* -------------------------------------------------------------------------- */
#define TSL2563_ADDR        0x39 /**< TSL2563 slave address */
/* -------------------------------------------------------------------------- */
#define TSL2563_READ        0xAC   /**< TSL2563 read register */
#define TSL2563_PWRN        0x03   /**< TSL2563 enable register */
#define TSL2563_PWROFF      0x00   /**< TSL2563 Power OFF */
/* -------------------------------------------------------------------------- */
#define K1T                 0X0040 /**< Calibration values (hardcoded) */
#define B1T                 0x01f2
#define M1T                 0x01b2
#define K2T                 0x0080
#define B2T                 0x0214
#define M2T                 0x02d1
#define K3T                 0x00c0
#define B3T                 0x023f
#define M3T                 0x037b
#define K4T                 0x0100
#define B4T                 0x0270
#define M4T                 0x03fe
#define K5T                 0x0138
#define B5T                 0x016f
#define M5T                 0x01fc
#define K6T                 0x019a
#define B6T                 0x00d2
#define M6T                 0x00fb
#define K7T                 0x029a
#define B7T                 0x0018
#define M7T                 0x0012
#define K8T                 0x029a
#define B8T                 0x0000
#define M8T                 0x0000
/** @} */
/* -------------------------------------------------------------------------- */
#define TSL2563_SUCCESS     0x00
#define TSL2563_LIGHT       0x01
#define TSL2563_ERROR         -1
/* -------------------------------------------------------------------------- */
#define TSL2563_VAL_READ    0x01
/* -------------------------------------------------------------------------- */
#define TSL2563_SENSOR "TSL2563 Light Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor tsl2563;
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

