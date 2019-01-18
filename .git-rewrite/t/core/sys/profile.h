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
 * $Id: profile.h,v 1.3 2008/01/17 12:19:26 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Contiki profiling system
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __PROFILE_H__
#define __PROFILE_H__

/* XXX: the profiling code is under development and may not work at
   present. */

#define TIMETABLE_WITH_TYPE 1
#include "sys/timetable.h"

#ifdef PROFILE_CONF_TIMETABLE_SIZE
#define PROFILE_TIMETABLE_SIZE PROFILE_CONF_TIMETABLE_SIZE
#else
#define PROFILE_TIMETABLE_SIZE 128
#endif

#ifdef PROFILE_CONF_AGGREGATE_SIZE
#define PROFILE_AGGREGATE_SIZE PROFILE_CONF_AGGREGATE_SIZE
#else
#define PROFILE_AGGREGATE_SIZE 128
#endif

#define PROFILE_BEGIN(id) TIMETABLE_TIMESTAMP_TYPE(profile_timetable, id, 1)
#define PROFILE_END(id) TIMETABLE_TIMESTAMP_TYPE(profile_timetable, id, 2)

/*#define PROFILE_COND_BEGIN(cond, id) TIMETABLE_COND_TIMESTAMP(profile_begin_timetable, \
								  cond, id)
#define PROFILE_COND_END(cond, id) TIMETABLE_COND_TIMESTAMP(profile_end_timetable, \
								  cond, id)
*/

#define profile_begin_timetable_size PROFILE_TIMETABLE_SIZE
TIMETABLE_DECLARE(profile_begin_timetable);
#define profile_end_timetable_size PROFILE_TIMETABLE_SIZE
TIMETABLE_DECLARE(profile_end_timetable);

#define profile_timetable_size PROFILE_TIMETABLE_SIZE
TIMETABLE_DECLARE(profile_timetable);

void profile_init(void);

void profile_episode_start(void);
void profile_episode_end(void);

void profile_aggregate_print_detailed(void);
void profile_aggregate_compute_detailed(void);


#endif /* __PROFILE_H__ */
