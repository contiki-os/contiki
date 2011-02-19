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
 * $Id: timetable.c,v 1.4 2010/04/06 19:10:21 anthony-a Exp $
 */

/**
 * \file
 *         Implementation of timetable, a data structure containing timestamps for events
 * \author
 *         Adam Dunkels <adam@sics.se>
 */
#include "sys/clock.h"
#include "sys/timetable.h"

#include <stdio.h>

rtimer_clock_t timetable_timestamp_time;


/*---------------------------------------------------------------------------*/
struct timetable_timestamp *
timetable_entry(struct timetable *t, int num)
{
  if(t == NULL) {
    return NULL;
  }
  return &(t->timestamps[num]);
}
/*---------------------------------------------------------------------------*/
int
timetable_ptr(struct timetable *t)
{
  return *t->ptr;
}
/*---------------------------------------------------------------------------*/
void
timetable_clear(struct timetable *t)
{
  *t->ptr = 0;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timetable_timediff(struct timetable *t,
		   const char *id1, const char *id2)
{
#ifdef SDCC_mcs51
  char i; /* SDCC tracker 2982753 */
#else
  int i;
#endif
  int t1, t2;
  
  t1 = t2 = t->size;
  
  for(i = *t->ptr - 1; i >= 0; --i) {
    if(t->timestamps[i].id == id1) {
      t1 = i;
      break;
    }
  }
  
  for(i = i - 1; i >= 0; --i) {
    if(t->timestamps[i].id == id2) {
      t2 = i;
      break;
    }
  }
  if(t1 != t->size && t2 != t->size) {
    return t->timestamps[t1].time - t->timestamps[t2].time;
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void
timetable_init(void)
{
  char dummy1, dummy2;
#define temp_size 4
  TIMETABLE_STATIC(temp);
  
  timetable_clear(&temp);

  /* Measure the time for taking a timestamp. */
  TIMETABLE_TIMESTAMP(temp, &dummy1);
  TIMETABLE_TIMESTAMP(temp, &dummy2);
  timetable_timestamp_time = timetable_timediff(&temp, &dummy1, &dummy2);
}
/*---------------------------------------------------------------------------*/
void
timetable_print(struct timetable *t)
{
  unsigned int i;
  int time;
  
  time = t->timestamps[0].time;

  printf("---\n");
  for(i = 1; i < *t->ptr; ++i) {
    printf("%s: %u\n", t->timestamps[i - 1].id, t->timestamps[i].time - time);
    time = t->timestamps[i].time;
  }
}
/*---------------------------------------------------------------------------*/
