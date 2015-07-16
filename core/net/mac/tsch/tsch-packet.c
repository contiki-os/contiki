/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 *         TSCH packet format management
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-security.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/llsec/anti-replay.h"
#include "lib/ccm-star.h"
#include "lib/aes-128.h"
#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/* Construct enhanced ACK packet and return ACK length */
int
tsch_packet_create_eack(uint8_t *buf, int buf_size,
    linkaddr_t *dest_addr, uint8_t seqno, int16_t drift, int nack)
{
  int ret;
  uint8_t curr_len = 0;
  frame802154_t p;
  struct ieee802154_ies ies;

  memset(&p, 0, sizeof(p));
  p.fcf.frame_type = FRAME802154_ACKFRAME;
  p.fcf.frame_version = FRAME802154_IEEE802154;
  p.fcf.ie_list_present = 1;
  /* Compression unset. According to IEEE802.15.4e-2012:
   * - if no address is present: elide PAN ID
   * - if at least one address is present: include exactly one PAN ID (dest by default) */
  p.fcf.panid_compression = 0;
  p.dest_pid = IEEE802154_PANID;
  p.seq = seqno;
#if TSCH_PACKET_DEST_ADDR_IN_ACK
  if(dest_addr != NULL) {
    p.fcf.dest_addr_mode = FRAME802154_LONGADDRMODE;
    linkaddr_copy((linkaddr_t*)&p.dest_addr, dest_addr);
  }
#endif
#if TSCH_PACKET_SRC_ADDR_IN_ACK
  p.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;
  p.src_pid = IEEE802154_PANID;
  linkaddr_copy((linkaddr_t*)&p.src_addr, &linkaddr_node_addr);
#endif
#if LLSEC802154_SECURITY_LEVEL
  if(tsch_is_pan_secured) {
    p.fcf.security_enabled = 1;
    p.aux_hdr.security_control.security_level = TSCH_SECURITY_KEY_SEC_LEVEL_ACK;
    p.aux_hdr.security_control.key_id_mode = 1;
    p.aux_hdr.security_control.frame_counter_suppression = 1;
    p.aux_hdr.security_control.frame_counter_size = 1;
    p.aux_hdr.key_index = TSCH_SECURITY_KEY_INDEX_ACK;
  }
#endif

  if((curr_len = frame802154_create(&p, buf)) == 0) {
    return 0;
  }

  /* Append IE timesync */
  memset(&ies, 0, sizeof(ies));
  ies.ie_time_correction = drift;
  ies.ie_is_nack = nack;

  if((ret = frame80215e_create_ie_header_ack_nack_time_correction(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  return curr_len;
}

/* Parse enhanced ACK packet, extract drift and nack */
int
tsch_packet_parse_eack(uint8_t *buf, int buf_size,
    uint8_t seqno, frame802154_t *frame, struct ieee802154_ies *ies, uint8_t *hdr_len)
{
  uint8_t curr_len = 0;
  int ret;
  linkaddr_t dest;

  if(frame == NULL || buf_size < 0) {
    return 0;
  }

  /* Parse 802.15.4-2006 frame, i.e. all fields before Information Elements */
  if((ret = frame802154_parse(buf, buf_size, frame)) < 3) {
    return 0;
  }
  if(hdr_len != NULL) {
    *hdr_len = ret;
  }
  curr_len += ret;

  /* Check seqno */
  if(seqno != frame->seq) {
    return 0;
  }

  /* Check destination address (if any) */
  ret = frame802154_packet_extract_addresses(frame, NULL, &dest);
  if(ret == 0 ||
      (!linkaddr_cmp(&dest, &linkaddr_node_addr)
          && !linkaddr_cmp(&dest, &linkaddr_null))) {
    return 0;
  }

  if(ies != NULL) {
    memset(ies, 0, sizeof(struct ieee802154_ies));
  }

  if(frame->fcf.ie_list_present) {
    int mic_len = 0;
#if LLSEC802154_SECURITY_LEVEL
    /* Check if there is space for the security MIC (if any) */
    mic_len = tsch_security_mic_len(frame);
    if(buf_size < curr_len + mic_len) {
      return 0;
    }
#endif
    /* Parse information elements. We need to substract the MIC length, as the exact payload len is needed while parsing */
    if((ret = frame802154e_parse_information_elements(buf + curr_len, buf_size - curr_len - mic_len, ies)) == -1) {
      return 0;
    }
    curr_len += ret;
  }

  if(hdr_len != NULL) {
    *hdr_len += ies->ie_payload_ie_offset;
  }

  return curr_len;
}

/* Create an EB packet */
int
tsch_packet_create_eb(uint8_t *buf, int buf_size, uint8_t seqno,
    uint8_t *hdr_len, uint8_t *tsch_sync_ie_offset)
{
  int ret = 0;
  uint8_t curr_len = 0;
  uint8_t mlme_ie_offset;

  frame802154_t p;
  struct ieee802154_ies ies;

  if(buf_size < PACKETBUF_SIZE) {
    return 0;
  }

  /* Create 802.15.4 header */
  memset(&p, 0, sizeof(p));
  p.fcf.frame_type = FRAME802154_BEACONFRAME;
  p.fcf.ie_list_present = 1;
  p.fcf.frame_version = FRAME802154_IEEE802154;
  p.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;
  p.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
  p.seq = seqno;
  p.fcf.sequence_number_suppression = FRAME802154_SUPPR_SEQNO;
  /* It is important not to compress PAN ID, as this would result in not including either
   * source nor destination PAN ID, leaving potential joining devices unaware of the PAN ID. */
  p.fcf.panid_compression = 0;

  p.src_pid = frame802154_get_pan_id();
  p.dest_pid = frame802154_get_pan_id();
  linkaddr_copy((linkaddr_t*)&p.src_addr, &linkaddr_node_addr);
  p.dest_addr[0] = 0xff;
  p.dest_addr[1] = 0xff;

#if LLSEC802154_SECURITY_LEVEL
  if(tsch_is_pan_secured) {
    p.fcf.security_enabled = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL) > 0;
    p.aux_hdr.security_control.security_level = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);
    p.aux_hdr.security_control.key_id_mode = packetbuf_attr(PACKETBUF_ATTR_KEY_ID_MODE);
    p.aux_hdr.security_control.frame_counter_suppression = 1;
    p.aux_hdr.security_control.frame_counter_size = 1;
    p.aux_hdr.key_index = packetbuf_attr(PACKETBUF_ATTR_KEY_INDEX);
  }
#endif

  if((curr_len = frame802154_create(&p, buf)) == 0) {
    return 0;
  }

  /* Prepare Information Elements for inclusion in the EB */
  memset(&ies, 0, sizeof(ies));

  /* 6TiSCH minimal timeslot timing sequence ID: 0 */
  ies.ie_tsch_timeslot_id = 0;
  /* Explicit inclusion of timeslot timing with: */
  /* ies.ie_tsch_timeslot_id = 1;
  ies.ie_tsch_timeslot.cca_offset = TSCH_DEFAULT_TS_CCA_OFFSET;
  ies.ie_tsch_timeslot.cca = TSCH_DEFAULT_TS_CCA;
  ies.ie_tsch_timeslot.tx_offset = TSCH_DEFAULT_TS_TX_OFFSET;
  ies.ie_tsch_timeslot.rx_offset = TSCH_DEFAULT_TS_RX_OFFSET;
  ies.ie_tsch_timeslot.rx_ack_delay = TSCH_DEFAULT_TS_RX_ACK_DELAY;
  ies.ie_tsch_timeslot.tx_ack_delay = TSCH_DEFAULT_TS_TX_ACK_DELAY;
  ies.ie_tsch_timeslot.rx_wait = TSCH_DEFAULT_TS_RX_WAIT;
  ies.ie_tsch_timeslot.ack_wait = TSCH_DEFAULT_TS_ACK_WAIT;
  ies.ie_tsch_timeslot.rx_tx = TSCH_DEFAULT_TS_RX_TX;
  ies.ie_tsch_timeslot.max_ack = TSCH_DEFAULT_TS_MAX_ACK;
  ies.ie_tsch_timeslot.max_tx = TSCH_DEFAULT_TS_MAX_TX;
  ies.ie_tsch_timeslot.timeslot_length = TSCH_DEFAULT_TS_TIMESLOT_LENGTH; */

  /* 6TiSCH minimal hopping sequence ID: 0 */
  ies.ie_channel_hopping_sequence_id = 0;
  /* Explicit inclusion of channel hopping sequence with: */
  /* if(tsch_hopping_sequence_length.val <= sizeof(ies.ie_hopping_sequence_list)) {
    ies.ie_channel_hopping_sequence_id = 1;
    ies.ie_hopping_sequence_len = tsch_hopping_sequence_length.val;
    memcpy(ies.ie_hopping_sequence_list, tsch_hopping_sequence, ies.ie_hopping_sequence_len);
  } */

  /* Include no slotframe */
  /* ies.ie_tsch_slotframe_and_link.num_slotframes = 0; */
  /* Add Slotframe and Link IE: 6TiSCH minimal schedule of length */
  ies.ie_tsch_slotframe_and_link.num_slotframes = 1;
  ies.ie_tsch_slotframe_and_link.slotframe_handle = 0;
  ies.ie_tsch_slotframe_and_link.slotframe_size = TSCH_SCHEDULE_DEFAULT_LENGTH;
  ies.ie_tsch_slotframe_and_link.num_links = 1;
  ies.ie_tsch_slotframe_and_link.links[0].timeslot = 0;
  ies.ie_tsch_slotframe_and_link.links[0].channel_offset = 0;
  ies.ie_tsch_slotframe_and_link.links[0].link_options = LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED | LINK_OPTION_TIME_KEEPING;

  /* First add header-IE termination IE to stipulate that next come payload IEs */
  if((ret = frame80215e_create_ie_header_list_termination_1(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  /* We start payload IEs, save offset */
  if(hdr_len != NULL) {
    *hdr_len = curr_len;
  }

  /* Save offset of the MLME IE descriptor, we need to know the total length
   * before writing it */
  mlme_ie_offset = curr_len;
  curr_len += 2; /* Space needed for MLME descriptor */

  /* Save the offset of the TSCH Synchronization IE, needed to update ASN and join priority before sending */
  if(tsch_sync_ie_offset != NULL) {
    *tsch_sync_ie_offset = curr_len;
  }
  if((ret = frame80215e_create_ie_tsch_synchronization(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  if((ret = frame80215e_create_ie_tsch_timeslot(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  if((ret = frame80215e_create_ie_tsch_channel_hopping_sequence(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  if((ret = frame80215e_create_ie_tsch_slotframe_and_link(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;

  ies.ie_mlme_len = curr_len - mlme_ie_offset - 2;
  if((ret = frame80215e_create_ie_mlme(buf+mlme_ie_offset, buf_size-mlme_ie_offset, &ies)) == -1) {
    return -1;
  }

  /* Payload IE list termination: optional */
  /*
  if((ret = frame80215e_create_ie_payload_list_termination(buf+curr_len, buf_size-curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += ret;
  */

  return curr_len;
}

/* Update ASN in EB packet */
int
tsch_packet_update_eb(uint8_t *buf, int buf_size, uint8_t tsch_sync_ie_offset)
{
  struct ieee802154_ies ies;
  ies.ie_asn = current_asn;
  ies.ie_join_priority = tsch_join_priority;
  frame80215e_create_ie_tsch_synchronization(buf+tsch_sync_ie_offset, buf_size-tsch_sync_ie_offset, &ies);
  return 1;
}

/* Parse a IEEE 802.15.4e TSCH Enhanced Beacon (EB) */
int
tsch_packet_parse_eb(uint8_t *buf, int buf_size,
    frame802154_t *frame, struct ieee802154_ies *ies, uint8_t *hdr_len, int frame_without_mic)
{
  uint8_t curr_len = 0;
  int ret;

  if(frame == NULL || buf_size < 0) {
    return 0;
  }

  /* Parse 802.15.4-2006 frame, i.e. all fields before Information Elements */
  if((ret = frame802154_parse(buf, buf_size, frame)) == 0) {
    LOG("TSCH:! parse_eb: failed to parse frame\n");
    return 0;
  }

  if(frame->fcf.frame_type != FRAME802154_BEACONFRAME) {
    LOG("TSCH:! parse_eb: frame is not a beacon. Frame type %u, FCF %02x %02x\n", frame->fcf.frame_type, buf[0], buf[1]);
    return 0;
  }

  if(hdr_len != NULL) {
    *hdr_len = ret;
  }
  curr_len += ret;

  if(ies != NULL) {
    memset(ies, 0, sizeof(struct ieee802154_ies));
    ies->ie_join_priority = 0xff; /* Use max value in case the Beacon does not include a join priority */
  }
  if(frame->fcf.ie_list_present) {
    /* Check if there is space for the security MIC (if any) */
    int mic_len = 0;
#if LLSEC802154_SECURITY_LEVEL
    if(!frame_without_mic) {
      mic_len = tsch_security_mic_len(frame);
      if(buf_size < curr_len + mic_len) {
        return 0;
      }
    }
#endif

    /* Parse information elements. We need to substract the MIC length, as the exact payload len is needed while parsing */
    if((ret = frame802154e_parse_information_elements(buf + curr_len, buf_size - curr_len - mic_len, ies)) == -1) {
      LOG("TSCH:! parse_eb: failed to parse IEs\n");
      return 0;
    }
    curr_len += ret;
  }

  if(hdr_len != NULL) {
    *hdr_len += ies->ie_payload_ie_offset;
  }

  return curr_len;
}
