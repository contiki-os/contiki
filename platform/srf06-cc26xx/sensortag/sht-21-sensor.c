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
 * \addtogroup sensortag-cc26xx-sht-sensor
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26xx Sensirion SHT21 Humidity sensor
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "sht-21-sensor.h"
#include "sensor-common.h"
#include "board-i2c.h"

#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Sensor I2C address */
#define HAL_SHT21_I2C_ADDRESS      0x40
/*---------------------------------------------------------------------------*/
#define S_REG_LEN                  2
#define DATA_LEN                   3
/*---------------------------------------------------------------------------*/
/* Internal commands */
#define SHT21_CMD_TEMP_T_NH        0xF3 /* command trig. temp meas. no hold master */
#define SHT21_CMD_HUMI_T_NH        0xF5 /* command trig. humidity meas. no hold master */
#define SHT21_CMD_WRITE_U_R        0xE6 /* command write user register */
#define SHT21_CMD_READ_U_R         0xE7 /* command read user register */
#define SHT21_CMD_SOFT_RST         0xFE /* command soft reset */
/*---------------------------------------------------------------------------*/
#define HUMIDITY                   0x00
#define TEMPERATURE                0x01
/*---------------------------------------------------------------------------*/
#define USR_REG_MASK               0x38  /* Mask off reserved bits (3,4,5) */
#define USR_REG_DEFAULT            0x02  /* Disable OTP reload */
#define USR_REG_RES_MASK           0x7E  /* Only change bits 0 and 7 (meas. res.) */
#define USR_REG_11BITRES           0x81  /* 11-bit resolution */
/*---------------------------------------------------------------------------*/
#define USR_REG_TEST_VAL           0x83
/*---------------------------------------------------------------------------*/
#define DATA_SIZE                  6
/*---------------------------------------------------------------------------*/
static uint8_t usr;                         /* User register value */
static uint8_t buf[DATA_SIZE];              /* Data buffer */
static bool success;
/*---------------------------------------------------------------------------*/
static int enabled = SHT_21_SENSOR_STATUS_DISABLED;
/*---------------------------------------------------------------------------*/
/*
 * Maximum measurement durations in clock ticks. We use 11bit resolution, thus:
 * - Tmp: 11ms
 * - RH:  15ms
 */
#define MEASUREMENT_DURATION 3

/*
 * Wait SENSOR_STARTUP_DELAY clock ticks between activation and triggering a
 * reading
 */
#define SENSOR_STARTUP_DELAY 4

static struct ctimer startup_timer;
/*---------------------------------------------------------------------------*/
/**
 * \brief Select the SHT sensor on the I2C-bus
 */
