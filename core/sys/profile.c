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
 *         Implementation of the Contiki profiling system
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/profile.h"
#include "sys/clock.h"

#include <stdio.h>

/* XXX: the profiling code is under development and may not work at
   present. */


TIMETABLE_NONSTATIC(profile_timetable);

TIMETABLE_NONSTATIC(profile_begin_timetable);
TIMETABLE_NONSTATIC(profile_end_timetable);
TIMETABLE_AGGREGATE(profile_aggregate, PROFILE_AGGREGATE_SIZE);

static rtimer_clock_t episode_start_time;
static unsigned int invalid_episode_overflow, invalid_episode_toolong,
  max_queuelen;

/* The number of fine grained ticks per coarse grained ticks. We
   currently (MSP430) have 2457600 ticks per second for the fine
   grained timer, and 32678 / 8 ticks per second for the coarse. */
#define XXX_HACK_FINE_TICKS_PER_COARSE_TICK (2457600/(32678/8))

/*---------------------------------------------------------------------------*/
void
profile_init(void)
{
  timetable_init();
  timetable_clear(&profile_begin_timetable);
  timetable_clear(&profile_end_timetable);
}
/*---------------------------------------------------------------------------*/
void
profile_episode_start(void)
{
  struct timetable_timestamp *e;
  timetable_clear(&profile_begin_timetable);
  timetable_clear(&profile_end_timetable);
  episode_start_time = clock_time();
  
  e = timetable_entry(&profile_begin_timetable,
		      PROFILE_TIMETABLE_SIZE - 1);
  if(e != NULL) {
    e->id = NULL;
  }
  e = timetable_entry(&profile_end_timetable,
		      PROFILE_TIMETABLE_SIZE - 1);
  if(e != NULL) {
    e->id = NULL;
  }
}
/*---------------------------------------------------------------------------*/
void
profile_episode_end(void)
{
  struct timetable_timestamp *e;
  rtimer_clock_t episode_end_time = clock_time();

/*   printf("timetable_episode_end start %u, end %u, max time %u\n", episode_start_time, episode_end_time, 65536/FINE_TICKS_PER_COARSE_TICK); */
  e = timetable_entry(&profile_begin_timetable,
		      PROFILE_TIMETABLE_SIZE - 1);
  if(e != NULL && e->id != NULL) {
    /* Invalid episode because of list overflow. */
    invalid_episode_overflow++;
    max_queuelen = PROFILE_TIMETABLE_SIZE;
  } else if(episode_end_time - episode_start_time >
	    65536/XXX_HACK_FINE_TICKS_PER_COARSE_TICK) {
    /* Invalid episode because of timer overflow. */
    invalid_episode_toolong++;
  } else {
    /* Compute aggregates. */
    if(timetable_ptr(&profile_begin_timetable) > max_queuelen) {
      max_queuelen = timetable_ptr(&profile_begin_timetable);
    }
    /*    timetable_aggregates_compute();*/
  }
}
/*---------------------------------------------------------------------------*/
/*
 *
 * Find a specific aggregate ID in the list of aggregates.
 *
 */
static struct timetable_aggregate_entry *
find_aggregate(struct timetable_aggregate *a,
	       const char *id)
{
  int i;
  for(i = 0; i < a->ptr; ++i) {
    if(a->entries[i].id == id) {
      return &a->entries[i];
    }
  }
  if(i == a->size) {
    return NULL;
  }
  a->entries[a->ptr].id = NULL;
  return &a->entries[a->ptr++];
}
/*---------------------------------------------------------------------------*/
void
profile_aggregate_print_detailed(void)
{
  int i;
  struct timetable_aggregate *a = &profile_aggregate;
  
  /*  printf("timetable_aggregate_print_detailed: a ptr %d\n", a->ptr);*/
  for(i = 0; i < a->ptr; ++i) {
    printf("-- %s: %lu / %u = %lu\n", a->entries[i].id,
	   a->entries[i].time,
	   a->entries[i].episodes,
	   a->entries[i].time / a->entries[i].episodes);
  }
  
  printf("Memory for entries: %d * %d = %d\n",
	 (int)sizeof(struct timetable_aggregate), a->ptr,
	 (int)sizeof(struct timetable_aggregate) * a->ptr);
}
/*---------------------------------------------------------------------------*/
void
profile_aggregate_compute_detailed(void)
{
  int i;
  int last;
  rtimer_clock_t t;
  struct timetable_aggregate *a = &profile_aggregate;
  struct timetable *timetable = &profile_timetable;
  struct timetable_aggregate_entry *entry;

  last = timetable_ptr(&profile_begin_timetable);
  t = profile_begin_timetable.timestamps[0].time;
  for(i = 0; i < last; ++i) {

    entry = find_aggregate(a, profile_begin_timetable.timestamps[i].id);
    if(entry == NULL) {
      /* The list is full, skip this entry */
      /*      printf("detailed_timetable_aggregate_compute: list full\n");*/
    } else if(entry->id == NULL) {
      /* The id was found in the list, so we add it. */
      entry->id = timetable->timestamps[i - 1].id;
      entry->time = (unsigned long)(timetable->timestamps[i].time - t -
				    timetable_timestamp_time);
      entry->episodes = 1;
      /*      printf("New entry %s %lu\n", entry->id, entry->time);*/
    } else {
      entry->time += (unsigned long)(timetable->timestamps[i].time - t -
				     timetable_timestamp_time);
				     entry->episodes++;
    }
    t = timetable->timestamps[i].time;
    /*    printf("a ptr %d\n", a->ptr);*/

  }

}
/*---------------------------------------------------------------------------*/
