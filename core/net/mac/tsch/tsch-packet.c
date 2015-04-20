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
#include "net/mac/frame802154.h"
#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/* Fixed offset of the sync IE in EBs. Needed for quick update of the fields from interrupt.
 * FCF + seqno + pan ID + source MAC + MLME outer ID */
#define EB_IE_SYNC_OFFSET (2+1+2+8+2)

/* Parse 802.15.4e time correction IE */
static int
parse_ie_time_correction(uint8_t *buf, int buf_size,
    int32_t *drift, int *nack)
{
  if(buf_size >= TSCH_SYNC_IE_LEN) {
    /* If this is a Sync IE */
    if(buf[0] == 0x02 && buf[1] == 0x1e) {
      /* If the originator was a time source neighbor, the receiver adjust
       * its own clock by incorporating the received drift correction */
      uint16_t time_sync_field = 0;
      int16_t drift_us = 0;
      /* Extract drift correction from Sync-IE, cast from 12 to 16-bit,
       * and convert it to RTIMER ticks.
       * See page 88 in IEEE Std 802.15.4e-2012. */
      time_sync_field = buf[2] | buf[3] << 8;
      /* First extract NACK */
      if(nack != NULL) {
        *nack = (time_sync_field & (uint16_t)0x8000) ? 1 : 0;
      }
      /* Then cast from 12 to 16 bit signed */
      if(time_sync_field & 0x0800) { /* Negative integer */
        drift_us = time_sync_field | 0xf000;
      } else { /* Positive integer */
        drift_us = time_sync_field & 0x0fff;
      }
      /* Convert to RTIMER ticks */
      if(drift != NULL) {
        *drift = US_TO_RTIMERTICKS(drift_us);
      }
      return 1;
    }
  }
  return 0;
}

/* Parse pan ID */
static int
parse_panid(uint8_t *buf, int buf_size, uint16_t *panid)
{
  if(buf_size < 2) {
    return 0;
  } else {
    if(panid) {
      *panid = (buf[1] << 8) + buf[0];
    }
    return 2;
  }
}

/* Parse destination address and check it is ours */
static int
parse_addr(uint8_t *buf, int buf_size, linkaddr_t *addr)
{
  if(buf_size < LINKADDR_SIZE) {
    return 0;
  } else {
    if(addr) {
      int i;
      for(i = 0; i<LINKADDR_SIZE ; i++) {
        addr->u8[LINKADDR_SIZE - i - 1] = buf[i];
      }
    }
    return LINKADDR_SIZE;
  }
}

/* Append PANID to header */
static int
append_panid(uint8_t* const buf, int buf_size)
{
  if(buf_size < 2) {
    return 0;
  } else {
    buf[0] = IEEE802154_PANID & 0xff;
    buf[1] = (IEEE802154_PANID >> 8) & 0xff;
    return 2;
  }
}

/* Append MAC address to header */
static int
append_addr(uint8_t* const buf, int buf_size, linkaddr_t *addr)
{
  if(buf_size < LINKADDR_SIZE) {
    return 0;
  } else {
    int i;
    for(i = 0; i<LINKADDR_SIZE ; i++) {
      buf[i] = addr->u8[LINKADDR_SIZE - i - 1];
    }
    return LINKADDR_SIZE;
  }
}