static void
select(void)
{
  /* Select the SHT21 address */
  board_i2c_select(BOARD_I2C_INTERFACE_0, HAL_SHT21_I2C_ADDRESS);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Write a command over I2C
 * \param cmd The command to send
 * \return TRUE if the command was sent successfully
 */
static bool
write_cmd(uint8_t cmd)
{
  /* Send command */
  return board_i2c_write_single(cmd);
}
/*---------------------------------------------------------------------------*/
/*
 * \brief Read data from the SHT over I2C
 * \param buf Pointer to buffer where data will be stored
 * \param len Number of bytes to read
 * \return TRUE if the required number of bytes were received
 */
static bool
read_data(uint8_t *buf, uint8_t len)
{
  /* Read data */
  return board_i2c_read(buf, len);
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Initialise the SHT21 sensor
 */
static void
sensor_init(void)
{
  select();

  /* Set 11 bit resolution */
  sensor_common_read_reg(SHT21_CMD_READ_U_R, &usr, 1);
  usr &= USR_REG_RES_MASK;
  usr |= USR_REG_11BITRES;
  sensor_common_write_reg(SHT21_CMD_WRITE_U_R, &usr, 1);
  success = true;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Start a temperature measurement
 */
static void
start_temp(void)
{
  if(success) {
    select();
    success = write_cmd(SHT21_CMD_TEMP_T_NH);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Latch the last temperature measurement
 */
static void
latch_temp(void)
{
  if(success) {
    select();
    success = read_data(buf, DATA_LEN);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Start a humidity measurement
 */
static void
start_humidity(void)
{
  if(success) {
    select();
    success = write_cmd(SHT21_CMD_HUMI_T_NH);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Latch the last humidity measurement
 */
static void
latch_humidity(void)
{
  if(success) {
    select();
    success = read_data(buf + DATA_LEN, DATA_LEN);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Retrieve latched raw readings from buffer and store in variables
 * \param raw_temp Pointer to a buffer where the temperature reading will be
 *        stored
 * \param raw_hum Pointer to a buffer where the humidity reading will be
 *        stored
 * \return TRUE on success
 */
static bool
get_readings(uint16_t *raw_temp, uint16_t *raw_hum)
{
  bool valid;

  valid = success;
  if(!success) {
    sensor_common_set_error_data(buf, DATA_SIZE);
  }

  /* Store temperature */
  *raw_temp = buf[0] << 8 | buf[1];

  /* [2] We ignore the CRC */

  /* Store humidity */
  *raw_hum = buf[3] << 8 | buf[4];

  /* [5] We ignore the CRC */

  success = true;

  return valid;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to temperature (degrees C) and rel. humidity (%RH)
 * \param raw_temp Raw temperature data (little endian)
 * \param raw_hum Raw humidity data (little endian)
 * \param temp Converted temperature value
 * \param hum Converted humidity value
 */
static void
convert(uint16_t raw_temp, uint16_t raw_hum, float *temp, float *hum)
{
  /* Convert temperature to degrees C */
  raw_temp &= ~0x0003; /* clear bits [1..0] (status bits) */
  *temp = -46.85 + 175.72 / 65536 * (double)(int16_t)raw_temp;

  /* Convert relative humidity to a %RH value */
  raw_hum &= ~0x0003; /* clear bits [1..0] (status bits) */

  /* RH= -6 + 125 * SRH / 2^16 */
  *hum = -6.0 + 125.0 / 65536 * (double)raw_hum;
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void *not_used)
{
  switch(enabled) {
  case SHT_21_SENSOR_STATUS_INITIALISED:
    start_temp();
    enabled = SHT_21_SENSOR_STATUS_READING_TEMP;
    break;
  case SHT_21_SENSOR_STATUS_READING_TEMP:
    latch_temp();
    start_humidity();
    enabled = SHT_21_SENSOR_STATUS_READING_HUMIDITY;
    break;
  case SHT_21_SENSOR_STATUS_READING_HUMIDITY:
    latch_humidity();
    enabled = SHT_21_SENSOR_STATUS_READINGS_READY;
    sensors_changed(&sht_21_sensor);
    return;
  case SHT_21_SENSOR_STATUS_READINGS_READY:
  case SHT_21_SENSOR_STATUS_DISABLED:
  default:
    ctimer_stop(&startup_timer);
    return;
  }

  ctimer_set(&startup_timer, MEASUREMENT_DURATION, state_machine, NULL);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type SHT_21_SENSOR_TYPE_TEMP or SHT_21_SENSOR_TYPE_HUMIDITY
 * \return Temperature (centi degrees C) or Humidity (centi %RH)
 */
static int
value(int type)
{
  int rv;
  uint16_t raw_temp;
  uint16_t raw_hum;
  float temp;
  float hum;

  if(enabled != SHT_21_SENSOR_STATUS_READINGS_READY) {
    PRINTF("Sensor disabled or starting up (%d)\n", enabled);
    return CC26XX_SENSOR_READING_ERROR;
  }

  if((type != SHT_21_SENSOR_TYPE_TEMP) && type != SHT_21_SENSOR_TYPE_HUMIDITY) {
    PRINTF("Invalid type\n");
    return CC26XX_SENSOR_READING_ERROR;
  } else {
    rv = get_readings(&raw_temp, &raw_hum);

    if(rv == 0) {
      return CC26XX_SENSOR_READING_ERROR;
    }

    convert(raw_temp, raw_hum, &temp, &hum);
    PRINTF("SHT: %04X %04X       t=%d h=%d\n", raw_temp, raw_hum,
           (int)(temp * 100), (int)(hum * 100));

    if(type == SHT_21_SENSOR_TYPE_TEMP) {
      rv = (int)(temp * 100);
    } else if(type == SHT_21_SENSOR_TYPE_HUMIDITY) {
      rv = (int)(hum * 100);
    }
  }
  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the SHT21 sensor.
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
    sensor_init();
    enabled = SHT_21_SENSOR_STATUS_INITIALISED;
    break;
  case SENSORS_ACTIVE:
    /* Must be initialised first */
    if(enabled == SHT_21_SENSOR_STATUS_DISABLED) {
      return SHT_21_SENSOR_STATUS_DISABLED;
    }
    if(enable) {
      enabled = SHT_21_SENSOR_STATUS_INITIALISED;
      ctimer_set(&startup_timer, SENSOR_STARTUP_DELAY, state_machine, NULL);
    } else {
      ctimer_stop(&startup_timer);
      enabled = SHT_21_SENSOR_STATUS_INITIALISED;
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
 * \return One of the SENSOR_STATUS_xyz defines
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
  return SHT_21_SENSOR_STATUS_DISABLED;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht_21_sensor, "SHT21", value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
