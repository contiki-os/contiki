/*
 * Copyright (c) 2015, Copyright Markus Hidell, Robert Olsson
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
 *
 * Authors  : Markus Hidell, Robert Olsson  {mahidell, roolss} @kth.se
 * Created : 2015-10-27
 * Updated : $Date: 2010/01/14 20:23:02 $
 *           $Revision: 1.2 $
 */

#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <i2c.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include "dev/watchdog.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "co2_sa_kxx-sensor.h"

static int
status(int type)
{
  return 0;
}
static int
configure(int type, int c)
{
  return 0;
}
static int
value(int var)
{
  int val, status;
  uint8_t buf[2], csum;
  int16_t res;
  (void) status;
  (void) csum;

  res = 0;
  i2c_start_wait(I2C_CO2SA_ADDR | I2C_WRITE);
  if(res) {
    goto err;
  }

  i2c_write(0x22);
  i2c_write(0x00);

  if(var == CO2_SA_KXX_CO2) {
    i2c_write(0x08);
    i2c_write(0x2A);
  }

  if(var == CO2_SA_KXX_TEMP) {
    i2c_write(0x12);
    i2c_write(0x34);
  }

  if(var == CO2_SA_KXX_RH) {
    i2c_write(0x14);
    i2c_write(0x36);
  }

  i2c_stop();

  if(res) {
    goto err;
  }

  clock_delay_msec(20);

  res = 0;
  i2c_start(I2C_CO2SA_ADDR | I2C_READ);

  if(res) {
    goto err;
  }


  status = i2c_readAck();

  if((status & 0x01) == 0) 
    goto err;

  buf[0] = i2c_readAck();
  buf[1] = i2c_readAck();
  csum = i2c_readNak();
  i2c_stop();

  val = ((int16_t)(buf[0] << 8)) | buf[1];

  return val;

err:
  i2c_stop();
  return 0;
}
SENSORS_SENSOR(co2_sa_kxx_sensor, "CO2", value, configure, status);
