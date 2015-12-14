/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-opt-sensor
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26xx Opt3001 light sensor
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "lib/sensors.h"
#include "opt-3001-sensor.h"
#include "sys/ctimer.h"
#include "ti-lib.h"
#include "board-i2c.h"
#include "sensor-common.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Slave address */
#define OPT3001_I2C_ADDRESS             0x45
/*---------------------------------------------------------------------------*/
/* Register addresses */
#define REG_RESULT                      0x00
#define REG_CONFIGURATION               0x01
#define REG_LOW_LIMIT                   0x02
#define REG_HIGH_LIMIT                  0x03

#define REG_MANUFACTURER_ID             0x7E
#define REG_DEVICE_ID                   0x7F
/*---------------------------------------------------------------------------*/
/*
 * Configuration Register Bits and Masks.
 * We use uint16_t to read from / write to registers, meaning that the
 * register's MSB is the variable's LSB.
 */
#define CONFIG_RN      0x00F0 /* [15..12] Range Number */
#define CONFIG_CT      0x0008 /* [11] Conversion Time */
#define CONFIG_M       0x0006 /* [10..9] Mode of Conversion */
#define CONFIG_OVF     0x0001 /* [8] Overflow */
#define CONFIG_CRF     0x8000 /* [7] Conversion Ready Field */
#define CONFIG_FH      0x4000 /* [6] Flag High */
#define CONFIG_FL      0x2000 /* [5] Flag Low */
#define CONFIG_L       0x1000 /* [4] Latch */
#define CONFIG_POL     0x0800 /* [3] Polarity */
#define CONFIG_ME      0x0400 /* [2] Mask Exponent */
#define CONFIG_FC      0x0300 /* [1..0] Fault Count */

/* Possible Values for CT */
#define CONFIG_CT_100      0x0000
#define CONFIG_CT_800      CONFIG_CT

/* Possible Values for M */
#define CONFIG_M_CONTI     0x0004
#define CONFIG_M_SINGLE    0x0002
#define CONFIG_M_SHUTDOWN  0x0000

/* Reset Value for the register 0xC810. All zeros except: */
#define CONFIG_RN_RESET    0x00C0
#define CONFIG_CT_RESET    CONFIG_CT_800
#define CONFIG_L_RESET     0x1000
#define CONFIG_DEFAULTS    (CONFIG_RN_RESET | CONFIG_CT_100 | CONFIG_L_RESET)

/* Enable / Disable */
#define CONFIG_ENABLE_CONTINUOUS  (CONFIG_M_CONTI | CONFIG_DEFAULTS)
#define CONFIG_ENABLE_SINGLE_SHOT (CONFIG_M_SINGLE | CONFIG_DEFAULTS)
#define CONFIG_DISABLE             CONFIG_DEFAULTS
/*---------------------------------------------------------------------------*/
/* Register length */
#define REGISTER_LENGTH                 2
/*---------------------------------------------------------------------------*/
/* Sensor data size */
#define DATA_LENGTH                     2
/*---------------------------------------------------------------------------*/
/*
 * SENSOR_STATE_SLEEPING and SENSOR_STATE_ACTIVE are mutually exclusive.
 * SENSOR_STATE_DATA_READY can be ORd with both of the above. For example the
 * sensor may be sleeping but with a conversion ready to read out.
 */
#define SENSOR_STATE_SLEEPING     0
#define SENSOR_STATE_ACTIVE       1
#define SENSOR_STATE_DATA_READY   2

static int state = SENSOR_STATE_SLEEPING;
/*---------------------------------------------------------------------------*/
/* Wait SENSOR_STARTUP_DELAY for the sensor to be ready - 125ms */
#define SENSOR_STARTUP_DELAY (CLOCK_SECOND >> 3)

static struct ctimer startup_timer;
/*---------------------------------------------------------------------------*/
/**
 * \brief Select the sensor on the I2C bus
 */
