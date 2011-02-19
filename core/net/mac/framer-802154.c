/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: framer-802154.c,v 1.12 2010/06/14 19:19:16 adamdunkels Exp $
 */

/**
 * \file
 *         MAC framer for IEEE 802.15.4
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "net/mac/framer-802154.h"
#include "net/mac/frame802154.h"
#include "net/packetbuf.h"
#include "lib/random.h"
#include <string.h>

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])
#else
#define PRINTF(...)
#define PRINTADDR(addr)
#endif

static uint8_t mac_dsn;
static uint8_t initialized = 0;
static const uint16_t mac_dst_pan_id = IEEE802154_PANID;
static const uint16_t mac_src_pan_id = IEEE802154_PANID;

/*---------------------------------------------------------------------------*/
static int
is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = mode == FRAME802154_SHORTADDRMODE ? 2 : 8;
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
create(void)
{
  frame802154_t params;
  uint8_t len;

  /* init to zeros */
  memset(&params, 0, sizeof(params));

  if(!initialized) {
    initialized = 1;
    mac_dsn = random_rand() & 0xff;
  }

  /* Build the FCF. */
  params.fcf.frame_type = FRAME802154_DATAFRAME;
  params.fcf.security_enabled = 0;
  params.fcf.frame_pending = packetbuf_attr(PACKETBUF_ATTR_PENDING);
  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &rimeaddr_null)) {
    params.fcf.ack_required = 0;
  } else {
    params.fcf.ack_required = packetbuf_attr(PACKETBUF_ATTR_MAC_ACK);
  }
  params.fcf.panid_compression = 0;

  /* Insert IEEE 802.15.4 (2003) version bit. */
  params.fcf.frame_version = FRAME802154_IEEE802154_2003;

  /* Increment and set the data sequence number. */
  if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO)) {
    params.seq = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
  } else {
    params.seq = mac_dsn++;
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, params.seq);
  }
/*   params.seq = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID); */

  /* Complete the addressing fields. */
  /**
     \todo For phase 1 the addresses are all long. We'll need a mechanism
     in the rime attributes to tell the mac to use long or short for phase 2.
  */
  if(sizeof(rimeaddr_t) == 2) {
    /* Use short address mode if rimeaddr size is short. */
    params.fcf.src_addr_mode = FRAME802154_SHORTADDRMODE;
  } else {
    params.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;
  }
  params.dest_pid = mac_dst_pan_id;

  /*
   *  If the output address is NULL in the Rime buf, then it is broadcast
   *  on the 802.15.4 network.
   */
  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &rimeaddr_null)) {
    /* Broadcast requires short address mode. */
    params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    params.dest_addr[0] = 0xFF;
    params.dest_addr[1] = 0xFF;

  } else {
    rimeaddr_copy((rimeaddr_t *)&params.dest_addr,
                  packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    /* Use short address mode if rimeaddr size is small */
    if(sizeof(rimeaddr_t) == 2) {
      params.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
    } else {
      params.fcf.dest_addr_mode = FRAME802154_LONGADDRMODE;
    }
  }

  /* Set the source PAN ID to the global variable. */
  params.src_pid = mac_src_pan_id;
  
  /*
   * Set up the source address using only the long address mode for
   * phase 1.
   */
  rimeaddr_copy((rimeaddr_t *)&params.src_addr, &rimeaddr_node_addr);

  params.payload = packetbuf_dataptr();
  params.payload_len = packetbuf_datalen();
  len = frame802154_hdrlen(&params);
  if(packetbuf_hdralloc(len)) {
    frame802154_create(&params, packetbuf_hdrptr(), len);

    PRINTF("15.4-OUT: %2X", params.fcf.frame_type);
    PRINTADDR(params.dest_addr.u8);
    PRINTF("%u %u (%u)\n", len, packetbuf_datalen(), packetbuf_totlen());

    return len;
  } else {
    PRINTF("15.4-OUT: too large header: %u\n", len);
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
parse(void)
{
  frame802154_t frame;
  int len;
  len = packetbuf_datalen();
  if(frame802154_parse(packetbuf_dataptr(), len, &frame) &&
     packetbuf_hdrreduce(len - frame.payload_len)) {
    if(frame.fcf.dest_addr_mode) {
      if(frame.dest_pid != mac_src_pan_id &&
         frame.dest_pid != FRAME802154_BROADCASTPANDID) {
        /* Packet to another PAN */
        PRINTF("15.4: for another pan %u\n", frame.dest_pid);
        return 0;
      }
      if(!is_broadcast_addr(frame.fcf.dest_addr_mode, frame.dest_addr)) {
        packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (rimeaddr_t *)&frame.dest_addr);
      }
    }
    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (rimeaddr_t *)&frame.src_addr);
    packetbuf_set_attr(PACKETBUF_ATTR_PENDING, frame.fcf.frame_pending);
    /*    packetbuf_set_attr(PACKETBUF_ATTR_RELIABLE, frame.fcf.ack_required);*/
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, frame.seq);

    PRINTF("15.4-IN: %2X", frame.fcf.frame_type);
    PRINTADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
    PRINTADDR(packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    PRINTF("%u (%u)\n", packetbuf_datalen(), len);

    return len - frame.payload_len;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct framer framer_802154 = {
  create, parse
};