/* Update packet with 802.15.4e time correction IE */
static int
append_ie_time_correction(uint8_t* const buf, int buf_size,
    int32_t drift, int nack)
{
  if(buf_size < TSCH_SYNC_IE_LEN) {
    return 0;
  } else {
    int16_t drift_us;
    uint16_t time_sync_field;
    drift_us = (int16_t)RTIMERTICKS_TO_US(drift);
    time_sync_field = drift_us & 0x0fff;
    if(nack) {
      time_sync_field |= 0x8000;
    }
    buf[0] = 0x02;
    buf[1] = 0x1e;
    buf[2] = time_sync_field & 0xff;
    buf[3] = (time_sync_field >> 8) & 0xff;
    return TSCH_SYNC_IE_LEN;
  }
}
/* Parse enhanced ACK packet, extract drift and nack */
int tsch_packet_parse_sync_ack(int32_t *drift, int *nack,
    uint8_t *ackbuf, int ackbuf_len, uint8_t seqno, int extract_sync_ie)
{
  if(ackbuf_len >= TSCH_BASE_ACK_LEN && 2 == ackbuf[0] && seqno == ackbuf[2]) {
    int ret;
    int is_ack = 1;
    int has_sync_ie = 0;
    int curr_len = TSCH_BASE_ACK_LEN;
#if TSCH_PACKET_DEST_ADDR_IN_ACK
    /* Check FCF byte 1: b10-b11:dest-addr-mode=3 */
    /* Check FCF byte 1: b12-b13:frame version=2 */
    if((ackbuf[1] & (0x0C | 0x20)) == (0x0C | 0x20)) {
      uint16_t panid;
      linkaddr_t addr;

      ret = parse_panid(&ackbuf[curr_len], ackbuf_len-curr_len, &panid);
      if(ret == 0 || panid != IEEE802154_PANID) {
        is_ack = 0;
      }
      curr_len += ret;

      ret = parse_addr(&ackbuf[curr_len], ackbuf_len-curr_len, &addr);
      if(ret == 0 || !linkaddr_cmp(&addr, &linkaddr_node_addr)) {
        is_ack = 0;
      }
      curr_len += ret;
    }
#endif
#if TSCH_PACKET_WITH_SYNC_IE
    /* Check FCF byte 1: FCF byte 1: b9:IE-list-present=1 */
    /* Check FCF byte 1: b12-b13:frame version=2 */
    if((ackbuf[1] & (0x02 | 0x20)) == (0x02 | 0x20)) {
      if(extract_sync_ie) {
        ret = parse_ie_time_correction(&ackbuf[curr_len], ackbuf_len-curr_len, drift, nack);
        if(ret) {
          has_sync_ie = 1;
        }
        curr_len += ret;
      }
    }
#endif
    return TSCH_ACK_OK*is_ack + TSCH_ACK_HAS_SYNC_IE*has_sync_ie;
  }
  return 0;
}
/* Construct enhanced ACK packet and return ACK length */
int
tsch_packet_make_sync_ack(int32_t drift, int nack,
    uint8_t *ackbuf, int ackbuf_len, linkaddr_t *dest_addr, uint8_t seqno)
{
  if(ackbuf_len < TSCH_ACK_LEN) {
    return 0;
  } else {
    int curr_len = TSCH_BASE_ACK_LEN;
    /* ACK frame */
    ackbuf[0] = 0x02; /* FCF byte 0: frame type: ACK, other fileds zeroed */
    ackbuf[1] = 0x00; /* FCF byte 1: 0 */
    ackbuf[2] = seqno;
#if TSCH_PACKET_DEST_ADDR_IN_ACK
    ackbuf[1] |= 0x0C; /* FCF byte 1: b10-b11:dest-addr-mode=3 */
    ackbuf[1] |= 0x20; /* FCF byte 1: b12-b13:frame version=2 */
    /* Append pan ID and destination MAC address */
    curr_len += append_panid(&ackbuf[curr_len], ackbuf_len-curr_len);
    curr_len += append_addr(&ackbuf[curr_len], ackbuf_len-curr_len, dest_addr);
#endif
#if TSCH_PACKET_WITH_SYNC_IE
    ackbuf[1] |= 0x02; /* FCF byte 1: b9:IE-list-present=1 */
    ackbuf[1] |= 0x20; /* FCF byte 1: b12-b13:frame version=2 */
    /* Append IE timesync */
    curr_len += append_ie_time_correction(&ackbuf[curr_len], ackbuf_len-curr_len, drift, nack);
#endif
    return curr_len;
  }
}

/* Extract 802.15.4 frame type from FCF least-significant byte */
uint8_t
tsch_packet_parse_frame_type_from_fcf_lsb(uint8_t fcf_lsb)
{
  /* Decode the FCF */
  uint8_t do_ack = ((fcf_lsb >> 5) & 1) == 1 ? DO_ACK : 0;
  uint8_t is_data = (fcf_lsb & 7) == FRAME802154_DATAFRAME ? IS_DATA : 0;
  uint8_t is_ack = (fcf_lsb & 7) == FRAME802154_ACKFRAME ? IS_ACK : 0;
  uint8_t is_eb = (fcf_lsb & 7) == FRAME802154_BEACONFRAME ? IS_EB : 0;
  return do_ack | is_data | is_ack | is_eb;
}
/* Extract 802.15.4 frame type from a struct packet_input */
uint8_t
tsch_packet_parse_frame_type(uint8_t *buf, uint8_t len, uint8_t *seqno)
{
  if(len < 3) {
    return 0;
  }

  /* Get the FCF */
  uint8_t fcf_lsb = 0;
  fcf_lsb = buf[0];
  if(seqno != NULL) {
    *seqno = buf[2];
  }

  return tsch_packet_parse_frame_type_from_fcf_lsb(fcf_lsb);
}

