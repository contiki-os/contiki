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
#define RTIMER_ARCH_SECOND 65536
/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void);

/* HW oscillator frequency is 32 kHz, not 64 kHz and RTIMER_NOW() never returns
 * an odd value; so US_TO_RTIMERTICKS always rounds to the nearest even number.
 */
#define US_TO_RTIMERTICKS(US)  (2 * ((US) >= 0 ?                        \
                               (((int32_t)(US) * (RTIMER_ARCH_SECOND / 2) + 500000) / 1000000L) :      \
                                ((int32_t)(US) * (RTIMER_ARCH_SECOND / 2) - 500000) / 1000000L))

#define RTIMERTICKS_TO_US(T)   ((T) >= 0 ?                     \
                               (((int32_t)(T) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                               ((int32_t)(T) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND))

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
