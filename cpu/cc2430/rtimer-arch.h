/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         Hardware-dependent function declarations used to
 *         support the contiki rtimer module.
 *
 *         We use Timer 1 on the cc2431.
 *
 * \author
 *         Zach Shelby (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *           (rtimer-arch implementation for cc2430)
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "contiki-conf.h"
#include "cc2430_sfr.h"

/*
 * 32 MHz clock, prescaled down to 500 kHz for all 4 timers in clock_init().
 * Further prescaled factor 32 for T1, thus T1 is 15625 Hz
 */
#define RTIMER_ARCH_SECOND (15625U)

#define rtimer_arch_now() ((rtimer_clock_t)(T1CNTL + (T1CNTH << 8)))

void cc2430_timer_1_ISR(void) __interrupt(T1_VECTOR);

#endif /* __RTIMER_ARCH_H__ */
