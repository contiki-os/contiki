/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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

/*
 * Small and portable implementation of a bit-banging I2C bus master.
 *
 * The code should port really easily to platforms other than the
 * msp430 but has some hardcoded constants in it.
 *
 * More info at:
 *   http://i2c-bus.org/
 *   http://www.esacademy.com/faq/i2c/
 */

#include <stdio.h>
#include <contiki.h>
#include <dev/spi.h>
#include <dev/leds.h>

#include "dev/i2c.h"

/*
 * On the Tmote sky access to I2C/SPI/UART0 must always be exclusive.
 */

void     i2c_enable(void);
void     i2c_disable(void);
int      i2c_start(void);
unsigned i2c_read(int send_ack);
int      i2c_write(unsigned);
void     i2c_stop(void);

#define I2C_PxDIR   P3DIR
#define I2C_PxIN    P3IN
#define I2C_PxOUT   P3OUT
#define I2C_PxSEL   P3SEL
/*
 * SDA == P3.1
 * SCL == P3.3
 */
#define SDA       1
#define SCL       3

#define SDA_0()   (I2C_PxDIR |=  BV(SDA))		/* SDA Output */
#define SDA_1()   (I2C_PxDIR &= ~BV(SDA))		/* SDA Input */
#define SDA_IS_1  (I2C_PxIN & BV(SDA))

#define SCL_0()   (I2C_PxDIR |=  BV(SCL))		/* SCL Output */
#define SCL_1()   (I2C_PxDIR &= ~BV(SCL))		/* SCL Input */
#define SCL_IS_1  (I2C_PxIN & BV(SCL))

/*
 * Should avoid infinite looping while waiting for SCL_IS_1. xxx/bg
 */
#define SCL_WAIT_FOR_1() do{}while (!SCL_IS_1)

#define delay_4_7us() do{ _NOP(); _NOP(); _NOP(); _NOP(); \
                          _NOP(); _NOP(); _NOP(); _NOP(); \
                          _NOP(); _NOP(); _NOP(); _NOP(); }while(0)

#define delay_4us()   do{ _NOP(); _NOP(); _NOP(); _NOP(); \
                          _NOP(); _NOP(); _NOP(); _NOP(); \
                          _NOP(); _NOP(); }while(0)

static unsigned char old_pxsel, old_pxout, old_pxdir;

/*
 * Grab SDA and SCL pins for exclusive use but remember old
 * configuration so that it may be restored when we are done.
 */
void
i2c_enable(void)
{
  unsigned char sda_scl = BV(SDA)|BV(SCL);

  old_pxsel = I2C_PxSEL & sda_scl;
  old_pxout = I2C_PxOUT & sda_scl;
  old_pxdir = I2C_PxDIR & sda_scl;

  spi_busy = 1;

  I2C_PxSEL &= ~sda_scl;

  I2C_PxOUT &= ~sda_scl;

  I2C_PxDIR |=  BV(SCL);		/* SCL Output */
  I2C_PxDIR &= ~BV(SDA);		/* SDA Input */
}

/*
 * Restore bus to what it was before i2c_enable.
 *
 */
void
i2c_disable(void)
{
  unsigned char not_sda_scl = ~(BV(SDA)|BV(SCL));

  I2C_PxDIR = (I2C_PxDIR & not_sda_scl) | old_pxdir;
  I2C_PxOUT = (I2C_PxOUT & not_sda_scl) | old_pxout;
  I2C_PxSEL = (I2C_PxSEL & not_sda_scl) | old_pxsel;

  spi_busy = 0;
}

int
i2c_start(void)
{
  SDA_1();
  SCL_1();
#if 1
  SCL_WAIT_FOR_1();
#else
  {
    unsigned long n;
    for (n = 0; n < 100000 && !SCL_IS_1; n++)
      ;
    if (!SCL_IS_1)
      return -1;
  }
#endif
  delay_4_7us();
  SDA_0();
  delay_4us();
  SCL_0();
  return 0;
}

void
i2c_stop(void)
{
  SDA_0();
  delay_4us();
  SCL_1();
  SCL_WAIT_FOR_1();
  SDA_1();
}

/*
 * Return true if we received an ACK.
 */
int
i2c_write(unsigned _c)
{
  unsigned char c = _c;
  unsigned long n;
  int i;
  int ret;

  for (i = 0; i < 8; i++, c <<= 1) {
    if (c & 0x80)
      SDA_1();
    else
      SDA_0();
    SCL_1();
    SCL_WAIT_FOR_1();
    SCL_0();
  }

  SDA_1();
  SCL_1();
  ret = 0;		   /* Loop waiting for an ACK to arrive. */
  for (n = 0; n < 250000; n++) {
    if (!SDA_IS_1) {
      ret = 1;
      break;
    }
  }
  SCL_WAIT_FOR_1();		/* clock stretching? */
  SCL_0();

  return ret;
}

unsigned
i2c_read(int send_ack)
{
  int i;
  unsigned char c = 0x00;

  SDA_1();
  for (i = 0; i < 8; i++) {
    c <<= 1;
    SCL_1();
    SCL_WAIT_FOR_1();
    if (SDA_IS_1)
      c |= 0x1;
    SCL_0();
  }

  if (send_ack)
    SDA_0();
  SCL_1();
  SCL_WAIT_FOR_1();
  SCL_0();

  return c;
}
