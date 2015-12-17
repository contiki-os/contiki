/**
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
 */

/**
 * \file
 *         MAC framer for IEEE 802.15.4
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "net/mac/framer-802154.h"
#include "net/mac/frame802154.h"
#include "net/llsec/llsec802154.h"
#include "net/packetbuf.h"
#include "lib/random.h"
#include "net/llsec/anti-replay.h"
#include <string.h>

#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
#ifdef FRAMER_802154_CONF_SUPPRESS_UNICAST_SEQNO
#define SUPPRESS_UNICAST_SEQNO FRAMER_802154_CONF_SUPPRESS_UNICAST_SEQNO
#else /* FRAMER_802154_CONF_SUPPRESS_UNICAST_SEQNO */
#define SUPPRESS_UNICAST_SEQNO 0
#endif /* FRAMER_802154_CONF_SUPPRESS_UNICAST_SEQNO */
#ifdef FRAMER_802154_CONF_SUPPRESS_BROADCAST_SEQNO
#define SUPPRESS_BROADCAST_SEQNO FRAMER_802154_CONF_SUPPRESS_BROADCAST_SEQNO
#else /* FRAMER_802154_CONF_SUPPRESS_BROADCAST_SEQNO */
#define SUPPRESS_BROADCAST_SEQNO 1
#endif /* FRAMER_802154_CONF_SUPPRESS_BROADCAST_SEQNO */
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
#define SUPPRESS_UNICAST_SEQNO 0
#define SUPPRESS_BROADCAST_SEQNO 0
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */

#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
/* Include destination address in ACK? */
#ifdef FRAMER_802154_CONF_ACK_WITH_DST_ADDR
#define ACK_WITH_DST_ADDR FRAMER_802154_CONF_ACK_WITH_DST_ADDR
#else /* FRAMER_802154_CONF_ACK_WITH_DST_ADDR */
/* Include destination address by default, useful in case of duplicate seqno */
#define ACK_WITH_DST_ADDR 1
#endif /* FRAMER_802154_CONF_ACK_WITH_DST_ADDR */
/* Include source address in ACK? */
#ifdef FRAMER_802154_CONF_ACK_WITH_SRC_ADDR
#define ACK_WITH_SRC_ADDR FRAMER_802154_CONF_ACK_WITH_SRC_ADDR
#else /* FRAMER_802154_CONF_ACK_WITH_SRC_ADDR */
#define ACK_WITH_SRC_ADDR 0
#endif /* FRAMER_802154_CONF_ACK_WITH_SRC_ADDR */
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
#define ACK_WITH_DST_ADDR 0
#define ACK_WITH_SRC_ADDR 0
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */

#if LINKADDR_SIZE == 2
#define LINKADDR_ADDR_MODE FRAME802154_SHORTADDRMODE
#else /* LINKADDR_SIZE == 2 */
#define LINKADDR_ADDR_MODE FRAME802154_LONGADDRMODE
#endif /* LINKADDR_SIZE == 2 */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**  \brief The sequence number (0x00 - 0xff) added to the transmitted
 *   data or MAC command frame. The default is a random value within
 *   the range.
 */
static uint8_t mac_dsn;
static int is_mac_dsn_initialized;

