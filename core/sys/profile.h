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
 * $Id: profile.h,v 1.2 2007/11/17 10:14:19 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Contiki profiling system
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "sys/rtimer.h"

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

#include "contiki-conf.h"

#ifdef PROFILE_CONF_LIST_LENGTH
#define PROFILE_LIST_LENGTH PROFILE_CONF_LIST_LENGTH
#else
#define PROFILE_LIST_LENGTH 128
#endif

struct profile_timestamp {
  const char *ptr;
  rtimer_clock_t time;
};

extern struct profile_timestamp profile_timestamps[PROFILE_LIST_LENGTH];
extern unsigned int profile_timestamp_ptr;
extern rtimer_clock_t profile_timestamp_time;

#define PROFILE_TIMESTAMP_PTR (profile_timestamp_ptr / sizeof(struct profile_timestamp))

#if PROFILE_CONF_ON
#define PROFILE_TIMESTAMP(str) \
   do { \
     profile_timestamps[profile_timestamp_ptr / sizeof(struct profile_timestamp)].ptr = str; \
     profile_timestamps[profile_timestamp_ptr / sizeof(struct profile_timestamp)].time = RTIMER_NOW(); \
     profile_timestamp_ptr = (profile_timestamp_ptr + sizeof(struct profile_timestamp)) % (PROFILE_LIST_LENGTH * sizeof(struct profile_timestamp)); \
   } while(0)
#define PROFILE_RESUME(num) PROFILE_TIMESTAMP(profile_timestamps[num].ptr)

#define PROFILE_COND_TIMESTAMP(cond, ptr) do { if(cond) {PROFILE_TIMESTAMP(ptr);} } while(0)
#define PROFILE_COND_RESUME(cond, num) PROFILE_COND_TIMESTAMP(cond, profile_timestamps[num].ptr)
#else /* PROFILE_CONF_ON */
#define PROFILE_TIMESTAMP(id)
#define PROFILE_RESUME(num)
#define PROFILE_COND_TIMESTAMP(cond, id)
#define PROFILE_COND_RESUME(cond, num)
#endif /* PROFILE_CONF_ON */

rtimer_clock_t profile_timediff(const char *ptr1, const char *ptr2);

#define PROFILE_GETPTR() (PROFILE_TIMESTAMP_PTR)

void profile_clear_timestamps(void);
void profile_init(void);

void profile_episode_start(void);
void profile_episode_end(void);

void profile_aggregates_print(void);
void profile_aggregates_compute(void);

void profile_print_stats(void);


enum {
  PROFILE_TYPE_STACK,
  PROFILE_TYPE_HW,
  PROFILE_TYPE_RADIO,
  PROFILE_TYPE_SYSTEM,
  PROFILE_TYPE_APP,
};

#endif /* __PROFILE_H__ */

/* profile_timestamp_ptr code:

  2e:   1f 42 00 00     mov     &0x0000,r15     ;0x0000
  32:   0e 4f           mov     r15,    r14     ;
  34:   0e 5e           rla     r14             ;
  36:   0e 5e           rla     r14             ;
  38:   3e 50 00 00     add     #0,     r14     ;#0x0000
  3c:   be 40 00 00     mov     #0,     0(r14)  ;#0x0000
  40:   00 00
  42:   9e 42 90 01     mov     &0x0190,2(r14)  ;0x0190
  46:   02 00
  48:   1f 53           inc     r15             ;
  4a:   3f f0 3f 00     and     #63,    r15     ;#0x003f
  4e:   82 4f 00 00     mov     r15,    &0x0000 ;

  msp430-specific profile_timetstamp_2ptr code:

  2e:   1f 42 00 00     mov     &0x0000,r15     ;0x0000
  32:   0e 4f           mov     r15,    r14     ;
  34:   3e 50 00 00     add     #0,     r14     ;#0x0000
  38:   be 40 00 00     mov     #0,     0(r14)  ;#0x0000
  3c:   00 00
  3e:   9e 42 90 01     mov     &0x0190,2(r14)  ;0x0190
  42:   02 00
  44:   2f 53           incd    r15             ;
  46:   3f f0 7f 00     and     #127,   r15     ;#0x007f
  4a:   82 4f 00 00     mov     r15,    &0x0000 ;

  generic timestamp_2ptr code:

  2e:   1f 42 00 00     mov     &0x0000,r15     ;0x0000
  32:   0e 4f           mov     r15,    r14     ;
  34:   1e c3           bic     #1,     r14     ;r3 As==01
  36:   0e 5e           rla     r14             ;
  38:   3e 50 00 00     add     #0,     r14     ;#0x0000
  3c:   be 40 00 00     mov     #0,     0(r14)  ;#0x0000
  40:   00 00
  42:   9e 42 90 01     mov     &0x0190,2(r14)  ;0x0190
  46:   02 00
  48:   2f 53           incd    r15             ;
  4a:   3f f0 7f 00     and     #127,   r15     ;#0x007f
  4e:   82 4f 00 00     mov     r15,    &0x0000 ;
  
*/
