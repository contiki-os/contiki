/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
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
 */
/**
 * \file

 *         Orchestra: a slotframe dedicated to transmission of EBs.
 *         Nodes transmit at a timeslot defined as hash(MAC) % ORCHESTRA_EBSF_PERIOD
 *         Nodes listen at a timeslot defined as hash(time_source.MAC) % ORCHESTRA_EBSF_PERIOD
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "orchestra.h"
#include "net/packetbuf.h"

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
static struct tsch_slotframe *sf_eb;

/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot(const linkaddr_t *addr)
{
#if ORCHESTRA_EBSF_PERIOD > 0
  return ORCHESTRA_LINKADDR_HASH(addr) % ORCHESTRA_EBSF_PERIOD;
#else
  return 0xffff;
#endif
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  /* Select EBs only */
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_BEACONFRAME) {
    if(slotframe != NULL) {
      *slotframe = slotframe_handle;
    }
    if(timeslot != NULL) {
      *timeslot = get_node_timeslot(&linkaddr_node_addr);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  uint16_t old_ts = old != NULL ? get_node_timeslot(&old->addr) : 0xffff;
  uint16_t new_ts = new != NULL ? get_node_timeslot(&new->addr) : 0xffff;

  if(new_ts == old_ts) {
    return;
  }

  if(old_ts != 0xffff) {
    /* Stop listening to the old time source's EBs */
    if(old_ts == get_node_timeslot(&linkaddr_node_addr)) {
      /* This was the same timeslot as slot. Reset original link options */
      tsch_schedule_add_link(sf_eb, LINK_OPTION_TX, LINK_TYPE_ADVERTISING_ONLY,
        &tsch_broadcast_address, old_ts, 0);
    } else {
      /* Remove slot */
      tsch_schedule_remove_link_by_timeslot(sf_eb, old_ts);
    }
  }
  if(new_ts != 0xffff) {
    uint8_t link_options = LINK_OPTION_RX;
    if(new_ts == get_node_timeslot(&linkaddr_node_addr)) {
      /* This is also our timeslot, add necessary flags */
      link_options |= LINK_OPTION_TX;
    }
    /* Listen to the time source's EBs */
    tsch_schedule_add_link(sf_eb, link_options, LINK_TYPE_ADVERTISING_ONLY,
      &tsch_broadcast_address, new_ts, 0);
  }
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  slotframe_handle = sf_handle;
  channel_offset = sf_handle;
  sf_eb = tsch_schedule_add_slotframe(slotframe_handle, ORCHESTRA_EBSF_PERIOD);
  /* EB link: every neighbor uses its own to avoid contention */
  tsch_schedule_add_link(sf_eb,
                         LINK_OPTION_TX,
                         LINK_TYPE_ADVERTISING_ONLY, &tsch_broadcast_address,
                         get_node_timeslot(&linkaddr_node_addr), 0);
}
/*---------------------------------------------------------------------------*/
struct orchestra_rule eb_per_time_source = {
  init,
  new_time_source,
  select_packet,
  NULL,
  NULL,
};