static void
select(void)
{
  /* Select slave and set clock rate */
  board_i2c_select(BOARD_I2C_INTERFACE_0, OPT3001_I2C_ADDRESS);
}
/*---------------------------------------------------------------------------*/
static void
notify_ready(void *not_used)
{
  /*
   * Depending on the CONFIGURATION.CONVERSION_TIME bits, a conversion will
   * take either 100 or 800 ms. Here we inspect the CONVERSION_READY bit and
   * if the reading is ready we notify, otherwise we just reschedule ourselves
   */
  uint16_t val;

  select();

  sensor_common_read_reg(REG_CONFIGURATION, (uint8_t *)&val, REGISTER_LENGTH);

  if(val & CONFIG_CRF) {
    sensors_changed(&opt_3001_sensor);
    state = SENSOR_STATE_DATA_READY;
  } else {
    ctimer_set(&startup_timer, SENSOR_STARTUP_DELAY, notify_ready, NULL);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Turn the sensor on/off
 * \param enable TRUE: on, FALSE: off
 */
static void
enable_sensor(bool enable)
{
  uint16_t val;
  uint16_t had_data_ready = state & SENSOR_STATE_DATA_READY;

  select();

  if(enable) {
    val = CONFIG_ENABLE_SINGLE_SHOT;

    /* Writing CONFIG_ENABLE_SINGLE_SHOT to M bits will clear CRF bits */
    state = SENSOR_STATE_ACTIVE;
  } else {
    val = CONFIG_DISABLE;

    /* Writing CONFIG_DISABLE to M bits will not clear CRF bits */
    state = SENSOR_STATE_SLEEPING | had_data_ready;
  }

  sensor_common_write_reg(REG_CONFIGURATION, (uint8_t *)&val, REGISTER_LENGTH);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the result register
 * \param raw_data Pointer to a buffer to store the reading
 * \return TRUE if valid data
 */
static bool
read_data(uint16_t *raw_data)
{
  bool success;
  uint16_t val;

  if((state & SENSOR_STATE_DATA_READY) != SENSOR_STATE_DATA_READY) {
    return false;
  }

  select();

  success = sensor_common_read_reg(REG_CONFIGURATION, (uint8_t *)&val,
                                   REGISTER_LENGTH);

  if(success) {
    success = sensor_common_read_reg(REG_RESULT, (uint8_t *)&val, DATA_LENGTH);
  }

  if(success) {
    /* Swap bytes */
    *raw_data = (val << 8) | (val >> 8 & 0xFF);
  } else {
    sensor_common_set_error_data((uint8_t *)raw_data, DATA_LENGTH);
  }

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to a value in lux
 * \param raw_data data Pointer to a buffer with a raw sensor reading
 * \return Converted value (lux)
 */
static float
convert(uint16_t raw_data)
{
  uint16_t e, m;

  m = raw_data & 0x0FFF;
  e = (raw_data & 0xF000) >> 12;

  return m * (0.01 * exp2(e));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type Ignored
 * \return Illuminance in centilux
 */
static int
value(int type)
{
  int rv;
  uint16_t raw_val;
  float converted_val;

  rv = read_data(&raw_val);

  if(rv == false) {
    return CC26XX_SENSOR_READING_ERROR;
  }

  converted_val = convert(raw_val);
  PRINTF("OPT: %04X            r=%d (centilux)\n", raw_val,
         (int)(converted_val * 100));

  rv = (int)(converted_val * 100);

  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the OPT3001 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
static int
configure(int type, int enable)
{
  int rv = 0;

  switch(type) {
  case SENSORS_HW_INIT:
    /*
     * Device reset won't reset the sensor, so we put it to sleep here
     * explicitly
     */
    enable_sensor(0);
    rv = 0;
    break;
  case SENSORS_ACTIVE:
    if(enable) {
      enable_sensor(1);
      ctimer_set(&startup_timer, SENSOR_STARTUP_DELAY, notify_ready, NULL);
      rv = 1;
    } else {
      ctimer_stop(&startup_timer);
      enable_sensor(0);
      rv = 0;
    }
    break;
  default:
    break;
  }
  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the status of the sensor
 * \param type ignored
 * \return The state of the sensor SENSOR_STATE_xyz
 */
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
  default:
    break;
  }
  return state;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(opt_3001_sensor, "OPT3001", value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
