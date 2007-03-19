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
 * $Id: energest.h,v 1.1 2007/03/19 00:30:18 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __ENERGEST_H__
#define __ENERGEST_H__

typedef unsigned long energest_t;

enum {
  ENERGEST_TYPE_NONE,

  ENERGEST_TYPE_CPU,
  ENERGEST_TYPE_LPM1,
  ENERGEST_TYPE_LPM2,
  ENERGEST_TYPE_LPM3,
  ENERGEST_TYPE_LPM4,
  ENERGEST_TYPE_LED_GREEN,
  ENERGEST_TYPE_LED_YELLOW,
  ENERGEST_TYPE_LED_RED,
  ENERGEST_TYPE_TRANSMIT,
  ENERGEST_TYPE_RECEIVE,
  
  ENERGEST_TYPE_SENSORS,

  ENERGEST_TYPE_MAX
} energest_type;

void energest_init(void);
energest_t energest_type_time(int type);

#if ENERGEST_CONF_ON
extern energest_t energest_total_time[ENERGEST_TYPE_MAX];
extern energest_t energest_current_time[ENERGEST_TYPE_MAX];

#define ENERGEST_ON(type)  do { \
                           energest_current_time[type] = energest_arch_now(); \
                           } while(0)
#define ENERGEST_OFF(type) do { \
                           energest_total_time[type] += energest_arch_now() - \
                           energest_current_time[type]; \
                           } while(0)
#else /* ENERGEST_CONF_ON */
#define ENERGEST_ON(type) do { } while(0)
#define ENERGEST_OFF(type) do { } while(0)
#endif /* ENERGEST_CONF_ON */

energest_t energest_arch_current_estimate(void);
energest_t energest_arch_now(void);

#endif /* __ENERGEST_H__ */
