/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: rtimer-arch.c,v 1.4 2010/03/23 13:13:17 fros4943 Exp $
 */

#include <signal.h>
#include <sys/time.h>
#include <stddef.h>

#include "sys/rtimer.h"
#include "sys/clock.h"
#include "sys/cooja_mt.h"

#include "lib/simEnvChange.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern clock_time_t simCurrentTime;

static int pending_rtimer = 0;
static rtimer_clock_t next_rtimer = 0;
static clock_time_t last_rtimer_now = 0;

/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  next_rtimer = 0;
  last_rtimer_now = 0;
  pending_rtimer = 0;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  next_rtimer = t;
  pending_rtimer = 1;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_next(void)
{
  return next_rtimer;
}
/*---------------------------------------------------------------------------*/
int
rtimer_arch_pending(void)
{
  return pending_rtimer;
}
/*---------------------------------------------------------------------------*/
int
rtimer_arch_check(void)
{
  if (simCurrentTime == next_rtimer) {
    /* Execute rtimer */
    pending_rtimer = 0;
    rtimer_run_next();
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
  if(last_rtimer_now == simCurrentTime) {
    /* Yield to COOJA, to allow time to change */
    simProcessRunValue = 1;
    simNextExpirationTime = simCurrentTime + 1;
    cooja_mt_yield();
  }
  last_rtimer_now = simCurrentTime;
  return simCurrentTime;
}
/*---------------------------------------------------------------------------*/

