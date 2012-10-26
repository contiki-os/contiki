/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/timetable-aggregate.h"

#define XXX_HACK_MAX_CATEGORIES 32

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/*
 *
 * Find an aggregation category in the list of aggregates. If the
 * category could not be found, the function returns a pointer to an
 * empty entry. If the list is full, the function returns NULL.
 *
 */
static struct timetable_aggregate_entry *
find_aggregate_category(struct timetable_aggregate *a,
			const uint16_t cat)
{
  int i;
  uint16_t acat;

  for(i = 0; i < a->ptr; ++i) {
    acat = (a->entries[i].id[0] << 8) + a->entries[i].id[1];
    if(acat == cat) {

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
timetable_aggregate_print_detailed(struct timetable_aggregate *a)
{
  int i;
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
timetable_aggregate_reset(struct timetable_aggregate *a)
{
  int i;
  for(i = 0; i < a->ptr; ++i) {
    a->entries[i].time = 0;
    a->entries[i].episodes = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
timetable_aggregate_print_categories(struct timetable_aggregate *a)
{
  int i;

  /*  printf("timetable_aggregate_print_categories: a ptr %d\n", a->ptr);*/
  for(i = 0; i < a->ptr; ++i) {
    printf("-- %c%c: %lu / %u = %lu\n",
	   a->entries[i].id[0], a->entries[i].id[1],
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
timetable_aggregate_compute_detailed(struct timetable_aggregate *a,
				     struct timetable *timetable)
{
  unsigned int i;
  rtimer_clock_t t;
  
  t = timetable->timestamps[0].time;
  
  for(i = 1; i < *timetable->ptr; ++i) {
    struct timetable_aggregate_entry *entry;
    entry = find_aggregate(a, timetable->timestamps[i - 1].id);
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
void
timetable_aggregate_compute_categories(struct timetable_aggregate *a,
				       struct timetable *timetable)
{
  unsigned int i;
  int j;
  rtimer_clock_t t;
  uint16_t categories[XXX_HACK_MAX_CATEGORIES];
  int categories_ptr = 0;
  
  t = timetable->timestamps[0].time;
  
  for(i = 1; i < *timetable->ptr; ++i) {
    struct timetable_aggregate_entry *entry;
    uint16_t cat;

    /*    printf("category_timetable_aggregate_compute %s %d\n",
	  timetable->timestamps[i - 1].id, i);*/
    cat = (timetable->timestamps[i - 1].id[0] << 8) +
      (timetable->timestamps[i - 1].id[1] & 0xff);
    entry = find_aggregate_category(a, cat);
    if(entry == NULL) {
      /* The list is full, skip this entry */
      /*      printf("category_timetable_aggregate_compute: list full\n");*/
    } else if(entry->id == NULL) {
      /* The category was not found in the list, so we add it. */
      entry->id = timetable->timestamps[i - 1].id;
      entry->time = (unsigned long)(timetable->timestamps[i].time - t -
				    timetable_timestamp_time);
      entry->episodes = 1;
      /*      printf("New category %c%c time %lu\n",
	     timetable->timestamps[i - 1].id[0],
	     timetable->timestamps[i - 1].id[1], entry->time);*/
    } else {
      
      entry->time += (unsigned long)(timetable->timestamps[i].time - t -
				     timetable_timestamp_time);
      /*      printf("Adding time to %c%c time %lu\n",
	     timetable->timestamps[i - 1].id[0],
	     timetable->timestamps[i - 1].id[1], entry->time);*/

      /* Make sure that we only update the episodes of each category
	 once per run. We keep track of all updated categories in the
	 "categories" array. If the category is already present in the
	 array, we do not update it. Otherwise, we insert the category
	 in the array and update the episodes counter of the
	 category. */
      
      for(j = 0; j < categories_ptr; ++j) {
	if(categories[j] == cat) {
	  break;
	}
      }
      if(j == categories_ptr) {
	categories[j] = cat;
	categories_ptr++;
	entry->episodes++;
      }
    }
    t = timetable->timestamps[i].time;
  }
}
/*---------------------------------------------------------------------------*/
