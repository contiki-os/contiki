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

#ifndef __TIMETABLE_H__
#define __TIMETABLE_H__

#include "sys/cc.h"
#include "sys/rtimer.h"


struct timetable_timestamp {
  const char *id;
  rtimer_clock_t time;
#if TIMETABLE_WITH_TYPE
  uint8_t type;
#endif /* TIMETABLE_WITH_TYPE */
};
struct timetable {
  struct timetable_timestamp *timestamps;
  const int size;
  unsigned int * const ptr;
};

#define TIMETABLE_NONSTATIC(name)					       \
struct timetable_timestamp CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_size)]; \
unsigned int CC_CONCAT(name,_ptr);					       \
struct timetable name = {						       \
  CC_CONCAT(name,_timestamps),						       \
    CC_CONCAT(name,_size),						       \
    &CC_CONCAT(name,_ptr)}

#define TIMETABLE_STATIC(name)							      \
static struct timetable_timestamp CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_size)]; \
static unsigned int CC_CONCAT(name,_ptr);					      \
static struct timetable name = {						      \
  CC_CONCAT(name,_timestamps),							      \
    CC_CONCAT(name,_size),							      \
    &CC_CONCAT(name,_ptr)}

#define TIMETABLE_DECLARE(name)							       \
extern unsigned int CC_CONCAT(name,_ptr);					       \
extern struct timetable_timestamp CC_CONCAT(name, _timestamps)[CC_CONCAT(name,_size)]; \
extern struct timetable name

#define TIMETABLE(name) TIMETABLE_STATIC(name)

#define TIMETABLE_TIMESTAMP(name, str)					 \
do {									 \
  CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_ptr)].id = str;		 \
  CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_ptr)].time = RTIMER_NOW(); \
  CC_CONCAT(name,_ptr) = (CC_CONCAT(name,_ptr) + 1) %			 \
    CC_CONCAT(name,_size);						 \
} while(0)

#if TIMETABLE_WITH_TYPE
#define TIMETABLE_TIMESTAMP_TYPE(name, str, t)				 \
do {									 \
  CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_ptr)].id = str;		 \
  CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_ptr)].type = t;		 \
  CC_CONCAT(name,_timestamps)[CC_CONCAT(name,_ptr)].time = RTIMER_NOW(); \
  CC_CONCAT(name,_ptr) = (CC_CONCAT(name,_ptr) + 1) %			 \
    CC_CONCAT(name,_size);						 \
} while(0)
#else /* TIMETABLE_WITH_TYPE */
#define TIMETABLE_TIMESTAMP_TYPE(name, str, t) TIMETABLE_TIMESTAMP(name, str)
#endif /* TIMETABLE_WITH_TYPE */


#define TIMETABLE_RESUME(name,num) \
        TIMETABLE_TIMESTAMP(CC_CONCAT(name,_timestamps[num].id))

#define TIMETABLE_COND_TIMESTAMP(cond,name,id)		\
			    do { if(cond) {		\
			      TIMETABLE_TIMESTAMP(id);	\
			    } while(0)

#define TIMETABLE_COND_RESUME(cond,name,num) \
        TIMETABLE_COND_TIMESTAMP(cond,name,  \
				 CC_CONCAT(name,_timestamps[num].id))

#define TIMETABLE_ENTRY(name, num) CC_CONCAT(name,_timestamps)[num]
#define TIMETABLE_PTR(name) CC_CONCAT(name,_ptr)

/**
 * The time for taking a timestamp.
 */
extern rtimer_clock_t timetable_timestamp_time;


struct timetable_timestamp *timetable_entry(struct timetable *t,
					    int num);
int timetable_ptr(struct timetable *t);
 
void timetable_clear(struct timetable *t);
rtimer_clock_t timetable_timediff(struct timetable *t,
				  const char *id1, const char *id2);
void timetable_init(void);

void timetable_print(struct timetable *t);

#include "sys/timetable-aggregate.h"

#endif /* __TIMETABLE_H__ */
