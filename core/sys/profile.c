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
 * $Id: profile.c,v 1.1 2007/10/23 20:39:07 adamdunkels Exp $
 */

/**
 * \file
 *         Implementation of the Contiki profiling system
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/profile.h"

#include <stdlib.h> /* For NULL */

unsigned int profile_timestamp_ptr;
struct profile_timestamp profile_timestamps[PROFILE_LIST_LENGTH];
rtimer_clock_t profile_timestamp_time;

int profile_invalid_episode_overflow, profile_invalid_episode_toolong;

int profile_max_queuelen = 0;

static rtimer_clock_t episode_start_time;

/* The number of fine grained ticks per coarse grained ticks. We
   currently (MSP430) have 2457600 ticks per second for the fine
   grained timer, and 32678 / 8 ticks per second for the coarse. */
#define FINE_TICKS_PER_COARSE_TICK (2457600/(32678/8))

/*---------------------------------------------------------------------------*/
rtimer_clock_t
profile_timediff(const char *ptr1, const char *ptr2)
{
  int i;
  int t1, t2;
  int timestamp_ptr = PROFILE_TIMESTAMP_PTR;

  /*  printf("profile_timestamp_ptr %d max %d\n", profile_timestamp_ptr, profile_max_queuelen);*/
  
  t1 = t2 = PROFILE_LIST_LENGTH;
  
  for(i = timestamp_ptr - 1; i >= 0; --i) {
    /*    printf("Checking 1 %s %u == %s i %d\n",
	   profile_timestamps[i].ptr,
	   profile_timestamps[i].time,
	   ptr1, i);*/
    if(profile_timestamps[i].ptr == ptr1) {
      t1 = i;
      break;
    }
  }
  
  for(i = i - 1; i >= 0; --i) {
    /*    printf("Checking 2 %s %u == %s i %d\n",
	   profile_timestamps[i].ptr,
	   profile_timestamps[i].time,
	   ptr1, i);*/
    if(profile_timestamps[i].ptr == ptr2) {
      t2 = i;
      break;
    }
  }
  /*  printf("t1 %d t2 %d\n", t1, t2);*/
  if(t1 != PROFILE_LIST_LENGTH && t2 != PROFILE_LIST_LENGTH) {
    return profile_timestamps[t1].time - profile_timestamps[t2].time;
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void
profile_clear_timestamps(void)
{
  /*  int i;
  for(i = 0; i < PROFILE_LIST_LENGTH; ++i) {
    profile_timestamps[i].str = "NULL";
    profile_timestamps[i].time = 0;
    }*/
  profile_timestamp_ptr = 0;
}
/*---------------------------------------------------------------------------*/
void
profile_init(void)
{
  profile_clear_timestamps();

  /* Measure the time for taking a timestamp. */
  PROFILE_TIMESTAMP(NULL);
  PROFILE_TIMESTAMP(NULL);
  profile_timestamp_time = profile_timestamps[1].time - profile_timestamps[0].time;

  profile_clear_timestamps();
}
/*---------------------------------------------------------------------------*/
void
profile_episode_start(void)
{
  profile_timestamp_ptr = 0;
  
  episode_start_time = clock_counter();
  
  profile_timestamps[PROFILE_LIST_LENGTH - 1].ptr = NULL;
}
/*---------------------------------------------------------------------------*/
void
profile_episode_end(void)
{
  rtimer_clock_t episode_end_time = clock_counter();

  PROFILE_TIMESTAMP("profile_episode_end");
  
/*   printf("profile_episode_end start %u, end %u, max time %u\n", episode_start_time, episode_end_time, 65536/FINE_TICKS_PER_COARSE_TICK); */
  if(profile_timestamps[PROFILE_LIST_LENGTH - 1].ptr != NULL) {
    /* Invalid episode because of list overflow. */
    profile_invalid_episode_overflow++;
    profile_max_queuelen = PROFILE_LIST_LENGTH;
  } else if(episode_end_time - episode_start_time > 65536/FINE_TICKS_PER_COARSE_TICK) {
    /* Invalid episode because of timer overflow. */
    profile_invalid_episode_toolong++;
  } else {
    /* Compute aggregates. */
    if(PROFILE_TIMESTAMP_PTR > profile_max_queuelen) {
      profile_max_queuelen = PROFILE_TIMESTAMP_PTR;
    }
    profile_aggregates_compute();
    /*    printf("Episode length %d\n", profile_timestamp_ptr);*/
  }

/*   profile_aggregates_print(); */
/*   profile_print_stats(); */
}
/*---------------------------------------------------------------------------*/
void
profile_print_stats(void)
{
  printf("Memory for profiling: %d * %d = %d\n",
	 sizeof(struct profile_timestamp), profile_max_queuelen,
	 sizeof(struct profile_timestamp) * profile_max_queuelen);
  printf("Invalid episodes overflow %d time %d\n",
	 profile_invalid_episode_overflow,
	 profile_invalid_episode_toolong);
}
/*---------------------------------------------------------------------------*/