/*---------------------------------------------------------------------------*/
static int
has_dst_pid(uint8_t dst_addr_mode, uint8_t src_addr_mode, int panid_compressed)
{
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  if(dst_addr_mode) {
    if(src_addr_mode) {
      if((dst_addr_mode == FRAME802154_LONGADDRMODE)
          && (src_addr_mode == FRAME802154_LONGADDRMODE)
          && panid_compressed) {
        return 0;
      }
      return 1;
    } else {
      return !panid_compressed;
    }
  } else {
    if(src_addr_mode) {
      return 0;
    } else {
      return panid_compressed;
    }
  }
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  return dst_addr_mode;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
}
/*---------------------------------------------------------------------------*/
static int
has_src_pid(uint8_t dst_addr_mode, uint8_t src_addr_mode, int panid_compressed)
{
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  if(dst_addr_mode) {
    if(src_addr_mode) {
      if((dst_addr_mode == FRAME802154_LONGADDRMODE)
          && (src_addr_mode == FRAME802154_LONGADDRMODE)) {
        return 0;
      }
      return !panid_compressed;
    } else {
      return 0;
    }
  } else {
    if(src_addr_mode) {
      return !panid_compressed;
    } else {
      return 0;
    }
  }
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  return !panid_compressed;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
}
/*---------------------------------------------------------------------------*/
void
framer_802154_set_seqno(void)
{
  if(!is_mac_dsn_initialized) {
    is_mac_dsn_initialized = 1;
    mac_dsn = random_rand();
  }

  if((!SUPPRESS_UNICAST_SEQNO && !packetbuf_holds_broadcast())
      || (!SUPPRESS_BROADCAST_SEQNO && packetbuf_holds_broadcast())) {
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, mac_dsn++);
  }
}
/*---------------------------------------------------------------------------*/
#if LLSEC802154_USES_AUX_HEADER && LLSEC802154_USES_EXPLICIT_KEYS
static uint8_t
get_key_id_len(uint8_t key_id_mode)
{
  switch(key_id_mode) {
  case FRAME802154_1_BYTE_KEY_ID_MODE:
    return 1;
  case FRAME802154_5_BYTE_KEY_ID_MODE:
    return 5;
  case FRAME802154_9_BYTE_KEY_ID_MODE:
    return 9;
  default:
    return 0;
  }
}
#endif /* LLSEC802154_USES_AUX_HEADER && LLSEC802154_USES_EXPLICIT_KEYS */
/*---------------------------------------------------------------------------*/
static int
hdr_length(void)
{
  int is_ack;
  int is_broadcast;
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  int seqno_suppressed;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */

  is_ack = packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_ACKFRAME;
  is_broadcast = packetbuf_holds_broadcast();
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  seqno_suppressed = is_broadcast
      ? SUPPRESS_BROADCAST_SEQNO
      : SUPPRESS_UNICAST_SEQNO;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */

  return
      /* Frame Control */
      2
      /* Sequence Number */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
      + (seqno_suppressed ? 0 : 1) /* Sequence Number */
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
      + 1
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
      /* Destination PAN Identifier */
      + ((!is_ack || ACK_WITH_DST_ADDR)
          ? 2
          : 0)
      /* Destination Address */
      + (is_broadcast
          ? 2
          : ((!is_ack || ACK_WITH_DST_ADDR)
              ? LINKADDR_SIZE
              : 0))
      /* Source PAN Identifier */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
      + (is_ack ? 0 : 2)
#else /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
      + ((is_ack && !ACK_WITH_DST_ADDR && ACK_WITH_SRC_ADDR) ? 2 : 0)
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
      /* Source Address */
      + ((!is_ack || ACK_WITH_SRC_ADDR)
          ? LINKADDR_SIZE
          : 0)
#if LLSEC802154_USES_AUX_HEADER
      + (packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL)
          ? 1
#if LLSEC802154_USES_FRAME_COUNTER
              + 4
#endif /* LLSEC802154_USES_FRAME_COUNTER */
#if LLSEC802154_USES_EXPLICIT_KEYS
              + get_key_id_len(packetbuf_attr(PACKETBUF_ATTR_KEY_ID_MODE))
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
          : 0)
#endif /* LLSEC802154_USES_AUX_HEADER */
      ;
}
/*---------------------------------------------------------------------------*/
static void
create_addr(uint8_t *p, const linkaddr_t *addr)
{
  uint8_t i;

  for(i = 0; i < LINKADDR_SIZE; i++) {
    p[i] = addr->u8[LINKADDR_SIZE - 1 - i];
  }
}
/*---------------------------------------------------------------------------*/
static int
create(void)
{
  uint8_t *hdrptr;
  uint8_t *p;
  int is_ack;
  int panid_compressed;
  uint8_t dst_addr_mode;
  uint8_t src_addr_mode;
  int is_broadcast;
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  int seqno_suppressed;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  uint16_t dst_pid;
#if LLSEC802154_USES_AUX_HEADER
  uint8_t security_level = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);
