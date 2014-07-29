/*
 * Contiki PIC32 Port project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup pic32 PIC32 Contiki Port
 *
 * @{
 */

/**
 * \file   cpu/pic32/clock.c
 * \brief  Clock routines.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \author Daniele Alessandrelli <d.alessandrelli@sssup.it>
 * \date   2012-03-21
 */

#include <pic32_clock.h>
#include <pic32_timer.h>
#include <pic32_irq.h>

#include <sys/clock.h>
#include <sys/etimer.h>

#include "contiki.h"

static volatile clock_time_t ticks;
static volatile unsigned long seconds;

/*---------------------------------------------------------------------------*/
static inline void
clock_callback(void)
{
  ++ticks;

  if(etimer_pending()) {
    etimer_request_poll();
  }

#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change CLOCK_CONF_SECOND in contiki-conf.h.
#endif

  if((ticks % CLOCK_SECOND) == 0) {
    ++seconds;
  }
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return ticks;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  seconds = sec;
}
/*---------------------------------------------------------------------------*/
#define CLOCK_INIT(XX)                 \
void                                   \
clock_init(void)                       \
{                                      \
  ticks = 0;                           \
  seconds = 0;                         \
  pic32_timer##XX##_init(CLOCK_SECOND);\
  pic32_timer##XX##_enable_irq();      \
  pic32_timer##XX##_start();           \
}

#if PIC32_TIMER_CLOCK == 1
CLOCK_INIT(1)
#elif PIC32_TIMER_CLOCK == 2
CLOCK_INIT(2)
#elif PIC32_TIMER_CLOCK == 3
CLOCK_INIT(3)
#elif PIC32_TIMER_CLOCK == 4
CLOCK_INIT(4)
#elif PIC32_TIMER_CLOCK == 5
CLOCK_INIT(5)
#endif

/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t dt)
{
  uint32_t now;
  uint32_t stop;

  asm volatile("mfc0   %0, $9" : "=r"(now));

  /* The Count register is incremented every two system clock (SYSCLK) cycles. */

  stop = now + dt * ((pic32_clock_get_system_clock() / 1000000) / 2);

  for(;;) {
    asm volatile("mfc0   %0, $9" : "=r"(now));
    if((int32_t)(now - stop) >= 0) {
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Deprecated platform-specific routines.
 *
 */
inline void
clock_delay(unsigned int delay)
{
  clock_delay_usec(delay);
}
/*---------------------------------------------------------------------------*/

#if PIC32_TIMER_CLOCK == 1
TIMER_INTERRUPT(1, clock_callback);
#elif PIC32_TIMER_CLOCK == 2
TIMER_INTERRUPT(2, clock_callback);
#elif PIC32_TIMER_CLOCK == 3
TIMER_INTERRUPT(3, clock_callback);
#elif PIC32_TIMER_CLOCK == 4
TIMER_INTERRUPT(4, clock_callback);
#elif PIC32_TIMER_CLOCK == 5
TIMER_INTERRUPT(5, clock_callback);
#endif

/** @} */
