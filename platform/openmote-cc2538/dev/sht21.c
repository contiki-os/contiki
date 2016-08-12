/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 * \addtogroup openmote-sht21-sensor
 * @{
 *
 * \file
 * Driver for the SHT21 temperature and relative humidity sensor
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#include "dev/i2c.h"
#include "dev/sht21.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/**
 * \name SHT21 address
 */
#define SHT21_ADDRESS                   (0x40)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SHT21 register addresses and values
 * @{
 */
#define SHT21_USER_REG_READ             (0xE7)
#define SHT21_USER_REG_WRITE            (0xE6)
#define SHT21_USER_REG_RESERVED_BITS    (0x38)

#define SHT21_TEMPERATURE_HM_CMD        (0xE3)
#define SHT21_HUMIDITY_HM_CMD           (0xE5)
#define SHT21_TEMPERATURE_NHM_CMD       (0xF3)
#define SHT21_HUMIDITY_NHM_CMD          (0xF5)
#define SHT21_RESET_CMD                 (0xFE)

#define SHT21_STATUS_MASK               (0xFC)

#define SHT21_RESOLUTION_12b_14b        ((0 << 7) | (0 << 0))
#define SHT21_RESOLUTION_8b_12b         ((0 << 7) | (1 << 0))
#define SHT21_RESOLUTION_10b_13b        ((1 << 7) | (0 << 0))
#define SHT21_RESOLUTION_11b_11b        ((1 << 7) | (1 << 0))
#define SHT21_BATTERY_ABOVE_2V25        (0 << 6)
#define SHT21_BATTERY_BELOW_2V25        (1 << 6)
#define SHT21_ONCHIP_HEATER_ENABLE      (1 << 2)
#define SHT21_ONCHIP_HEATER_DISABLE     (0 << 2)
#define SHT21_OTP_RELOAD_ENABLE         (0 << 1)
#define SHT21_OTP_RELOAD_DISABLE        (1 << 1)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SHT21 configuration values
 * @{
 */
#define SHT21_DEFAULT_CONFIG            (SHT21_RESOLUTION_12b_14b | \
                                         SHT21_ONCHIP_HEATER_DISABLE | \
                                         SHT21_BATTERY_ABOVE_2V25 | \
                                         SHT21_OTP_RELOAD_DISABLE)

#define SHT21_USER_CONFIG               (SHT21_RESOLUTION_12b_14b | \
                                         SHT21_ONCHIP_HEATER_DISABLE | \
                                         SHT21_BATTERY_ABOVE_2V25 | \
                                         SHT21_OTP_RELOAD_DISABLE)
/** @} */
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
static void
sht21_init(void)
{
  uint8_t config[2];

  /* Setup the configuration vector, the first position holds address */
  /* and the second position holds the actual configuration */
  config[0] = SHT21_USER_REG_WRITE;
  config[1] = 0;

  /* Read the current configuration according to the datasheet (pag. 9, fig. 18) */
  i2c_single_send(SHT21_ADDRESS, SHT21_USER_REG_READ);
  i2c_single_receive(SHT21_ADDRESS, &config[1]);

  /* Clean all the configuration bits except those reserved */
  config[1] &= SHT21_USER_REG_RESERVED_BITS;

  /* Set the configuration bits without changing those reserved */
  config[1] |= SHT21_USER_CONFIG;

  i2c_burst_send(SHT21_ADDRESS, config, sizeof(config));
}
/*---------------------------------------------------------------------------*/
static void
sht21_reset(void)
{
  /* Send a soft-reset command according to the datasheet (pag. 9, fig. 17) */
  i2c_single_send(SHT21_ADDRESS, SHT21_RESET_CMD);
}
/*---------------------------------------------------------------------------*/
static uint8_t
sht21_is_present(void)
{
  uint8_t status;
  uint8_t is_present;

  /* Read the current configuration according to the datasheet (pag. 9, fig. 18) */
  i2c_single_send(SHT21_ADDRESS, SHT21_USER_REG_READ);
  status = i2c_single_receive(SHT21_ADDRESS, &is_present);
  if(status != I2C_MASTER_ERR_NONE) {
    return 0;
  }

  /* Clear the reserved bits according to the datasheet (pag. 9, tab. 8) */
  is_present &= ~SHT21_USER_REG_RESERVED_BITS;

  return (is_present == SHT21_USER_CONFIG) || (is_present == SHT21_DEFAULT_CONFIG);
}
/*---------------------------------------------------------------------------*/
static uint32_t
sht21_read_temperature(void)
{
  uint8_t sht21_temperature[2];
  uint16_t temperature;

  /* Read the current temperature according to the datasheet (pag. 8, fig. 15) */
  i2c_single_send(SHT21_ADDRESS, SHT21_TEMPERATURE_HM_CMD);
  i2c_burst_receive(SHT21_ADDRESS, sht21_temperature, sizeof(sht21_temperature));

  temperature = (sht21_temperature[0] << 8) | ((sht21_temperature[1] & SHT21_STATUS_MASK));

  return temperature;
}
/*---------------------------------------------------------------------------*/
static int16_t
sht21_convert_temperature(uint32_t temperature)
{
  int16_t result;

  temperature *= 17572;
  temperature = temperature >> 16;
  result = (int16_t)temperature - 4685;

  return result;
}
/*---------------------------------------------------------------------------*/
static uint32_t
sht21_read_humidity(void)
{
  uint8_t sht21_humidity[2];
  uint16_t humidity;

  /* Read the current humidity according to the datasheet (pag. 8, fig. 15) */
  i2c_single_send(SHT21_ADDRESS, SHT21_HUMIDITY_HM_CMD);
  i2c_burst_receive(SHT21_ADDRESS, sht21_humidity, sizeof(sht21_humidity));

  humidity = (sht21_humidity[0] << 8) | ((sht21_humidity[1] & SHT21_STATUS_MASK));

  return humidity;
}
/*---------------------------------------------------------------------------*/
static int16_t
sht21_convert_humidity(uint32_t humidity)
{
  int16_t result;

  humidity *= 12500;
  humidity = humidity >> 16;
  result = (int16_t)humidity - 600;
  result = (result > 10000) ? 10000 : result;

  return result;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint32_t value;

  if(!enabled) {
    PRINTF("SHT21: sensor not started\n");
    return SHT21_ERROR;
  }

  if(type == SHT21_READ_RAW_TEMP) {
    return sht21_read_temperature();
  } else if(type == SHT21_READ_RAW_RHUM) {
    return sht21_read_humidity();
  } else if(type == SHT21_READ_TEMP) {
    value = sht21_read_temperature();
    return sht21_convert_temperature(value);
  } else if(type == SHT21_READ_RHUM) {
    value = sht21_read_humidity();
    return sht21_convert_humidity(value);
  } else {
    PRINTF("SHT21: invalid value requested\n");
    return SHT21_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == SHT21_ACTIVATE) {
    if(!sht21_is_present()) {
      PRINTF("SHT21: is not present\n");
      return SHT21_ERROR;
    } else {
      sht21_init();
      enabled = 1;
      return SHT21_SUCCESS;
    }
  }

  if(type == SHT21_RESET && enabled) {
    sht21_reset();
    return SHT21_SUCCESS;
  } else {
    PRINTF("SHT21: is not enabled\n");
    return SHT21_ERROR;
  }

  return SHT21_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht21, SHT21_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
