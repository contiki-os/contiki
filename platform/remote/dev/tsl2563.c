/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup remote-tsl2563-sensor
 * @{
 *
 * \file
 *  Driver for the Re-Mote external TSL2563 light sensor (Ziglet)
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "lib/sensors.h"
#include "tsl2563.h"
/*---------------------------------------------------------------------------*/
#warning I2C SDA AND SCL are inverted in JP8 connector, inverted in init() call
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
static uint16_t
calculateLux(uint8_t *buf)
{
  uint32_t ch0, ch1 = 0;
  uint32_t aux = (1 << 14);
  uint32_t ratio, lratio, tmp = 0;
  uint16_t buffer[2];

  buffer[0] = (buf[1] << 8 | (buf[0]));
  buffer[1] = (buf[3] << 8 | (buf[2]));
  ch0 = (buffer[0] * aux) >> 10;
  ch1 = (buffer[1] * aux) >> 10;
  ratio = (ch1 << 10);
  ratio = ratio / ch0;
  lratio = (ratio + 1) >> 1;

  if((lratio >= 0) && (lratio <= K1T)) {
    tmp = (ch0 * B1T) - (ch1 * M1T);
  } else if(lratio <= K2T) {
    tmp = (ch0 * B2T) - (ch1 * M2T);
  } else if(lratio <= K3T) {
    tmp = (ch0 * B3T) - (ch1 * M3T);
  } else if(lratio <= K4T) {
    tmp = (ch0 * B4T) - (ch1 * M4T);
  } else if(lratio <= K5T) {
    tmp = (ch0 * B5T) - (ch1 * M5T);
  } else if(lratio <= K6T) {
    tmp = (ch0 * B6T) - (ch1 * M6T);
  } else if(lratio <= K7T) {
    tmp = (ch0 * B7T) - (ch1 * M7T);
  } else if(lratio > K8T) {
    tmp = (ch0 * B8T) - (ch1 * M8T);
  }

  if(tmp < 0) {
    tmp = 0;
  }

  tmp += (1 << 13);
  return tmp >> 14;
}
/*---------------------------------------------------------------------------*/
static int
tsl2563_read_reg(uint8_t reg, uint8_t *buf, uint8_t regNum)
{
  if(i2c_single_send(TSL2563_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(TSL2563_ADDR, buf, regNum) == I2C_MASTER_ERR_NONE) {
      return TSL2563_SUCCESS;
    }
  }
  return TSL2563_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
light_ziglet_on(void)
{
  if(i2c_single_send(TSL2563_ADDR, (uint8_t)TSL2563_PWRN) == I2C_MASTER_ERR_NONE) {
    return TSL2563_SUCCESS;
  }
  return TSL2563_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
light_ziglet_off(void)
{
  if(i2c_single_send(TSL2563_ADDR, (uint8_t)TSL2563_PWROFF) == I2C_MASTER_ERR_NONE) {
    return TSL2563_SUCCESS;
  }
  return TSL2563_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
light_ziglet_read(uint16_t *lux)
{
  uint8_t buf[4];
  if(light_ziglet_on() == TSL2563_SUCCESS) {
    if(tsl2563_read_reg(TSL2563_READ, buf, 4) == TSL2563_SUCCESS) {
      *lux = calculateLux(buf);
      return light_ziglet_off();
    }
  }
  return TSL2563_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return TSL2563_ERROR;
  }
  enabled = value;
  if(value) {
    i2c_init(I2C_SCL_PORT, I2C_SCL_PIN, I2C_SDA_PORT, I2C_SDA_PIN,
             I2C_SCL_NORMAL_BUS_SPEED);
  } else {
    light_ziglet_off();
  }
  return TSL2563_SUCCESS;
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
  uint16_t lux;
  if(type == TSL2563_VAL_READ) {
    if(light_ziglet_read(&lux) != TSL2563_ERROR) {
      return lux;
    }
  }
  return TSL2563_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(tsl2563, TSL2563_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
