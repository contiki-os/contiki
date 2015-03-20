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
 * Driver for the ADXL346 acceleration sensor in OpenMote-CC2538.
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */

/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/i2c.h"
#include "dev/adxl346.h"
/*---------------------------------------------------------------------------*/
/* ADDRESS AND IDENTIFIER */
#define ADXL346_ADDRESS                     (0x53)
#define ADXL346_DEVID_VALUE                 (0xE6)

/* REGISTER ADDRESSES */
#define ADXL346_DEVID_ADDR                  (0x00)
#define ADXL346_THRES_TAP_ADDR              (0x1D)
#define ADXL346_OFSX_ADDR                   (0x1E)
#define ADXL346_OFSY_ADDR                   (0x1F)
#define ADXL346_OFSZ_ADDR                   (0x20)
#define ADXL346_DUR_ADDR                    (0x21)
#define ADXL346_LATENT_ADDR                 (0x22)
#define ADXL346_WINDOW_ADDR                 (0x23)
#define ADXL346_THRESH_ACT_ADDR             (0x24)
#define ADXL346_THRESH_INACT_ADDR           (0x25)
#define ADXL346_TIME_INACT_ADDR             (0x26)
#define ADXL346_ACT_INACT_CTL_ADDR          (0x27)
#define ADXL346_THRESH_FF_ADDR              (0x28)
#define ADXL346_TIME_FF_ADDR                (0x29)
#define ADXL346_TAP_AXES_ADDR               (0x2A)
#define ADXL346_ACT_TAP_STATUS_ADDR         (0x2B)
#define ADXL346_BW_RATE_ADDR                (0x2C)
#define ADXL346_POWER_CTL_ADDR              (0x2D)
#define ADXL346_INT_ENABLE_ADDR             (0x2E)
#define ADXL346_INT_MAP_ADDR                (0x2F)
#define ADXL346_INT_SOURCE_ADDR             (0x30)
#define ADXL346_DATA_FORMAT_ADDR            (0x31)
#define ADXL346_DATAX0_ADDR                 (0x32)
#define ADXL346_DATAX1_ADDR                 (0x33)
#define ADXL346_DATAY0_ADDR                 (0x34)
#define ADXL346_DATAY1_ADDR                 (0x35)
#define ADXL346_DATAZ0_ADDR                 (0x36)
#define ADXL346_DATAZ1_ADDR                 (0x37)
#define ADXL346_FIFO_CTL_ADDR               (0x38)
#define ADXL346_FIFO_STATUS_ADDR            (0x39)
#define ADXL346_TAP_SIGN_ADDR               (0x3A)
#define ADXL346_ORIENT_CONF_ADDR            (0x3B)
#define ADXL346_ORIENT_ADDR                 (0x3C)

/* INT_ENABLE/INT_MAP/INT_SOURCE */
#define ADXL346_INT_ENABLE_DATA_READY      (1 << 7)
#define ADXL346_INT_ENABLE_SINGLE_TAP      (1 << 6)
#define ADXL346_INT_ENABLE_DOUBLE_TAP      (1 << 5)
#define ADXL346_INT_ENABLE_ACTIVITY        (1 << 4)
#define ADXL346_INT_ENABLE_INACTIVITY      (1 << 3)
#define ADXL346_INT_ENABLE_FREE_FALL       (1 << 2)
#define ADXL346_INT_ENABLE_WATERMARK       (1 << 1)
#define ADXL346_INT_ENABLE_OVERRUN         (1 << 0)

/* ACT_INACT_CONTROL */
#define ADXL346_ACT_INACT_CTL_ACT_ACDC     (1 << 7)
#define ADXL346_ACT_INACT_CTL_ACT_X_EN     (1 << 6)
#define ADXL346_ACT_INACT_CTL_ACT_Y_EN     (1 << 5)
#define ADXL346_ACT_INACT_CTL_ACT_Z_EN     (1 << 4)
#define ADXL346_ACT_INACT_CTL_INACT_ACDC   (1 << 3)
#define ADXL346_ACT_INACT_CTL_INACT_X_EN   (1 << 2)
#define ADXL346_ACT_INACT_CTL_INACT_Y_EN   (1 << 1)
#define ADXL346_ACT_INACT_CTL_INACT_Z_EN   (1 << 0)

/* TAP_AXES */
#define ADXL346_TAP_AXES_SUPPRESS           (1 << 3)
#define ADXL346_TAP_AXES_TAP_X_EN           (1 << 2)
#define ADXL346_TAP_AXES_TAP_Y_EN           (1 << 1)
#define ADXL346_TAP_AXES_TAP_Z_EN           (1 << 0)

/* ACT_TAP_STATUS */
#define ADXL346_ACT_TAP_STATUS_ACT_X_SRC    (1 << 6)
#define ADXL346_ACT_TAP_STATUS_ACT_Y_SRC    (1 << 5)
#define ADXL346_ACT_TAP_STATUS_ACT_Z_SRC    (1 << 4)
#define ADXL346_ACT_TAP_STATUS_ASLEEP       (1 << 3)
#define ADXL346_ACT_TAP_STATUS_TAP_X_SRC    (1 << 2)
#define ADXL346_ACT_TAP_STATUS_TAP_Y_SRC    (1 << 1)
#define ADXL346_ACT_TAP_STATUS_TAP_Z_SRC    (1 << 0)

