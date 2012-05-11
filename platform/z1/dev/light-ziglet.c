/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * \file
 *         Device drivers for light ziglet sensor in Zolertia Z1.
 * \author
 *         Antonio Lignan, Zolertia <alinan@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */


#include <stdio.h>
#include "contiki.h"
#include "i2cmaster.h"
#include "light-ziglet.h"

#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif

/* Bitmasks and bit flag variable for keeping track of tmp102 status. */
enum TSL2563_STATUSTYPES
{
  /* must be a bit and not more, not using 0x00. */
  INITED = 0x01,
  RUNNING = 0x02,
  STOPPED = 0x04,
};

static enum TSL2563_STATUSTYPES _TSL2563_STATUS = 0x00;

uint16_t 
calculateLux(uint16_t *buffer)
{
  uint32_t ch0, ch1 = 0;
  uint32_t aux = (1<<14);
  uint32_t ratio, lratio, tmp = 0;

  ch0 = (buffer[0]*aux) >> 10;
  ch1 = (buffer[1]*aux) >> 10;

  PRINTFDEBUG("B0 %u, B1 %u\n", buffer[0], buffer[1]);
  PRINTFDEBUG("ch0 %lu, ch1 %lu\n", ch0, ch1);

  ratio = (ch1 << 10);
  ratio = ratio/ch0;
  lratio = (ratio+1) >> 1;

  PRINTFDEBUG("ratio %lu, lratio %lu\n", ratio, lratio);

  if ((lratio >= 0) && (lratio <= K1T))
    tmp = (ch0*B1T) - (ch1*M1T);
  else if (lratio <= K2T)
    tmp = (ch0*B2T) - (ch1*M2T);
  else if (lratio <= K3T)
    tmp = (ch0*B3T) - (ch1*M3T);
  else if (lratio <= K4T)
    tmp = (ch0*B4T) - (ch1*M4T);
  else if (lratio <= K5T)
    tmp = (ch0*B5T) - (ch1*M5T);
  else if (lratio <= K6T)
    tmp = (ch0*B6T) - (ch1*M6T);
  else if (lratio <= K7T)
    tmp = (ch0*B7T) - (ch1*M7T);
  else if (lratio > K8T)
    tmp = (ch0*B8T) - (ch1*M8T);

  if (tmp < 0) tmp = 0;
    
  tmp += (1<<13);

  PRINTFDEBUG("tmp %lu\n", tmp);

  return (tmp >> 14);
}

/*---------------------------------------------------------------------------*/
/* Init the light ziglet sensor: ports, pins, registers, interrupts (none enabled), I2C,
    default threshold values etc. */

void
light_ziglet_init (void)
{
  if (!(_TSL2563_STATUS & INITED))
    {
      PRINTFDEBUG ("light ziglet init\n");
      _TSL2563_STATUS |= INITED;

      /* Set up ports and pins for I2C communication */
      i2c_enable ();
      return;
   }
}

/*---------------------------------------------------------------------------*/
/* Write to a 16-bit register.
    args:
      reg       register to write to
      val       value to write
*/

void
tsl2563_write_reg (uint8_t reg, uint16_t val)
{
  uint8_t tx_buf[] = { reg, 0x00, 0x00 };

  tx_buf[1] = (uint8_t) (val >> 8);
  tx_buf[2] = (uint8_t) (val & 0x00FF);

  i2c_transmitinit (TSL2563_ADDR);
  while (i2c_busy ());
  PRINTFDEBUG ("I2C Ready to TX\n");

  i2c_transmit_n (3, tx_buf);
  while (i2c_busy ());
  PRINTFDEBUG ("WRITE_REG 0x%04X @ reg 0x%02X\n", val, reg);
}

/*---------------------------------------------------------------------------*/
/* Read register.
    args:
      reg       what register to read
    returns the value of the read register type uint16_t
*/

uint16_t
tsl2563_read_reg (uint8_t reg)
{
  uint16_t readBuf[] = {0x00, 0x00};
  uint8_t buf[] = { 0x00, 0x00, 0x00, 0x00};
  uint16_t retVal = 0;
  uint8_t rtx = reg;

  // Transmit the register to read 
  i2c_transmitinit (TSL2563_ADDR);
  while (i2c_busy ());
  i2c_transmit_n (1, &rtx);
  while (i2c_busy ());

  // Receive the data 
  i2c_receiveinit (TSL2563_ADDR);
  while (i2c_busy ());
  i2c_receive_n (4, &buf[0]);
  while (i2c_busy ());

  PRINTFDEBUG("\nb0 %u, b1 %u, b2 %u, b3 %u\n", buf[0], buf[1], buf[2], buf[3]);

  readBuf[0] = (buf[1] << 8 | (buf[0]));
  readBuf[1] = (buf[3] << 8 | (buf[2]));

  /* XXX Quick hack, was receiving dups bytes */

  if(readBuf[0] == readBuf[1]){
    tsl2563_read_reg(TSL2563_READ);
    return;
  } else {
    retVal = calculateLux(&readBuf);
    return retVal;
  }
}

uint16_t
light_ziglet_on(void)
{
  uint16_t data;
  uint8_t regon[] = { 0x00, TSL2563_PWRN };
  // Turn on the sensor
  i2c_transmitinit (TSL2563_ADDR);
  while (i2c_busy ());
  i2c_transmit_n (2, &regon);
  while (i2c_busy ());
  data = (uint16_t) tsl2563_read_reg (TSL2563_READ);
  return data;
}

void
light_ziglet_off(void)
{
  uint8_t regoff = 0x00;
  // Turn off the sensor
  i2c_transmitinit (TSL2563_ADDR);
  while (i2c_busy ());
  i2c_transmit_n (1, &regoff);
  while (i2c_busy ());
  return;
}


/*---------------------------------------------------------------------------*/
/* Read light ziglet sensor
*/

uint16_t
light_ziglet_read(void)
{
  uint16_t lux = 0;
  lux = light_ziglet_on();
  light_ziglet_off();
  return lux;
}

