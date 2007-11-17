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
 * $Id: energest.h,v 1.7 2007/11/17 21:24:50 oliverschmidt Exp $
 */

/**
 * \file
 *         Header file for the energy estimation mechanism
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __ENERGEST_H__
#define __ENERGEST_H__

#include "sys/rtimer.h"

typedef struct {
  /*  unsigned long cummulative[2];*/
  unsigned long current;
} energest_t;

enum energest_type {
  ENERGEST_TYPE_CPU,
  ENERGEST_TYPE_LPM,
  ENERGEST_TYPE_IRQ,
  ENERGEST_TYPE_LED_GREEN,
  ENERGEST_TYPE_LED_YELLOW,
  ENERGEST_TYPE_LED_RED,
  ENERGEST_TYPE_TRANSMIT,
  ENERGEST_TYPE_LISTEN,
  
  ENERGEST_TYPE_SENSORS,

  ENERGEST_TYPE_SERIAL,
  
  ENERGEST_TYPE_MAX
};

void energest_init(void);
unsigned long energest_type_time(int type);
void energest_type_set(int type, unsigned long value);

#if ENERGEST_CONF_ON
/*extern int energest_total_count;*/
extern energest_t energest_total_time[ENERGEST_TYPE_MAX];
extern unsigned short energest_current_time[ENERGEST_TYPE_MAX];

#define ENERGEST_ON(type)  do { \
                           /*++energest_total_count;*/ \
                           energest_current_time[type] = RTIMER_NOW(); \
                           } while(0)
#define ENERGEST_OFF(type) do { \
                           energest_total_time[type].current += (unsigned long)((signed short)RTIMER_NOW() - \
                           (signed short)energest_current_time[type]); \
                           } while(0)
#else /* ENERGEST_CONF_ON */
#define ENERGEST_ON(type) do { } while(0)
#define ENERGEST_OFF(type) do { } while(0)
#endif /* ENERGEST_CONF_ON */

unsigned long energest_arch_current_estimate(void);
unsigned short energest_arch_now(void);

#endif /* __ENERGEST_H__ */
