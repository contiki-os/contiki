/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: rtimer-arch.c,v 1.4 2007/05/22 20:59:47 adamdunkels Exp $
 */

/**
 * \file
 *         MSP430-specific rtimer code
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <io.h>
#include <signal.h>

#include "lib/energest.h"
#include "sys/rtimer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static rtimer_clock_t offset;

/*---------------------------------------------------------------------------*/
interrupt(TIMERB1_VECTOR) timerb1 (void) {
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(TBIV == 2) {
    rtimer_run_next();
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  dint();

  offset = 0;
  
  /* Select SMCLK (2.4576MHz), clear TAR; This makes the rtimer count
     the number of processor cycles executed by the CPU. */
  //TBCTL = TBSSEL1 | TBCLR;
  /* Select ACLK 32768Hz clock, divide by 8 */
  TBCTL = TBSSEL0 | TBCLR | ID_3;

  /* CCR1 interrupt enabled, interrupt occurs when timer equals CCR1. */
  TBCCTL1 = CCIE;

  /* Start Timer_B in continuous mode. */
  TBCTL |= MC1;

  BCSCTL1 &= ~(DIVA1 + DIVA0);   /* remove /8 divisor from ACLK again    */

  /* Enable interrupts. */
  eint();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  PRINTF("rtimer_arch_schedule time %u\n", t);
  TBCCR1 = t + offset;
}
/*---------------------------------------------------------------------------*/
/*rtimer_clock_t
rtimer_arch_now(void)
{
  return TBR + offset;
}*/
/*---------------------------------------------------------------------------*/
void
rtimer_arch_set(rtimer_clock_t t)
{
  offset = t - TBR;
}
/*---------------------------------------------------------------------------*/
