/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-timers cc2538dk Timer Test Project
 *
 *    This example tests the correct functionality of clocks and timers.
 *
 *    More specifically, it tests clock_seconds, rtimers, etimers and
 *    clock_delay_usec.
 *
 *    This is largely-based on the same example of the cc2530 port.
 * @{
 *
 * \file
 *         Tests related to clocks and timers
 */
#include "contiki.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define TIMER_TEST_CONF_TEST_CLOCK_DELAY_USEC 1
#define TIMER_TEST_CONF_TEST_RTIMER           1
#define TIMER_TEST_CONF_TEST_ETIMER           1
#define TIMER_TEST_CONF_TEST_CLOCK_SECONDS    1
/*---------------------------------------------------------------------------*/
static struct etimer et;

#if TIMER_TEST_CONF_TEST_CLOCK_DELAY_USEC
static rtimer_clock_t start_count, end_count, diff;
#endif

#if TIMER_TEST_CONF_TEST_CLOCK_SECONDS
static unsigned long sec;
#endif

#if TIMER_TEST_CONF_TEST_ETIMER
static clock_time_t count;
#endif

#if TIMER_TEST_CONF_TEST_RTIMER
static struct rtimer rt;
rtimer_clock_t rt_now, rt_for;
static clock_time_t ct;
#endif

static uint8_t i;
/*---------------------------------------------------------------------------*/
PROCESS(timer_test_process, "Timer test process");
AUTOSTART_PROCESSES(&timer_test_process);
/*---------------------------------------------------------------------------*/
#if TIMER_TEST_CONF_TEST_RTIMER
void
rt_callback(struct rtimer *t, void *ptr)
{
  rt_now = RTIMER_NOW();
  ct = clock_time();
  printf("Task called at %lu (clock = %lu)\n", rt_now, ct);
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(timer_test_process, ev, data)
{

  PROCESS_BEGIN();

  etimer_set(&et, 2 * CLOCK_SECOND);

  PROCESS_YIELD();

#if TIMER_TEST_CONF_TEST_CLOCK_DELAY_USEC
  printf("-----------------------------------------\n");
  printf("clock_delay_usec test, (10,000 x i) usec:\n");
  printf("N.B. clock_delay_usec is more accurate than rtimers\n");
  i = 1;
  while(i < 7) {
    start_count = RTIMER_NOW();
    clock_delay_usec(10000 * i);
    end_count = RTIMER_NOW();
    diff = end_count - start_count;
    printf("Requested: %u usec, Real: %lu rtimer ticks = ~%lu us\n",
           10000 * i, diff, diff * 1000000 / RTIMER_SECOND);
    i++;
  }
#endif

#if TIMER_TEST_CONF_TEST_RTIMER
  printf("-----------------------------------------\n");
  printf("Rtimer Test, 1 sec (%u rtimer ticks):\n", RTIMER_SECOND);
  i = 0;
  while(i < 5) {
    etimer_set(&et, 2 * CLOCK_SECOND);
    printf("=======================\n");
    ct = clock_time();
    rt_now = RTIMER_NOW();
    rt_for = rt_now + RTIMER_SECOND;
    printf("Now=%lu (clock = %lu) - For=%lu\n", rt_now, ct, rt_for);
    if(rtimer_set(&rt, rt_for, 1, rt_callback, NULL) != RTIMER_OK) {
      printf("Error setting\n");
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    i++;
  }
#endif

#if TIMER_TEST_CONF_TEST_ETIMER
  printf("-----------------------------------------\n");
  printf("Clock tick and etimer test, 1 sec (%u clock ticks):\n",
         CLOCK_SECOND);
  i = 0;
  while(i < 10) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    count = clock_time();
    printf("%lu ticks\n", count);

    leds_toggle(LEDS_RED);
    i++;
  }
#endif

#if TIMER_TEST_CONF_TEST_CLOCK_SECONDS
  printf("-----------------------------------------\n");
  printf("Clock seconds test (5s):\n");
  i = 0;
  while(i < 10) {
    etimer_set(&et, 5 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    sec = clock_seconds();
    printf("%lu seconds\n", sec);

    leds_toggle(LEDS_GREEN);
    i++;
  }
#endif

  printf("Done!\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