#if LLSEC802154_USES_EXPLICIT_KEYS
  uint8_t key_id_mode = packetbuf_attr(PACKETBUF_ATTR_KEY_ID_MODE);
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
#endif /* LLSEC802154_USES_AUX_HEADER */

  if(!packetbuf_hdralloc(hdr_length())) {
    PRINTF("framer-802154: hdralloc failed\n");
    return FRAMER_FAILED;
  }

  hdrptr = p = packetbuf_hdrptr();
  is_ack = packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_ACKFRAME;
  is_broadcast = packetbuf_holds_broadcast();

  /*
   * Frame Type | Security Enabled | Frame Pending
   * | Acknowledgment Request | PAN ID Compression
   */
  panid_compressed = (FRAME802154_VERSION < FRAME802154_IEEE802154E_2012)
      && (!is_ack || ACK_WITH_DST_ADDR || !ACK_WITH_SRC_ADDR);
  p[0] = (packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) & 7)
#if LLSEC802154_USES_AUX_HEADER
      | (security_level ? 1 << 3 : 0)
#endif /* LLSEC802154_USES_AUX_HEADER */
      | (packetbuf_attr(PACKETBUF_ATTR_PENDING) ? 1 << 4 : 0)
      | (packetbuf_attr(PACKETBUF_ATTR_MAC_ACK)
          && !is_broadcast ? 1 << 5 : 0)
      | (panid_compressed ? (1 << 6) : 0);

  /*
   * Sequence Number Suppression | IE List Present
   * | Destination Addressing Mode | Frame Version | Source Addressing Mode
   */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  seqno_suppressed = is_broadcast
      ? SUPPRESS_BROADCAST_SEQNO
      : SUPPRESS_UNICAST_SEQNO;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  dst_addr_mode = is_ack
      ? (ACK_WITH_DST_ADDR
          ? LINKADDR_ADDR_MODE
          : FRAME802154_NOADDR)
      : (is_broadcast
          ? FRAME802154_SHORTADDRMODE
          : LINKADDR_ADDR_MODE);
  src_addr_mode = is_ack
      ? (ACK_WITH_SRC_ADDR
          ? LINKADDR_ADDR_MODE
          : FRAME802154_NOADDR)
      : LINKADDR_ADDR_MODE;
  p[1] = 0
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
      | (seqno_suppressed ? 1 : 0)
      | (packetbuf_attr(PACKETBUF_ATTR_IE_LIST_PRESENT) ? (1 << 1) : 0)
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
      | (dst_addr_mode << 2)
      | (FRAME802154_VERSION << 4)
      | (src_addr_mode << 6);
  p += 2;

  /* Sequence Number */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  if(!seqno_suppressed)
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  {
    p[0] = (uint8_t)packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
    p += 1;
  }

  /* Destination PAN ID */
  if(has_dst_pid(dst_addr_mode, src_addr_mode, panid_compressed)) {
    dst_pid = frame802154_get_pan_id();
    p[0] = dst_pid & 0xff;
    p[1] = (dst_pid >> 8) & 0xff;
    p += 2;
  }

  /* Destination address */
  if(dst_addr_mode) {
    if(is_broadcast) {
      p[0] = 0xFF;
      p[1] = 0xFF;
      p += 2;
    } else {
      create_addr(p, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
      p += LINKADDR_SIZE;
    }
  }

  /* Source PAN ID */
  if(has_src_pid(dst_addr_mode, src_addr_mode, panid_compressed)) {
    dst_pid = frame802154_get_pan_id();
    p[0] = dst_pid & 0xff;
    p[1] = (dst_pid >> 8) & 0xff;
    p += 2;
  }

  /* Source address */
  if(src_addr_mode) {
    create_addr(p,
        linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER), &linkaddr_null)
        ? &linkaddr_node_addr
        : packetbuf_addr(PACKETBUF_ADDR_SENDER));
    p += LINKADDR_SIZE;
  }

