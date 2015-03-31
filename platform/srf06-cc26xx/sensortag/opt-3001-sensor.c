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
/* Register values */
#define MANUFACTURER_ID                 0x5449  /* TI */
#define DEVICE_ID                       0x3001  /* Opt 3001 */
#define CONFIG_RESET                    0xC810
#define CONFIG_TEST                     0xCC10
#define CONFIG_ENABLE                   0x10CC /* 0xCC10 */
#define CONFIG_DISABLE                  0x108C /* 0xC810 */
/*---------------------------------------------------------------------------*/
/* Bit values */
#define DATA_RDY_BIT                    0x0080  /* Data ready */
/*---------------------------------------------------------------------------*/
/* Register length */
#define REGISTER_LENGTH                 2
/*---------------------------------------------------------------------------*/
/* Sensor data size */
#define DATA_LENGTH                     2
/*---------------------------------------------------------------------------*/
#define SENSOR_STATUS_DISABLED     0
#define SENSOR_STATUS_NOT_READY    1
#define SENSOR_STATUS_ENABLED      2

static int enabled = SENSOR_STATUS_DISABLED;
/*---------------------------------------------------------------------------*/
/* Wait SENSOR_STARTUP_DELAY for the sensor to be ready - 125ms */
#define SENSOR_STARTUP_DELAY (CLOCK_SECOND >> 3)

static struct ctimer startup_timer;
/*---------------------------------------------------------------------------*/
static void
notify_ready(void *not_used)
{
  enabled = SENSOR_STATUS_ENABLED;
  sensors_changed(&opt_3001_sensor);
}
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
/**
 * \brief Turn the sensor on/off
 * \param enable TRUE: on, FALSE: off
 */
static void
enable_sensor(bool enable)
{
  uint16_t val;

  select();

  if(enable) {
    val = CONFIG_ENABLE;
  } else {
    val = CONFIG_DISABLE;
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

  select();

  success = sensor_common_read_reg(REG_CONFIGURATION, (uint8_t *)&val,
                                   REGISTER_LENGTH);

  if(success) {
    success = (val & DATA_RDY_BIT) == DATA_RDY_BIT;
  }

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
 * \param data Pointer to a buffer with a raw sensor reading
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

  if(enabled != SENSOR_STATUS_ENABLED) {
    PRINTF("Sensor disabled or starting up (%d)\n", enabled);
    return CC26XX_SENSOR_READING_ERROR;
  }

  rv = read_data(&raw_val);

  if(rv == 0) {
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
  switch(type) {
  case SENSORS_HW_INIT:
    break;
  case SENSORS_ACTIVE:
    if(enable) {
      enable_sensor(1);
      ctimer_set(&startup_timer, SENSOR_STARTUP_DELAY, notify_ready, NULL);
      enabled = SENSOR_STATUS_NOT_READY;
    } else {
      ctimer_stop(&startup_timer);
      enable_sensor(0);
      enabled = SENSOR_STATUS_DISABLED;
    }
    break;
  default:
    break;
  }
  return enabled;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the status of the sensor
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the sensor is enabled
 */
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
    break;
  default:
    break;
  }
  return SENSOR_STATUS_DISABLED;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(opt_3001_sensor, "OPT3001", value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
