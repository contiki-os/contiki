/*
 * Copyright (c) 2015, Copyright Robert Olsson
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
 *
 * Author  : Robert Olsson rolss@kth.se/robert@radio-sensors.com
 *
 * Created : 2016-09-14
 */

/**
 * \file
 *         Definitions for the Bosch BME280 based on datasheet Rev 1.1
 */

#ifndef BME280_H
#define BME280_H

#ifndef BME280_32BIT
#define BME280_64BIT
#endif

uint8_t bme280_init(uint8_t mode);
void bme280_read(uint8_t mode);

#ifdef BME280_CONF_ADDR
#define BME280_ADDR         BME280_CONF_ADDR
#else
#define BME280_ADDR              (0x77 << 1) /* Alternative 0x76 */
#endif

/* Diffrent BOSCH chip id's */
#define BMP085_CHIP_ID                 0x55  /* And also BMP180 */
#define BMP280_CHIP_ID                 0x58
#define BME280_CHIP_ID                 0x60

/* Address map */
#define BME280_DIG_T1_ADDR             0x88
#define BME280_DIG_T2_ADDR             0x8A
#define BME280_DIG_T3_ADDR             0x8C
#define BME280_DIG_P1_ADDR             0x8E
#define BME280_DIG_P2_ADDR             0x90
#define BME280_DIG_P3_ADDR             0x92
#define BME280_DIG_P4_ADDR             0x94
#define BME280_DIG_P5_ADDR             0x96
#define BME280_DIG_P6_ADDR             0x98
#define BME280_DIG_P7_ADDR             0x9A
#define BME280_DIG_P8_ADDR             0x9C
#define BME280_DIG_P9_ADDR             0x9E
#define BME280_DIG_H1_ADDR             0xA1
#define BMP_CHIP_ID_ADDR               0xD0
#define BME280_CNTL_RESET              0xE0
#define BME280_DIG_H2_ADDR             0xE1
#define BME280_DIG_H3_ADDR             0xE3
#define BME280_DIG_H4_ADDR             0xE4
#define BME280_DIG_H5_ADDR             0xE5
#define BME280_DIG_H6_ADDR             0xE7
#define BME280_CNTL_HUM                0xF2
#define BME280_STATUS                  0xF3
#define BME280_CNTL_MEAS               0xF4
#define BME280_CONTROL                 0xF5
#define BME280_PRESS                   0xF7

/* Function modes outlined in datasheet */
#define BME280_MODE_NONE               0
#define BME280_MODE_WEATHER            1
#define BME280_MODE_HUMIDITY           2
#define BME280_MODE_INDOOR_NAVIGATION  3
#define BME280_MODE_GAMING             4

#define BME280_MAX_WAIT                300 /* ms. Forced mode max wait */


struct {
  int32_t t_overscale100;
  uint32_t h_overscale1024;
#ifdef BME280_64BIT
  uint32_t p_overscale256;
#else
  uint32_t p;
#endif
} bme280_mea;

#endif /* BME280_H */
