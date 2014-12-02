/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         K60 specific rtimer library implementation.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "rtimer-arch.h"
#include "K60.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Convenience macro to access the channel struct directly. */
#define RTIMER_ARCH_CHANNEL (RTIMER_ARCH_PIT_DEV->CHANNEL[RTIMER_ARCH_PIT_CHANNEL])
/* Convenience macro to enable and disable the rtimer timer hardware. */
#define RTIMER_ARCH_TIMER_ENABLE() (BITBAND_REG(RTIMER_ARCH_CHANNEL.TCTRL, PIT_TCTRL_TEN_SHIFT) = 1)
#define RTIMER_ARCH_TIMER_DISABLE() (BITBAND_REG(RTIMER_ARCH_CHANNEL.TCTRL, PIT_TCTRL_TEN_SHIFT) = 0)

/** Offset between current counter/timer and t=0 (boot instant) */
static rtimer_clock_t offset;
/** set to 0 while a task is scheduled */
static int free_running;
/** LDVAL of the free running timer */
#define RTIMER_ARCH_FREE_RUNNING_LDVAL (PIT_LDVAL_TSV_MASK >> PIT_LDVAL_TSV_SHIFT)

void
rtimer_arch_init(void) {
  offset = 0;

  /* Free running when not waiting for any rtimer tasks. */
  free_running = 1;

  /* Enable module clock */
  BITBAND_REG(SIM->SCGC6, SIM_SCGC6_PIT_SHIFT) = 1;

  /* Disable timer until we have something to schedule. */
  RTIMER_ARCH_TIMER_DISABLE();

  /* Enable interrupt generation */
  BITBAND_REG(RTIMER_ARCH_CHANNEL.TCTRL, PIT_TCTRL_TIE_SHIFT) = 1;

  /* Clear interrupt flag */
  BITBAND_REG(RTIMER_ARCH_CHANNEL.TFLG, PIT_TFLG_TIF_MASK) = 1;

  /* Load largest possible value to begin counting time for RTIMER_NOW. */
  RTIMER_ARCH_CHANNEL.LDVAL = RTIMER_ARCH_FREE_RUNNING_LDVAL;

  /* Enable to load the value. */
  RTIMER_ARCH_TIMER_ENABLE();

  /* Enable interrupts for PIT module in NVIC */
  NVIC_EnableIRQ(RTIMER_ARCH_PIT_IRQn);

#if DEBUG
  int64_t err = ((int64_t)SystemBusClock) - ((int64_t)RTIMER_SECOND);
  err *= (int64_t)1000000ull;
  err /= (int64_t)(RTIMER_SECOND);
  PRINTF("rtimer_arch_init: Real rtimer frequency = %lu, RTIMER_SECOND = %lu\n", (unsigned long)SystemBusClock, (unsigned long)RTIMER_SECOND);
  PRINTF("rtimer_arch_init: Expect ca %ld ppm error on scheduled task times.\n", (signed long)err);
#endif
  PRINTF("rtimer_arch_init: Done\n");
}

void
rtimer_arch_schedule(rtimer_clock_t t) {
  if (t > 0xffffffffull) {
    PRINTF("rtimer_arch_schedule: Schedule out of range! This task will run much sooner than expected.\n");
  }
  /* Increase time base */
  offset = rtimer_arch_now();

  /* Set timer value */
  RTIMER_ARCH_CHANNEL.LDVAL = PIT_LDVAL_TSV(t);

  /* No longer free running */
  free_running = 0;

  /* Disable timer and re-enable to load the new value. */
  RTIMER_ARCH_TIMER_DISABLE();
  RTIMER_ARCH_TIMER_ENABLE();

  PRINTF("rtimer_arch_schedule: %lu\n", (unsigned long)t);
}

rtimer_clock_t
rtimer_arch_now(void) {
  /* Timer is down-counting, we flip it here. */
  return offset + ((rtimer_clock_t)(
    ((RTIMER_ARCH_CHANNEL.LDVAL & PIT_LDVAL_TSV_MASK) >> PIT_LDVAL_TSV_SHIFT) -
    ((RTIMER_ARCH_CHANNEL.CVAL & PIT_CVAL_TVL_MASK) >> PIT_CVAL_TVL_SHIFT)));
}

/* Interrupt handler for rtimer triggers */
void
_isr_pit0(void) {
  /* Save old timeout */
  static uint32_t prev_timeout;
  prev_timeout = ((RTIMER_ARCH_CHANNEL.LDVAL & PIT_LDVAL_TSV_MASK) >> PIT_LDVAL_TSV_SHIFT);

  /* Clear interrupt flag */
  BITBAND_REG(RTIMER_ARCH_CHANNEL.TFLG, PIT_TFLG_TIF_SHIFT) = 1;

  /* Update offset with the time that has passed since the counter reached zero.
   * This time offset could potentially be significant, for example when
   * multiple interrupts occur right before the PIT interrupt causing the PIT
   * interrupt to become pending while the other interrupts are handled, or
   * interrupts with a higher priority cause this ISR to be */
  /* After triggering this interrupt we are currently in, the counter will start
   * over at LDVAL and continue counting down. This new time is added to the
   * offset by the line below, however, we still have not added the initial
   * length of the timer that actually triggered the interrupt. */
  offset = rtimer_arch_now(); /* offset = [old offset] + [ISR processing time] */

  /* rtimer schedulings are one-shot, go back to counting time for RTIMER_NOW */
  /* Load largest possible value */
  RTIMER_ARCH_CHANNEL.LDVAL = RTIMER_ARCH_FREE_RUNNING_LDVAL;

  RTIMER_ARCH_TIMER_DISABLE();
  RTIMER_ARCH_TIMER_ENABLE();
  /* We do the above few lines as close as possible to the
   * offset = rtimer_arch_now(); in order to avoid losing a lot of ticks. */

  /* Increase time base for RTIMER_NOW by the initial timer amount. */
  offset += (rtimer_clock_t)prev_timeout;
  /* offset is now [offset before ISR] + [ISR processing time] + [timer interval] */

  /** \todo compensate for time between offset = rtimer_arch_now() and RTIMER_ARCH_TIMER_ENABLE in rtimer ISR. */

  if (free_running == 0) {
    /* We hit a scheduled task time. */
    free_running = 1;
    /* Call higher level rtimer module */
    rtimer_run_next();
  }
  /* Else: Timeout while counting time (happens after 89 seconds at 48MHz).
   *  nothing to do here then but to continue counting. */
}
