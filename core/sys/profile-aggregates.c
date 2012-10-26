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
 *         Compuation of aggregates for the Contiki profiling system
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/profile.h"

#include <stdlib.h>
#include <stdio.h>

struct aggregate {
  const char *ptr;
  unsigned short episodes;
  unsigned long cycles;
};

#define DETAILED_AGGREGATES 0

#define MAX_CATEGORIES 32
#define LIST_LEN 100

static struct aggregate aggregates[LIST_LEN];

static int aggregates_list_ptr = 0;

/*---------------------------------------------------------------------------*/
static struct aggregate *
find_aggregate_category(const uint16_t cat)
{
  int i;
  uint16_t acat;

/*   printf("find_aggregate_category 0x%04x %c%c\n", */
/* 	 cat, cat >> 8, cat & 0xff); */
  
  for(i = 0; i < aggregates_list_ptr; ++i) {
    acat = (aggregates[i].ptr[0] << 8) + aggregates[i].ptr[1];

/*     printf("acat 0x%04x %c%c\n", */
/* 	   acat, acat >> 8, acat & 0xff); */
      
    if(acat == cat) {
      return &aggregates[i];
    }
  }

  if(i == LIST_LEN) {
    return NULL;
  }

  aggregates[aggregates_list_ptr].ptr = NULL;
  return &aggregates[aggregates_list_ptr++];
}
/*---------------------------------------------------------------------------*/
#if DETAILED_AGGREGATES
static struct aggregate *
find_aggregate(const unsigned char *ptr)
{
  int i;
  for(i = 0; i < aggregates_list_ptr; ++i) {
    if(aggregates[i].ptr == ptr) {
      return &aggregates[i];
    }
  }
  if(i == LIST_LEN) {
    return NULL;
  }

  return &aggregates[aggregates_list_ptr++];
}
#endif /* DETAILED_AGGREGATES */
/*---------------------------------------------------------------------------*/
void
profile_aggregates_print(void)
{
  int i;

#if DETAILED_AGGREGATES
  for(i = 0; i < aggregates_list_ptr; ++i) {
    printf("-- %s: %lu / %u = %lu\n", aggregates[i].ptr,
	   aggregates[i].cycles,
	   aggregates[i].episodes,
	   aggregates[i].cycles / aggregates[i].episodes);
  }
#else
  for(i = 0; i < aggregates_list_ptr; ++i) {
    printf("-- %c%c: %lu / %u = %lu\n",
	   aggregates[i].ptr[0], aggregates[i].ptr[1],
	   aggregates[i].cycles,
	   aggregates[i].episodes,
	   aggregates[i].cycles / aggregates[i].episodes);
  }
#endif

  printf("Memory for aggregates: %d * %d = %d\n",
	 (int)sizeof(struct aggregate), aggregates_list_ptr,
	 (int)sizeof(struct aggregate) * aggregates_list_ptr);
}
/*---------------------------------------------------------------------------*/
#if DETAILED_AGGREGATES
static void
detailed_profile_aggregates_compute(void)
{
  int i;
  rtimer_clock_t t;
  /*  const char *str = "profile_aggregates_compute";

  PROFILE_TIMESTAMP(str);*/
  
  t = profile_timestamps[0].time;
  
  for(i = 1; i < PROFILE_TIMESTAMP_PTR; ++i) {
    struct aggregate *a;
    a = find_aggregate(profile_timestamps[i - 1].ptr);
    if(a == NULL) {
      /* The list is full, skip this entry */
      printf("profile_aggregates_compute: list full\n");
    } else if(a->ptr == NULL) {
      a->ptr = profile_timestamps[i - 1].ptr;
      a->cycles = (unsigned long)(profile_timestamps[i].time - t);
      a->episodes = 1;
    } else {
      a->cycles += (unsigned long)(profile_timestamps[i].time - t);
      a->episodes++;
    }
    t = profile_timestamps[i].time;
  }

  /*  PROFILE_TIMESTAMP(str);*/

  /*printf("Aggregating time %u, len %d, list len %d, overhead %d\n",
	 profile_timediff(str, str), PROFILE_TIMESTAMP_PTR,
	 aggregates_list_ptr, profile_timestamp_time);*/
  

  /* print_aggregates();*/
}
#endif /* DETAILED_AGGREGATES */
/*---------------------------------------------------------------------------*/
static void
category_profile_aggregates_compute(void)
{
  int i,j;
  rtimer_clock_t t;
  uint16_t categories[MAX_CATEGORIES];
  int categories_ptr = 0;
  /*  const char *str = "profile_aggregates_compute";

  PROFILE_TIMESTAMP(str);*/
  
  t = profile_timestamps[0].time;
  
  for(i = 1; i < PROFILE_TIMESTAMP_PTR; ++i) {
    struct aggregate *a;
    uint16_t cat;

/*     printf("category_profile_aggregates_compute %s\n", */
/* 	   profile_timestamps[i - 1].ptr); */
    cat = (profile_timestamps[i - 1].ptr[0] << 8) +
      (profile_timestamps[i - 1].ptr[1] & 0xff);
    a = find_aggregate_category(cat);
    if(a == NULL) {
      /* The list is full, skip this entry */
      printf("profile_aggregates_compute: list full\n");
    } else if(a->ptr == NULL) {
      a->ptr = profile_timestamps[i - 1].ptr;
      a->cycles = (unsigned long)(profile_timestamps[i].time - t - profile_timestamp_time);
      a->episodes = 1;
    } else {
      
      a->cycles += (unsigned long)(profile_timestamps[i].time - t - profile_timestamp_time);

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
	a->episodes++;
      }
    }
    t = profile_timestamps[i].time;
  }

  /*  PROFILE_TIMESTAMP(str);*/

  /*printf("Aggregating time %u, len %d, list len %d, overhead %d\n",
	 profile_timediff(str, str), PROFILE_TIMESTAMP_PTR,
	 aggregates_list_ptr, profile_timestamp_time);*/
  

  /* print_aggregates();*/
}
/*---------------------------------------------------------------------------*/
void
profile_aggregates_compute(void)
{
#if DETAILED_AGGREGATES
  detailed_profile_aggregates_compute();
#else
  category_profile_aggregates_compute();
#endif
}
/*---------------------------------------------------------------------------*/
