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
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup zoul-sensors
 * @{
 * \defgroup zoul-iaq-sensor IAQ driver (Indoor Air Quality) Sensor Module
 * IAQ driver RE-Mote Indoor Air Quality Sensor Module
 * Driver for the RE-Mote Sensor pack: Air Quality Sensor Module (IAQ)
 * @{
 * \file
 * Header file for the RE-Mote Sensor IAQ
 */
/* -------------------------------------------------------------------------- */
#ifndef IAQ_H_
#define IAQ_H_
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "iaq.h"
#include "i2c.h"
#include "sys/timer.h"
#include "sys/rtimer.h"
/* -------------------------------------------------------------------------- */
/** \name IAQ address and definition
 * @{
 */
/* Address of the sensor: 1011010(1) Addr (R/W bit) */
#define IAQ_ADDR                  0x5A
#define IAQ_SENSOR               "iAQ"
/** @} */
/* -------------------------------------------------------------------------- */
/** \name IAQ sensor types in iAQ-Core module 
 * @{
 */
extern const struct sensors_sensor iaq;
/** @} */
/* -------------------------------------------------------------------------- */
/** \name IAQ error values and definitions
 * @{
 */
#define IAQ_ACTIVE                SENSORS_ACTIVE
#define IAQ_INIT_STATE            SENSORS_HW_INIT
#define IAQ_ERROR                 (-1)
#define IAQ_SUCCESS               0x00
#define IAQ_FRAME_SIZE            0x09

/* Variables used by external driver to get the state */
#define IAQ_INTERNAL_SUCCESS      0x00
#define IAQ_INTERNAL_RUNIN        0x10
#define IAQ_INTERNAL_BUSY         0x01
#define IAQ_INTERNAL_ERROR        0x80

/* Value types for the sensor readings */
#define IAQ_CO2_VALUE             0x00
#define IAQ_VOC_VALUE             0x01
#define IAQ_STATUS                0x02
#define IAQ_DRIVER_STATUS         0x03

/* Definition that corresponds with the two models of iAQ Sensor */
#ifdef IAQ_PULSE_MODE
#define IAQ_POLLING_TIME         (CLOCK_SECOND * 11)
#else
#define IAQ_POLLING_TIME         (CLOCK_SECOND)
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/** \name IAQ enumeration and options
 * @{
 */
enum {
  IAQ_INIT = 0,
  IAQ_STARTED,
};
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Readable IAQ-Core interface result conversion implementation as 
 * datasheet specification.
 *
 * @{
 */
typedef struct iaq_struct_simple_td_reg {
  uint16_t co2;
  uint8_t status;
  int32_t resistance;
  uint16_t tvoc;
}  __attribute__ ((packed)) simple_iaq_data;
/** @} */
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
