/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zoul-sht25-sensor
 * @{
 *
 * \file
 *         SHT25 temperature and humidity sensor driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/sht25.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t user_reg;
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
static uint16_t
sht25_read_reg(uint8_t reg, uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    return SHT25_ERROR;
  }

  i2c_master_enable();
  if(i2c_single_send(SHT25_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(SHT25_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
      return SHT25_SUCCESS;
    }
  }
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int16_t
sht25_convert(uint8_t variable, uint16_t value)
{
  int16_t rd;
  uint32_t buff;

  /* Clear the status bits */
  buff = (uint32_t)(value & ~SHT25_STATUS_BITS_MASK);

  if(variable == SHT25_VAL_TEMP) {
    buff *= 17572;
    buff = buff >> 16;
    rd = (int16_t)buff - 4685;
  } else {
    buff *= 12500;
    buff = buff >> 16;
    rd = (int16_t)buff - 600;
    rd = (rd > 10000) ? 10000 : rd;
  }
  return rd;
}
/*---------------------------------------------------------------------------*/
static int
sht25_read(uint8_t variable, uint16_t *rd)
{
  uint8_t buf[2];
  uint16_t raw;

  if((variable != SHT25_VAL_TEMP) && (variable != SHT25_VAL_HUM)) {
    PRINTF("SHT25: invalid sensor requested\n");
    return SHT25_ERROR;
  }

  if(sht25_read_reg(variable, buf, 2) == SHT25_SUCCESS) {
    raw = (buf[0] << 8) + buf[1];
    *rd = sht25_convert(variable, raw);
    return SHT25_SUCCESS;
  }

  PRINTF("SHT25: failed to read sensor\n");
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
sht25_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("SHT25: invalid write values\n");
    return SHT25_ERROR;
  }

  i2c_master_enable();
  if(i2c_burst_send(SHT25_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return SHT25_SUCCESS;
  }
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
sht25_read_user_register(void)
{
  if(sht25_read_reg(SHT2X_UREG_READ, &user_reg, 1) == SHT25_SUCCESS) {
    PRINTF("SHT25: user register 0x%02X\n", user_reg);
    return SHT25_SUCCESS;
  }
  PRINTF("SHT25: failed to read user register\n");
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint16_t val;

  if(!enabled) {
    PRINTF("SHT25: sensor not started\n");
    return SHT25_ERROR;
  }

  if((type != SHT25_VAL_TEMP) && (type != SHT25_VAL_HUM) &&
     (type != SHT25_VOLTAGE_ALARM)) {
    PRINTF("SHT25: invalid value requested\n");
    return SHT25_ERROR;
  }

  if(type == SHT25_VOLTAGE_ALARM) {
    if(sht25_read_user_register() == SHT25_SUCCESS) {
      return (user_reg & SHT2x_LOW_VOLTAGE_MASK) >> SHT2x_LOW_VOLTAGE_SHIFT;
    }
  } else {
    if(sht25_read(type, &val) == SHT25_SUCCESS) {
      return val;
    }
  }
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  uint8_t buf[2];

  if((type != SHT25_ACTIVE) && (type != SHT25_SOFT_RESET) &&
     (type != SHT25_RESOLUTION)) {
    PRINTF("SHT25: option not supported\n");
    return SHT25_ERROR;
  }

  switch(type) {
  case SHT25_ACTIVE:
    if(value) {
      i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
               I2C_SCL_NORMAL_BUS_SPEED);

      /* Read the user config register */
      if(sht25_read_user_register() == SHT25_SUCCESS) {
        enabled = value;
        return SHT25_SUCCESS;
      }
    }

  case SHT25_SOFT_RESET:
    buf[0] = SHT2X_SOFT_RESET;
    if(sht25_write_reg(&buf[0], 1) != SHT25_SUCCESS) {
      PRINTF("SHT25: failed to reset the sensor\n");
      return SHT25_ERROR;
    }
    clock_delay_usec(SHT25_RESET_DELAY);
    return SHT25_SUCCESS;

  case SHT25_RESOLUTION:
    if((value != SHT2X_RES_14T_12RH) && (value != SHT2X_RES_12T_08RH) &&
       (value != SHT2X_RES_13T_10RH) && (value != SHT2X_RES_11T_11RH)) {
      PRINTF("SHT25: invalid resolution value\n");
      return SHT25_ERROR;
    }

    user_reg &= ~SHT2X_RES_11T_11RH;
    user_reg |= value;
    buf[0] = SHT2X_UREG_WRITE;
    buf[1] = user_reg;

    if(sht25_write_reg(buf, 2) == SHT25_SUCCESS) {
      PRINTF("SHT25: new user register value 0x%02X\n", user_reg);
      return SHT25_SUCCESS;
    }

  default:
    return SHT25_ERROR;
  }

  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht25, SHT25_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
