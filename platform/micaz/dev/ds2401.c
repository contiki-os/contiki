/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * Device driver for the Dallas Semiconductor DS2401 chip. Heavily
 * based on the application note 126 "1-Wire Communications Through
 * Software".
 *
 * http://www.maxim-ic.com/appnotes.cfm/appnote_number/126
 */

/*
 * For now we stuff in Crossbow Technology, Inc's unique OUI.
 * 00:1A:4C    Crossbow Technology, Inc
 *
 * The EUI-64 is a concatenation of the 24-bit OUI value assigned by
 * the IEEE Registration Authority and a 40-bit extension identifier
 * assigned by the organization with that OUI assignment.
 */

#include <avr/io.h>
#include <string.h>
#include "contiki.h"

#include "ds2401.h"

unsigned char ds2401_id[8];

/* 1-wire is at PortA.4 */
#define SERIAL_ID_PIN_READ PINA
#define SERIAL_ID_PIN_MASK _BV(4)
#define SERIAL_ID_PxOUT PORTA
#define SERIAL_ID_PxDIR DDRA

#define SET_PIN_INPUT() (SERIAL_ID_PxDIR &= ~SERIAL_ID_PIN_MASK)
#define SET_PIN_OUTPUT() (SERIAL_ID_PxDIR |= SERIAL_ID_PIN_MASK)

#define OUTP_0() (SERIAL_ID_PxOUT &= ~SERIAL_ID_PIN_MASK)
#define OUTP_1() (SERIAL_ID_PxOUT |= SERIAL_ID_PIN_MASK)

#define PIN_INIT() do{  \
                     SET_PIN_INPUT();    \
                     OUTP_0();           \
                   } while(0)


/* Drive the one wire interface low */
#define OW_DRIVE() do {                    \
                     SET_PIN_OUTPUT();     \
                     OUTP_0();             \
                   } while (0)

/* Release the one wire by turning on the internal pull-up. */
#define OW_RELEASE() do {                  \
                       SET_PIN_INPUT();    \
                       OUTP_1();           \
                     } while (0)

/* Read one bit. */
#define INP()  (SERIAL_ID_PIN_READ & SERIAL_ID_PIN_MASK)

/*
 * Delay times in us.
 */
#define tA 6          /* min-5, recommended-6, max-15 */
#define tB 64         /* min-59, recommended-64, max-N/A */
#define tC 60         /* min-60, recommended-60, max-120 */
#define tD 10         /* min-5.3, recommended-10, max-N/A */
#define tE 9          /* min-0.3, recommended-9, max-9.3 */
#define tF 55         /* min-50, recommended-55, max-N/A */
#define tG 0          /* min-0, recommended-0, max-0 */
#define tH 480        /* min-480, recommended-480, max-640 */
#define tI 70         /* min-60.3, recommended-70, max-75.3 */
#define tJ 410        /* min-410, recommended-410, max-N/A */
/*---------------------------------------------------------------------------*/
/*
 * The delay caused by calling the delay_loop is given by the following
 * formula.
 *           delay(us) = (4n + 1)/XTAL
 * where n is the number of iterations and XTAL is the clock frequency(in MHz).
 * TODO: Moving the delay_loop to dev/clock.c
 */
static void
delay_loop(uint16_t __count)
{
  asm volatile ("1: sbiw %0,1" "\n\t"
                "brne 1b"
                : "=w" (__count)
                : "0" (__count)
               );
}
/*---------------------------------------------------------------------------*/
/*
 * This macro relies on the compiler doing the arithmetic during compile time
 * for the needed iterations.!!
 * In MICAz, XTAL = 7.3728 MHz
 */
#define udelay(u) delay_loop(((7.3728F * u)-1)/4)
/*---------------------------------------------------------------------------*/
static uint8_t
reset(void)
{
  uint8_t result;
  OW_DRIVE();
  udelay(500);     /* 480 < tH < 640 */
  OW_RELEASE();    /* Releases the bus */
  udelay(tI);
  result = INP();
  udelay(tJ);
  return result;
}
/*---------------------------------------------------------------------------*/
static void
write_byte(uint8_t byte)
{
  uint8_t i = 7;
  do {
    if (byte & 0x01) {
      OW_DRIVE();
      udelay(tA);
      OW_RELEASE();    /* Releases the bus */
      udelay(tB);
    } else {
      OW_DRIVE();
      udelay(tC);
      OW_RELEASE();    /* Releases the bus */
      udelay(tD);
    }
    if (i == 0)
      return;
    i--;
    byte >>= 1;
  } while (1);
}
/*---------------------------------------------------------------------------*/
static unsigned
read_byte(void)
{
  unsigned result = 0;
  int i = 7;
  do {
    OW_DRIVE();
    udelay(tA);
    OW_RELEASE();	/* Releases the bus */
    udelay(tE);
    if (INP())
      result |= 0x80;	/* LSbit first */
    udelay(tF);
    if (i == 0)
      return result;
    i--;
    result >>= 1;
  } while (1);
}
/*---------------------------------------------------------------------------*/
/* Polynomial ^8 + ^5 + ^4 + 1 */
static unsigned
crc8_add(unsigned acc, unsigned byte)
{
  int i;
  acc ^= byte;
  for (i = 0; i < 8; i++)
    if (acc & 1)
      acc = (acc >> 1) ^ 0x8c;
    else
      acc >>= 1;

  return acc;
}
/*---------------------------------------------------------------------------*/
int
ds2401_init()
{
  int i;
  uint8_t volatile sreg;
  unsigned family, crc, acc;

  PIN_INIT();

  sreg = SREG;    /* Save status register before disabling interrupts. */
  cli();    /* Disable interrupts. */

  if (reset() == 0) {
    write_byte(0x33);    /* Read ROM command. */
    family = read_byte();
    for (i = 7; i >= 2; i--) {
      ds2401_id[i] = read_byte();
    }
    crc = read_byte();

    SREG = sreg;    /* Enable interrupts. */

    if(family != 0x01) {
      goto fail;
    }
    acc = crc8_add(0x0, family);
    for (i = 7; i >= 2; i--) {
      acc = crc8_add(acc, ds2401_id[i]);
    }
    if (acc == crc) {
      /* 00:1A:4C    OUI for Crossbow Technology, Inc. */
      ds2401_id[0] = 0x00;
      ds2401_id[1] = 0x1A;
      ds2401_id[2] = 0x4C;
      return 1;	/* Success! */
    }
  } else {
    SREG = sreg;    /* Enable interrupts. */
  }

fail:
  memset(ds2401_id, 0x0, sizeof(ds2401_id));
  return 0;  /* Fail! */
}
/*---------------------------------------------------------------------------*/
