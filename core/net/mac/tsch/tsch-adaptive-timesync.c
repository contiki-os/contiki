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

/**
 * \file
 *         TSCH adaptive time synchronization
 * \author
 *         Atis Elsts <atis.elsts@sics.se>
 *
 */

#include "tsch-adaptive-timesync.h"
#include "tsch-log.h"
#include <stdio.h>

#if TSCH_ADAPTIVE_TIMESYNC

/* Estimated drift of the time-source neighbor. Can be negative.
 * Units used: ppm multiplied by 256. */
static int32_t drift_ppm;
/* Ticks compensated locally since the last timesync time */
static int32_t compensated_ticks;
/* Number of already recorded timesync history entries */
static uint8_t timesync_entry_count;
/* Since last learning of the  drift; may be more than time since last timesync */
static uint32_t asn_since_last_learning;

/* Units in which drift is stored: ppm * 256 */
#define TSCH_DRIFT_UNIT (1000L * 1000 * 256)

/*---------------------------------------------------------------------------*/
/* Add a value to a moving average estimator */
static int32_t
timesync_entry_add(int32_t val, uint32_t time_delta)
{
#define NUM_TIMESYNC_ENTRIES 8
  static int32_t buffer[NUM_TIMESYNC_ENTRIES];
  static uint8_t pos;
  int i;
  if(timesync_entry_count == 0) {
    pos = 0;
  }
  buffer[pos] = val;
  if(timesync_entry_count < NUM_TIMESYNC_ENTRIES) {
    timesync_entry_count++;
  }
  pos = (pos + 1) % NUM_TIMESYNC_ENTRIES;

  val = 0;
  for(i = 0; i < timesync_entry_count; ++i) {
    val += buffer[i];
  }
  return val / timesync_entry_count;
}
/*---------------------------------------------------------------------------*/
/* Learn the neighbor drift rate at ppm */
static void
timesync_learn_drift_ticks(uint32_t time_delta_asn, int32_t drift_ticks)
{
  /* should fit in 32-bit unsigned integer */
  uint32_t time_delta_ticks = time_delta_asn * tsch_timing[tsch_ts_timeslot_length];
  int32_t real_drift_ticks = drift_ticks + compensated_ticks;
  int32_t last_drift_ppm = (int32_t)((int64_t)real_drift_ticks * TSCH_DRIFT_UNIT / time_delta_ticks);

  drift_ppm = timesync_entry_add(last_drift_ppm, time_delta_ticks);
}
/*---------------------------------------------------------------------------*/
/* Either reset or update the neighbor's drift */
void
tsch_timesync_update(struct tsch_neighbor *n, uint16_t time_delta_asn, int32_t drift_correction)
{
  /* Account the drift if either this is a new timesource,
   * or the timedelta is not too small, as smaller timedelta
   * means proportionally larger measurement error. */
  if(last_timesource_neighbor != n) {
    last_timesource_neighbor = n;
    drift_ppm = 0;
    timesync_entry_count = 0;
    compensated_ticks = 0;
    asn_since_last_learning = 0;
  } else {
    asn_since_last_learning += time_delta_asn;
    if(asn_since_last_learning >= 4 * TSCH_SLOTS_PER_SECOND) {
      timesync_learn_drift_ticks(asn_since_last_learning, drift_correction);
      compensated_ticks = 0;
      asn_since_last_learning = 0;
    } else {
      /* Too small timedelta, do not recalculate the drift to avoid introducing error. instead account for the corrected ticks */
      compensated_ticks += drift_correction;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Error-accumulation free compensation algorithm */
static int32_t
compensate_internal(uint32_t time_delta_usec, int32_t drift_ppm, int32_t *remainder, int16_t *tick_conversion_error)
{
  int64_t d = (int64_t)time_delta_usec * drift_ppm + *remainder;
  int32_t amount = d / TSCH_DRIFT_UNIT;
  int32_t amount_ticks;

  *remainder = (int32_t)(d - amount * TSCH_DRIFT_UNIT);

  amount += *tick_conversion_error;
  amount_ticks = US_TO_RTIMERTICKS(amount);
  *tick_conversion_error = amount - RTIMERTICKS_TO_US(amount_ticks);

  if(ABS(amount_ticks) > RTIMER_ARCH_SECOND / 128) {
    TSCH_LOG_ADD(tsch_log_message,
        snprintf(log->message, sizeof(log->message),
            "!too big compensation %ld delta %ld", amount_ticks, time_delta_usec));
    amount_ticks = (amount_ticks > 0 ? RTIMER_ARCH_SECOND : -RTIMER_ARCH_SECOND) / 128;
  }

  return amount_ticks;
}
/*---------------------------------------------------------------------------*/
/* Do the compensation step before scheduling a new timeslot */
int32_t
tsch_timesync_adaptive_compensate(rtimer_clock_t time_delta_ticks)
{
  int32_t result = 0;
  uint32_t time_delta_usec = RTIMERTICKS_TO_US_64(time_delta_ticks);

  /* compensate, but not if the neighbor is not known */
  if(drift_ppm && last_timesource_neighbor != NULL) {
    static int32_t remainder;
    static int16_t tick_conversion_error;
    result = compensate_internal(time_delta_usec, drift_ppm,
        &remainder, &tick_conversion_error);
    compensated_ticks += result;
  }

  if(TSCH_BASE_DRIFT_PPM) {
    static int32_t base_drift_remainder;
    static int16_t base_drift_tick_conversion_error;
    result += compensate_internal(time_delta_usec, 256L * TSCH_BASE_DRIFT_PPM,
        &base_drift_remainder, &base_drift_tick_conversion_error);
  }

  return result;
}
/*---------------------------------------------------------------------------*/
#else /* TSCH_ADAPTIVE_TIMESYNC */
/*---------------------------------------------------------------------------*/
void
tsch_timesync_update(struct tsch_neighbor *n, uint16_t time_delta_asn, int32_t drift_correction)
{
}
/*---------------------------------------------------------------------------*/
int32_t
tsch_timesync_adaptive_compensate(rtimer_clock_t delta_ticks)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
#endif /* TSCH_ADAPTIVE_TIMESYNC */
