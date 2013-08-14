/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Implementation of the clock functions for the cc243x
 * \author
 *         Zach Shelby (zach@sensinode.com) - original
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "sys/clock.h"
#include "sys/etimer.h"
#include "cc2430_sfr.h"
#include "sys/energest.h"

/* Sleep timer runs on the 32k RC osc. */
/* One clock tick is 7.8 ms */
#define TICK_VAL (32768/128)  /* 256 */
/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/
/* Used in sleep timer interrupt for calculating the next interrupt time */
static unsigned long timer_value;
static volatile CC_AT_DATA clock_time_t count = 0; /* Uptime in ticks */
static volatile CC_AT_DATA clock_time_t seconds = 0; /* Uptime in secs */
/*---------------------------------------------------------------------------*/
/**
 * Each iteration is ~1.0xy usec, so this function delays for roughly len usec
 */
void
clock_delay_usec(uint16_t len)
{
  DISABLE_INTERRUPTS();
  while(len--) {
    ASM(nop); ASM(nop);
    ASM(nop); ASM(nop);
  }
  ENABLE_INTERRUPTS();
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of ~8 ms (a tick)
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  CLKCON = OSC32K | TICKSPD2 | TICKSPD1; /* tickspeed 500 kHz for timers[1-4] */

  /* Initialize tick value */
  timer_value = ST0;                              /* ST low bits [7:0] */
  timer_value += ((unsigned long int) ST1) << 8;  /* middle bits [15:8] */
  timer_value += ((unsigned long int) ST2) << 16; /*   high bits [23:16] */
  timer_value += TICK_VAL;                        /* Init value 256 */
  ST2 = (unsigned char) (timer_value >> 16);
  ST1 = (unsigned char) (timer_value >> 8);
  ST0 = (unsigned char) timer_value;
  
  IEN0_STIE = 1; /* IEN0.STIE acknowledge Sleep Timer Interrupt */
}
/*---------------------------------------------------------------------------*/
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
clock_ISR(void) __interrupt(ST_VECTOR)
{
  DISABLE_INTERRUPTS();
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /*
   * If the Sleep timer throws an interrupt while we are powering down to
   * PM1, we need to abort the power down. Clear SLEEP.MODE, this will signal
   * main() to abort the PM1 transition
   */
  SLEEP &= 0xFC;

  /*
   * Read value of the ST0:ST1:ST2, add TICK_VAL and write it back.
   * Next interrupt occurs after the current time + TICK_VAL
   */
  timer_value = ST0;
  timer_value += ((unsigned long int) ST1) << 8;
  timer_value += ((unsigned long int) ST2) << 16;
  timer_value += TICK_VAL;
  ST2 = (unsigned char) (timer_value >> 16);
  ST1 = (unsigned char) (timer_value >> 8);
  ST0 = (unsigned char) timer_value;
  
  ++count;
  
  /* Make sure the CLOCK_CONF_SECOND is a power of two, to ensure
     that the modulo operation below becomes a logical and and not
     an expensive divide. Algorithm from Wikipedia:
     http://en.wikipedia.org/wiki/Power_of_two */
#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change CLOCK_CONF_SECOND in contiki-conf.h.
#endif
  if(count % CLOCK_CONF_SECOND == 0) {
    ++seconds;
  }
  
#if CLOCK_CONF_STACK_FRIENDLY
  sleep_flag = 1;
#else
  if(etimer_pending()
      && (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {
    etimer_request_poll();
  }
#endif
  
  IRCON_STIF = 0;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  ENABLE_INTERRUPTS();
}
#pragma restore
/*---------------------------------------------------------------------------*/
