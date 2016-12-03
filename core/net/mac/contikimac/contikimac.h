/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *         Header file for the ContikiMAC radio duty cycling protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef CONTIKIMAC_H
#define CONTIKIMAC_H

#include "sys/rtimer.h"
#include "net/mac/rdc.h"
#include "dev/radio.h"
#include "contiki.h"

extern const struct rdc_driver contikimac_driver;
/*---------------------------------------------------------------------------*/
/* CCA_SLEEP_TIME is the time between two successive CCA checks. */
/* Add 1 when rtimer ticks are coarse */
#ifdef CONTIKIMAC_CONF_CCA_SLEEP_TIME
#define CCA_SLEEP_TIME CONTIKIMAC_CONF_CCA_SLEEP_TIME
#else
#if RTIMER_ARCH_SECOND > 8000
#define CCA_SLEEP_TIME                     (RTIMER_ARCH_SECOND / 2000)
#else
#define CCA_SLEEP_TIME                     ((RTIMER_ARCH_SECOND / 2000) + 1)
#endif /* RTIMER_ARCH_SECOND > 8000 */
#endif /* CONTIKIMAC_CONF_CCA_SLEEP_TIME */

/* ContikiMAC performs periodic channel checks. Each channel check
   consists of two or more CCA checks. CCA_COUNT_MAX is the number of
   CCAs to be done for each periodic channel check. The default is
   two.*/
#ifdef CONTIKIMAC_CONF_CCA_COUNT_MAX
#define CCA_COUNT_MAX                      (CONTIKIMAC_CONF_CCA_COUNT_MAX)
#else
#define CCA_COUNT_MAX                      2
#endif

/* CCA_CHECK_TIME is the time it takes to perform a CCA check. */
/* Note this may be zero. AVRs have 7612 ticks/sec, but block until cca is done */
#ifdef CONTIKIMAC_CONF_CCA_CHECK_TIME
#define CCA_CHECK_TIME                     (CONTIKIMAC_CONF_CCA_CHECK_TIME)
#else
#define CCA_CHECK_TIME                     (RTIMER_ARCH_SECOND / 8192)
#endif

/* CHECK_TIME is the total time it takes to perform CCA_COUNT_MAX
   CCAs. */
#define CHECK_TIME          (CCA_COUNT_MAX * (CCA_CHECK_TIME + CCA_SLEEP_TIME))

/* CYCLE_TIME for channel cca checks, in rtimer ticks. */
#ifdef CONTIKIMAC_CONF_CYCLE_TIME
#define CYCLE_TIME (CONTIKIMAC_CONF_CYCLE_TIME)
#else
#define CYCLE_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE)
#endif

/* INTER_PACKET_INTERVAL is the interval between two successive packet transmissions */
#ifdef CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#define INTER_PACKET_INTERVAL              CONTIKIMAC_CONF_INTER_PACKET_INTERVAL
#else
#define INTER_PACKET_INTERVAL              (RTIMER_ARCH_SECOND / 2500)
#endif

/* AFTER_ACK_DETECTECT_WAIT_TIME is the time to wait after a potential
   ACK packet has been detected until we can read it out from the
   radio. */
#ifdef CONTIKIMAC_CONF_AFTER_ACK_DETECTECT_WAIT_TIME
#define AFTER_ACK_DETECTECT_WAIT_TIME      CONTIKIMAC_CONF_AFTER_ACK_DETECTECT_WAIT_TIME
#else
#define AFTER_ACK_DETECTECT_WAIT_TIME      (RTIMER_ARCH_SECOND / 1500)
#endif

/* STROBE_TIME is the maximum amount of time a transmitted packet
   should be repeatedly transmitted as part of a transmission. */
#define STROBE_TIME                        (CYCLE_TIME + 2 * CHECK_TIME)

#define ACK_LEN 3

/* INTER_PACKET_DEADLINE is the maximum time a receiver waits for the
   next packet of a burst when FRAME_PENDING is set. */
#ifdef CONTIKIMAC_CONF_INTER_PACKET_DEADLINE
#define INTER_PACKET_DEADLINE               CONTIKIMAC_CONF_INTER_PACKET_DEADLINE
#else
#define INTER_PACKET_DEADLINE               CLOCK_SECOND / 32
#endif

/* MAX_SILENCE_PERIODS is the maximum amount of periods (a period is
   CCA_CHECK_TIME + CCA_SLEEP_TIME) that we allow to be silent before
   we turn of the radio. */
#ifdef CONTIKIMAC_CONF_MAX_SILENCE_PERIODS
#define MAX_SILENCE_PERIODS                CONTIKIMAC_CONF_MAX_SILENCE_PERIODS
#else
#define MAX_SILENCE_PERIODS                5
#endif

/* MAX_NONACTIVITY_PERIODS is the maximum number of periods we allow
   the radio to be turned on without any packet being received, when
   WITH_FAST_SLEEP is enabled. */
#ifdef CONTIKIMAC_CONF_MAX_NONACTIVITY_PERIODS
#define MAX_NONACTIVITY_PERIODS            CONTIKIMAC_CONF_MAX_NONACTIVITY_PERIODS
#else
#define MAX_NONACTIVITY_PERIODS            10
#endif

/* LISTEN_TIME_AFTER_PACKET_DETECTED is the time that we keep checking
   for activity after a potential packet has been detected by a CCA
   check. */
#ifdef CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED
#define LISTEN_TIME_AFTER_PACKET_DETECTED  CONTIKIMAC_CONF_LISTEN_TIME_AFTER_PACKET_DETECTED
#else
#define LISTEN_TIME_AFTER_PACKET_DETECTED  (RTIMER_ARCH_SECOND / 80)
#endif
/*---------------------------------------------------------------------------*/
/* Before starting a transmission, Contikimac checks the availability
   of the channel with CCA_COUNT_MAX_TX consecutive CCAs */
#ifdef CONTIKIMAC_CONF_CCA_COUNT_MAX_TX
#define CCA_COUNT_MAX_TX                   (CONTIKIMAC_CONF_CCA_COUNT_MAX_TX)
#else
#define CCA_COUNT_MAX_TX                   6
#endif


/* CHECK_TIME_TX is the total time it takes to perform CCA_COUNT_MAX_TX
   CCAs. */
#define CHECK_TIME_TX                      (CCA_COUNT_MAX_TX * (CCA_CHECK_TIME + CCA_SLEEP_TIME))
/*---------------------------------------------------------------------------*/
/* More aggressive radio sleeping when channel is busy with other traffic */
#ifndef WITH_FAST_SLEEP
#define WITH_FAST_SLEEP              1
#endif
/*---------------------------------------------------------------------------*/
/* TX/RX cycles are synchronized with neighbor wake periods */
#ifdef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define WITH_PHASE_OPTIMIZATION      CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#else /* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION */
#define WITH_PHASE_OPTIMIZATION      1
#endif /* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION */
/*---------------------------------------------------------------------------*/
/* GUARD_TIME is the time before the expected phase of a neighbor that
   a transmitted should begin transmitting packets. */
#ifdef CONTIKIMAC_CONF_GUARD_TIME
#define GUARD_TIME                         CONTIKIMAC_CONF_GUARD_TIME
#else
#define GUARD_TIME                         10 * CHECK_TIME + CHECK_TIME_TX
#endif
/*---------------------------------------------------------------------------*/
#endif /* CONTIKIMAC_H */
