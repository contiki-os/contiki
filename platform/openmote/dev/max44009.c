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

/**
 * \addtogroup platform
 * @{
 *
 * \defgroup openmote The OpenMote Platform
 *
 * \file
 * Driver for the MAX44009 light sensor in OpenMote-CC2538.
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */

/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/i2c.h"
#include "dev/max44009.h"
/*---------------------------------------------------------------------------*/
/* ADDRESS AND NOT_FOUND VALUE */
#define MAX44009_ADDRESS                    (0x4A)
#define MAX44009_NOT_FOUND                  (0x00)

/* REGISTER ADDRESSES */
#define MAX44009_INT_STATUS_ADDR            (0x00)      /* R */
#define MAX44009_INT_ENABLE_ADDR            (0x01)      /* R/W */
#define MAX44009_CONFIG_ADDR                (0x02)      /* R/W */
#define MAX44009_LUX_HIGH_ADDR              (0x03)      /* R */
#define MAX44009_LUX_LOW_ADDR               (0x04)      /* R */
#define MAX44009_THR_HIGH_ADDR              (0x05)      /* R/W */
#define MAX44009_THR_LOW_ADDR               (0x06)      /* R/W */
#define MAX44009_THR_TIMER_ADDR             (0x07)      /* R/W */

/* INTERRUPT VALUES */
#define MAX44009_INT_STATUS_OFF             (0x00)
#define MAX44009_INT_STATUS_ON              (0x01)
#define MAX44009_INT_DISABLED               (0x00)
#define MAX44009_INT_ENABLED                (0x01)

/* CONFIGURATION VALUES */
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

/* DEFAULT CONFIGURATION */
#define MAX44009_DEFAULT_CONFIGURATION      (MAX44009_CONFIG_DEFAULT | \
                                             MAX44009_CONFIG_AUTO | \
                                             MAX44009_CONFIG_CDR_NORMAL | \
                                             MAX44009_CONFIG_INTEGRATION_100ms)
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(light_sensor, "Light Sensor", max44009_value, max44009_config, max44009_status);
/**
 *
 */
int max44009_value(int type)
{
  switch(type) {
    case MAX44009_LIGHT_VAL :
      return (int)max44009_read_light();
  }
  return 0;
}
/**
 *
 */
int max44009_config(int type, int value)
{
  switch(type) {
    case SENSORS_HW_INIT :
      max44009_init();
      return 0;
    case SENSORS_ACTIVE :
      return 0;
    case SENSORS_CONFIG :
      //max44009_set_config(value[0], value[1]);
      break;
  }
  return 0;
}
/**
 *
 */
int max44009_status(int type){
  return (int)max44009_is_present();
}
/**
 *
 */
void max44009_set_config(uint8_t reg, uint8_t config){
  uint8_t data[2];
  data[0] = reg;
  data[1] = config;
  i2c_burst_send(MAX44009_ADDRESS, data, sizeof(data));
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
max44009_init(void)
{
  uint8_t max44009_address[5] = { MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                  MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                  MAX44009_THR_TIMER_ADDR };
  uint8_t max44009_value[5];
  uint8_t max44009_data[2];
  uint8_t i;

  max44009_value[0] = (MAX44009_INT_STATUS_ON);
  max44009_value[1] = (MAX44009_DEFAULT_CONFIGURATION);
  max44009_value[2] = (0xFF);
  max44009_value[3] = (0x00);
  max44009_value[4] = (0xFF);

  for(i = 0; i < sizeof(max44009_address); i++) {
    max44009_data[0] = max44009_address[i];
    max44009_data[1] = max44009_value[i];
    i2c_burst_send(MAX44009_ADDRESS, max44009_data, 2);
  }
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
max44009_reset(void)
{
  uint8_t max44009_address[5] = { MAX44009_INT_ENABLE_ADDR, MAX44009_CONFIG_ADDR, \
                                  MAX44009_THR_HIGH_ADDR, MAX44009_THR_LOW_ADDR, \
                                  MAX44009_THR_TIMER_ADDR };
  uint8_t max44009_value[5] = { 0x00, 0x03, 0xFF, 0x00, 0xFF };
  uint8_t max44009_data[2];
  uint8_t i;

  for(i = 0; i < sizeof(max44009_address); i++) {
    max44009_data[0] = max44009_address[i];
    max44009_data[1] = max44009_value[i];
    i2c_burst_send(MAX44009_ADDRESS, max44009_data, 2);
  }
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint8_t
max44009_is_present(void)
{
  uint8_t is_present;

  i2c_single_send(MAX44009_ADDRESS, MAX44009_CONFIG_ADDR);
  i2c_single_receive(MAX44009_ADDRESS, &is_present);

  return is_present != MAX44009_NOT_FOUND;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
max44009_read_light(void)
{
  uint8_t exponent, mantissa;
  uint8_t max44009_data[2];
  uint16_t result;
  
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
/**
 *
 */
float
max44009_convert_light(uint16_t lux)
{
  uint8_t exponent, mantissa;
  float result = 0.045;

  exponent = (lux >> 8) & 0x0F;
  if(exponent == 15){
    return -1;
  }
  
  mantissa = lux & 0xFF;

  result *= (1 << exponent) * mantissa;

  return result;
}
/*---------------------------------------------------------------------------*/
/** @} */
