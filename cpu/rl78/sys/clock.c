/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Ian Martin <martini@redwirellc.com>
 */

#include <time.h>

#include "contiki.h"

#ifndef BIT
#define BIT(n) (1 << (n))
#endif

#define clock() (0xffff - TCR[CLOCK_CHANNEL])

void
clock_init(void)
{
#if (CLOCK_CHANNEL <= 7)
  TAU0EN = 1; /* Enable Timer Array Unit 0. */
  TT0 = 0x00ff; /* Stop the Timer Array Unit. */
  TPS0 = (TPS0 & 0xfff0) | CLOCK_SCALER;
  TMR[CLOCK_CHANNEL] = 0x0000; /* default value */

#if (CLOCK_CHANNEL == 0)
  TDR00 = 0xffff;
#elif (CLOCK_CHANNEL == 1)
  TDR01 = 0xffff;
#elif (CLOCK_CHANNEL == 2)
  TDR02 = 0xffff;
#elif (CLOCK_CHANNEL == 3)
  TDR03 = 0xffff;
#elif (CLOCK_CHANNEL == 4)
  TDR04 = 0xffff;
#elif (CLOCK_CHANNEL == 5)
  TDR05 = 0xffff;
#elif (CLOCK_CHANNEL == 6)
  TDR06 = 0xffff;
#elif (CLOCK_CHANNEL == 7)
  TDR07 = 0xffff;
#else
#error Invalid CLOCK_CHANNEL
#endif

  TE0 |= BIT(CLOCK_CHANNEL); /* Start timer channel 0. */
  TS0 |= BIT(CLOCK_CHANNEL); /* Start counting. */
#else
  TAU1EN = 1; /* Enable Timer Array Unit 1. */
  TT1 = 0x00ff; /* Stop the Timer Array Unit. */
  TPS1 = (TPS1 & 0xfff0) | CLOCK_SCALER;
  TMR[CLOCK_CHANNEL] = 0x0000; /* default value */

#if (CLOCK_CHANNEL == 8)
  TDR00 = 0xffff;
#elif (CLOCK_CHANNEL == 9)
  TDR01 = 0xffff;
#elif (CLOCK_CHANNEL == 10)
  TDR02 = 0xffff;
#elif (CLOCK_CHANNEL == 11)
  TDR03 = 0xffff;
#elif (CLOCK_CHANNEL == 12)
  TDR04 = 0xffff;
#elif (CLOCK_CHANNEL == 13)
  TDR05 = 0xffff;
#elif (CLOCK_CHANNEL == 14)
  TDR06 = 0xffff;
#elif (CLOCK_CHANNEL == 15)
  TDR07 = 0xffff;
#else
#error Invalid CLOCK_CHANNEL
#endif

  TE1 |= BIT(CLOCK_CHANNEL); /* Start timer channel. */
  TS1 |= BIT(CLOCK_CHANNEL); /* Start counting. */
#endif
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return clock();
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return clock() / CLOCK_CONF_SECOND;
}
/*---------------------------------------------------------------------------*/

void
clock_wait(clock_time_t t)
{
  clock_time_t t0;
  t0 = clock();
  while(clock() - t0 < t) ;
}
