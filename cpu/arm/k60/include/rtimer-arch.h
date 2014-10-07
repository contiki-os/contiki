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
 *         K60 specific rtimer library header.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "contiki-conf.h"
#include "config-board.h"
#include "config-clocks.h"
#include "sys/rtimer.h"

/* Use first timer unless told otherwise. */
#ifdef RTIMER_ARCH_CONF_PIT_DEV
#define RTIMER_ARCH_PIT_DEV RTIMER_ARCH_CONF_PIT_DEV
#else
#define RTIMER_ARCH_PIT_DEV PIT
#endif

#ifdef RTIMER_ARCH_CONF_PIT_CHANNEL
#define RTIMER_ARCH_PIT_CHANNEL RTIMER_ARCH_CONF_PIT_CHANNEL
#else
#define RTIMER_ARCH_PIT_CHANNEL 0
#endif

#ifdef RTIMER_ARCH_CONF_PIT_IRQn
#define RTIMER_ARCH_PIT_IRQn RTIMER_ARCH_CONF_PIT_IRQn
#else
#define RTIMER_ARCH_PIT_IRQn PIT0_IRQn
#endif

#define RTIMER_ARCH_SECOND (F_BUS)

void rtimer_arch_set(rtimer_clock_t t);

rtimer_clock_t rtimer_arch_now(void);

#endif /* RTIMER_ARCH_H_ */