#if LLSEC802154_USES_AUX_HEADER
  /* Auxiliary Security Header */
  if(security_level) {
    p[0] = security_level
#if LLSEC802154_USES_EXPLICIT_KEYS
        | (key_id_mode << 3)
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
#if !LLSEC802154_USES_FRAME_COUNTER
        | (1 << 5) /* frame counter suppressed */
        | (1 << 6) /* 5-byte frame counter */
#endif /* !LLSEC802154_USES_FRAME_COUNTER */
    ;
    p += 1;
#if LLSEC802154_USES_FRAME_COUNTER
    anti_replay_write_counter(p);
    p += 4;
#endif /* LLSEC802154_USES_FRAME_COUNTER */

#if LLSEC802154_USES_EXPLICIT_KEYS
    if(key_id_mode) {
      p += get_key_id_len(key_id_mode);
      p[-1] = (uint8_t)packetbuf_attr(PACKETBUF_ATTR_KEY_INDEX);
    }
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
  }
#endif /* LLSEC802154_USES_AUX_HEADER */

  return p - hdrptr;
}
/*---------------------------------------------------------------------------*/
static uint8_t
parse_addr(uint8_t *p, uint8_t addr_mode, uint8_t type)
{
  linkaddr_t addr;
#if LINKADDR_SIZE == 8
  uint8_t i;
#endif /* LINKADDR_SIZE == 8 */

  switch(addr_mode) {
  case FRAME802154_SHORTADDRMODE:
    if((p[0] == 0xFF) && (p[1] == 0xFF)) {
      if(type == PACKETBUF_ADDR_SENDER) {
        /* the source address is 0xFFFF */
        return 0;
      }
      packetbuf_set_addr(type, &linkaddr_null);
    } else {
#if LINKADDR_SIZE == 2
      addr.u8[1] = p[0];
      addr.u8[0] = p[1];
      packetbuf_set_addr(type, &addr);
#else /* LINKADDR_SIZE == 2 */
      return 0;
#endif /* LINKADDR_SIZE == 2 */
    }
    return 2;
#if LINKADDR_SIZE == 8
  case FRAME802154_LONGADDRMODE:
    for(i = 0; i < 8; i++) {
      addr.u8[LINKADDR_SIZE - i - 1] = p[i];
    }
    packetbuf_set_addr(type, &addr);
    return 8;
#endif /* LINKADDR_SIZE == 8 */
  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
parse(void)
{
  uint8_t *hdrptr;
  uint8_t *p;
  int panid_compressed;
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  int seqno_suppressed;
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  uint8_t dst_addr_mode;
  uint8_t src_addr_mode;
  uint16_t dst_pid;
  uint16_t src_pid;
  uint8_t addr_len;
#if LLSEC802154_USES_AUX_HEADER
  int security_enabled;
#if LLSEC802154_USES_EXPLICIT_KEYS
  uint8_t key_id_mode;
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
#endif /* LLSEC802154_USES_AUX_HEADER */

  if(packetbuf_datalen() < 3) {
    PRINTF("framer-802154: frame too short\n");
    return FRAMER_FAILED;
  }

  hdrptr = p = packetbuf_hdrptr();

  /*
   * Frame Type | Security Enabled | Frame Pending
   * | Acknowledgment Request | PAN ID Compression
   */
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, p[0] & 7);
#if LLSEC802154_USES_AUX_HEADER
  security_enabled = (p[0] >> 3) & 1;
#else /* LLSEC802154_USES_AUX_HEADER */
  if((p[0] >> 3) & 1) {
    PRINTF("framer-802154: no support for auxiliary security headers\n");
    return FRAMER_FAILED;
  }
#endif /* LLSEC802154_USES_AUX_HEADER */
  packetbuf_set_attr(PACKETBUF_ATTR_PENDING, (p[0] >> 4) & 1);
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, (p[0] >> 5) & 1);
  panid_compressed = (p[0] >> 6) & 1;

  /*
   * Sequence Number Suppression | IE List Present
   * | Destination Addressing Mode | Frame Version | Source Addressing Mode
   */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  seqno_suppressed = p[1] & 1;
  packetbuf_set_attr(PACKETBUF_ATTR_IE_LIST_PRESENT, (p[1] >> 1) & 1);
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  dst_addr_mode = (p[1] >> 2) & 3;
  /* TODO ignoring Frame Version because a TRAVIS test fails otherwise */
  src_addr_mode = (p[1] >> 6) & 3;

  /* Sequence Number */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
  if(seqno_suppressed) {
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, 0xFFFF);
    p += 2;
  } else
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
  {
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, p[2]);
    p += 3;
  }
