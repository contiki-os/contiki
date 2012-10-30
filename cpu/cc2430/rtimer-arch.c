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
 *
 */

/**
 * \file
 *         Hardware-dependent functions used to support the
 *         contiki rtimer module.
 *
 *         clock and etimer are using the sleep timer on the cc2430
 *
 *         clock_init() has set our tick speed prescaler already, so we
 *         are ticking with 500 kHz freq.
 *
 *         rtimer_clock_t is unsigned short (16bit on the cc2430)
 *         It thus makes sense to use the 16bit clock (Timer 1)
 *
 *         This file contains an ISR and must reside in the HOME bank
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "sys/rtimer.h" /* Includes rtimer-arch.h for us */
#include "cc2430_sfr.h"
#include "sys/energest.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  PRINTF("rtimer_arch_init() ");
  /*
   * - Free running mode
   * - Prescale by 32:
   *   Tick Speed has been prescaled to 500 kHz already in clock_init()
   *   We further prescale by 32 resulting in 15625 Hz for this timer.
   */
  T1CTL = (T1DIV1 | T1MODE0); /* 00001001 */
  PRINTF("T1CTL=0x%02x\n", T1CTL);
  /* Acknowledge Timer 1 Interrupts */
  IEN1_T1IE = 1;
  PRINTF("IEN1_T1IE=0x%02x\n", IEN1_T1IE);

  /* Timer 1, Channel 1. Compare Mode (0x04), Interrupt mask on (0x40) */
  T1CCTL1 = T1MODE + T1IM;
  PRINTF("T1CCTL1=0x%02x\n", T1CCTL1);

  /* Interrupt Mask Flags: No interrupt on overflow */
  TIMIF &= ~OVFIM;
  PRINTF("TIMIF=0x%02x\n", TIMIF);

  PRINTF("done\n");
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  PRINTF("rtimer_arch_schedule(%u)\n", t);

  /* set the compare mode values so we can get an interrupt after t */
  T1CC1L = (unsigned char) t;
  T1CC1H = (unsigned char) (t >> 8);
  PRINTF("T1CC1=%u, t=%u\n", (T1CC1L + (T1CC1H << 8)), t);

  /* Turn on compare mode interrupt */
  PRINTF("T1CTL=0x%02x\n", T1CTL);
  T1CCTL1 |= T1IM;
}
/*---------------------------------------------------------------------------*/
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
cc2430_timer_1_ISR(void) __interrupt (T1_VECTOR)
{
  IEN1_T1IE = 0; /* Ignore Timer 1 Interrupts */
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* No more interrupts from Channel 1 till next rtimer_arch_schedule() call.
   * Setting the mask will instantly generate an interrupt so we clear the
   * flag first. */
  T1CTL &= ~(CH1IF);
  T1CCTL1 &= ~T1IM;

  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  IEN1_T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}
#pragma restore
