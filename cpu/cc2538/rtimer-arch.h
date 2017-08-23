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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-rtimer cc2538 rtimer
 *
 * Implementation of the rtimer module for the cc2538
 *
 * The rtimer runs on the Sleep Timer. This is a design choice, as many parts
 * of Contiki like rtimers with a value of RTIMER_ARCH_SECOND being a power of
 * two. The ST runs on the 32kHz clock, which can provide us with an excellent
 * value of 32768 for RTIMER_ARCH_SECOND.
 *
 * Additionally, since the ST keeps running in PM2, we can do things like drop
 * to PM2 and schedule a wake-up time through the rtimer API.
 *
 * \note If the 32kHz clock is running on the 32kHz RC OSC, the rtimer is
 * not 100% accurate (the RC OSC does not run at exactly 32.768 kHz). For
 * applications requiring higher accuracy, the 32kHz clock should be changed to
 * use the XOSC as its source. To see which low-frequency OSC the 32kHz clock
 * is running on, see cpu/cc2538/clock.c.
 *
 * \sa cpu/cc2538/clock.c
 * @{
 */
/**
 * \file
 * Header file for the cc2538 rtimer driver
 */
#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "contiki.h"
#include "dev/gptimer.h"

#define RTIMER_ARCH_SECOND 32768

/* Do the math in 32bits to save precision.
 * Round to nearest integer rather than truncate. */
#define US_TO_RTIMERTICKS(US)  ((US) >= 0 ?                        \
                               (((int32_t)(US) * (RTIMER_ARCH_SECOND) + 500000) / 1000000L) :      \
                               ((int32_t)(US) * (RTIMER_ARCH_SECOND) - 500000) / 1000000L)

#define RTIMERTICKS_TO_US(T)   ((T) >= 0 ?                     \
                               (((int32_t)(T) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                               ((int32_t)(T) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND))

/* A 64-bit version because the 32-bit one cannot handle T >= 4295 ticks.
   Intended only for positive values of T. */
#define RTIMERTICKS_TO_US_64(T)  ((uint32_t)(((uint64_t)(T) * 1000000 + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))

/** \sa RTIMER_NOW() */
rtimer_clock_t rtimer_arch_now(void);

/**
 * \brief Get the time of the next scheduled rtimer trigger
 * \return The time next rtimer ISR is scheduled for
 */
rtimer_clock_t rtimer_arch_next_trigger(void);

#endif /* RTIMER_ARCH_H_ */

/**
 * @}
 * @}
 */
