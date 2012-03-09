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
 * @(#)$Id: watchdog.c,v 1.12 2010/11/12 15:54:41 nifi Exp $
 */

#include "contiki-conf.h"
#include "dev/watchdog.h"
#include "isr_compat.h"

static int counter = 0;

#define PRINT_STACK_ON_REBOOT 0

/*---------------------------------------------------------------------------*/
#if PRINT_STACK_ON_REBOOT
#ifdef CONTIKI_TARGET_SKY
static void
printchar(char c)
{
  /* Transmit the data. */
  TXBUF1 = c;

  /* Loop until the transmission buffer is available. */
  while((SFRIFG2 & UTXIFG1) == 0);

}
/*---------------------------------------------------------------------------*/
static void
hexprint(uint8_t v)
{
  const char hexconv[] = "0123456789abcdef";
  printchar(hexconv[v >> 4]);
  printchar(hexconv[v & 0x0f]);
}
/*---------------------------------------------------------------------------*/
static void
printstring(char *s)
{
  while(*s) {
    printchar(*s++);
  }
}
#endif /* CONTIKI_TARGET_SKY */
#endif /* PRINT_STACK_ON_REBOOT */
/*---------------------------------------------------------------------------*/
ISR(WDT, watchdog_interrupt)
{
#ifdef CONTIKI_TARGET_SKY
#if PRINT_STACK_ON_REBOOT
  uint8_t dummy;
  static uint8_t *ptr;
  static int i;

  ptr = &dummy;
  printstring("Watchdog reset");
  printstring("\nStack at $");
  hexprint(((int)ptr) >> 8);
  hexprint(((int)ptr) & 0xff);
  printstring(":\n");

  for(i = 0; i < 64; ++i) {
    hexprint(ptr[i]);
    printchar(' ');
    if((i & 0x0f) == 0x0f) {
      printchar('\n');
    }
  }
  printchar('\n');
#endif /* PRINT_STACK_ON_REBOOT */
#endif /* CONTIKI_TARGET_SKY */

  watchdog_reboot();
}
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  /* The MSP430 watchdog is enabled at boot-up, so we stop it during
     initialization. */
  counter = 0;
  watchdog_stop();

  SFRIFG1 &= ~WDTIFG;
  SFRIE1 |= WDTIE;
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  /* We setup the watchdog to reset the device after one second,
     unless watchdog_periodic() is called. */
  counter--;
  if(counter == 0) {
    WDTCTL = WDTPW | WDTCNTCL | WDT_ARST_1000 | WDTTMSEL;
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  /* This function is called periodically to restart the watchdog
     timer. */
  /*  if(counter < 0) {*/
    WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL | WDTTMSEL;
    /*  }*/
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  counter++;
  if(counter == 1) {
    WDTCTL = WDTPW | WDTHOLD;
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  WDTCTL = 0;
}
/*---------------------------------------------------------------------------*/