static int
is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = ((mode == FRAME802154_SHORTADDRMODE) ? 2 : 8);
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}

/* Extract addresses from raw packet */
int
tsch_packet_extract_addresses(uint8_t *buf, uint8_t len, linkaddr_t *source_address, linkaddr_t *dest_address)
{
  frame802154_t frame;
  uint8_t parsed = frame802154_parse(buf, len, &frame);
  if(parsed) {
    if(dest_address != NULL) {
      linkaddr_copy(dest_address, &linkaddr_null);
    }
    if(frame.fcf.dest_addr_mode) {
      if(frame.dest_pid != IEEE802154_PANID
          && frame.dest_pid != FRAME802154_BROADCASTPANDID) {
        /* Packet to another PAN */
        PRINTF("tsch_packet_extract_addresses: for another pan %u\n", frame.dest_pid);
        return 0;
      }
      if(!is_broadcast_addr(frame.fcf.dest_addr_mode, frame.dest_addr)) {
        if(dest_address != NULL) {
          linkaddr_copy(dest_address, (linkaddr_t *)frame.dest_addr);
        }
      }
    } else { /* broadcast (EB) packet with no addresses */
      if(frame.fcf.src_addr_mode && frame.src_pid != IEEE802154_PANID) {
        /* Reject if from another PAN */
        PRINTF("tsch_packet_extract_addresses: from another pan %u\n", frame.src_pid);
        return 0;
      }
    }
    if(source_address != NULL) {
      linkaddr_copy(source_address, (linkaddr_t *)frame.src_addr);
    }
  } else {
    PRINTF("tsch_packet_extract_addresses: failed to parse\n");
  }
  return parsed;
}

/* Parse 802.15.4e Sync Information Element */
static int
parse_ie_sync(uint8_t* const buf, int buf_size,
    struct asn_t *asn, uint8_t *join_priority)
{
  if(buf_size < 8) {
    return 0;
  } else {
    /* Short IE: 2 bytes header, c.f. fig 48r in IEEE 802.15.4e
     * b0-7: length=6, b8-14: sub-ID=0x1a, b15: type=0 */
    if(buf[0] != 6 || buf[1] != 0x1a) {
      return 0;
    }

    if(asn) {
      asn->ls4b = (uint32_t)buf[2];
      asn->ls4b |= (uint32_t)buf[3] << 8;
      asn->ls4b |= (uint32_t)buf[4] << 16;
      asn->ls4b |= (uint32_t)buf[5] << 24;
      asn->ms1b = (uint8_t)buf[6];
    }
    if(join_priority) {
      *join_priority = buf[7];
    }

    return 8;
  }
}

/* Parse with 802.15.4e timeslot template */
static int
parse_ie_timeslot_template(uint8_t* const buf, int buf_size,
    uint8_t *hop_sequence_id)
{
  if(buf_size < 3) {
    return 0;
  } else {
    /* Long IE: 2 bytes header, c.f. fig 48s in IEEE 802.15.4e
     * b0-10: length=1, b11-14: sub-ID=9, b15: type=1 */
    if(buf[0] != 1 || (buf[1] != ((9 << 3) | (1 << 7)))) {
      return 0;
    }
    if(hop_sequence_id) {
      *hop_sequence_id = buf[2];
    }
    return 3;
  }
}

/* Parse 802.15.4e hop sequence template */
static int
parse_ie_hop_sequence_template(uint8_t* const buf, int buf_size,
    uint8_t *slot_template_id)
{
  if(buf_size < 3) {
    return 0;
  } else {
    /* Short IE: 2 bytes header, c.f. fig 48r in IEEE 802.15.4e
     * b0-7: length=1, b8-14: sub-ID=0x1c, b15: type=0 */
    if(buf[0] != 1 || buf[1] != 0x1c) {
      return 0;
    }
    if(slot_template_id) {
      *slot_template_id = buf[2];
    }
    return 3;
  }
}

