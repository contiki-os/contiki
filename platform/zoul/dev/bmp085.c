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
 * \addtogroup zoul-bmp085-sensor
 * @{
 *
 * BMP085 driver implementation, calculations taken from Inga sensor driver
 *
 * \file
 *  Driver for the external BMP085 light sensor
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/gpio.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "bmp085.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled = 0;
/*---------------------------------------------------------------------------*/
typedef struct {
  int16_t ac1;
  int16_t ac2;
  int16_t ac3;
  uint16_t ac4;
  uint16_t ac5;
  uint16_t ac6;
  int16_t b1;
  int16_t b2;
  int16_t mb;
  int16_t mc;
  int16_t md;
} bmp085_calibration_values;

typedef struct {
  uint8_t oversampling_mode;
  int32_t b5;
  bmp085_calibration_values calib;
} bmp085_config;

static bmp085_config bmp085_values;
/*---------------------------------------------------------------------------*/
static int
bmp085_read_reg(uint8_t reg, uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("BMP085: invalid read values\n");
    return BMP085_ERROR;
  }

  i2c_master_enable();
  if(i2c_single_send(BMP085_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    while(i2c_master_busy());
    if(i2c_burst_receive(BMP085_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
      return BMP085_SUCCESS;
    }
  }
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("BMP085: invalid write values\n");
    return BMP085_ERROR;
  }

  i2c_master_enable();
  if(i2c_burst_send(BMP085_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return BMP085_SUCCESS;
  }
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_read_calib(void)
{
  uint8_t buf[BMP085_CALIB_TABLE_SIZE];

  if(bmp085_read_reg(BMP085_AC1_CALIB, buf,
                     BMP085_CALIB_TABLE_SIZE) == BMP085_SUCCESS) {

    /*  MSB first */
    bmp085_values.calib.ac1 = ((buf[0] << 8) + buf[1]);
    bmp085_values.calib.ac2 = ((buf[2] << 8) + buf[3]);
    bmp085_values.calib.ac3 = ((buf[4] << 8) + buf[5]);
    bmp085_values.calib.ac4 = ((buf[6] << 8) + buf[7]);
    bmp085_values.calib.ac5 = ((buf[8] << 8) + buf[9]);
    bmp085_values.calib.ac6 = ((buf[10] << 8) + buf[11]);
    bmp085_values.calib.b1 = ((buf[12] << 8) + buf[13]);
    bmp085_values.calib.b2 = ((buf[14] << 8) + buf[15]);
    bmp085_values.calib.mb = ((buf[16] << 8) + buf[17]);
    bmp085_values.calib.mc = ((buf[18] << 8) + buf[19]);
    bmp085_values.calib.md = ((buf[20] << 8) + buf[21]);

    return BMP085_SUCCESS;
  }

  PRINTF("BMP085: failed to read calibration\n");
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_read_uncompensated_pressure(int32_t *pressure)
{
  uint8_t buf[3];
  uint16_t delay;
  int32_t upres;

  buf[0] = BMP085_CTRL_REG;

  switch(bmp085_values.oversampling_mode) {
  case BMP085_MODE_ULTRA_LOW_POWER:
    buf[1] = BMP085_CTRL_REG_PRESS_4_5MS;
    delay = BMP085_DELAY_4_5MS;
    break;
  case BMP085_MODE_STANDARD:
    buf[1] = BMP085_CTRL_REG_PRESS_7_5MS;
    delay = BMP085_DELAY_7_5MS;
    break;
  case BMP085_MODE_HIGH_RES:
    buf[1] = BMP085_CTRL_REG_PRESS_13_5MS;
    delay = BMP085_DELAY_13_5MS;
    break;
  case BMP085_MODE_ULTRA_HIGH_RES:
    buf[1] = BMP085_CTRL_REG_PRESS_25_5MS;
    delay = BMP085_DELAY_25_5MS;
    break;
  default:
    return BMP085_ERROR;
  }

  if(bmp085_write_reg(buf, 2) == BMP085_SUCCESS) {
    clock_delay_usec(delay);
    if(bmp085_read_reg(BMP085_DATA_MSB, buf, 3) == BMP085_SUCCESS) {
      upres = (buf[0] << 16) + (buf[1] << 8) + buf[2];
      *pressure = (upres >> (8 - bmp085_values.oversampling_mode));
      return BMP085_SUCCESS;
    }
  }
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_read_uncompensated_temperature(int32_t *temp)
{
  uint8_t buf[2];
  buf[0] = BMP085_CTRL_REG;
  buf[1] = BMP085_CTRL_REG_TEMP;

  if(bmp085_write_reg(buf, 2) == BMP085_SUCCESS) {
    clock_delay_usec(BMP085_DELAY_4_5MS);
    if(bmp085_read_reg(BMP085_DATA_MSB, buf, 2) == BMP085_SUCCESS) {
      *temp = (int32_t)((buf[0] << 8) + buf[1]);
      return BMP085_SUCCESS;
    }
  }
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_read_temperature(int16_t *temp)
{
  int32_t ut = 0;
  int32_t x1, x2;

  if(bmp085_read_uncompensated_temperature(&ut) == BMP085_ERROR) {
    return BMP085_ERROR;
  }

  x1 = ((int32_t)ut - (int32_t)bmp085_values.calib.ac6)
    * (int32_t)bmp085_values.calib.ac5 >> 15;
  x2 = ((int32_t)bmp085_values.calib.mc << 11) / (x1 + bmp085_values.calib.md);
  bmp085_values.b5 = x1 + x2;
  *temp = (int16_t)((bmp085_values.b5 + 8) >> 4);
  return BMP085_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
bmp085_read_pressure(int32_t *pressure)
{
  int32_t ut = 0;
  int32_t up = 0;
  int32_t x1, x2, b6, x3, b3, p;
  uint32_t b4, b7;

  if(bmp085_read_uncompensated_pressure(&up) == BMP085_ERROR) {
    return BMP085_ERROR;
  }

  if(bmp085_read_uncompensated_temperature(&ut) == BMP085_ERROR) {
    return BMP085_ERROR;
  }

  b6 = bmp085_values.b5 - 4000;
  x1 = (bmp085_values.calib.b2 * (b6 * b6 >> 12)) >> 11;
  x2 = bmp085_values.calib.ac2 * b6 >> 11;
  x3 = x1 + x2;
  b3 = ((((int32_t)bmp085_values.calib.ac1) * 4 + x3) + 2) >> 2;

  x1 = (bmp085_values.calib.ac3 * b6) >> 13;
  x2 = (bmp085_values.calib.b1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (bmp085_values.calib.ac4 * ((uint32_t)(x3 + 32768))) >> 15;
  b7 = ((uint32_t)up - b3) * 50000;

  if(b7 < 0x80000000) {
    p = (b7 << 1) / b4;
  } else {
    p = (b7 / b4) << 1;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  *pressure = (p + ((x1 + x2 + 3791) >> 4));
  *pressure /= 10;

  return BMP085_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if((type != BMP085_ACTIVE) && (type != BMP085_OVERSAMPLING)) {
    PRINTF("BMP085: invalid start value\n");
    return BMP085_ERROR;
  }

  if(type == BMP085_ACTIVE) {
    if(value) {
      i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
               I2C_SCL_NORMAL_BUS_SPEED);

      /* Read the calibration values */
      if(bmp085_read_calib() != BMP085_ERROR) {
        PRINTF("BMP085: sensor started\n");
        enabled = 1;
        bmp085_values.oversampling_mode = BMP085_MODE_ULTRA_LOW_POWER;
        return BMP085_SUCCESS;
      }

      PRINTF("BMP085: failed to enable\n");
      return BMP085_ERROR;
    } else {
      enabled = 0;
      return BMP085_SUCCESS;
    }
  } else if(type == BMP085_OVERSAMPLING) {
    if((value < BMP085_MODE_ULTRA_LOW_POWER) ||
       (value > BMP085_MODE_ULTRA_HIGH_RES)) {
      PRINTF("BMP085: invalid oversampling value\n");
      return BMP085_ERROR;
    }
    bmp085_values.oversampling_mode = value;
    return BMP085_SUCCESS;
  }

  return BMP085_ERROR;
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
bmp085_read_sensor(int32_t *value, uint8_t type)
{
  int16_t temp = 0;

  /* The temperature is required to compensate the pressure value */
  if(bmp085_read_temperature(&temp) != BMP085_SUCCESS) {
    return BMP085_ERROR;
  }

  switch(type) {
  case BMP085_READ_PRESSURE:
    return bmp085_read_pressure(value);

  case BMP085_READ_TEMP:
    *value = (int16_t) temp;
    return BMP085_SUCCESS;
  }

  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  int32_t value;

  if(!enabled) {
    PRINTF("BMP085: sensor not started\n");
    return BMP085_ERROR;
  }

  if((type != BMP085_READ_PRESSURE) && (type != BMP085_READ_TEMP)) {
    PRINTF("BMP085: invalid read value\n");
    return BMP085_ERROR;
  }

  if(bmp085_read_sensor(&value, type) == BMP085_SUCCESS) {
    return (int)value;
  }

  PRINTF("BMP085: fail to read\n");
  return BMP085_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(bmp085, BMP085_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
