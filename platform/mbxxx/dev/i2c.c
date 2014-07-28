/*
 * Copyright (c) 2013, IDentification Automation Laboratory
 * IDALab (http://www.idalab.unisalento.it)
 * Department of Innovation Engineering - University of Salento
 *
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

/**
 * \file  platform/mbxxx/dev/i2c.c
 * \brief I2C bus master driver for mbxxx platform.
 * \author Maria Laura Stefanizzi <laura28582@gmail.com>
 * \date   2013-11-20
 */

#include <stdio.h>
#include <contiki.h>
#include "dev/i2c.h"
#include PLATFORM_HEADER

#define WAIT_FIN_SC2(FLAG) while(!SC2_STAT(FLAG)) {}

/**
 * Configure serial controller in I2C mode and set I2C speed.
 */
void
i2c_enable(void)
{
  /* Configure serial controller to I2C mode */
  SC2_MODE = SC2_MODE_I2C;

  /*
   * The SCL is produced by dividing down 12MHz according to
   * this equation:
   *    Rate = 12 MHz / ( (LIN + 1) * (2^EXP) )
   *
   * Configure rate registers for Fast Mode operation (400 kbps)
   */
  SC2_RATELIN = 14;
  SC2_RATEEXP = 1;

  /* Reset control registers */
  SC2_TWICTRL1 = SC2_TWICTRL1_RESET;
  SC2_TWICTRL2 = SC2_TWICTRL2_RESET;
}

/**
 * Configure serial controller in disabled mode
 */
void
i2c_disable(void)
{
  SC2_MODE = SC2_MODE_DISABLED;
}

/**
 * Generate I2C START condition
 */
void
i2c_start(void)
{
  SC2_TWICTRL1 |= SC_TWISTART;
  WAIT_FIN_SC2(SC_TWICMDFIN);
}

/**
 * Generate I2C STOP condition
 */
void
i2c_stop(void)
{
  SC2_TWICTRL1 |= SC_TWISTOP;
  WAIT_FIN_SC2(SC_TWICMDFIN);
}

/**
 * Send a byte to I2C bus
 * @param data The data that must be sent
 */
void
i2c_write(uint8_t data)
{
  SC2_DATA = data;

  /* Configure control register 1 for byte transmission */
  SC2_TWICTRL1 |= SC_TWISEND;
  WAIT_FIN_SC2(SC_TWITXFIN);
}

/**
 * Read a byte from I2C bus
 * @param ack If true enable ACK generation after byte reception
 * @return The received byte
 */
uint8_t
i2c_read(int ack)
{
  if(ack) {
    /* Enable ACK generation after current received byte */
    SC2_TWICTRL2 |= SC_TWIACK;
  } else {
    /* Disable ACK generation */
    SC2_TWICTRL2 &= ~SC_TWIACK;
  }

  /* Configure control register 1 for byte reception */
  SC2_TWICTRL1 |= SC_TWIRECV;
  WAIT_FIN_SC2(SC_TWIRXFIN);

  return SC2_DATA;
}
