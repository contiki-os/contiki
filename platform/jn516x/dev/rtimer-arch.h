/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 *         Header file for NXP jn516x-specific rtimer code
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *         Atis Elsts <atis.elsts@sics.se>
 */

#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "sys/rtimer.h"

#ifdef RTIMER_CONF_SECOND
# define RTIMER_ARCH_SECOND RTIMER_CONF_SECOND
#else
#if RTIMER_USE_32KHZ
# if JN516X_EXTERNAL_CRYSTAL_OSCILLATOR
#  define RTIMER_ARCH_SECOND 32768
# else
#  define RTIMER_ARCH_SECOND 32000
#endif
#else
/* 32MHz CPU clock => 16MHz timer */
# define RTIMER_ARCH_SECOND (F_CPU / 2)
#endif
#endif

#if RTIMER_USE_32KHZ
#define US_TO_RTIMERTICKS(US)  ((US) >= 0 ?                        \
                               (((int32_t)(US) * (RTIMER_ARCH_SECOND) + 500000) / 1000000L) :      \
                               ((int32_t)(US) * (RTIMER_ARCH_SECOND) - 500000) / 1000000L)

#define RTIMERTICKS_TO_US(T)   ((T) >= 0 ?                     \
                               (((int32_t)(T) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                               ((int32_t)(T) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND))

/* A 64-bit version because the 32-bit one cannot handle T >= 4295 ticks.
   Intended only for positive values of T. */
#define RTIMERTICKS_TO_US_64(T)  ((uint32_t)(((uint64_t)(T) * 1000000 + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))

#else

#define US_TO_RTIMERTICKS(D)    ((int64_t)(D) << 4)
#define RTIMERTICKS_TO_US(T)    ((int64_t)(T) >> 4)
#define RTIMERTICKS_TO_US_64(T) RTIMERTICKS_TO_US(T)

#endif

rtimer_clock_t rtimer_arch_now(void);

rtimer_clock_t rtimer_arch_time_to_rtimer(void);

void rtimer_arch_reinit(rtimer_clock_t sleep_start, rtimer_clock_t wakeup_time);

void clock_arch_init(int is_reinitialization);

void clock_arch_calibrate(void);

void clock_arch_reinit(void);

void clock_arch_schedule_interrupt(clock_time_t time_to_etimer, rtimer_clock_t ticks_to_rtimer);

clock_t clock_arch_time_to_etimer(void);

/* Use 20 ms: enough for TSCH with the default schedule to sleep */
#define JN516X_MIN_SLEEP_TIME (RTIMER_SECOND / 50)
/* 1 second by default: arbitrary picked value which could be increased */
#define JN516X_MAX_SLEEP_TIME RTIMER_SECOND
/* Assume conservative 10 ms maximal system wakeup time */
#define JN516X_SLEEP_GUARD_TIME (RTIMER_ARCH_SECOND / 100)

#define WAKEUP_TIMER      E_AHI_WAKE_TIMER_0
#define WAKEUP_TIMER_MASK E_AHI_SYSCTRL_WK0_MASK

#define TICK_TIMER        E_AHI_WAKE_TIMER_1
#define TICK_TIMER_MASK   E_AHI_SYSCTRL_WK1_MASK

#define WAIT_FOR_EDGE(edge_t) do {                            \
    uint64_t start_t = u64AHI_WakeTimerReadLarge(TICK_TIMER); \
    do {                                                      \
      edge_t = u64AHI_WakeTimerReadLarge(TICK_TIMER);         \
    } while(edge_t == start_t);                               \
  } while(0)

#endif /* RTIMER_ARCH_H_ */