/* BW_RATE */
#define ADXL346_BW_RATE_POWER               (1 << 4)
#define ADXL346_BW_RATE_RATE(x)             ((x) & 0x0F)

/* POWER CONTROL */
#define ADXL346_POWER_CTL_LINK              (1 << 5)
#define ADXL346_POWER_CTL_AUTO_SLEEP        (1 << 4)
#define ADXL346_POWER_CTL_MEASURE           (1 << 3)
#define ADXL346_POWER_CTL_SLEEP             (1 << 2)
#define ADXL346_POWER_CTL_WAKEUP(x)         ((x) & 0x03)

/* DATA_FORMAT */
#define ADXL346_DATA_FORMAT_SELF_TEST       (1 << 7)
#define ADXL346_DATA_FORMAT_SPI             (1 << 6)
#define ADXL346_DATA_FORMAT_INT_INVERT      (1 << 5)
#define ADXL346_DATA_FORMAT_FULL_RES        (1 << 3)
#define ADXL346_DATA_FORMAT_JUSTIFY         (1 << 2)
#define ADXL346_DATA_FORMAT_RANGE(x)        ((x) & 0x03)
#define ADXL346_DATA_FORMAT_RANGE_PM_2g     (0)
#define ADXL346_DATA_FORMAT_RANGE_PM_4g     (1)
#define ADXL346_DATA_FORMAT_RANGE_PM_8g     (2)
#define ADXL346_DATA_FORMAT_RANGE_PM_16g    (3)
/*---------------------------------------------------------------------------*/
/**
 *
 */
static int
adxl346_value(int type)
{
  switch(type) {
    case ADXL346_X_VAL :
      return (int)adxl346_read_x();
    case ADXL346_Y_VAL :
      return (int)adxl346_read_y();
    case ADXL346_Z_VAL :
      return (int)adxl346_read_z();
  }
  return 0;
}
/**
 *
 */
static int
adxl346_config(int type, int value)
{
  switch(type) {
    case SENSORS_HW_INIT :
      adxl346_init();
      return 0;
    case SENSORS_ACTIVE :
      switch(value) {
        case 0 :
          adxl346_set_config(ADXL346_POWER_CTL_ADDR, 0);
          break;
        case 1 :
          adxl346_set_config(ADXL346_POWER_CTL_ADDR, ADXL346_POWER_CTL_MEASURE);
          break;
      }
      break;
    //case SENSORS_CONFIG :
      //adxl346_set_config(&value, &value[1]);
      //break;
  }
  return 0;
}
/**
 *
 */
static int
adxl346_status(int type){
  return (int)adxl346_is_present();
}
/**
 *
 */
void
adxl346_set_config(int reg, int config){
  uint8_t data[2];
  data[0] = reg;
  data[1] = config;
  i2c_burst_send(ADXL346_ADDRESS, data, sizeof(data));
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
adxl346_init(void)
{
  uint8_t config[2];

  config[0] = ADXL346_BW_RATE_ADDR;
  config[1] = (ADXL346_BW_RATE_RATE(11));
  i2c_burst_send(ADXL346_ADDRESS, config, sizeof(config));

  config[0] = ADXL346_DATA_FORMAT_ADDR;
  config[1] = (ADXL346_DATA_FORMAT_SELF_TEST |
               ADXL346_DATA_FORMAT_FULL_RES |
               ADXL346_DATA_FORMAT_RANGE_PM_16g);
  i2c_burst_send(ADXL346_ADDRESS, config, sizeof(config));

  config[0] = ADXL346_POWER_CTL_ADDR;
  config[1] = (ADXL346_POWER_CTL_MEASURE);
  i2c_burst_send(ADXL346_ADDRESS, config, sizeof(config));
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
adxl346_reset(void)
{
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint8_t
adxl346_is_present(void)
{
  uint8_t is_present;

  i2c_single_send(ADXL346_ADDRESS, ADXL346_DEVID_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &is_present);

  return is_present == ADXL346_DEVID_VALUE;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
adxl346_read_x(void)
{
  uint8_t acceleration[2];
  uint16_t x;

  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAX0_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[0]);
  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAX1_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[1]);

  x = (acceleration[0] << 8) | acceleration[1];

  return x;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
adxl346_read_y(void)
{
  uint8_t acceleration[2];
  uint16_t y;

  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAY0_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[0]);
  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAY1_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[1]);

  y = (acceleration[0] << 8) | acceleration[1];

  return y;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
adxl346_read_z(void)
{
  uint8_t acceleration[2];
  uint16_t z;

  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAZ0_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[0]);
  i2c_single_send(ADXL346_ADDRESS, ADXL346_DATAZ1_ADDR);
  i2c_single_receive(ADXL346_ADDRESS, &acceleration[1]);

  z = (acceleration[0] << 8) | acceleration[1];

  return z;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(acceleration_sensor, "Acceleration Sensor", adxl346_value, adxl346_config, adxl346_status);
/** @} */
