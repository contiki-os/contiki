/**
 * \addtogroup timesynch
 * @{
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
 * $Id: timesynch.c,v 1.9 2009/12/09 18:08:26 adamdunkels Exp $
 */

/**
 * \file
 *         A simple time synchronization mechanism
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/timesynch.h"
#include "net/rime/packetbuf.h"
#include "net/rime.h"
#include "dev/cc2420.h"

#if TIMESYNCH_CONF_ENABLED
static int authority_level;
static rtimer_clock_t offset;

/*---------------------------------------------------------------------------*/
int
timesynch_authority_level(void)
{
  return authority_level;
}
/*---------------------------------------------------------------------------*/
void
timesynch_set_authority_level(int level)
{
  authority_level = level;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_time(void)
{
  return RTIMER_NOW() + offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_time_to_rtimer(rtimer_clock_t synched_time)
{
  return synched_time - offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_rtimer_to_time(rtimer_clock_t rtimer_time)
{
  return rtimer_time + offset;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
timesynch_offset(void)
{
  return offset;
}
/*---------------------------------------------------------------------------*/
static void
adjust_offset(rtimer_clock_t authoritative_time, rtimer_clock_t local_time)
{
  offset = offset + authoritative_time - local_time;
}
/*---------------------------------------------------------------------------*/
static void
incoming_packet(void)
{
  if(packetbuf_totlen() != 0) {
    /* We check the authority level of the sender of the incoming
       packet. If the sending node has a lower authority level than we
       have, we synchronize to the time of the sending node and set our
       own authority level to be one more than the sending node. */
    if(cc2420_authority_level_of_sender < authority_level) {
      adjust_offset(cc2420_time_of_departure,
		    cc2420_time_of_arrival);
      if(cc2420_authority_level_of_sender + 1 != authority_level) {
	authority_level = cc2420_authority_level_of_sender + 1;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
#if 0
static void
periodic_authority_increase(void *ptr)
{
  /* XXX the authority level should be increased over time except
     for the sink node (which has authority 0). */
}
#endif
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(sniffer, incoming_packet, NULL);
/*---------------------------------------------------------------------------*/
void
timesynch_init(void)
{
  rime_sniffer_add(&sniffer);
}
/*---------------------------------------------------------------------------*/
#endif /* TIMESYNCH_CONF_ENABLED */
/** @} */
