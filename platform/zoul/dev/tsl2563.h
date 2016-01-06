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
 * \defgroup zoul-tsl2563-sensor TSL2563 Sensor
 *
 * Driver for the TSL2563 sensor
 *
 * The TSL2563 driver returns the converted light value value in lux
 * @{
 *
 * \file
 * Header file for the external TSL2563 Sensor Driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef TSL2563_H_
#define TSL2563_H_
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name TSL2563 digital Light sensor address and registers
 * @{
 */
/* -------------------------------------------------------------------------- */
#define TSL2563_ADDR                  0x39
/* -------------------------------------------------------------------------- */
#define TSL2563_CONTROL               0x00
#define TSL2563_TIMMING               0x01
#define TSL2563_THRLOWLOW             0x02
#define TSL2563_THRLOWHIGH            0x03
#define TSL2563_THRHIGHLOW            0x04
#define TSL2563_THRHIGHHIGH           0x05
#define TSL2563_INTERRUPT             0x06
#define TSL2563_CRC                   0x08
#define TSL2563_ID_REG                0x0A
#define TSL2563_D0LOW                 0x0C
#define TSL2563_D0HIGH                0x0D
#define TSL2563_D1LOW                 0x0E
#define TSL2563_D1HIGH                0x0F
/* -------------------------------------------------------------------------- */
/* Uses the word read/write operation protocol */
#define TSL2563_COMMAND               0xA0
#define TSL2563_CLEAR_INTERRUPT       0x40
/* -------------------------------------------------------------------------- */
#define TSL2563_CONTROL_POWER_ON      0x03
#define TSL2563_CONTROL_POWER_OFF     0x00
#define TSL2563_TIMMING_GAIN          0x10
#define TSL2563_TIMMING_MANUAL        0x08
#define TSL2563_TIMMING_INTEG_MANUAL  0x03
#define TSL2563_TIMMING_INTEG_402MS   0x02
#define TSL2563_TIMMING_INTEG_101MS   0x01
#define TSL2563_TIMMING_INTEG_13_7MS  0x00
#define TSL2563_TIMMING_INTEG_MASK    0x03

#define TSL2563_G16X_402MS (TSL2563_TIMMING_INTEG_402MS + TSL2563_TIMMING_GAIN)
#define TSL2563_G1X_402MS                            TSL2563_TIMMING_INTEG_402MS
#define TSL2563_G1X_101MS                            TSL2563_TIMMING_INTEG_101MS
#define TSL2563_G1X_13_7MS                          TSL2563_TIMMING_INTEG_13_7MS

#define TSL2563_INTR_SHIFT            0x04
#define TSL2563_INTR_DISABLED         0x00
#define TSL2563_INTR_LEVEL            0x01
#define TSL2563_INTR_SMB_ALERT        0x02
#define TSL2563_INTR_TEST             0x03

#define TSL2563_INT_PERSIST_EVERY     0x00
#define TSL2563_INT_PERSIST_ANY       0x01
#define TSL2563_INT_PERSIST_2_CYCLES  0x02
#define TSL2563_INT_PERSIST_3_CYCLES  0x03
#define TSL2563_INT_PERSIST_4_CYCLES  0x04
#define TSL2563_INT_PERSIST_5_CYCLES  0x05
#define TSL2563_INT_PERSIST_6_CYCLES  0x06
#define TSL2563_INT_PERSIST_7_CYCLES  0x07
#define TSL2563_INT_PERSIST_8_CYCLES  0x08
#define TSL2563_INT_PERSIST_9_CYCLES  0x09
#define TSL2563_INT_PERSIST_10_CYCLES 0x0A
#define TSL2563_INT_PERSIST_11_CYCLES 0x0B
#define TSL2563_INT_PERSIST_12_CYCLES 0x0C
#define TSL2563_INT_PERSIST_13_CYCLES 0x0D
#define TSL2563_INT_PERSIST_14_CYCLES 0x0E
#define TSL2563_INT_PERSIST_15_CYCLES 0x0F

#define TSL2563_ID_PARTNO_MASK        0xF0
#define TSL2563_ID_REV_MASK           0x0F
#define TSL2563_EXPECTED_PARTNO       0x30
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL2563 convertion and calibration values
 * @{
 */

#define LUX_SCALE           14     /**< scale by 2^14 */
#define RATIO_SCALE         9      /**< scale ratio */
#define CH_SCALE            10     /**< scale channel values by 2^10 */
#define CHSCALE_TINT0       0x7517 /**< 322/11 * 2^CH_SCALE */
#define CHSCALE_TINT1       0x0fe7 /**< 322/81 * 2^CH_SCALE */

/* T/FN/CL package coefficients (hardcoded) */
#define K1T                 0X0040
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
/**
 * \name Callback function to handle the TSL2563 alarm interrupt and macro
 * @{
 */
#define TSL2563_REGISTER_INT(ptr) tsl2563_int_callback = ptr;
extern void (*tsl2563_int_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL2563 return and command values
 * @{
 */
#define TSL2563_SUCCESS         0x00
#define TSL2563_LIGHT           0x01
#define TSL2563_ERROR             -1

#define TSL2563_ACTIVE          SENSORS_ACTIVE
#define TSL2563_INT_OVER        HW_INT_OVER_THRS
#define TSL2563_INT_BELOW       HW_INT_BELOW_THRS
#define TSL2563_INT_DISABLE     HW_INT_DISABLE
#define TSL2563_TIMMING_CFG     (HW_INT_DISABLE + 1)

#define TSL2563_VAL_READ        0x01
/** @} */
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

