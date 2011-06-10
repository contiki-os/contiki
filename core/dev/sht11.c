/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: sht11.c,v 1.3 2008/11/10 21:10:36 adamdunkels Exp $
 */

/*
 * Device driver for the Sensirion SHT1x/SHT7x family of humidity and
 * temperature sensors.
 */

#include "contiki.h"
#include <stdio.h>

#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif

#include <dev/sht11.h>
#include "sht11-arch.h"

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef SDA_0
#define SDA_0()   (SHT11_PxDIR |=  BV(SHT11_ARCH_SDA))	/* SDA Output=0 */
#define SDA_1()   (SHT11_PxDIR &= ~BV(SHT11_ARCH_SDA))	/* SDA Input */
#define SDA_IS_1  (SHT11_PxIN & BV(SHT11_ARCH_SDA))

#define SCL_0()   (SHT11_PxOUT &= ~BV(SHT11_ARCH_SCL))	/* SCL Output=0 */
#define SCL_1()   (SHT11_PxOUT |=  BV(SHT11_ARCH_SCL))	/* SCL Output=1 */
#endif
				/* adr   command  r/w */
#define  STATUS_REG_W   0x06	/* 000    0011    0 */
#define  STATUS_REG_R   0x07	/* 000    0011    1 */
#define  MEASURE_TEMP   0x03	/* 000    0001    1 */
#define  MEASURE_HUMI   0x05	/* 000    0010    1 */
#define  RESET          0x1e	/* 000    1111    0 */

/* This can probably be reduced to 250ns according to data sheet. */
#ifndef delay_400ns
#define delay_400ns() _NOP()
#endif
/*---------------------------------------------------------------------------*/
static void
sstart(void)
{
  SDA_1(); SCL_0();
  delay_400ns();
  SCL_1();
  delay_400ns();
  SDA_0();
  delay_400ns();
  SCL_0();
  delay_400ns();
  SCL_1();
  delay_400ns();
  SDA_1();
  delay_400ns();
  SCL_0();
}
/*---------------------------------------------------------------------------*/
static void
sreset(void)
{
  int i;
  SDA_1();
  SCL_0();
  for(i = 0; i < 9 ; i++) {
    SCL_1();
    delay_400ns();
    SCL_0();
    delay_400ns();
  }
  sstart();			/* Start transmission, why??? */
}
/*---------------------------------------------------------------------------*/
/*
 * Return true if we received an ACK.
 */
static int
swrite(unsigned _c)
{
  unsigned char c = _c;
  int i;
  int ret;

  for(i = 0; i < 8; i++, c <<= 1) {
    if(c & 0x80) {
      SDA_1();
    } else {
      SDA_0();
    }
    SCL_1();
    delay_400ns();
    SCL_0();
    delay_400ns();
  }

  SDA_1();
  SCL_1();
  delay_400ns();
  ret = !SDA_IS_1;

  SCL_0();

  return ret;
}
/*---------------------------------------------------------------------------*/
static unsigned
sread(int send_ack)
{
  int i;
  unsigned char c = 0x00;

  SDA_1();
  for(i = 0; i < 8; i++) {
    c <<= 1;
    SCL_1();
    delay_400ns();
    if(SDA_IS_1) {
      c |= 0x1;
    }
    SCL_0();
    delay_400ns();
  }

  if(send_ack) {
    SDA_0();
  }
  SCL_1();
  delay_400ns();
  SCL_0();

  SDA_1();			/* Release SDA */

  return c;
}
/*---------------------------------------------------------------------------*/
#define CRC_CHECK
#ifdef CRC_CHECK
static unsigned char
rev8bits(unsigned char v)
{
  unsigned char r = v;
  int s = 7;

  for (v >>= 1; v; v >>= 1) {
    r <<= 1;
    r |= v & 1;
    s--;
  }
  r <<= s;		    /* Shift when v's highest bits are zero */
  return r;
}
/*---------------------------------------------------------------------------*/
/* BEWARE: Bit reversed CRC8 using polynomial ^8 + ^5 + ^4 + 1 */
static unsigned
crc8_add(unsigned acc, unsigned byte)
{
  int i;
  acc ^= byte;
  for(i = 0; i < 8; i++) {
    if(acc & 0x80) {
      acc = (acc << 1) ^ 0x31;
    } else {
      acc <<= 1;
    }
  }
  return acc & 0xff;
}
#endif /* CRC_CHECK */
/*---------------------------------------------------------------------------*/
/*
 * Power up the device. The device can be used after an additional
 * 11ms waiting time.
 */
