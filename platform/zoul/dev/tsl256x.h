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
 * \defgroup zoul-tsl256x-sensor TSL256X Sensor
 *
 * Driver for the TSL256X sensor
 *
 * The TSL256X driver returns the converted light value value in lux
 * @{
 *
 * \file
 * Header file for the external TSL256X Sensor Driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef TSL256X_H_
#define TSL256X_H_
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name TSL256x digital Light sensor specific model information
 * @{
 */
/* This driver supports the TSL2563 (Zolertia) and the TSL2561 (Grove) */
#define TSL2561_SENSOR_REF            0
#define TSL2563_SENSOR_REF            1

/* The TSL2563 (from Zolertia) has a different part number than the TSL2561 from
 * Grove (digital light sensor)
 */
#define TSL2563_EXPECTED_PARTNO       0x30
#define TSL2561_EXPECTED_PARTNO       0x50

#ifndef TSL256X_CONF_REF
#define TSL256X_REF                   TSL2561_SENSOR_REF
#else
#define TSL256X_REF                   TSL256X_CONF_REF
#endif

#if TSL256X_CONF_REF == TSL2561_SENSOR
#define TSL256X_ADDR                  0x29
#define TSL256X_EXPECTED_PARTNO       TSL2561_EXPECTED_PARTNO
#else
#define TSL256X_ADDR                  0x39
#define TSL256X_EXPECTED_PARTNO       TSL2563_EXPECTED_PARTNO
#endif

/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL256X digital Light registers
 * @{
 */
/* -------------------------------------------------------------------------- */
#define TSL256X_CONTROL               0x00
#define TSL256X_TIMMING               0x01
#define TSL256X_THRLOWLOW             0x02
#define TSL256X_THRLOWHIGH            0x03
#define TSL256X_THRHIGHLOW            0x04
#define TSL256X_THRHIGHHIGH           0x05
#define TSL256X_INTERRUPT             0x06
#define TSL256X_CRC                   0x08
#define TSL256X_ID_REG                0x0A
#define TSL256X_D0LOW                 0x0C
#define TSL256X_D0HIGH                0x0D
#define TSL256X_D1LOW                 0x0E
#define TSL256X_D1HIGH                0x0F
/* -------------------------------------------------------------------------- */
/* Uses the word read/write operation protocol */
#define TSL256X_COMMAND               0xA0
#define TSL256X_CLEAR_INTERRUPT       0x40
/* -------------------------------------------------------------------------- */
#define TSL256X_CONTROL_POWER_ON      0x03
#define TSL256X_CONTROL_POWER_OFF     0x00
#define TSL256X_TIMMING_GAIN          0x10
#define TSL256X_TIMMING_MANUAL        0x08
#define TSL256X_TIMMING_INTEG_MANUAL  0x03
#define TSL256X_TIMMING_INTEG_402MS   0x02
#define TSL256X_TIMMING_INTEG_101MS   0x01
#define TSL256X_TIMMING_INTEG_13_7MS  0x00
#define TSL256X_TIMMING_INTEG_MASK    0x03

#define TSL256X_G16X_402MS (TSL256X_TIMMING_INTEG_402MS + TSL256X_TIMMING_GAIN)
#define TSL256X_G1X_402MS                            TSL256X_TIMMING_INTEG_402MS
#define TSL256X_G1X_101MS                            TSL256X_TIMMING_INTEG_101MS
#define TSL256X_G1X_13_7MS                          TSL256X_TIMMING_INTEG_13_7MS

#define TSL256X_INTR_SHIFT            0x04
#define TSL256X_INTR_DISABLED         0x00
#define TSL256X_INTR_LEVEL            0x01
#define TSL256X_INTR_SMB_ALERT        0x02
#define TSL256X_INTR_TEST             0x03

#define TSL256X_INT_PERSIST_EVERY     0x00
#define TSL256X_INT_PERSIST_ANY       0x01
#define TSL256X_INT_PERSIST_2_CYCLES  0x02
#define TSL256X_INT_PERSIST_3_CYCLES  0x03
#define TSL256X_INT_PERSIST_4_CYCLES  0x04
#define TSL256X_INT_PERSIST_5_CYCLES  0x05
#define TSL256X_INT_PERSIST_6_CYCLES  0x06
#define TSL256X_INT_PERSIST_7_CYCLES  0x07
#define TSL256X_INT_PERSIST_8_CYCLES  0x08
#define TSL256X_INT_PERSIST_9_CYCLES  0x09
#define TSL256X_INT_PERSIST_10_CYCLES 0x0A
#define TSL256X_INT_PERSIST_11_CYCLES 0x0B
#define TSL256X_INT_PERSIST_12_CYCLES 0x0C
#define TSL256X_INT_PERSIST_13_CYCLES 0x0D
#define TSL256X_INT_PERSIST_14_CYCLES 0x0E
#define TSL256X_INT_PERSIST_15_CYCLES 0x0F

#define TSL256X_ID_PARTNO_MASK        0xF0
#define TSL256X_ID_REV_MASK           0x0F
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL256X convertion and calibration values
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
 * \name Callback function to handle the TSL256X alarm interrupt and macro
 * @{
 */
#define TSL256X_REGISTER_INT(ptr) tsl256x_int_callback = ptr;
extern void (*tsl256x_int_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL256X return and command values
 * @{
 */
#define TSL256X_SUCCESS         0x00
#define TSL256X_LIGHT           0x01
#define TSL256X_ERROR             -1

#define TSL256X_ACTIVE          SENSORS_ACTIVE
#define TSL256X_INT_OVER        HW_INT_OVER_THRS
#define TSL256X_INT_BELOW       HW_INT_BELOW_THRS
#define TSL256X_INT_DISABLE     HW_INT_DISABLE
#define TSL256X_TIMMING_CFG     (HW_INT_DISABLE + 1)

#define TSL256X_VAL_READ        0x01
/** @} */
/* -------------------------------------------------------------------------- */
#define TSL256X_SENSOR "TSL256X Light Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor tsl256x;
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

