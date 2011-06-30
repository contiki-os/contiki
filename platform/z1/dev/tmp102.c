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
 *         Device drivers for tmp102 temperature sensor in Zolertia Z1.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */


#include <stdio.h>
#include "contiki.h"
#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif

#include "i2cmaster.h"
#include "tmp102.h"



/* Bitmasks and bit flag variable for keeping track of tmp102 status. */
enum TMP102_STATUSTYPES
{
  /* must be a bit and not more, not using 0x00. */
  INITED = 0x01,
  RUNNING = 0x02,
  STOPPED = 0x04,
  LOW_POWER = 0x08,
  AAA = 0x10,			// available to extend this...
  BBB = 0x20,			// available to extend this...
  CCC = 0x40,			// available to extend this...
  DDD = 0x80 			// available to extend this...
};
static enum TMP102_STATUSTYPES _TMP102_STATUS = 0x00;


/*---------------------------------------------------------------------------*/
//PROCESS(tmp102_process, "Temperature Sensor process");

/*---------------------------------------------------------------------------*/
/* Init the temperature sensor: ports, pins, registers, interrupts (none enabled), I2C,
    default threshold values etc. */

void
tmp102_init (void)
{
  if (!(_TMP102_STATUS & INITED))
    {
      PRINTFDEBUG ("TMP102 init\n");
      _TMP102_STATUS |= INITED;
      /* Power Up TMP102 via pin */
      TMP102_PWR_DIR |= TMP102_PWR_PIN;
      TMP102_PWR_SEL &= ~TMP102_PWR_SEL;
      TMP102_PWR_SEL2 &= ~TMP102_PWR_SEL;
      TMP102_PWR_REN &= ~TMP102_PWR_SEL;
      TMP102_PWR_OUT |= TMP102_PWR_PIN;

      /* Set up ports and pins for I2C communication */
      i2c_enable ();

   }
}

/*---------------------------------------------------------------------------*/
/* Write to a 16-bit register.
    args:
      reg       register to write to
      val       value to write
*/

void
tmp102_write_reg (u8_t reg, u16_t val)
{
  u8_t tx_buf[] = { reg, 0x00, 0x00 };

  tx_buf[1] = (u8_t) (val >> 8);
  tx_buf[2] = (u8_t) (val & 0x00FF);

  i2c_transmitinit (TMP102_ADDR);
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
    returns the value of the read register type u16_t
*/

u16_t
tmp102_read_reg (u8_t reg)
{
  u8_t buf[] = { 0x00, 0x00 };
  u16_t retVal = 0;
  u8_t rtx = reg;
  PRINTFDEBUG ("READ_REG 0x%02X\n", reg);

  // transmit the register to read 
  i2c_transmitinit (TMP102_ADDR);
  while (i2c_busy ());
  i2c_transmit_n (1, &rtx);
  while (i2c_busy ());

  // receive the data 
  i2c_receiveinit (TMP102_ADDR);
  while (i2c_busy ());
  i2c_receive_n (2, &buf[0]);
  while (i2c_busy ());

  retVal = (u16_t) (buf[0] << 8 | (buf[1]));

  return retVal;
}

/*---------------------------------------------------------------------------*/
/* Read temperature in a raw format. Further processing will be needed
   to make an interpretation of these 12 or 13-bit data, depending on configuration
*/

u16_t
tmp102_read_temp_raw (void)
{
  u16_t rd = 0;

  rd = tmp102_read_reg (TMP102_TEMP);

  return rd;
}

/*---------------------------------------------------------------------------*/
/* Simple Read temperature. Return is an integer with temperature in 1deg. precision 
   Return value is a signed 8 bit integer.
*/

int8_t
tmp102_read_temp_simple (void)
{
  int16_t raw = 0;
  int8_t rd = 0;
  int16_t sign = 1;
  int16_t abstemp, temp_int;

  raw = (int16_t) tmp102_read_reg (TMP102_TEMP);
  if(raw < 0) {
    abstemp = (raw ^ 0xFFFF) + 1;
    sign = -1;
  } else {
    abstemp = raw;
  }

  /* Integer part of the temperature value */
  temp_int = (abstemp >> 8) * sign;

  /* See test-tmp102.c on how to print values of temperature with decimals 
     fractional part in 1/10000 of degree 
     temp_frac = ((abstemp >>4) % 16) * 625; 
     Data could be multiplied by 63 to have less bit-growth and 1/1000 precision 
     Data could be multiplied by 64 (<< 6) to trade-off precision for speed 
  */

  rd = (int8_t) (temp_int);
  return rd;
}
