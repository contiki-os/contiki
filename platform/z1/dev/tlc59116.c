/*
 * Copyright (c) 2013, Jelmer Tiete.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * This file is part of the Contiki operating system.
 * 
 */

/**
 * \file
 *         Device drivers for tlc59116 i2c led driver on Zolertia Z1.
 *         See http://www.ti.com/product/tlc59116 for datasheet.
 * \author
 *         Jelmer Tiete, VUB <jelmer@tiete.be>
 */


#include <stdio.h>
#include "contiki.h"
#include "tlc59116.h"
#include "i2cmaster.h"


/*---------------------------------------------------------------------------*/
/* Write to a register.
 *  args:
 *    reg       register to write to
 *    val       value to write
 */

void
tlc59116_write_reg(uint8_t reg, uint8_t val)
{
  uint8_t tx_buf[] = { reg, val };

  i2c_transmitinit(TLC59116_ADDR);
  while(i2c_busy());
  PRINTFDEBUG("I2C Ready to TX\n");

  i2c_transmit_n(2, tx_buf);
  while(i2c_busy());
  PRINTFDEBUG("WRITE_REG 0x%02X @ reg 0x%02X\n", val, reg);
}
/*---------------------------------------------------------------------------*/
/* Write several registers from a stream.
 *  args:
 *    len       number of bytes to write
 *    data      pointer to where the data is written from
 *
 * First byte in stream must be the register address to begin writing to.
 * The data is then written from second byte and increasing. 
 */

void
tlc59116_write_stream(uint8_t len, uint8_t * data)
{
  i2c_transmitinit(TLC59116_ADDR);
  while(i2c_busy());
  PRINTFDEBUG("I2C Ready to TX(stream)\n");

  i2c_transmit_n(len, data);    // start tx and send conf reg
  while(i2c_busy());
  PRINTFDEBUG("WRITE_STR %u B to 0x%02X\n", len, data[0]);
}

/*---------------------------------------------------------------------------*/
/* Read one register.
 *  args:
 *    reg       what register to read
 *  returns the value of the read register
 */

uint8_t
tlc59116_read_reg(uint8_t reg)
{
  uint8_t retVal = 0;
  uint8_t rtx = reg;

  PRINTFDEBUG("READ_REG 0x%02X\n", reg);

  /* transmit the register to read */
  i2c_transmitinit(TLC59116_ADDR);
  while(i2c_busy());
  i2c_transmit_n(1, &rtx);
  while(i2c_busy());

  /* receive the data */
  i2c_receiveinit(TLC59116_ADDR);
  while(i2c_busy());
  i2c_receive_n(1, &retVal);
  while(i2c_busy());

  return retVal;
}

/*---------------------------------------------------------------------------*/
/* Read several registers in a stream.
 *  args:
 *    reg       what register to start reading from
 *    len       number of bytes to read
 *    whereto   pointer to where the data is saved
 */

void
tlc59116_read_stream(uint8_t reg, uint8_t len, uint8_t * whereto)
{
  uint8_t rtx = reg;

  PRINTFDEBUG("READ_STR %u B from 0x%02X\n", len, reg);

  /* transmit the register to start reading from */
  i2c_transmitinit(TLC59116_ADDR);
  while(i2c_busy());
  i2c_transmit_n(1, &rtx);
  while(i2c_busy());

  /* receive the data */
  i2c_receiveinit(TLC59116_ADDR);
  while(i2c_busy());
  i2c_receive_n(len, whereto);
  while(i2c_busy());
}

/*---------------------------------------------------------------------------*/
/* Set pwm value for individual led. Make sure PWM mode is enabled.
 *  args:
 *    led       led output -> 0 till 15
 *    pwm       led pwm value
 */

void
tlc59116_led(uint8_t led, uint8_t pwm)
{
  if(led < 0 | led > 15) {
    PRINTFDEBUG("TLC59116: wrong led value.");
  } else {
    tlc59116_write_reg(led + TLC59116_PWM0, pwm);
  }
}

/*---------------------------------------------------------------------------*/
/* Init the led driver: ports, pins, registers, interrupts (none enabled), I2C,
 * default threshold values etc.
 */

void
tlc59116_init(void)
{
  /* Set up ports and pins for I2C communication */
  i2c_enable();

  /* set default register values. */
  tlc59116_write_reg(TLC59116_MODE1, TLC59116_MODE1_DEFAULT);
  tlc59116_write_reg(TLC59116_MODE2, TLC59116_MODE2_DEFAULT);

  /*Set all PWM values to 0x00 (off) */
  /*This would maybe be better with a SWRST */
  uint8_t tx_buf[] =
    { TLC59116_PWM0_AUTOINCR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  tlc59116_write_stream(17, &tx_buf);

  /* set all leds to PWM control */
  tlc59116_write_reg(TLC59116_LEDOUT0, TLC59116_LEDOUT_PWM);
  tlc59116_write_reg(TLC59116_LEDOUT1, TLC59116_LEDOUT_PWM);
  tlc59116_write_reg(TLC59116_LEDOUT2, TLC59116_LEDOUT_PWM);
  tlc59116_write_reg(TLC59116_LEDOUT3, TLC59116_LEDOUT_PWM);
}
