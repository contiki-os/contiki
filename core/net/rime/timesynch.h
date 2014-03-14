/**
 * \addtogroup sys
 * @{
 */

/**
 * \defgroup timesynch Implicit network time synchronization
 * @{
 *
 * This crude and simple network time synchronization module
 * synchronizes clocks of all nodes in a network. The time
 * synchronization is implicit in that no explicit time
 * synchronization messages are sent: the module relies on the
 * underlying network device driver to timestamp all radio messages,
 * both outgoing and incoming. The code currently only works on the
 * Tmote Sky platform and the cc2420 driver.
 *
 * Every node has an authority level, which is included in every
 * outgoing packet. If a message is received from a node with higher
 * authority (lower authority number), the node adjusts its clock
 * towards the clock of the sending node.
 *
 * The timesynch module is implemented as a meta-MAC protocol, so that
 * the module is invoked for every incoming packet.
 *
 */

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
 *         Header file for a simple time synchronization mechanism
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef TIMESYNCH_H_
#define TIMESYNCH_H_

#include "net/mac/mac.h"
#include "sys/rtimer.h"

/**
 * \brief      Initialize the timesynch module
 *
 *             This function initializes the timesynch module. This
 *             function must not be called before rime_init().
 *
 */
void timesynch_init(void);

/**
 * \brief      Get the current time-synchronized time
 * \return     The current time-synchronized time
 *
 *             This function returns the current time-synchronized
 *             time.
 *
 */
rtimer_clock_t timesynch_time(void);

/**
 * \brief      Get the current time-synchronized time, suitable for use with the rtimer module
 * \return     The current time-synchronized rtimer time
 *
 *             This function returns the (local) rtimer-equivalent
 *             time corresponding to the current time-synchronized
 *             (global) time. The rtimer-equivalent time is used for
 *             setting rtimer timers that are synchronized to other
 *             nodes in the network.
 *
 */
rtimer_clock_t timesynch_time_to_rtimer(rtimer_clock_t synched_time);

/**
 * \brief      Get the synchronized equivalent of an rtimer time
 * \return     The synchronized equivalent of an rtimer time
 *
 *             This function returns the time synchronized equivalent
 *             time corresponding to a (local) rtimer time.
 *
 */
rtimer_clock_t timesynch_rtimer_to_time(rtimer_clock_t rtimer_time);

/**
 * \brief      Get the current time-synchronized offset from the rtimer clock, which is used mainly for debugging
 * \return     The current time-synchronized offset from the rtimer clock
 *
 *             This function returns the current time-synchronized
 *             offset from the rtimer arch clock. This is mainly
 *             useful for debugging the timesynch module.
 *
 */
rtimer_clock_t timesynch_offset(void);

/**
 * \brief      Get the current authority level of the time-synchronized time
 * \return     The current authority level of the time-synchronized time
 *
 *             This function returns the current authority level of
 *             the time-synchronized time. A node with a lower
 *             authority level is defined to have a better notion of
 *             time than a node with a higher authority
 *             level. Authority level 0 is best and should be used by
 *             a sink node that has a connection to an outside,
 *             "true", clock source.
 *
 */
int timesynch_authority_level(void);

/**
 * \brief      Set the authority level of the current time
 * \param level The authority level
 */
void timesynch_set_authority_level(int level);

#endif /* TIMESYNCH_H_ */

/** @} */
/** @} */
