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
 * \defgroup zoul-bmpx8x-sensor BMP085/BMP180 Sensor
 *
 * Driver for the BMP085/BMP180 sensor
 *
 * BMP085/BMP180 digital atmospheric pressure and temperature driver
 * @{
 *
 * \file
 * Header file for the external BMP085/BMP180 Sensor Driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef BMPX8X_H_
#define BMPX8X_H_
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name BMPx8x address and registers
 * @{
 */
/* -------------------------------------------------------------------------- */
#define BMPx8x_ADDR                      0x77
/* -------------------------------------------------------------------------- */
/* Control register */
#define BMPx8x_CTRL_REG                  0xF4
/* Read uncompensated temperature  */
#define BMPx8x_CTRL_REG_TEMP             0x2E
/* Read uncompensated pressure, no oversampling */
#define BMPx8x_CTRL_REG_PRESS_4_5MS      0x34
/* Read uncompensated pressure, oversampling 1*/
#define BMPx8x_CTRL_REG_PRESS_7_5MS      0x74
/* Read uncompensated pressure, oversampling 2 */
#define BMPx8x_CTRL_REG_PRESS_13_5MS     0xB4
/* Read uncompensated pressure, oversampling 3 */
#define BMPx8x_CTRL_REG_PRESS_25_5MS     0xF4
/* -------------------------------------------------------------------------- */
#define BMPx8x_DATA_MSB                  0xF6
#define BMPx8x_DATA_LSB                  0xF7
/* 19-bit resolution */
#define BMPx8x_DATA_XLSB                 0xF8
/* -------------------------------------------------------------------------- */
/* Calibration registers, 16-bit wide */
#define BMPx8x_AC1_CALIB                 0xAA
#define BMPx8x_AC2_CALIB                 0xAC
#define BMPx8x_AC3_CALIB                 0xAE
#define BMPx8x_AC4_CALIB                 0xB0
#define BMPx8x_AC5_CALIB                 0xB2
#define BMPx8x_AC6_CALIB                 0xB4
#define BMPx8x_B1_CALIB                  0xB6
#define BMPx8x_B2_CALIB                  0xB8
#define BMPx8x_MB_CALIB                  0xBA
#define BMPx8x_MC_CALIB                  0xBC
#define BMPx8x_MD_CALIB                  0xBE
#define BMPx8x_CALIB_TABLE_SIZE          22    /**< size in bytes */
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name BMPx8x operation modes
 * @{
 */
#define BMPx8x_MODE_ULTRA_LOW_POWER      0x00
#define BMPx8x_MODE_STANDARD             0x01
#define BMPx8x_MODE_HIGH_RES             0x02
#define BMPx8x_MODE_ULTRA_HIGH_RES       0x03
/* -------------------------------------------------------------------------- */
#define BMPx8x_DELAY_4_5MS               4700
#define BMPx8x_DELAY_7_5MS               7700
#define BMPx8x_DELAY_13_5MS              13700
#define BMPx8x_DELAY_25_5MS              25700
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name BMPx8x return and command values
 * @{
 */
#define BMPx8x_SUCCESS         0x00
#define BMPx8x_ERROR             -1

#define BMPx8x_ACTIVE          SENSORS_ACTIVE
#define BMPx8x_OVERSAMPLING    0x00
#define BMPx8x_READ_PRESSURE   0x01
#define BMPx8x_READ_TEMP       0x02
/** @} */
/* -------------------------------------------------------------------------- */
#define BMPx8x_SENSOR "BMP085/BMP180 pressure and temperature sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor bmpx8x;
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

