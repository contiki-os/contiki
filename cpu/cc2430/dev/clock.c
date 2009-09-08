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
 *
 * $Id: clock.c,v 1.1 2009/09/08 20:07:35 zdshelby Exp $
 */

/**
 * \file
 *         Implementation of the clock functions for the 8051 CPU
 * \author
 *         Zach Shelby (zach@sensinode.com)
 */

/**
 * TODO: Implement clock_fine() and clock_fine_max_ticks() using another timer?
 */

#include <stdio.h> /*for debug printf*/
#include "sys/clock.h"
#include "sys/etimer.h"
#include "cc2430_sfr.h"


/*Sleep timer runs on the 32k RC osc. */
/* One clock tick is 7.8 ms */
#define TICK_VAL (32768/128)

#define MAX_TICKS (~((clock_time_t)0) / 2)

/* Used in sleep timer interrupt for calculating the next interrupt time */
static unsigned long timer_value;
/*starts calculating the ticks right after reset*/
static volatile clock_time_t count = 0;
/*calculates seconds*/
static volatile clock_time_t seconds = 0;

/*---------------------------------------------------------------------------*/
/**
 * One delay is about 0.6 us, so this function delays for len * 0.6 us
 */
void
clock_delay(unsigned int len)
{
  unsigned int i;
  for(i = 0; i< len; i++) {
    __asm
      nop
      __endasm;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of ~8 ms (a tick)
 */
void
clock_wait(int i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
clock_time_t
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
  CLKCON = OSC32K |  TICKSPD2|TICKSPD1|TICKSPD0;	/*tickspeed 250 kHz*/
  
  /*Initialize tick value*/
  timer_value = ST0;									/*sleep timer 0. low bits [7:0]*/
  timer_value += ((unsigned long int)ST1) << 8;		/*middle bits [15:8]*/
  timer_value += ((unsigned long int)ST2) << 16;		/*high bits [23:16]*/
  timer_value += TICK_VAL;							/*init value 256*/
  ST2 = (unsigned char) (timer_value >> 16);
  ST1 = (unsigned char) (timer_value >> 8);
  ST0 = (unsigned char) timer_value;
  
  IEN0 |= STIE;		/*interrupt enable for sleep timers. STIE=Interrupt mask, CPU. */
}
/*---------------------------------------------------------------------------*/
void
cc2430_clock_ISR( void ) __interrupt (ST_VECTOR)
{
  IEN0_EA = 0;	/*interrupt disable*/
  /* When using the cooperative scheduler the timer 2 ISR is only
     required to increment the RTOS tick count. */
  
  /*Read value of the ST0,ST1,ST2 and then add TICK_VAL and write it back.
    Next interrupt occurs after the current time + TICK_VAL*/
  timer_value = ST0;
  timer_value += ((unsigned long int)ST1) << 8;
  timer_value += ((unsigned long int)ST2) << 16;
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
  
  if(etimer_pending() &&
     (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {	/*core/sys/etimer.c*/
    etimer_request_poll();
  }
  
  IRCON &= ~STIF;		/*IRCON.STIF=Sleep timer interrupt flag. This flag called this interrupt func, now reset it*/
  IEN0_EA = 1;		/*interrupt enable*/
}
/*---------------------------------------------------------------------------*/
