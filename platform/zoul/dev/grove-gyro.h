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
 * \defgroup zoul-grove-gyro-sensor Grove 3-axis gyroscope based on ITG-3200
 * @{
 *
 * \file
 *         Grove 3-axis gyroscope header file
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/* -------------------------------------------------------------------------- */
#ifndef GROVE_GYRO_H_
#define GROVE_GYRO_H_
/* -------------------------------------------------------------------------- */
/**
 * \name Callback function to handle the interrupt
 * @{
 */
#define GROVE_GYRO_REGISTER_INT(ptr) grove_gyro_int_callback = ptr;
extern void (*grove_gyro_int_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Gyroscope data values structure
 * @{
 */
typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t x_offset;
  uint16_t y_offset;
  uint16_t z_offset;
  int16_t temp;
} grove_gyro_values_t;

extern grove_gyro_values_t gyro_values;
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Grove 3-axis gyroscope address and registers
 * @{
 */
#define GROVE_GYRO_ADDR              0x68

#define GROVE_GYRO_WHO_AM_I          0x00
#define GROVE_GYRO_SMPLRT_DIV        0x15
#define GROVE_GYRO_DLPF_FS           0x16
#define GROVE_GYRO_INT_CFG           0x17
#define GROVE_GYRO_INT_STATUS        0x1A
#define GROVE_GYRO_TEMP_OUT_H        0x1B
#define GROVE_GYRO_TEMP_OUT_L        0x1C
#define GROVE_GYRO_XOUT_H            0x1D
#define GROVE_GYRO_XOUT_L            0x1E
#define GROVE_GYRO_YOUT_H            0x1F
#define GROVE_GYRO_YOUT_L            0x20
#define GROVE_GYRO_ZOUT_H            0x21
#define GROVE_GYRO_ZOUT_L            0x22
#define GROVE_GYRO_PWR_MGMT          0x3E
/** @} */
/*--------------------------------------------------------------------------*/
/**
 * \name Grove 3-axis gyroscope bitmasks and config
 * @{
 */
#define GROVE_GYRO_DLPF_FS_SEL                0x18
#define GROVE_GYRO_DLPF_FS_CGF_8KHZ_LP256HZ   0x00
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP188HZ   0x01
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP98HZ    0x02
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP42HZ    0x03
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP20HZ    0x04
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP10HZ    0x05
#define GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP5HZ     0x06

#define GROVE_GYRO_INT_CFG_RAW_READY_EN       0x01
#define GROVE_GYRO_INT_CFG_READY_EN           0x04
#define GROVE_GYRO_INT_CFG_LATCH_CLR_ANY      0x10
#define GROVE_GYRO_INT_CFG_LATCH_EN           0x20
#define GROVE_GYRO_INT_CFG_PIN_OPEN           0x40
#define GROVE_GYRO_INT_CFG_PIN_ACTL           0x80
#define GROVE_GYRO_INT_CFG_DISABLE            0x00

#define GROVE_GYRO_INT_STATUS_DATA_RDY_MASK   0x01
#define GROVE_GYRO_INT_STATUS_PLL_RDY_MASK    0x04

#define GROVE_GYRO_PWR_MGMT_CLK_SEL_INTOSC    0x00
#define GROVE_GYRO_PWR_MGMT_CLK_SEL_PLL_X     0x01
#define GROVE_GYRO_PWR_MGMT_CLK_SEL_PLL_Y     0x02
#define GROVE_GYRO_PWR_MGMT_CLK_SEL_PLL_Z     0x03
#define GROVE_GYRO_PWR_MGMT_CLK_SEL_EXT_32K   0x04
#define GROVE_GYRO_PWR_MGMT_CLK_SEL_EXT_19K   0x05
#define GROVE_GYRO_PWR_MGMT_STBY_ZG           0x08
#define GROVE_GYRO_PWR_MGMT_STBY_YG           0x10
#define GROVE_GYRO_PWR_MGMT_STBY_XG           0x20
#define GROVE_GYRO_PWR_MGMT_SLEEP             0x40
#define GROVE_GYRO_PWR_MGMT_RESET             0x80

#ifdef GROVE_GYRO_CONF_OSC
#define GROVE_GYRO_DEFAULT_OSC GROVE_GYRO_CONF_OSC
#else
#define GROVE_GYRO_DEFAULT_OSC GROVE_GYRO_PWR_MGMT_CLK_SEL_INTOSC
#endif

#define GROVE_GYRO_PWR_MGMT_CLK_SEL_MASK      0x07
#define GROVE_GYRO_MAX_DATA                   0x06
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Grove 3-axis gyroscope operation values
 * @{
 */
/* Configure request type */
#define GROVE_GYRO_ACTIVE               SENSORS_ACTIVE
#define GROVE_GYRO_DATA_INTERRUPT       0x01
#define GROVE_GYRO_SAMPLE_RATE          0x02
#define GROVE_GYRO_SAMPLE_RATE_DIVIDER  0x03
#define GROVE_GYRO_POWER_ON             0x04
#define GROVE_GYRO_POWER_OFF            0x05
#define GROVE_GYRO_CALIBRATE_ZERO       0x06

/* Sensor value request type, match to the stand-by mask to check if enabled */
#define GROVE_GYRO_X                    GROVE_GYRO_PWR_MGMT_STBY_XG
#define GROVE_GYRO_Y                    GROVE_GYRO_PWR_MGMT_STBY_YG
#define GROVE_GYRO_Z                    GROVE_GYRO_PWR_MGMT_STBY_ZG
#define GROVE_GYRO_SENSOR               GROVE_GYRO_PWR_MGMT_SLEEP
#define GROVE_GYRO_XYZ                  (GROVE_GYRO_X + GROVE_GYRO_Y + \
                                        GROVE_GYRO_Z)
#define GROVE_GYRO_ALL                  (GROVE_GYRO_XYZ + GROVE_GYRO_SENSOR)
#define GROVE_GYRO_TEMP                 0x06

/* Return types */
#define GROVE_GYRO_ERROR                (-1)
#define GROVE_GYRO_SUCCESS              0x00

/* Calibration constants */
#define GROVE_GYRO_CALIB_SAMPLES        200
#define GROVE_GYRO_CALIB_TIME_US        5000
/** @} */
/* -------------------------------------------------------------------------- */
#define GROVE_GYRO_STRING "Grove 3-axis gyroscope Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor grove_gyro;
/* -------------------------------------------------------------------------- */
#endif /* ifndef GROVE_GYRO_H_ */
/**
 * @}
 * @}
 */
