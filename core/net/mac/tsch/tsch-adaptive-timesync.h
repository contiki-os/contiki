/*
 * Copyright (c) 2015, SICS Swedish ICT.
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

#ifndef __TSCH_ADAPTIVE_TIMESYNC_H__
#define __TSCH_ADAPTIVE_TIMESYNC_H__

/********** Includes **********/

#include "contiki.h"
#include "net/mac/tsch/tsch-private.h"

/******** Configuration *******/

/* Use SFD timestamp for synchronization? By default we merely rely on rtimer and busy wait
 * until SFD is high, which we found to provide greater accuracy on JN516x and CC2420.
 * Note: for association, however, we always use SFD timestamp to know the time of arrival
 * of the EB (because we do not busy-wait for the whole scanning process)
 * */
#ifdef TSCH_CONF_RESYNC_WITH_SFD_TIMESTAMPS
#define TSCH_RESYNC_WITH_SFD_TIMESTAMPS TSCH_CONF_RESYNC_WITH_SFD_TIMESTAMPS
#else
#define TSCH_RESYNC_WITH_SFD_TIMESTAMPS 0
#endif

/* If enabled, remove jitter due to measurement errors */
#ifdef TSCH_CONF_TIMESYNC_REMOVE_JITTER
#define TSCH_TIMESYNC_REMOVE_JITTER TSCH_CONF_TIMESYNC_REMOVE_JITTER
#else
#define TSCH_TIMESYNC_REMOVE_JITTER TSCH_RESYNC_WITH_SFD_TIMESTAMPS
#endif

/* The jitter to remove in ticks.
 * This should be the sum of measurement errors on Tx and Rx nodes.
 * */
#define TSCH_TIMESYNC_MEASUREMENT_ERROR US_TO_RTIMERTICKS(32)

/* Base drift value.
 * Used to compensate locally know inaccuracies, such as
 * the effect of having a binary 32.768 kHz timer as the TSCH time base. */
#ifdef TSCH_CONF_BASE_DRIFT_PPM
#define TSCH_BASE_DRIFT_PPM TSCH_CONF_BASE_DRIFT_PPM
#else
#define TSCH_BASE_DRIFT_PPM 0
#endif

/* The approximate number of slots per second */
#define TSCH_SLOTS_PER_SECOND (1000000 / TSCH_DEFAULT_TS_TIMESLOT_LENGTH)

/***** External Variables *****/

/* The neighbor last used as our time source */
extern struct tsch_neighbor *last_timesource_neighbor;

/********** Functions *********/

void tsch_timesync_update(struct tsch_neighbor *n, uint16_t time_delta_asn, int32_t drift_correction);

int32_t tsch_timesync_adaptive_compensate(rtimer_clock_t delta_ticks);

#endif /* __TSCH_ADAPTIVE_TIMESYNC_H__ */
