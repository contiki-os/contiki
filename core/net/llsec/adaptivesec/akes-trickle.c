/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
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
 *         Trickles HELLOs.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "net/llsec/adaptivesec/akes.h"
#include "net/llsec/adaptivesec/akes-trickle.h"
#include "sys/ctimer.h"
#include "sys/cc.h"
#include <string.h>

#ifdef AKES_TRICKLE_CONF_IMIN
#define IMIN (AKES_TRICKLE_CONF_IMIN * CLOCK_SECOND)
#else /* AKES_TRICKLE_CONF_IMIN */
#define IMIN MAX(30 * CLOCK_SECOND, 2 * AKES_MAX_WAITING_PERIOD * CLOCK_SECOND + CLOCK_SECOND)
#endif /* AKES_TRICKLE_CONF_IMIN */

#ifdef AKES_TRICKLE_CONF_IMAX
#define IMAX AKES_TRICKLE_CONF_IMAX
#else /* AKES_TRICKLE_CONF_IMAX */
#define IMAX 8
#endif /* AKES_TRICKLE_CONF_IMAX */

#ifdef AKES_TRICKLE_CONF_REDUNDANCY_CONSTANT
#define REDUNDANCY_CONSTANT AKES_TRICKLE_CONF_REDUNDANCY_CONSTANT
#else /* AKES_TRICKLE_CONF_REDUNDANCY_CONSTANT */
#define REDUNDANCY_CONSTANT 2
#endif /* AKES_TRICKLE_CONF_REDUNDANCY_CONSTANT */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

static uint16_t counter;
static uint8_t new_nbrs_count;
static clock_time_t interval_size;
static struct etimer trickle_timer;
static int was_stopped;
static int was_reset;
PROCESS(trickle_process, "trickle_process");

/*---------------------------------------------------------------------------*/
static void
reset_trickle_info(void)
{
  struct akes_nbr_entry *next;

  next = akes_nbr_head();
  while(next) {
    if(next->permanent) {
      next->permanent->sent_authentic_hello = 0;
    }
    next = akes_nbr_next(next);
  }
}
/*---------------------------------------------------------------------------*/
void
akes_trickle_on_fresh_authentic_hello(struct akes_nbr *sender)
{
  if(!sender->sent_authentic_hello) {
    sender->sent_authentic_hello = 1;
    counter++;
  }
}
/*---------------------------------------------------------------------------*/
void
akes_trickle_on_new_nbr(void)
{
  PRINTF("akes-trickle: New neighbor\n");

  if(++new_nbrs_count < MAX(akes_nbr_count(AKES_NBR_PERMANENT) / 4, 1)) {
    return;
  }

  /* reset Trickle */
  if(interval_size == IMIN) {
    PRINTF("akes-trickle: Not resetting Trickle since I = I_min\n");
    return;
  }

  PRINTF("akes-trickle: Resetting Trickle\n");
  interval_size = IMIN / 2;
  was_reset = 1;
  process_poll(&trickle_process);
}
/*---------------------------------------------------------------------------*/
void
akes_trickle_stop(void)
{
  was_stopped = 1;
  process_poll(&trickle_process);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(trickle_process, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("akes-trickle: Starting Trickle\n");

  akes_broadcast_hello();
  interval_size = IMIN / 2;

  while(!was_stopped) {
    /* interval expired */
    interval_size = MIN(2 * interval_size, IMIN << IMAX);
    new_nbrs_count = 0;
    counter = 0;
    was_reset = 0;

    etimer_set(&trickle_timer,
        adaptivesec_random_clock_time(interval_size / 2, interval_size - 1));
    PRINTF("akes-trickle: I=%lus t=%lus\n",
        interval_size / CLOCK_SECOND,
        trickle_timer.timer.interval / CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&trickle_timer)
        || was_reset
        || was_stopped);

    /* timeout */
    if(was_stopped) {
      break;
    }
    if(was_reset) {
      continue;
    }

    if(counter >= REDUNDANCY_CONSTANT) {
      PRINTF("akes-trickle: Suppressed HELLO\n");
    } else {
      PRINTF("akes-trickle: Broadcasting HELLO\n");
      akes_broadcast_hello();
      reset_trickle_info();
    }

    etimer_set(&trickle_timer,
        interval_size - trickle_timer.timer.interval);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&trickle_timer)
        || was_reset
        || was_stopped);
  }

  PRINTF("akes-trickle: Stopping Trickle\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
akes_trickle_start(void)
{
  process_start(&trickle_process, NULL);
}
/*---------------------------------------------------------------------------*/