/* Parse 802.15.4e MLME outer IE */
static int
parse_ie_mlme_outer(uint8_t* const buf, int buf_size,
    uint8_t *sub_ies_length)
{
  if(buf_size < 2) {
    return 0;
  } else {
    /* MLME IE: 2 bytes header, c.f. fig 48q in IEEE 802.15.4e
     * b0-10: length, b11-14: group ID=1, b15: type=1 */
    if((buf[1] & 0x88) != 0x88) {
      return 0;
    }
    if(sub_ies_length) {
      *sub_ies_length = buf[0] | ((buf[1] & 0x07) << 8);
    }
    return 2;
  }
}

/* Update packet with 802.15.4e Sync Information Element */
static int
append_ie_sync(uint8_t* const buf, int buf_size,
    struct asn_t *asn, uint8_t join_priority)
{
  if(buf_size < 8) {
    return 0;
  } else {
    /* Short IE: 2 bytes header, c.f. fig 48r in IEEE 802.15.4e
     * b0-7: length=6, b8-14: sub-ID=0x1a, b15: type=0 */
    buf[0] = 6;
    buf[1] = 0x1a;

    if(asn != NULL) {
      buf[2] = asn->ls4b;
      buf[3] = asn->ls4b >> 8;
      buf[4] = asn->ls4b >> 16;
      buf[5] = asn->ls4b >> 24;
      buf[6] = asn->ms1b;
    } else {
      memset(buf, 0, 5);
    }
    buf[7] = join_priority;

    return 8;
  }
}

/* Update packet with 802.15.4e timeslot template */
static int
append_ie_timeslot_template(uint8_t* const buf, int buf_size,
    uint8_t hop_sequence_id)
{
  if(buf_size < 3) {
    return 0;
  } else {
    /* Long IE: 2 bytes header, c.f. fig 48s in IEEE 802.15.4e
     * b0-10: length=1, b11-14: sub-ID=9, b15: type=1 */
    buf[0] = 1;
    buf[1] = (9 << 3) | (1 << 7);
    buf[2] = hop_sequence_id;
    return 3;
  }
}

/* Update packet with 802.15.4e hop sequence template */
static int
append_ie_hop_sequence_template(uint8_t* const buf, int buf_size,
    uint8_t slot_template_id)
{
  if(buf_size < 3) {
    return 0;
  } else {
    /* Short IE: 2 bytes header, c.f. fig 48r in IEEE 802.15.4e
     * b0-7: length=1, b8-14: sub-ID=0x1c, b15: type=0 */
    buf[0] = 1;
    buf[1] = 0x1c;
    buf[2] = slot_template_id;
    return 3;
  }
}

/* Update packet with 802.15.4e MLME outer IE */
static int
append_ie_mlme_outer(uint8_t* const buf, int buf_size,
    uint8_t sub_ies_length)
{
  if(buf_size < 2) {
    return 0;
  } else {
    /* MLME IE: 2 bytes header, c.f. fig 48q in IEEE 802.15.4e
     * b0-10: length, b11-14: group ID=1, b15: type=1 */
    buf[0] = sub_ies_length & 0xff;
    buf[1] = 0x88 | ((sub_ies_length >> 8) & 0x07);
    return 2;
  }
}

