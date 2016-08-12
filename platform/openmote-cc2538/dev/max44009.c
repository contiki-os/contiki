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
 * \addtogroup openmote-max44009-sensor
 * @{
 *
 * \file
 * Driver for the MAX44009 light sensor
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#include "dev/i2c.h"
#include "dev/max44009.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/**
 * \name MAX44009 address and device identifier
 * @{
 */
#define MAX44009_ADDRESS                    (0x4A)
#define MAX44009_NOT_FOUND                  (0x00)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name MAX44009 register addresses
 * @{
 */
#define MAX44009_INT_STATUS_ADDR            (0x00)      /* R */
#define MAX44009_INT_ENABLE_ADDR            (0x01)      /* R/W */
#define MAX44009_CONFIG_ADDR                (0x02)      /* R/W */
#define MAX44009_LUX_HIGH_ADDR              (0x03)      /* R */
#define MAX44009_LUX_LOW_ADDR               (0x04)      /* R */
#define MAX44009_THR_HIGH_ADDR              (0x05)      /* R/W */
#define MAX44009_THR_LOW_ADDR               (0x06)      /* R/W */
#define MAX44009_THR_TIMER_ADDR             (0x07)      /* R/W */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name MAX44009 register values
 * @{
 */
#define MAX44009_INT_STATUS_OFF             (0x00)
#define MAX44009_INT_STATUS_ON              (0x01)
#define MAX44009_INT_DISABLED               (0x00)
#define MAX44009_INT_ENABLED                (0x01)

#define MAX44009_CONFIG_DEFAULT             (0 << 7)
#define MAX44009_CONFIG_CONTINUOUS          (1 << 7)
#define MAX44009_CONFIG_AUTO                (0 << 6)
#define MAX44009_CONFIG_MANUAL              (1 << 6)
#define MAX44009_CONFIG_CDR_NORMAL          (0 << 5)
#define MAX44009_CONFIG_CDR_DIVIDED         (1 << 5)
#define MAX44009_CONFIG_INTEGRATION_800ms   (0 << 0)
#define MAX44009_CONFIG_INTEGRATION_400ms   (1 << 0)
#define MAX44009_CONFIG_INTEGRATION_200ms   (2 << 0)
#define MAX44009_CONFIG_INTEGRATION_100ms   (3 << 0)
#define MAX44009_CONFIG_INTEGRATION_50ms    (4 << 0)
#define MAX44009_CONFIG_INTEGRATION_25ms    (5 << 0)
#define MAX44009_CONFIG_INTEGRATION_12ms    (6 << 0)
#define MAX44009_CONFIG_INTEGRATION_6ms     (7 << 0)

#define MAX44009_DEFAULT_CONFIGURATION      (MAX44009_CONFIG_DEFAULT | \
                                             MAX44009_CONFIG_AUTO | \
                                             MAX44009_CONFIG_CDR_NORMAL | \
                                             MAX44009_CONFIG_INTEGRATION_100ms)

#define MAX44009_USER_CONFIGURATION         (MAX44009_CONFIG_DEFAULT | \
                                             MAX44009_CONFIG_AUTO | \
                                             MAX44009_CONFIG_CDR_NORMAL | \
                                             MAX44009_CONFIG_INTEGRATION_800ms)

/** @} */
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
static void
max44009_init(void)
{
  uint8_t max44009_address[5] = { MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                  MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                  MAX44009_THR_TIMER_ADDR };
  uint8_t max44009_value[5];
  uint8_t max44009_data[2];
  uint8_t i;

  max44009_value[0] = (MAX44009_INT_STATUS_OFF);
  max44009_value[1] = (MAX44009_USER_CONFIGURATION);
  max44009_value[2] = (0xFF);
  max44009_value[3] = (0x00);
  max44009_value[4] = (0xFF);

  for(i = 0; i < sizeof(max44009_address) / sizeof(max44009_address[0]); i++) {
    max44009_data[0] = max44009_address[i];
    max44009_data[1] = max44009_value[i];
    i2c_burst_send(MAX44009_ADDRESS, max44009_data, 2);
  }
}
/*---------------------------------------------------------------------------*/
static void
max44009_reset(void)
{
  uint8_t max44009_address[5] = { MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                  MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                  MAX44009_THR_TIMER_ADDR };
  uint8_t max44009_value[5] = { 0x00, 0x03, 0xFF, 0x00, 0xFF };
  uint8_t max44009_data[2];
  uint8_t i;

  for(i = 0; i < sizeof(max44009_address) / sizeof(max44009_address[0]); i++) {
    max44009_data[0] = max44009_address[i];
    max44009_data[1] = max44009_value[i];
    i2c_burst_send(MAX44009_ADDRESS, max44009_data, 2);
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
max44009_is_present(void)
{
  uint8_t status;
  uint8_t is_present;

  i2c_single_send(MAX44009_ADDRESS, MAX44009_CONFIG_ADDR);
  status = i2c_single_receive(MAX44009_ADDRESS, &is_present);
  if(status != I2C_MASTER_ERR_NONE) {
    return 0;
  }

  return is_present != MAX44009_NOT_FOUND;
}
/*---------------------------------------------------------------------------*/
static uint16_t
max44009_read_light(void)
{
  uint8_t exponent, mantissa;
  uint8_t max44009_data[2];
  uint32_t result;

  i2c_single_send(MAX44009_ADDRESS, MAX44009_LUX_HIGH_ADDR);
  i2c_single_receive(MAX44009_ADDRESS, &max44009_data[0]);
  i2c_single_send(MAX44009_ADDRESS, MAX44009_LUX_LOW_ADDR);
  i2c_single_receive(MAX44009_ADDRESS, &max44009_data[1]);

  exponent = ((max44009_data[0] >> 4) & 0x0E);
  mantissa = ((max44009_data[0] & 0x0F) << 4) | (max44009_data[1] & 0x0F);

  result = ((uint16_t)exponent << 8) | ((uint16_t)mantissa << 0);

  return result;
}
/*---------------------------------------------------------------------------*/
static uint16_t
max44009_convert_light(uint16_t lux)
{
  uint8_t exponent, mantissa;
  uint32_t result;

  exponent = (lux >> 8) & 0xFF;
  exponent = (exponent == 0x0F ? exponent & 0x0E : exponent);
  mantissa = (lux >> 0) & 0xFF;

  result = 45 * (2 ^ exponent * mantissa) / 10;

  return (uint16_t)result;
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
  uint16_t value;

  if(!enabled) {
    PRINTF("MAX44009: sensor not started\n");
    return MAX44009_ERROR;
  }

  if(type == MAX44009_READ_RAW_LIGHT) {
    return max44009_read_light();
  } else if(type == MAX44009_READ_LIGHT) {
    value = max44009_read_light();
    return max44009_convert_light(value);
  } else {
    PRINTF("MAX44009: invalid value requested\n");
    return MAX44009_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == MAX44009_ACTIVATE) {
    if(!max44009_is_present()) {
      return MAX44009_ERROR;
    } else {
      max44009_init();
      enabled = 1;
      return MAX44009_SUCCESS;
    }
  }

  if((type == MAX44009_RESET) && enabled) {
    max44009_reset();
    return MAX44009_SUCCESS;
  } else {
    PRINTF("MAX44009: is not enabled\n");
    return MAX44009_ERROR;
  }

  return MAX44009_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(max44009, MAX44009_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
