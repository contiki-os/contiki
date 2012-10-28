/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
/*
 * Device driver for the Dallas Semiconductor DS2411 chip. Heavily
 * based on the application note 126 "1-Wire Communications Through
 * Software".
 *
 * http://www.maxim-ic.com/appnotes.cfm/appnote_number/126
 */

/*
 * For now we stuff in Moteiv Corporation's unique OUI.
 * From http://www.ethereal.com/distribution/manuf.txt:
 * 00:12:75    Moteiv    # Moteiv Corporation
 *
 * The EUI-64 is a concatenation of the 24-bit OUI value assigned by
 * the IEEE Registration Authority and a 40-bit extension identifier
 * assigned by the organization with that OUI assignment.
 */

#include <string.h>

#include "contiki.h"
#include "dev/ds2411.h"

unsigned char ds2411_id[8];

#ifdef CONTIKI_TARGET_SKY
/* 1-wire is at p2.4 */
#define PIN BV(4)

#define PIN_INIT() {\
  P2DIR &= ~PIN;		/* p2.4 in, resistor pull high */\
  P2OUT &= ~PIN;		/* p2.4 == 0 but still input */\
}

/* Set 1-Wire low or high. */
#define OUTP_0() (P2DIR |=  PIN) /* output and p2.4 == 0 from above */
#define OUTP_1() (P2DIR &= ~PIN) /* p2.4 in, external resistor pull high */

/* Read one bit. */
#define INP()    (P2IN & PIN)

/*
 * Delay for u microseconds on a MSP430 at 2.4756MHz.
 *
 * The loop in clock_delay consists of one add and one jnz, i.e 3
 * cycles.
 *
 * 3 cycles at 2.4756MHz ==> 1.2us = 6/5us.
 *
 * Call overhead is roughly 7 cycles and the loop 3 cycles, to
 * compensate for call overheads we make 7/3=14/6 fewer laps in the
 * loop.
 *
 * This macro will loose badly if not passed a constant argument, it
 * relies on the compiler doing the arithmetic during compile time!!
 * TODO: Fix above comment to be correct - below code is modified for 4Mhz
 */
#define udelay(u) clock_delay((u*8 - 14)/6)

/*
 * Where call overhead dominates, use a macro!
 * Note: modified for 4 Mhz
 */
#define udelay_6() { _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); }

#endif /* CONTIKI_TARGET_SKY */

/*
 * Recommended delay times in us.
 */
#define udelay_tA() udelay_6()
/*      tA 6			   max 15 */
#define tB 64
#define tC 60			/* max 120 */
#define tD 10
#define tE 9			/* max 12 */
#define tF 55
#define tG 0
#define tH 480
#define tI 70
#define tJ 410
/*---------------------------------------------------------------------------*/
static int
owreset(void)
{
  int result;
  OUTP_0();
  udelay(tH);
  OUTP_1();			/* Releases the bus */
  udelay(tI);
  result = INP();
  udelay(tJ);
  return result;
}
/*---------------------------------------------------------------------------*/
static void
owwriteb(unsigned byte)
{
  int i = 7;
  do {
    if(byte & 0x01) {
      OUTP_0();
      udelay_tA();
      OUTP_1();			/* Releases the bus */
      udelay(tB);
    } else {
      OUTP_0();
      udelay(tC);
      OUTP_1();			/* Releases the bus */
      udelay(tD);
    }
    if(i == 0) {
      return;
    }
    i--;
    byte >>= 1;
  } while(1);
}
/*---------------------------------------------------------------------------*/
static unsigned
owreadb(void)
{
  unsigned result = 0;
  int i = 7;
  do {
    OUTP_0();
    udelay_tA();
    OUTP_1();			/* Releases the bus */
    udelay(tE);
    if(INP()) {
      result |= 0x80;		/* LSbit first */
    }
    udelay(tF);
    if(i == 0) {
      return result;
    }
    i--;
    result >>= 1;
  } while(1);
}
/*---------------------------------------------------------------------------*/
/* Polynomial ^8 + ^5 + ^4 + 1 */
static unsigned
crc8_add(unsigned acc, unsigned byte)
{
  int i;
  acc ^= byte;
  for(i = 0; i < 8; i++) {
    if(acc & 1) {
      acc = (acc >> 1) ^ 0x8c;
    } else {
      acc >>= 1;
    }
  }
  return acc;
}
/*---------------------------------------------------------------------------*/
int
ds2411_init()
{
  int i;
  unsigned family, crc, acc;

  PIN_INIT();

  if(owreset() == 0) {	/* Something pulled down 1-wire. */
    /*
     * Read MAC id with interrupts disabled.
     */
    int s = splhigh();
    owwriteb(0x33);		/* Read ROM command. */
    family = owreadb();
    /* We receive 6 bytes in the reverse order, LSbyte first. */
    for(i = 7; i >= 2; i--) {
      ds2411_id[i] = owreadb();
    }
    crc = owreadb();
    splx(s);

    /* Verify family and that CRC match. */
    if(family != 0x01) {
      goto fail;
    }
    acc = crc8_add(0x0, family);
    for(i = 7; i >= 2; i--) {
      acc = crc8_add(acc, ds2411_id[i]);
    }
    if(acc == crc) {
#ifdef CONTIKI_TARGET_SKY
      /* 00:12:75    Moteiv    # Moteiv Corporation */
      ds2411_id[0] = 0x00;
      ds2411_id[1] = 0x12;
      ds2411_id[2] = 0x75;
#endif /* CONTIKI_TARGET_SKY */
      return 1;			/* Success! */
    }
  }

 fail:
  memset(ds2411_id, 0x0, sizeof(ds2411_id));
  return 0;			/* Fail! */
}
/*---------------------------------------------------------------------------*/