void
sht11_init(void)
{
  /*
   * SCL Output={0,1}
   * SDA 0: Output=0
   *     1: Input and pull-up (Output=0)
   */
#ifdef SHT11_INIT
  SHT11_INIT();
#else
  SHT11_PxOUT |= BV(SHT11_ARCH_PWR);
  SHT11_PxOUT &= ~(BV(SHT11_ARCH_SDA) | BV(SHT11_ARCH_SCL));
  SHT11_PxDIR |= BV(SHT11_ARCH_PWR) | BV(SHT11_ARCH_SCL);
#endif
}
/*---------------------------------------------------------------------------*/
/*
 * Power of device.
 */
void
sht11_off(void)
{
#ifdef SHT11_OFF
  SHT11_OFF();
#else
  SHT11_PxOUT &= ~BV(SHT11_ARCH_PWR);
  SHT11_PxOUT &= ~(BV(SHT11_ARCH_SDA) | BV(SHT11_ARCH_SCL));
  SHT11_PxDIR |= BV(SHT11_ARCH_PWR) | BV(SHT11_ARCH_SCL);
#endif
}
/*---------------------------------------------------------------------------*/
/*
 * Only commands MEASURE_HUMI or MEASURE_TEMP!
 */
static unsigned int
scmd(unsigned cmd)
{
  unsigned long n;

  if(cmd != MEASURE_HUMI && cmd != MEASURE_TEMP) {
    PRINTF("Illegal command: %d\n", cmd);
    return -1;
  }

  sstart();			/* Start transmission */
  if(!swrite(cmd)) {
    PRINTF("SHT11: scmd - swrite failed\n");
    goto fail;
  }

  for(n = 0; n < 250000; n++) {
    if(!SDA_IS_1) {
      unsigned t0, t1, rcrc;
      t0 = sread(1);
      t1 = sread(1);
      rcrc = sread(0);
      PRINTF("SHT11: scmd - read %d, %d\n", t0, t1);
#ifdef CRC_CHECK
      {
	unsigned crc;
	crc = crc8_add(0x0, cmd);
	crc = crc8_add(crc, t0);
	crc = crc8_add(crc, t1);
	if(crc != rev8bits(rcrc)) {
	  PRINTF("SHT11: scmd - crc check failed %d vs %d\n",
		 crc, rev8bits(rcrc));
	  goto fail;
	}
      }
#endif
      return (t0 << 8) | t1;
    }
  }

 fail:
  sreset();
  return -1;
}
/*---------------------------------------------------------------------------*/
/*
 * Call may take up to 210ms.
 */
unsigned int
sht11_temp(void)
{
  return scmd(MEASURE_TEMP);
}
/*---------------------------------------------------------------------------*/
/*
 * Call may take up to 210ms.
 */
unsigned int
sht11_humidity(void)
{
  return scmd(MEASURE_HUMI);
}
/*---------------------------------------------------------------------------*/
#if 1 /* But ok! */
unsigned
sht11_sreg(void)
{
  unsigned sreg, rcrc;

  sstart();			/* Start transmission */
  if(!swrite(STATUS_REG_R)) {
    goto fail;
  }

  sreg = sread(1);
  rcrc = sread(0);

#ifdef CRC_CHECK
  {
    unsigned crc;
    crc = crc8_add(0x0, STATUS_REG_R);
    crc = crc8_add(crc, sreg);
    if (crc != rev8bits(rcrc))
      goto fail;
  }
#endif

  return sreg;

 fail:
  sreset();
  return -1;
}
#endif
/*---------------------------------------------------------------------------*/
#if 0
int
sht11_set_sreg(unsigned sreg)
{
  sstart();			/* Start transmission */
  if(!swrite(STATUS_REG_W)) {
    goto fail;
  }
  if(!swrite(sreg)) {
    goto fail;
  }

  return 0;

 fail:
  sreset();
  return -1;
}
#endif
/*---------------------------------------------------------------------------*/
#if 0
int
sht11_reset(void)
{
  sstart();			/* Start transmission */
  if(!swrite(RESET)) {
    goto fail;
  }

  return 0;

 fail:
  sreset();
  return -1;
}
#endif
/*---------------------------------------------------------------------------*/
