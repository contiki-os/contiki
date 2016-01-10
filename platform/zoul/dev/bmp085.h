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
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-bmp085-sensor BMP085 Sensor
 *
 * Driver for the BMP085 sensor
 *
 * BMP085 digital pressure and temperature driver
 * @{
 *
 * \file
 * Header file for the external BMP085 Sensor Driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef BMP085_H_
#define BMP085_H_
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name BMP085 address and registers
 * @{
 */
/* -------------------------------------------------------------------------- */
#define BMP085_ADDR                      0x77
/* -------------------------------------------------------------------------- */
/* Control register */
#define BMP085_CTRL_REG                  0xF4
/* Read uncompensated temperature  */
#define BMP085_CTRL_REG_TEMP             0x2E
/* Read uncompensated pressure, no oversampling */
#define BMP085_CTRL_REG_PRESS_4_5MS      0x34
/* Read uncompensated pressure, oversampling 1*/
#define BMP085_CTRL_REG_PRESS_7_5MS      0x74
/* Read uncompensated pressure, oversampling 2 */
#define BMP085_CTRL_REG_PRESS_13_5MS     0xB4
/* Read uncompensated pressure, oversampling 3 */
#define BMP085_CTRL_REG_PRESS_25_5MS     0xF4
/* -------------------------------------------------------------------------- */
#define BMP085_DATA_MSB                  0xF6
#define BMP085_DATA_LSB                  0xF7
/* 19-bit resolution */
#define BMP085_DATA_XLSB                 0xF8
/* -------------------------------------------------------------------------- */
/* Calibration registers, 16-bit wide */
#define BMP085_AC1_CALIB                 0xAA
#define BMP085_AC2_CALIB                 0xAC
#define BMP085_AC3_CALIB                 0xAE
#define BMP085_AC4_CALIB                 0xB0
#define BMP085_AC5_CALIB                 0xB2
#define BMP085_AC6_CALIB                 0xB4
#define BMP085_B1_CALIB                  0xB6
#define BMP085_B2_CALIB                  0xB8
#define BMP085_MB_CALIB                  0xBA
#define BMP085_MC_CALIB                  0xBC
#define BMP085_MD_CALIB                  0xBE
#define BMP085_CALIB_TABLE_SIZE          22    /**< size in bytes */
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name BMP085 operation modes
 * @{
 */
#define BMP085_MODE_ULTRA_LOW_POWER      0x00
#define BMP085_MODE_STANDARD             0x01
#define BMP085_MODE_HIGH_RES             0x02
#define BMP085_MODE_ULTRA_HIGH_RES       0x03
/* -------------------------------------------------------------------------- */
#define BMP085_DELAY_4_5MS               4700
#define BMP085_DELAY_7_5MS               7700
#define BMP085_DELAY_13_5MS              13700
#define BMP085_DELAY_25_5MS              25700
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name BMP085 return and command values
 * @{
 */
#define BMP085_SUCCESS         0x00
#define BMP085_ERROR             -1

#define BMP085_ACTIVE          SENSORS_ACTIVE
#define BMP085_OVERSAMPLING    0x00
#define BMP085_READ_PRESSURE   0x01
#define BMP085_READ_TEMP       0x02
/** @} */
/* -------------------------------------------------------------------------- */
#define BMP085_SENSOR "BMP085 pressure and temperature sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor bmp085;
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

