/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-clocks
 * @{
 *
 * \defgroup cc26xx-rtimer CC13xx/CC26xx rtimer
 *
 * Implementation of the rtimer module for the CC13xx/CC26xx
 * @{
 */
/**
 * \file
 * Header file for the CC13xx/CC26xx rtimer driver
 */
/*---------------------------------------------------------------------------*/
#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/
// this is hardware specified default RTC sub-second resolution
#define RTIMER_ARCH_SECOND_NORM 65536u

#ifdef RTIMER_CONF_ARCH_SECOND
#if RTIMER_CONF_ARCH_SECOND >= (RTIMER_ARCH_SECOND_NORM*2)
#error  RTC inc value not support such high rate, use below 128k
#endif
#define RTIMER_ARCH_SECOND RTIMER_CONF_ARCH_SECOND
#else
#define RTIMER_ARCH_SECOND RTIMER_ARCH_SECOND_NORM
#endif
/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void);

/* HW oscillator frequency is 32 kHz, not 64 kHz and RTIMER_NOW() never returns
 * an odd value; so US_TO_RTIMERTICKS always rounds to the nearest even number.
 */
#if RTIMER_ARCH_SECOND >= (RTIMER_ARCH_SECOND_NORM*3/4)
#define RTIMER_ARCH_TOL  (RTIMER_ARCH_SECOND/0x8000ul)
#else
#define RTIMER_ARCH_TOL  1u
#endif

// this scale used for achieve maximum arguments range, avoiding overload on int32 value
#define RTIMER_USCALC_SCALE     32u
#define RTIMER_US               1000000ul
#define RTIMER_SCALED_US        (RTIMER_US/RTIMER_USCALC_SCALE)
#define RTIMER_SCALED_HALFUS    (RTIMER_US/RTIMER_USCALC_SCALE/2u)
#define RTIMER_SCALED_SECOND    (RTIMER_ARCH_SECOND/RTIMER_USCALC_SCALE)
#if ( (RTIMER_SCALED_SECOND/2u*RTIMER_USCALC_SCALE) != (RTIMER_ARCH_SECOND/2u) )
    || ( (RTIMER_SCALED_HALFUS *RTIMER_USCALC_SCALE) != (RTIMER_US/2u) )
#error please fix RTIMER_USCALC_SCALE selection for your RTIMER_ARCH_SECOND!!!
#endif

#define US_TO_RTIMERTICKS(US)  (RTIMER_ARCH_TOL * ( ((US) >= 0) ?                        \
                               (((uint32_t)(US) * (RTIMER_SCALED_SECOND / RTIMER_ARCH_TOL) + RTIMER_SCALED_US/2u) / RTIMER_SCALED_US) :      \
                                -(((uint32_t)(-US) * (RTIMER_SCALED_SECOND / RTIMER_ARCH_TOL) + RTIMER_SCALED_US/2u) / RTIMER_SCALED_US)\
                                ) )

#define RTIMERTICKS_TO_US(T)   (((T) >= 0) ?                     \
                               (((uint32_t)(T) * RTIMER_SCALED_US + ((RTIMER_SCALED_SECOND) / 2u)) / (RTIMER_SCALED_SECOND)) : \
                               -(((uint32_t)(-T) * RTIMER_SCALED_US + ((RTIMER_SCALED_SECOND) / 2u)) / (RTIMER_SCALED_SECOND))\
                               )

/*
#if US_TO_RTIMERTICKS(RTIMERTICKS_TO_US(10000)) != 10000
#error fix US_TO_RTIMERTICKS, RTIMERTICKS_TO_US functions !!!
#endif
#if US_TO_RTIMERTICKS(RTIMERTICKS_TO_US(-10000)) != -10000
#error fix US_TO_RTIMERTICKS, RTIMERTICKS_TO_US functions !!!
#endif
*/
/* A 64-bit version because the 32-bit one cannot handle T >= 4295 ticks.
   Intended only for positive values of T. */
#define RTIMERTICKS_TO_US_64(T)  ((uint32_t)(((uint64_t)(T) * 1000000 + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))
/*---------------------------------------------------------------------------*/
#endif /* RTIMER_ARCH_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
