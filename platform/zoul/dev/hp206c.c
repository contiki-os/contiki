/*
 * Copyright (c) 2017, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zoul-sensor
 * @{
 *
 * \file
 *         HP206C Barometer and altimeter sensor driver
 *
 * \author
 *         Javi Sanchez <jsanchez@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/hp206c.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
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
hp206c_read_reg(uint8_t reg, uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    return HP206C_ERROR;
  }

  i2c_master_enable();
  if(i2c_single_send(HP206C_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(HP206C_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
      return HP206C_SUCCESS;
    }
  }
  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
static int32_t
hpc206c_convert(int32_t val)
{
  if((val & 0xF00000) == 0xF00000) {
    val = (val & 0x0FFFFF) ^ 0xFFFFF;
    return -val;
  } else {
    return val;
  }
  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
hp206c_read(uint8_t variable, int32_t *rd)
{
  uint8_t buf[3];
  if((variable != HP206C_READ_P) && (variable != HP206C_READ_A)
     && (variable != HP206C_READ_T) && (variable != HP206C_READ_PT)
     && (variable != HP206C_READ_AT) && (variable != HP206C_READ_CAL)) {
    PRINTF("HP206C: invalid sensor requested\n");
    return HP206C_ERROR;
  }
  if(hp206c_read_reg(variable, buf, 3) == HP206C_SUCCESS) {
    *rd = ((buf[0] << 16) | (buf[1] << 8) | buf[2]);
    *rd = hpc206c_convert(*rd);
    return HP206C_SUCCESS;
  }
  PRINTF("HP206C: failed to read sensor\n");
  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
hp206c_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("HP206C: invalid write values\n");
    return HP206C_ERROR;
  }
  i2c_master_enable();
  if(i2c_burst_send(HP206C_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return HP206C_SUCCESS;
  }
  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  int32_t val;

  if(!enabled) {
    PRINTF("HP206C: sensor not started\n");
    return HP206C_ERROR;
  }

  if((type != HP206C_READ_P) && (type != HP206C_READ_A)
     && (type != HP206C_READ_T) && (type != HP206C_READ_PT)
     && (type != HP206C_READ_AT) && (type != HP206C_READ_CAL)) {
    PRINTF("HP206C: invalid value requested\n");
    return HP206C_ERROR;
  } else {
    if(hp206c_read(type, &val) == HP206C_SUCCESS) {
      return val;
    }
  }
  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  uint8_t buf[2];

  switch(type) {
  case HP206C_SOFT_RST:
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
             I2C_SCL_NORMAL_BUS_SPEED);
    buf[0] = HP206C_SOFT_RST_REG;
    if(hp206c_write_reg(&buf[0], 1) != HP206C_SUCCESS) {
      PRINTF("HP206C: failed to reset the sensor\n");
      return HP206C_ERROR;
    }
    buf[0] = REG_PARA;
    hp206c_write_reg(&buf[0], 0x80);
    clock_delay_usec(HP206C_RESET_DELAY);
    return HP206C_SUCCESS;

  case HP206C_CONVERT:
    buf[0] = value;
    hp206c_write_reg(&buf[0], 1);
    enabled = 1;
    return HP206C_SUCCESS;

  default:
    PRINTF("HP206C: option not supported yet\n");
    return HP206C_ERROR;
  }

  return HP206C_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(hp206c, HP206C_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
