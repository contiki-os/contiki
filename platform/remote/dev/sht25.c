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
 * \addtogroup remote-sht25-sensor
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
#warning I2C SDA AND SCL are inverted in JP8 connector, inverted in init() call
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return SHT25_ERROR;
  }
  if(value) {
    i2c_init(I2C_SCL_PORT, I2C_SCL_PIN, I2C_SDA_PORT, I2C_SDA_PIN, 
             I2C_SCL_NORMAL_BUS_SPEED);
  }
  enabled = value;
  return 0;
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
static uint16_t
sht25_read_reg(uint8_t reg, uint8_t *buf, uint8_t regNum)
{
  if(i2c_single_send(SHT25_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(SHT25_ADDR, buf, regNum) == I2C_MASTER_ERR_NONE) {
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
  buff = (uint32_t)value;
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
static int16_t
sht25_read(uint8_t variable, uint16_t *rd)
{
  uint8_t buf[2];
  uint16_t raw;

  if((variable != SHT25_VAL_TEMP) && (variable != SHT25_VAL_HUM)) {
    return SHT25_ERROR;
  }

  if (sht25_read_reg(variable, buf, 2) == SHT25_SUCCESS){
    raw = (buf[0] << 8) + buf[1];
    *rd = sht25_convert(variable, raw);
    return SHT25_SUCCESS;
  }
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint16_t val;
  if (sht25_read(type, &val) == SHT25_SUCCESS){
    return val;
  }
  return SHT25_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht25, SHT25_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
