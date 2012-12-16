/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Hardware-dependent functions used for the cc2430 watchdog timer.
 *
 *         This file contains an ISR and must reside in the HOME bank.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "sys/energest.h"
#include "cc2430_sfr.h"
#include "contiki-conf.h"
#include "dev/watchdog-cc2430.h"

/*---------------------------------------------------------------------------*/
/* The watchdog only throws interrupts in timer mode */
#if WDT_TIMER_MODE
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
cc4230_watchdog_ISR(void) __interrupt(WDT_VECTOR)
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* Do something */
  IRCON2 &= ~WDTIF;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}
#pragma restore
#endif
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  WDCTL = WDT_TIMER_MODE | WDT_INTERVAL;

#if WDT_TIMER_MODE
  /* Enable the watchdog interrupts in timer mode */
  IEN2 |= WDTIE;
#endif
  return;
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  WDCTL |= WDT_EN;
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
#if WDT_TIMER_MODE
  /* In timer mode, all we need to do is write 1 to WDT:CLR[0] */
  WDCTL |= WDT_CLR0;
#else
  /* Write the 'clear' sequence while maintaining mode and interval setting */
  WDCTL = (WDCTL & 0x0F) | WDT_CLR3 | WDT_CLR1;
  WDCTL = (WDCTL & 0x0F) | WDT_CLR2 | WDT_CLR0;
#endif
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
#if WDT_TIMER_MODE
  /* In timer mode, the watchdog can actually be stopped */
  WDCTL &= ~WDT_EN;
  IRCON2 &= ~WDTIF;
#else
  /* In watchdog mode, stopping is impossible so we just reset the timer */
  watchdog_periodic();
#endif
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
#if WDT_TIMER_MODE
  /* Switch modes to watchdog, minimum interval, enable */
  WDCTL = WDT_EN | WDT_TIMEOUT_2_MSEC;
#else
  /* Let's get this over with ASAP */
  WDCTL = WDT_TIMEOUT_2_MSEC;
#endif
  /* Dis-acknowledge all interrupts while we wait for the dog to bark */
  DISABLE_INTERRUPTS();
  /* NOP till the dog barks... */
  while(1) {
    __asm
      nop
    __endasm;
  }
}