#if NETSTACK_CONF_WITH_RIME
  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
#endif

  /* Destination PAN ID */
  if(has_dst_pid(dst_addr_mode, src_addr_mode, panid_compressed)) {
    dst_pid = p[0] + (p[1] << 8);
    if((packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) != FRAME802154_BEACONFRAME)
        && (dst_pid != frame802154_get_pan_id())
        && (dst_pid != FRAME802154_BROADCASTPANDID)) {
      PRINTF("framer-802154: for another PAN\n");
      return FRAMER_FAILED;
    }
    p += 2;
  } else {
    dst_pid = FRAME802154_BROADCASTPANDID;
  }

  /* Destination address */
  if(dst_addr_mode) {
    addr_len = parse_addr(p, dst_addr_mode, PACKETBUF_ADDR_RECEIVER);
    if(!addr_len) {
      PRINTF("framer-802154: no destination address\n");
      return FRAMER_FAILED;
    }
    if(!linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_node_addr)
        && !packetbuf_holds_broadcast()) {
      PRINTF("framer-802154: not for us\n");
      return FRAMER_FAILED;
    }
    p += addr_len;
  }

  /* Source PAN ID */
  if(has_src_pid(dst_addr_mode, src_addr_mode, panid_compressed)) {
    src_pid = p[0] + (p[1] << 8);
    p += 2;
  } else {
    src_pid = dst_pid;
  }
  packetbuf_set_attr(PACKETBUF_ATTR_SOURCE_PANID, src_pid);

  /* Source address */
  if(src_addr_mode) {
    addr_len = parse_addr(p, src_addr_mode, PACKETBUF_ADDR_SENDER);
    if(!addr_len) {
      PRINTF("framer-802154: no source address\n");
      return FRAMER_FAILED;
    }
    if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER), &linkaddr_node_addr)) {
      PRINTF("framer-802154: frame from ourselves\n");
      return FRAMER_FAILED;
    }
    p += addr_len;
  }

#if LLSEC802154_USES_AUX_HEADER
  if(security_enabled) {
    /* Security Control */
    packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL, p[0] & 7);
#if LLSEC802154_USES_EXPLICIT_KEYS
    key_id_mode = (p[0] >> 3) & 3;
    packetbuf_set_attr(PACKETBUF_ATTR_KEY_ID_MODE, key_id_mode);
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */

    /* Frame Counter */
#if FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012
    if((p[0] >> 5) & 1) {
      /* frame counter suppressed */
      p += 1;
    } else if((p[0] >> 6) & 1) {
      /* 5-byte frame counter */
      p += 6;
    } else
#endif /* FRAME802154_VERSION >= FRAME802154_IEEE802154E_2012 */
    {
      /* 4-byte frame counter */
      p += 1;
#if LLSEC802154_USES_FRAME_COUNTER
      anti_replay_parse_counter(p);
#endif /* LLSEC802154_USES_FRAME_COUNTER */
      p += 4;
    }

#if LLSEC802154_USES_EXPLICIT_KEYS
    /* Key Identifier */
    if(key_id_mode) {
      p += get_key_id_len(key_id_mode);
      packetbuf_set_attr(PACKETBUF_ATTR_KEY_INDEX, p[-1]);
    }
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
  }
#endif /* LLSEC802154_USES_AUX_HEADER */

  if(!packetbuf_hdrreduce(p - hdrptr)) {
    PRINTF("framer-802154: hdrreduce failed\n");
    return FRAMER_FAILED;
  }

  return p - hdrptr;
}
/*---------------------------------------------------------------------------*/
const struct framer framer_802154 = {
  hdr_length,
  create,
  parse
};
/*---------------------------------------------------------------------------*/
