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
 *         clock_init() has set our tick speed prescaler already, so we
 *         are ticking with 500 kHz freq.
 *
 *         Contiki typedefs rtimer_clock_t as unsigned short (16bit)
 *         It thus makes sense to use the 16bit timer (Timer 1)
 *
 *         This file contains an ISR and must reside in the HOME bank
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "sys/rtimer.h"
#include "sfr-bits.h"
#include "cc253x.h"
#include "sys/energest.h"

#include "debug.h"
#include <stdio.h>

#define RT_MODE_COMPARE() do { T1CCTL1 |= T1CCTL_MODE; } while(0)
#define RT_MODE_CAPTURE() do { T1CCTL1 &= ~T1CCTL_MODE; } while(0)
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /*
   * - Free running mode
   * - Prescale by 32:
   *   Tick Speed has been prescaled to 500 kHz already in clock_init()
   *   We further prescale by 32 resulting in 15625 Hz for this timer.
   */
  T1CTL = (T1CTL_DIV1 | T1CTL_MODE0);

  T1STAT = 0;

  /* Timer 1, Channel 1. Compare Mode (0x04), Interrupt mask on (0x40) */
  T1CCTL1 = T1CCTL_MODE | T1CCTL_IM;

  /* Interrupt Mask Flags: No interrupt on overflow */
  OVFIM = 0;

  /* Acknowledge Timer 1 Interrupts */
  T1IE = 1;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  /* Switch to capture mode before writing T1CC1x and
   * set the compare mode values so we can get an interrupt after t */
  RT_MODE_CAPTURE();
  T1CC1L = (unsigned char)t;
  T1CC1H = (unsigned char)(t >> 8);
  RT_MODE_COMPARE();

  /* Turn on compare mode interrupt */
  T1STAT = 0;
  T1CCTL1 |= T1CCTL_IM;
}
/*---------------------------------------------------------------------------*/
/* avoid referencing bits, we don't call code which use them */
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
rtimer_isr(void) __interrupt(T1_VECTOR)
{
  T1IE = 0; /* Ignore Timer 1 Interrupts */
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* No more interrupts from Channel 1 till next rtimer_arch_schedule() call */
  T1STAT &= ~T1STAT_CH1IF;
  T1CCTL1 &= ~T1CCTL_IM;

  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}
#pragma restore