/* Create an EB packet */
int
tsch_packet_make_eb(uint8_t* const buf, uint8_t buf_size, uint8_t seqno)
{
  uint8_t curr_len = 0;
  uint8_t ie_mlme_offset;

  /* FCF: 2 bytes */
  /* b0-2: frame type=0, b3: security=0, b4: pending=0, b5: AR, b6: PAN ID compression, b7: reserved */
  /* b8: seqno suppression, b9:IE-list-present=1, b10-11: destination address mode=0,
   * b12-b13:frame version=2, b14-15: src address mode=3 */
  buf[curr_len++] = 0x00;
  buf[curr_len++] = 0xe2;

  /* Seqno: 1 byte */
  buf[curr_len++] = seqno;

  /* Append PANID and src address */
  curr_len += append_panid(&buf[curr_len], buf_size-curr_len);
  curr_len += append_addr(&buf[curr_len], buf_size-curr_len, &linkaddr_node_addr);

  /* Save offset of MLME IE, which will be updated later with the total sub-IEs length */
  ie_mlme_offset = curr_len;
  /* Leave 2 bytes for the MLME outer IE descriptor */
  curr_len += 2;

  /* Sanity check: before adding sync IE, check that it is at the expected fixed offset.
   * Needed as it will be updated later from tsch_packet_update_eb, using the fixed offset */
  if(curr_len != EB_IE_SYNC_OFFSET) {
    return 0;
  }

  /* Sync IE */
  curr_len += append_ie_sync(&buf[curr_len], buf_size-curr_len, NULL, 0);
  /* Timeslot template IE */
  curr_len += append_ie_timeslot_template(&buf[curr_len], buf_size-curr_len, 1);
  /* Hop sequence template IE */
  curr_len += append_ie_hop_sequence_template(&buf[curr_len], buf_size-curr_len, 1);

  /* TODO append TSCH slotframe & link IE */

  /* MLME IE */
  curr_len += append_ie_mlme_outer(&buf[ie_mlme_offset], 2, curr_len-ie_mlme_offset-2);

  return curr_len;
}

/* Update ASN in EB packet */
int
tsch_packet_update_eb(uint8_t *buf, uint8_t buf_size)
{
  if(/* is beacon? */
     (FRAME802154_BEACONFRAME == (buf[0] & 7))
     /* IE FCF as expected? */
     && ((buf[1] & 0xe2) == 0xe2)) {
    /* Update ASN and join priority */
    return append_ie_sync(
        &buf[EB_IE_SYNC_OFFSET], buf_size-EB_IE_SYNC_OFFSET, &current_asn, tsch_join_priority);
  }
  return 0;
}

uint8_t
tsch_parse_eb(uint8_t *buf, uint8_t buf_size, linkaddr_t *source_address, struct asn_t *asn, uint8_t *join_priority)
{
  uint8_t curr_len = 0;
  uint8_t sub_ies_length = 0;
  uint8_t ie_mlme_offset;
  uint16_t panid;
  uint8_t slot_template_id;
  uint8_t hop_sequence_id;
  linkaddr_t addr;
  int ret;

  /* TODO support parsing of any EB rather than only the fields we expect
   * in pre-defined order */

  /* FCF: 2 bytes */
  if(buf[curr_len++] != 0x00) {
    return 0;
  }
  if(buf[curr_len++] != 0xe2) {
    return 0;
  }

  /* Seqno: 1 byte. We skip it. */
  curr_len++;

  /* Extract and check pan ID */
  ret = parse_panid(&buf[curr_len], buf_size-curr_len, &panid);
  if(ret == 0 || panid != IEEE802154_PANID) {
    return 0;
  }
  curr_len += ret;

  /* Extract and check source address */
  ret = parse_addr(&buf[curr_len], buf_size-curr_len, source_address);
  if(ret == 0) {
    return 0;
  }
  curr_len += ret;

  /* Parse MLME outer IE descriptor. Save offset so we can later check the length field */
  ie_mlme_offset = curr_len;
  ret = parse_ie_mlme_outer(&buf[curr_len], buf_size-curr_len, &sub_ies_length);
  if(ret == 0) {
    return 0;
  }
  curr_len += ret;

  /* Sanity check: check the sync IE is at the expected fixed offset. */
  if(curr_len != EB_IE_SYNC_OFFSET) {
    return 0;
  }

  /* Parse ASN and join priority */
  ret = parse_ie_sync(&buf[EB_IE_SYNC_OFFSET], buf_size-EB_IE_SYNC_OFFSET, asn, join_priority);
  if(ret == 0) {
    return 0;
  }
  curr_len += ret;

  /* Timeslot template IE */
  ret = parse_ie_timeslot_template(&buf[curr_len], buf_size-curr_len, &slot_template_id);
  if(ret == 0 || slot_template_id != 1) {
    return 0;
  }
  curr_len += ret;

  /* Hop sequence template IE */
  ret = parse_ie_hop_sequence_template(&buf[curr_len], buf_size-curr_len, &hop_sequence_id);
  if(ret == 0 || hop_sequence_id != 1) {
    return 0;
  }
  curr_len += ret;

  /* Finally, check sub_ies_length */
  if(sub_ies_length != curr_len-ie_mlme_offset-2) {
    return 0;
  }

  return curr_len;
}
