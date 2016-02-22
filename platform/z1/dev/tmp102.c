/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \file
 *         Device drivers for tmp102 temperature sensor in Zolertia Z1.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Antonio Lignan, Zolertia <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "i2cmaster.h"
#include "tmp102.h"
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
void
tmp102_init(void)
{
  /* Power Up TMP102 via pin */
  TMP102_PWR_DIR |= TMP102_PWR_PIN;
  TMP102_PWR_SEL &= ~TMP102_PWR_SEL;
  TMP102_PWR_SEL2 &= ~TMP102_PWR_SEL;
  TMP102_PWR_REN &= ~TMP102_PWR_SEL;
  TMP102_PWR_OUT |= TMP102_PWR_PIN;

  /* Set up ports and pins for I2C communication */
  i2c_enable();

  enabled = 1;
}
/*---------------------------------------------------------------------------*/
void
tmp102_stop(void)
{
  /* Power off */
  TMP102_PWR_OUT &= ~TMP102_PWR_PIN;
  enabled = 0;
}
/*---------------------------------------------------------------------------*/
void
tmp102_write_reg(uint8_t reg, uint16_t val)
{
  uint8_t tx_buf[] = { reg, 0x00, 0x00 };

  tx_buf[1] = (uint8_t)(val >> 8);
  tx_buf[2] = (uint8_t)(val & 0x00FF);

  i2c_transmitinit(TMP102_ADDR);
  while(i2c_busy());
  PRINTF("I2C Ready to TX\n");

  i2c_transmit_n(3, tx_buf);
  while(i2c_busy());
  PRINTF("WRITE_REG 0x%04X @ reg 0x%02X\n", val, reg);
}
/*---------------------------------------------------------------------------*/
uint16_t
tmp102_read_reg(uint8_t reg)
{
  uint8_t buf[] = { 0x00, 0x00 };
  uint16_t retVal = 0;
  uint8_t rtx = reg;
  PRINTF("READ_REG 0x%02X\n", reg);

  /* transmit the register to read */
  i2c_transmitinit(TMP102_ADDR);
  while(i2c_busy());
  i2c_transmit_n(1, &rtx);
  while(i2c_busy());

  /* receive the data */
  i2c_receiveinit(TMP102_ADDR);
  while(i2c_busy());
  i2c_receive_n(2, &buf[0]);
  while(i2c_busy());

  retVal = (uint16_t)(buf[0] << 8 | (buf[1]));

  return retVal;
}
/*---------------------------------------------------------------------------*/
uint16_t
tmp102_read_temp_raw(void)
{
  uint16_t rd = 0;
  rd = tmp102_read_reg(TMP102_TEMP);
  return rd;
}
/*---------------------------------------------------------------------------*/
int16_t
tmp102_read_temp_x100(void)
{
  int16_t raw = 0;
  int16_t sign = 1;
  int16_t abstemp, temp_int;

  raw = (int16_t)tmp102_read_reg(TMP102_TEMP);
  if(raw < 0) {
    abstemp = (raw ^ 0xFFFF) + 1;
    sign = -1;
  } else {
    abstemp = raw;
  }

  /* Integer part of the temperature value and percents*/
  temp_int = (abstemp >> 8) * sign * 100;
  temp_int += ((abstemp & 0xff) * 100) / 0x100;
  return temp_int;
}
/*---------------------------------------------------------------------------*/
int8_t
tmp102_read_temp_simple(void)
{
  /* Casted to int8_t: We don't expect temperatures outside -128 to 127 C */
  return tmp102_read_temp_x100() / 100;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return TMP102_ERROR;
  }
  if(value) {
    tmp102_init();
  } else {
    tmp102_stop();
  }
  enabled = value;
  return TMP102_SUCCESS;
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
  return TMP102_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return (int)tmp102_read_temp_x100();
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(tmp102, TMP102_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
