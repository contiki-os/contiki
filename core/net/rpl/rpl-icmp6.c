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
 *         ICMP6 I/O for RPL control messages.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Joel Hoglund <joel@sics.se>,
 *               Mathieu Pouillot <m.pouillot@watteco.com>
 *               George Oikonomou <oikonomou@users.sourceforge.net> (multicast)
 *               Antonio Arena <rnantn@gmail.com> (RPL Security features)
 */

/**
 * \addtogroup uip6
 * @{
 */

#include "net/ip/tcpip.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-nd6.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/rpl/rpl-private.h"
#include "net/rpl/rpl-ns.h"
#include "net/packetbuf.h"
#include "net/ipv6/multicast/uip-mcast6.h"

#include "random.h"
/* Security Libraries for CCM with CBC-MAC */
#if RPL_SECURITY
#include "lib/ccm-star.h"
#include "lib/aes-128.h"
#endif

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_NONE

#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED                 0x80
#define RPL_DIO_MOP_SHIFT                3
#define RPL_DIO_MOP_MASK                 0x38
#define RPL_DIO_PREFERENCE_MASK          0x07

#if RPL_SECURITY
#define RPL_NONCE_LENGTH       13
#define RPL_ENCRYPT            1
#define RPL_DECRYPT            0
#define RPL_SEC_LVL_1          0x01
#define RPL_KIM_0        0x00
#define RPL_KEY_INDEX_0      0x00
#define RPL_TIMESTAMP_0      0x00

#define RPL_DIS_RESPONDED      1
#define RPL_DIS_NOT_RESPONDED  0

#define RPL_TIMESTAMP_MASK     0x40
#define RPL_TIMESTAMP_SHIFT    7
#define RPL_KIM_MASK           0xC0
#define RPL_KIM_SHIFT          6
#define RPL_LVL_MASK           0x07

#endif /* RPL_SECURITY */

#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/*---------------------------------------------------------------------------*/

static void dis_input(void);
static void dio_input(void);
static void dao_input(void);
static void dao_ack_input(void);

static void dao_output_target_seq(rpl_parent_t *parent, uip_ipaddr_t *prefix,
                                  uint8_t lifetime, uint8_t seq_no);

/* some debug callbacks useful when debugging RPL networks */
#ifdef RPL_DEBUG_DIO_INPUT
void RPL_DEBUG_DIO_INPUT(uip_ipaddr_t *, rpl_dio_t *);
#endif

#ifdef RPL_DEBUG_DAO_OUTPUT
void RPL_DEBUG_DAO_OUTPUT(rpl_parent_t *);
#endif

#if RPL_SECURITY
static uint8_t rpl_key[16] = RPL_SECURITY_K;
static uint32_t rpl_sec_counter = 0;
static rpl_sec_section_t rpl_last_dis;
#endif /* RPL_SECURITY */

static uint8_t dao_sequence = RPL_LOLLIPOP_INIT;

#if RPL_WITH_MULTICAST
static uip_mcast6_route_t *mcast_group;
#endif
/*---------------------------------------------------------------------------*/
/* Initialize RPL ICMPv6 message handlers */
#if RPL_SECURITY
UIP_ICMP6_HANDLER(dis_handler, ICMP6_RPL, RPL_CODE_SEC_DIS, dis_input);
UIP_ICMP6_HANDLER(dio_handler, ICMP6_RPL, RPL_CODE_SEC_DIO, dio_input);
UIP_ICMP6_HANDLER(dao_handler, ICMP6_RPL, RPL_CODE_SEC_DAO, dao_input);
UIP_ICMP6_HANDLER(dao_ack_handler, ICMP6_RPL, RPL_CODE_SEC_DAO_ACK, dao_ack_input);
#else
UIP_ICMP6_HANDLER(dis_handler, ICMP6_RPL, RPL_CODE_DIS, dis_input);
UIP_ICMP6_HANDLER(dio_handler, ICMP6_RPL, RPL_CODE_DIO, dio_input);
UIP_ICMP6_HANDLER(dao_handler, ICMP6_RPL, RPL_CODE_DAO, dao_input);
UIP_ICMP6_HANDLER(dao_ack_handler, ICMP6_RPL, RPL_CODE_DAO_ACK, dao_ack_input);
#endif
/*---------------------------------------------------------------------------*/

#if RPL_WITH_DAO_ACK
static uip_ds6_route_t *
find_route_entry_by_dao_ack(uint8_t seq)
{
  uip_ds6_route_t *re;
  re = uip_ds6_route_head();
  while(re != NULL) {
    if(re->state.dao_seqno_out == seq && RPL_ROUTE_IS_DAO_PENDING(re)) {
      /* found it! */
      return re;
    }
    re = uip_ds6_route_next(re);
  }
  return NULL;
}
#endif /* RPL_WITH_DAO_ACK */

#if RPL_WITH_STORING
/* prepare for forwarding of DAO */
static uint8_t
prepare_for_dao_fwd(uint8_t sequence, uip_ds6_route_t *rep)
{
  /* not pending - or pending but not a retransmission */
  RPL_LOLLIPOP_INCREMENT(dao_sequence);

  /* set DAO pending and sequence numbers */
  rep->state.dao_seqno_in = sequence;
  rep->state.dao_seqno_out = dao_sequence;
  RPL_ROUTE_SET_DAO_PENDING(rep);
  return dao_sequence;
}
#endif /* RPL_WITH_STORING */
/*---------------------------------------------------------------------------*/
static int
get_global_addr(uip_ipaddr_t *addr)
{
  int i;
  int state;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      if(!uip_is_addr_linklocal(&uip_ds6_if.addr_list[i].ipaddr)) {
        memcpy(addr, &uip_ds6_if.addr_list[i].ipaddr, sizeof(uip_ipaddr_t));
        return 1;
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static uint32_t
get32(uint8_t *buffer, int pos)
{
  return (uint32_t)buffer[pos] << 24 | (uint32_t)buffer[pos + 1] << 16 |
         (uint32_t)buffer[pos + 2] << 8 | buffer[pos + 3];
}
/*---------------------------------------------------------------------------*/
static void
set32(uint8_t *buffer, int pos, uint32_t value)
{
  buffer[pos++] = value >> 24;
  buffer[pos++] = (value >> 16) & 0xff;
  buffer[pos++] = (value >> 8) & 0xff;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
static uint16_t
get16(uint8_t *buffer, int pos)
{
  return (uint16_t)buffer[pos] << 8 | buffer[pos + 1];
}
/*---------------------------------------------------------------------------*/
static void
set16(uint8_t *buffer, int pos, uint16_t value)
{
  buffer[pos++] = value >> 8;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
#if RPL_SECURITY
/* Add Security Section to ICMP_PAYLOAD with given security parameter */
static uint8_t
add_security_section(uint8_t timestamp, uint8_t kim, uint8_t sec_lvl, uint8_t key_index)
{
  unsigned char *buffer;
  int len = 0;

  buffer = UIP_ICMP_PAYLOAD;

  /*
   * Security Section for RPL messages:
   *     0                   1                   2                   3
   *	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *	|T|  Reserved   |   Algorithm   |KIM|Resvd| LVL |     Flags     |
   *	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *	|                            Counter                            |
   *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *	|                                                               |
   *	.                        Key Identifier                         .
   *	.                                                               .
   *	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  buffer[len] = 0;
  buffer[len++] |= timestamp << RPL_TIMESTAMP_SHIFT;

  buffer[len++] = 0;                /* Algorithm = 0    */

  buffer[len] = 0;
  buffer[len] |= kim << RPL_KIM_SHIFT;
  buffer[len++] |= sec_lvl;

  buffer[len++] = 0;                /* Flags */

  set32(buffer, len, rpl_sec_counter);
  len += 4;

  if(kim == 0) {
    buffer[len++] = key_index;    /* KIM = 0 => Only Key Identifier */
  }
  /* TODO : kim!= 0 */

  return len;
}
/*
 * Set mutable fields in IP and ICMP headers to zero/default values
 * Set other fields to value based on function parameters
 * In RPL secure mode IP and ICMP headers are covered in MAC computing.
 */
static void
set_ip_icmp_fields(uip_ipaddr_t *addr, uint8_t icmp_type)
{
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = 0;
  UIP_IP_BUF->len[0] = 0;
  UIP_IP_BUF->len[1] = 0;

  if(addr != NULL) {
    memcpy(&UIP_IP_BUF->destipaddr, addr, sizeof(*addr));
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  }

  UIP_ICMP_BUF->type = ICMP6_RPL;
  UIP_ICMP_BUF->icode = icmp_type;
  UIP_ICMP_BUF->icmpchksum = 0;
}
/* Create CCM_NONCE as described in RFC 6550 */
static void
set_ccm_nonce(uint8_t *ccm_nonce, uint32_t counter, uint8_t sec_lvl)
{
  /* RPL CCM Nonce Construction
   *     0                   1                   2                   3
   *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *     |                                                               |
   *     +                     Source Identifier                         +
   *     |                                                               |
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *     |                          Counter                              |
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *     |KIM|Resvd| LVL |
   *     +-+-+-+-+-+-+-+-+
   *
   */
  int i;
  for(i = 0; i < 8; i++) {
    ccm_nonce[i] = (UIP_IP_BUF->srcipaddr).u8[(8 + i)];
  }

  set32(ccm_nonce, 8, counter);
  ccm_nonce[12] = sec_lvl;
}
/* Get MIC length, based on security level */
static uint8_t
get_mic_len(uint8_t sec_lvl)
{
  uint8_t mic_len;
  if(sec_lvl == 0 || sec_lvl == 1) {
    mic_len = 4;
  } else {
    mic_len = 8;
  }

  return mic_len;
}
/* ENC/DEC function
 * In case of decryption the function return:
 * 0 if a MAC mismatch occurs
 * 1 otherwise
 */
static int
sec_aead(uint8_t *ccm_nonce, int buffer_len, int sec_len,
         uint8_t sec_lvl, int forward)
{
  unsigned char *buffer;
  uint8_t mic[8];
  int mic_len;
  buffer = UIP_ICMP_PAYLOAD;

  mic_len = get_mic_len(sec_lvl);

  CCM_STAR.set_key(rpl_key);

  if(forward == RPL_ENCRYPT) {
    if((sec_lvl == 1) || (sec_lvl == 3)) {
      CCM_STAR.aead(ccm_nonce,
                    buffer + sec_len, buffer_len,
                    uip_buf + UIP_LLH_LEN, UIP_IPICMPH_LEN + sec_len,
                    buffer + sec_len + buffer_len, mic_len, forward);
    } else {
      CCM_STAR.aead(ccm_nonce,
                    NULL, 0,
                    uip_buf + UIP_LLH_LEN, UIP_IPICMPH_LEN + sec_len + buffer_len,
				    mic, mic_len, forward);
    }
    return mic_len;
  } else {
    if(forward == RPL_DECRYPT) {
      if(sec_lvl == 1 || sec_lvl == 3) {
        CCM_STAR.aead(ccm_nonce,
                      buffer + sec_len, buffer_len,
                      uip_buf + UIP_LLH_LEN, UIP_IPICMPH_LEN + sec_len,
                      mic, mic_len, forward);
      } else {
        CCM_STAR.aead(ccm_nonce,
                      NULL, 0,
                      uip_buf + UIP_LLH_LEN, UIP_IPICMPH_LEN + sec_len + buffer_len,
    				  mic, mic_len, forward);
      }
      if(memcmp(mic, buffer + sec_len + buffer_len, mic_len) != 0) {
        return 0;
      } else {
        return 1;
      }
    } else {
      return 0;
    }
  }
}
#endif /* RPL_SECURITY */
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
rpl_icmp6_update_nbr_table(uip_ipaddr_t *from, nbr_table_reason_t reason, void *data)
{
  uip_ds6_nbr_t *nbr;

  if((nbr = uip_ds6_nbr_lookup(from)) == NULL) {
    if((nbr = uip_ds6_nbr_add(from, (uip_lladdr_t *)
                              packetbuf_addr(PACKETBUF_ADDR_SENDER),
                              0, NBR_REACHABLE, reason, data)) != NULL) {
      PRINTF("RPL: Neighbor added to neighbor cache ");
      PRINT6ADDR(from);
      PRINTF(", ");
      PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      PRINTF("\n");
    }
  }

  if(nbr != NULL) {
#if UIP_ND6_SEND_NA
    /* set reachable timer if we added or found the nbr entry - and update
       neighbor entry to reachable to avoid sending NS/NA, etc.  */
    stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
    nbr->state = NBR_REACHABLE;
#endif /* UIP_ND6_SEND_NA */
  }
  return nbr;
}
/*---------------------------------------------------------------------------*/
static void
dis_input(void)
{
  rpl_instance_t *instance;
  rpl_instance_t *end;

  /* DAG Information Solicitation */
  PRINTF("RPL: Received a DIS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

#if RPL_SECURITY
  uint8_t buffer_length;
  uint8_t *buffer;
  int sec_len;
  int pos;

  uint8_t timestamp;
  uint8_t kim;
  uint8_t lvl;
  uint32_t counter;

  uint8_t key_index;

  uint8_t ccm_nonce[RPL_NONCE_LENGTH];

  uint8_t mic_len;      /* n-byte MAC length */

  buffer = UIP_ICMP_PAYLOAD;

  pos = 0;

  timestamp = (buffer[pos++] & RPL_TIMESTAMP_MASK) >> RPL_TIMESTAMP_SHIFT;
  kim = (buffer[pos++] & RPL_KIM_MASK) >> RPL_KIM_SHIFT;
  lvl = (buffer[pos++] & RPL_LVL_MASK);
  ++pos;  /* Flags */

/*  Discard packets with time stamp instead of simple counters,
 *  Only accept packets with pre installed key
 *  Discard unassigned security levels
 *  Flags must be zero
 */

  if((timestamp == 1) || (kim != 0) || (lvl > 3)) {
    goto discard;
  }

  counter = get32(buffer, pos);
  pos += 4;

  /* We are in KIM = 0, only key_index is present */
  key_index = buffer[pos++];
  /* Key_index must be zero because we use preinstalled key */
  if(key_index != 0) {
    goto discard;
  }

  sec_len = pos;
  mic_len = get_mic_len(lvl);
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

/* IP Header and ICMP header mutable fields set to 0 */
  set_ip_icmp_fields(NULL, RPL_CODE_SEC_DIS);

  set_ccm_nonce(ccm_nonce, counter, lvl);

  if(sec_aead(ccm_nonce, buffer_length, sec_len, lvl, RPL_DECRYPT) == 0) {
    PRINTF("RPL: MIC mismatch\n");
    goto discard;
  }

  /* CARE: node not trusted can send multiple DIS to lower Enc levels */
  rpl_last_dis.responded = RPL_DIS_NOT_RESPONDED;
  rpl_last_dis.timestamp = timestamp;
  rpl_last_dis.kim = kim;
  rpl_last_dis.lvl = lvl;
  rpl_last_dis.key_source[0] = 0;
  rpl_last_dis.key_source[1] = 0;
  rpl_last_dis.key_index = key_index;
#endif

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES;
      instance < end; ++instance) {
    if(instance->used == 1) {
#if RPL_LEAF_ONLY
      if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
        PRINTF("RPL: LEAF ONLY Multicast DIS will NOT reset DIO timer\n");
#else /* !RPL_LEAF_ONLY */
      if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
        PRINTF("RPL: Multicast DIS => reset DIO timer\n");
        rpl_reset_dio_timer(instance);
      } else {
#endif /* !RPL_LEAF_ONLY */
        /* Check if this neighbor should be added according to the policy. */
        if(rpl_icmp6_update_nbr_table(&UIP_IP_BUF->srcipaddr,
                                      NBR_TABLE_REASON_RPL_DIS, NULL) == NULL) {
          PRINTF("RPL: Out of Memory, not sending unicast DIO, DIS from ");
          PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
          PRINTF(", ");
          PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
          PRINTF("\n");
        } else {
          PRINTF("RPL: Unicast DIS, reply to sender\n");
          dio_output(instance, &UIP_IP_BUF->srcipaddr);
        }
        /* } */
      }
    }
  }
#if RPL_SECURITY
discard:
#endif
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
void
dis_output(uip_ipaddr_t *addr)
{
  unsigned char *buffer;
  uip_ipaddr_t tmpaddr;
  int pos;
#if RPL_SECURITY
  int sec_len;
  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  uint8_t mic_len;      /* n-byte MAC length */
#endif

  pos = 0;
  buffer = UIP_ICMP_PAYLOAD;

#if RPL_SECURITY
  sec_len = add_security_section(RPL_TIMESTAMP_0, RPL_KIM_0,
                                 RPL_SEC_LVL_1, RPL_KEY_INDEX_0);
  pos += sec_len;
#endif

  /*
   * DAG Information Solicitation  - 2 bytes reserved
   *      0                   1                   2
   *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *     |     Flags     |   Reserved    |   Option(s)...
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  buffer[pos++] = 0;
  buffer[pos++] = 0;

  if(addr == NULL) {
    uip_create_linklocal_rplnodes_mcast(&tmpaddr);
    addr = &tmpaddr;
  }

  PRINTF("RPL: Sending a DIS to ");
  PRINT6ADDR(addr);
  PRINTF("\n");

#if RPL_SECURITY

  set_ip_icmp_fields(addr, RPL_CODE_SEC_DIS);

  set_ccm_nonce(ccm_nonce, rpl_sec_counter, RPL_SEC_LVL_1);

  rpl_sec_counter++;

  mic_len = sec_aead(ccm_nonce, pos - sec_len, sec_len, RPL_SEC_LVL_1, RPL_ENCRYPT);

  pos += mic_len;

  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_SEC_DIS, pos);
#else
  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_DIS, pos);
#endif
}
/*---------------------------------------------------------------------------*/
static void
dio_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  rpl_dio_t dio;
  uint8_t subopt_type;
  int pos;
  int len;
  int sec_len;
  uip_ipaddr_t from;

  memset(&dio, 0, sizeof(dio));

  /* Set default values in case the DIO configuration option is missing. */
  dio.dag_intdoubl = RPL_DIO_INTERVAL_DOUBLINGS;
  dio.dag_intmin = RPL_DIO_INTERVAL_MIN;
  dio.dag_redund = RPL_DIO_REDUNDANCY;
  dio.dag_min_hoprankinc = RPL_MIN_HOPRANKINC;
  dio.dag_max_rankinc = RPL_MAX_RANKINC;
  dio.ocp = RPL_OF_OCP;
  dio.default_lifetime = RPL_DEFAULT_LIFETIME;
  dio.lifetime_unit = RPL_DEFAULT_LIFETIME_UNIT;

  uip_ipaddr_copy(&from, &UIP_IP_BUF->srcipaddr);

  /* DAG Information Object */
  PRINTF("RPL: Received a DIO from ");
  PRINT6ADDR(&from);
  PRINTF("\n");

  buffer_length = uip_len - uip_l3_icmp_hdr_len;

  /* Process the DIO base option. */
  pos = 0;
  sec_len = 0;
  buffer = UIP_ICMP_PAYLOAD;

#if RPL_SECURITY
  uint8_t timestamp;
  uint8_t kim;
  uint8_t lvl;
  uint32_t counter;

  uint8_t key_index;

  uint8_t ccm_nonce[RPL_NONCE_LENGTH];

  uint8_t mic_len;      /* n-byte MAC length */

  timestamp = (buffer[pos++] & RPL_TIMESTAMP_MASK) >> RPL_TIMESTAMP_SHIFT;
  kim = (buffer[pos++] & RPL_KIM_MASK) >> RPL_KIM_SHIFT;
  lvl = (buffer[pos++] & RPL_LVL_MASK);
  ++pos;    /* Flags */

/*  Discard packets with time stamp instead of simple counters,
 *  Only accept packets with pre installed key
 *  Discard unassigned security levels
 */

  if((timestamp == 1) || (kim != 0) || (lvl > 3)) {
    goto discard;
  }

  counter = get32(buffer, pos);
  pos += 4;

  /* We are in KIM = 0, only key_index is present */
  key_index = buffer[pos++];
  /* Key_index must be zero because we use preinstalled key */
  if(key_index != 0) {
    goto discard;
  }

  sec_len = pos;
  mic_len = get_mic_len(lvl);
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

  /* IP Header and ICMP header mutable fields set to 0 */
  set_ip_icmp_fields(NULL, RPL_CODE_SEC_DIO);

  set_ccm_nonce(ccm_nonce, counter, lvl);

  if(sec_aead(ccm_nonce, buffer_length, sec_len, lvl, RPL_DECRYPT) == 0) {
    PRINTF("RPL: MIC mismatch\n");
    goto discard;
  }
#endif

  dio.instance_id = buffer[pos++];
  dio.version = buffer[pos++];
  dio.rank = get16(buffer, pos);
  pos += 2;

  PRINTF("RPL: Incoming DIO (id, ver, rank) = (%u,%u,%u)\n",
         (unsigned)dio.instance_id,
         (unsigned)dio.version,
         (unsigned)dio.rank);

  dio.grounded = buffer[pos] & RPL_DIO_GROUNDED;
  dio.mop = (buffer[pos] & RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT;
  dio.preference = buffer[pos++] & RPL_DIO_PREFERENCE_MASK;

  dio.dtsn = buffer[pos++];
  /* two reserved bytes */
  pos += 2;

  memcpy(&dio.dag_id, buffer + pos, sizeof(dio.dag_id));
  pos += sizeof(dio.dag_id);

  PRINTF("RPL: Incoming DIO (dag_id, pref) = (");
  PRINT6ADDR(&dio.dag_id);
  PRINTF(", %u)\n", dio.preference);

  /* Check if there are any DIO suboptions. */
  for(; pos < buffer_length + sec_len; pos += len) {
    subopt_type = buffer[pos];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* Suboption with a two-byte header + payload */
      len = 2 + buffer[pos + 1];
    }

    if(len + pos > buffer_length + sec_len) {
      PRINTF("RPL: Invalid DIO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      goto discard;
    }

    PRINTF("RPL: DIO option %u, length: %u\n", subopt_type, len - 2);

    switch(subopt_type) {
    case RPL_OPTION_DAG_METRIC_CONTAINER:
      if(len < 6) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }
      dio.mc.type = buffer[pos + 2];
      dio.mc.flags = buffer[pos + 3] << 1;
      dio.mc.flags |= buffer[pos + 4] >> 7;
      dio.mc.aggr = (buffer[pos + 4] >> 4) & 0x3;
      dio.mc.prec = buffer[pos + 4] & 0xf;
      dio.mc.length = buffer[pos + 5];

      if(dio.mc.type == RPL_DAG_MC_NONE) {
        /* No metric container: do nothing */
      } else if(dio.mc.type == RPL_DAG_MC_ETX) {
        dio.mc.obj.etx = get16(buffer, pos + 6);

        PRINTF("RPL: DAG MC: type %u, flags %u, aggr %u, prec %u, length %u, ETX %u\n",
               (unsigned)dio.mc.type,
               (unsigned)dio.mc.flags,
               (unsigned)dio.mc.aggr,
               (unsigned)dio.mc.prec,
               (unsigned)dio.mc.length,
               (unsigned)dio.mc.obj.etx);
      } else if(dio.mc.type == RPL_DAG_MC_ENERGY) {
        dio.mc.obj.energy.flags = buffer[pos + 6];
        dio.mc.obj.energy.energy_est = buffer[pos + 7];
      } else {
        PRINTF("RPL: Unhandled DAG MC type: %u\n", (unsigned)dio.mc.type);
        goto discard;
      }
      break;
    case RPL_OPTION_ROUTE_INFO:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      /* The flags field includes the preference value. */
      dio.destination_prefix.length = buffer[pos + 2];
      dio.destination_prefix.flags = buffer[pos + 3];
      dio.destination_prefix.lifetime = get32(buffer, pos + 4);

      if(((dio.destination_prefix.length + 7) / 8) + 8 <= len &&
         dio.destination_prefix.length <= 128) {
        PRINTF("RPL: Copying destination prefix\n");
        memcpy(&dio.destination_prefix.prefix, &buffer[pos + 8],
               (dio.destination_prefix.length + 7) / 8);
      } else {
        PRINTF("RPL: Invalid route info option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      break;
    case RPL_OPTION_DAG_CONF:
      if(len != 16) {
        PRINTF("RPL: Invalid DAG configuration option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      /* Path control field not yet implemented - at pos + 2 */
      dio.dag_intdoubl = buffer[pos + 3];
      dio.dag_intmin = buffer[pos + 4];
      dio.dag_redund = buffer[pos + 5];
      dio.dag_max_rankinc = get16(buffer, pos + 6);
      dio.dag_min_hoprankinc = get16(buffer, pos + 8);
      dio.ocp = get16(buffer, pos + 10);
      /* buffer + 12 is reserved */
      dio.default_lifetime = buffer[pos + 13];
      dio.lifetime_unit = get16(buffer, pos + 14);
      PRINTF("RPL: DAG conf:dbl=%d, min=%d red=%d maxinc=%d mininc=%d ocp=%d d_l=%u l_u=%u\n",
             dio.dag_intdoubl, dio.dag_intmin, dio.dag_redund,
             dio.dag_max_rankinc, dio.dag_min_hoprankinc, dio.ocp,
             dio.default_lifetime, dio.lifetime_unit);
      break;
    case RPL_OPTION_PREFIX_INFO:
      if(len != 32) {
        PRINTF("RPL: Invalid DAG prefix info, len != 32\n");
        RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }
      dio.prefix_info.length = buffer[pos + 2];
      dio.prefix_info.flags = buffer[pos + 3];
      /* valid lifetime is ingnored for now - at pos + 4 */
      /* preferred lifetime stored in lifetime */
      dio.prefix_info.lifetime = get32(buffer, pos + 8);
      /* 32-bit reserved at pos + 12 */
      PRINTF("RPL: Copying prefix information\n");
      memcpy(&dio.prefix_info.prefix, &buffer[pos + 16], 16);
      break;
    default:
      PRINTF("RPL: Unsupported suboption type in DIO: %u\n",
             (unsigned)subopt_type);
    }
  }

#ifdef RPL_DEBUG_DIO_INPUT
  RPL_DEBUG_DIO_INPUT(&from, &dio);
#endif

  rpl_process_dio(&from, &dio);

discard:
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
void
dio_output(rpl_instance_t *instance, uip_ipaddr_t *uc_addr)
{
  unsigned char *buffer;
  int pos;
  int is_root;
  rpl_dag_t *dag = instance->current_dag;
  uip_ipaddr_t addr;

#if RPL_SECURITY
  int sec_len;
  uint8_t sec_lvl;
  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  uint8_t mic_len;      /* n-byte MAC length */
#endif

#if RPL_LEAF_ONLY
  /* In leaf mode, we only send DIO messages as unicasts in response to
     unicast DIS messages. */
  if(uc_addr == NULL) {
    PRINTF("RPL: LEAF ONLY have multicast addr: skip dio_output\n");
    return;
  }
#endif /* RPL_LEAF_ONLY */

  /* DAG Information Object */
  pos = 0;

  buffer = UIP_ICMP_PAYLOAD;

#if RPL_SECURITY
  if(rpl_last_dis.responded == RPL_DIS_RESPONDED) {
    PRINTF("RPL: Sending Regular Secure DIO\n");

    sec_len = add_security_section(RPL_TIMESTAMP_0, RPL_KIM_0,
                                   RPL_SEC_LVL, RPL_KEY_INDEX_0);
    sec_lvl = RPL_SEC_LVL;
  } else {
    PRINTF("RPL: Sending Secure DIO replying a DIS, lvl:%u\n", rpl_last_dis.lvl);
    rpl_last_dis.responded = RPL_DIS_RESPONDED;

    sec_len = add_security_section(rpl_last_dis.timestamp, RPL_KIM_0,
                                   rpl_last_dis.lvl, rpl_last_dis.key_index);
    sec_lvl = rpl_last_dis.lvl;
  }

  pos += sec_len;
#endif

  buffer[pos++] = instance->instance_id;
  buffer[pos++] = dag->version;
  is_root = (dag->rank == ROOT_RANK(instance));

#if RPL_LEAF_ONLY
  PRINTF("RPL: LEAF ONLY DIO rank set to INFINITE_RANK\n");
  set16(buffer, pos, INFINITE_RANK);
#else /* RPL_LEAF_ONLY */
  set16(buffer, pos, dag->rank);
#endif /* RPL_LEAF_ONLY */
  pos += 2;

  buffer[pos] = 0;
  if(dag->grounded) {
    buffer[pos] |= RPL_DIO_GROUNDED;
  }

  buffer[pos] |= instance->mop << RPL_DIO_MOP_SHIFT;
  buffer[pos] |= dag->preference & RPL_DIO_PREFERENCE_MASK;
  pos++;

  buffer[pos++] = instance->dtsn_out;

  if(RPL_DIO_REFRESH_DAO_ROUTES && is_root && uc_addr == NULL) {
    /* Request new DAO to refresh route. We do not do this for unicast DIO
     * in order to avoid DAO messages after a DIS-DIO update,
     * or upon unicast DIO probing. */
    RPL_LOLLIPOP_INCREMENT(instance->dtsn_out);
  }

  /* reserved 2 bytes */
  buffer[pos++] = 0; /* flags */
  buffer[pos++] = 0; /* reserved */

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

#if !RPL_LEAF_ONLY
  if(instance->mc.type != RPL_DAG_MC_NONE) {
    instance->of->update_metric_container(instance);

    buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
    buffer[pos++] = 6;
    buffer[pos++] = instance->mc.type;
    buffer[pos++] = instance->mc.flags >> 1;
    buffer[pos] = (instance->mc.flags & 1) << 7;
    buffer[pos++] |= (instance->mc.aggr << 4) | instance->mc.prec;
    if(instance->mc.type == RPL_DAG_MC_ETX) {
      buffer[pos++] = 2;
      set16(buffer, pos, instance->mc.obj.etx);
      pos += 2;
    } else if(instance->mc.type == RPL_DAG_MC_ENERGY) {
      buffer[pos++] = 2;
      buffer[pos++] = instance->mc.obj.energy.flags;
      buffer[pos++] = instance->mc.obj.energy.energy_est;
    } else {
      PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
             (unsigned)instance->mc.type);
      return;
    }
  }
#endif /* !RPL_LEAF_ONLY */

  /* Always add a DAG configuration option. */
  buffer[pos++] = RPL_OPTION_DAG_CONF;
  buffer[pos++] = 14;
  buffer[pos++] = 0; /* No Auth, PCS = 0 */
  buffer[pos++] = instance->dio_intdoubl;
  buffer[pos++] = instance->dio_intmin;
  buffer[pos++] = instance->dio_redundancy;
  set16(buffer, pos, instance->max_rankinc);
  pos += 2;
  set16(buffer, pos, instance->min_hoprankinc);
  pos += 2;
  /* OCP is in the DAG_CONF option */
  set16(buffer, pos, instance->of->ocp);
  pos += 2;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = instance->default_lifetime;
  set16(buffer, pos, instance->lifetime_unit);
  pos += 2;

  /* Check if we have a prefix to send also. */
  if(dag->prefix_info.length > 0) {
    buffer[pos++] = RPL_OPTION_PREFIX_INFO;
    buffer[pos++] = 30; /* always 30 bytes + 2 long */
    buffer[pos++] = dag->prefix_info.length;
    buffer[pos++] = dag->prefix_info.flags;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    memset(&buffer[pos], 0, 4);
    pos += 4;
    memcpy(&buffer[pos], &dag->prefix_info.prefix, 16);
    pos += 16;
    PRINTF("RPL: Sending prefix info in DIO for ");
    PRINT6ADDR(&dag->prefix_info.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce (len %d)\n",
           dag->prefix_info.length);
  }

#if RPL_LEAF_ONLY
#if (DEBUG)&DEBUG_PRINT
  if(uc_addr == NULL) {
    PRINTF("RPL: LEAF ONLY sending unicast-DIO from multicast-DIO\n");
  }
#endif /* DEBUG_PRINT */
  PRINTF("RPL: Sending unicast-DIO with rank %u to ",
         (unsigned)dag->rank);
  PRINT6ADDR(uc_addr);
  PRINTF("\n");
  uip_ipaddr_copy(&addr, uc_addr);
#else /* RPL_LEAF_ONLY */
  /* Unicast requests get unicast replies! */
  if(uc_addr == NULL) {
    PRINTF("RPL: Sending a multicast-DIO with rank %u\n",
           (unsigned)instance->current_dag->rank);
    uip_create_linklocal_rplnodes_mcast(&addr);
  } else {
    PRINTF("RPL: Sending unicast-DIO with rank %u to ",
           (unsigned)instance->current_dag->rank);
    PRINT6ADDR(uc_addr);
    PRINTF("\n");
    uip_ipaddr_copy(&addr, uc_addr);
  }
#endif /* RPL_LEAF_ONLY */

#if RPL_SECURITY
  set_ip_icmp_fields(&addr, RPL_CODE_SEC_DIO);

  set_ccm_nonce(ccm_nonce, rpl_sec_counter, sec_lvl);

  rpl_sec_counter++;

  mic_len = sec_aead(ccm_nonce, pos - sec_len, sec_len, sec_lvl, RPL_ENCRYPT);

  pos += mic_len;

  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_SEC_DIO, pos);
#else
  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DIO, pos);
#endif
}
/*---------------------------------------------------------------------------*/
static void
dao_input_storing(int sec_len, uint8_t mic_len, void *sec_section)
{
#if RPL_WITH_STORING
  uip_ipaddr_t dao_sender_addr;
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint8_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  /*
     uint8_t pathcontrol;
     uint8_t pathsequence;
   */
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  uint8_t buffer_length;
  int pos;
  int len;
  int learned_from;
  rpl_parent_t *parent;
  uip_ds6_nbr_t *nbr;
  int is_root;

#if RPL_SECURITY
  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  rpl_sec_section_t *p = (rpl_sec_section_t *)sec_section;
#endif

  prefixlen = 0;
  parent = NULL;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

  pos = sec_len;

  instance_id = buffer[pos++];

  instance = rpl_get_instance(instance_id);

  lifetime = instance->default_lifetime;

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  dag = instance->current_dag;
  is_root = (dag->rank == ROOT_RANK(instance));

  /* Is the DAG ID present? */
  if(flags & RPL_DAO_D_FLAG) {
    if(memcmp(&dag->dag_id, &buffer[pos], sizeof(dag->dag_id))) {
      PRINTF("RPL: Ignoring a DAO for a DAG different from ours\n");
      return;
    }
    pos += 16;
  }

  learned_from = uip_is_addr_mcast(&dao_sender_addr) ?
    RPL_ROUTE_FROM_MULTICAST_DAO : RPL_ROUTE_FROM_UNICAST_DAO;

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a (%s) DAO with sequence number %u from ",
         learned_from == RPL_ROUTE_FROM_UNICAST_DAO ? "unicast" : "multicast", sequence);
  PRINT6ADDR(&dao_sender_addr);
  PRINTF("\n");

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    /* Check whether this is a DAO forwarding loop. */
    parent = rpl_find_parent(dag, &dao_sender_addr);
    /* check if this is a new DAO registration with an "illegal" rank */
    /* if we already route to this node it is likely */
    if(parent != NULL &&
       DAG_RANK(parent->rank, instance) < DAG_RANK(dag->rank, instance)) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from a node with a lower rank! (%u < %u)\n",
             DAG_RANK(parent->rank, instance), DAG_RANK(dag->rank, instance));
      parent->rank = INFINITE_RANK;
      parent->flags |= RPL_PARENT_FLAG_UPDATED;
      return;
    }

    /* If we get the DAO from our parent, we also have a loop. */
    if(parent != NULL && parent == dag->preferred_parent) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from our parent\n");
      parent->rank = INFINITE_RANK;
      parent->flags |= RPL_PARENT_FLAG_UPDATED;
      return;
    }
  }

  /* Check if there are any RPL options present. */
  for(; pos < buffer_length + sec_len; pos += len) {
    subopt_type = buffer[pos];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* The option consists of a two-byte header and a payload. */
      len = 2 + buffer[pos + 1];
    }

    if(len + pos - sec_len > buffer_length) {
      PRINTF("RPL: Invalid DAO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      return;
    }

    switch(subopt_type) {
    case RPL_OPTION_TARGET:
      /* Handle the target option. */
      prefixlen = buffer[pos + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + pos + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_OPTION_TRANSIT:
      /* The path sequence and control are ignored. */
      /*      pathcontrol = buffer[pos + 3];
              pathsequence = buffer[pos + 4];*/
      lifetime = buffer[pos + 5];
      /* The parent address is also ignored. */
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
         (unsigned)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF("\n");

#if RPL_WITH_MULTICAST
  if(uip_is_addr_mcast_global(&prefix)) {
    mcast_group = uip_mcast6_route_add(&prefix);
    if(mcast_group) {
      mcast_group->dag = dag;
      mcast_group->lifetime = RPL_LIFETIME(instance, lifetime);
    }
    goto fwd_dao;
  }
#endif

  rep = uip_ds6_route_lookup(&prefix);

  if(lifetime == RPL_ZERO_LIFETIME) {
    PRINTF("RPL: No-Path DAO received\n");
    /* No-Path DAO received; invoke the route purging routine. */
    if(rep != NULL &&
       !RPL_ROUTE_IS_NOPATH_RECEIVED(rep) &&
       rep->length == prefixlen &&
       uip_ds6_route_nexthop(rep) != NULL &&
       uip_ipaddr_cmp(uip_ds6_route_nexthop(rep), &dao_sender_addr)) {
      PRINTF("RPL: Setting expiration timer for prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      RPL_ROUTE_SET_NOPATH_RECEIVED(rep);
      rep->state.lifetime = RPL_NOPATH_REMOVAL_DELAY;

      /* We forward the incoming No-Path DAO to our parent, if we have
         one. */
      if(dag->preferred_parent != NULL &&
         rpl_get_parent_ipaddr(dag->preferred_parent) != NULL) {
        uint8_t out_seq;
        out_seq = prepare_for_dao_fwd(sequence, rep);

        PRINTF("RPL: Forwarding No-path DAO to parent ");
        PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
        PRINTF("\n");

        buffer = UIP_ICMP_PAYLOAD;
        buffer[sec_len + 3] = out_seq; /* add an outgoing seq no before fwd */

#if RPL_SECURITY
        add_security_section(p->timestamp, p->kim, p->lvl, p->key_index);
        set_ip_icmp_fields(rpl_get_parent_ipaddr(dag->preferred_parent),
                           RPL_CODE_SEC_DAO);
        set_ccm_nonce(ccm_nonce, rpl_sec_counter, p->lvl);

        rpl_sec_counter++;

        sec_aead(ccm_nonce, buffer_length, sec_len, p->lvl, RPL_ENCRYPT);
        uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                       ICMP6_RPL, RPL_CODE_SEC_DAO, buffer_length + sec_len + mic_len);
#else
        uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                       ICMP6_RPL, RPL_CODE_DAO, buffer_length);
#endif
      }
    }
    /* independent if we remove or not - ACK the request */
    if(flags & RPL_DAO_K_FLAG) {
      /* indicate that we accepted the no-path DAO */
      uip_clear_buf();
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
    }
    return;
  }

  PRINTF("RPL: Adding DAO route\n");

  /* Update and add neighbor - if no room - fail. */
  if((nbr = rpl_icmp6_update_nbr_table(&dao_sender_addr, NBR_TABLE_REASON_RPL_DAO, instance)) == NULL) {
    PRINTF("RPL: Out of Memory, dropping DAO from ");
    PRINT6ADDR(&dao_sender_addr);
    PRINTF(", ");
    PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
    PRINTF("\n");
    if(flags & RPL_DAO_K_FLAG) {
      /* signal the failure to add the node */
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     is_root ? RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT :
                     RPL_DAO_ACK_UNABLE_TO_ACCEPT);
    }
    return;
  }

  rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
  if(rep == NULL) {
    RPL_STAT(rpl_stats.mem_overflows++);
    PRINTF("RPL: Could not add a route after receiving a DAO\n");
    if(flags & RPL_DAO_K_FLAG) {
      /* signal the failure to add the node */
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     is_root ? RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT :
                     RPL_DAO_ACK_UNABLE_TO_ACCEPT);
    }
    return;
  }

  /* set lifetime and clear NOPATH bit */
  rep->state.lifetime = RPL_LIFETIME(instance, lifetime);
  RPL_ROUTE_CLEAR_NOPATH_RECEIVED(rep);

#if RPL_WITH_MULTICAST
fwd_dao:
#endif

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    int should_ack = 0;

    if(flags & RPL_DAO_K_FLAG) {
      /*
       * check if this route is already installed and we can ack now!
       * not pending - and same seq-no means that we can ack.
       * (e.g. the route is installed already so it will not take any
       * more room that it already takes - so should be ok!)
       */
      if((!RPL_ROUTE_IS_DAO_PENDING(rep) &&
          rep->state.dao_seqno_in == sequence) ||
         dag->rank == ROOT_RANK(instance)) {
        should_ack = 1;
      }
    }

    if(dag->preferred_parent != NULL &&
       rpl_get_parent_ipaddr(dag->preferred_parent) != NULL) {
      uint8_t out_seq = 0;
      /* if this is pending and we get the same seq no it is a retrans */
      if(RPL_ROUTE_IS_DAO_PENDING(rep) &&
         rep->state.dao_seqno_in == sequence) {
        /* keep the same seq-no as before for parent also */
        out_seq = rep->state.dao_seqno_out;
      } else {
        out_seq = prepare_for_dao_fwd(sequence, rep);
      }

      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
      PRINTF(" in seq: %d out seq: %d\n", sequence, out_seq);

      buffer = UIP_ICMP_PAYLOAD;

      buffer[sec_len + 3] = out_seq; /* add an outgoing seq no before fwd */

#if RPL_SECURITY
      add_security_section(p->timestamp, p->kim, p->lvl, p->key_index);
      set_ip_icmp_fields(rpl_get_parent_ipaddr(dag->preferred_parent),
                         RPL_CODE_SEC_DAO);
      set_ccm_nonce(ccm_nonce, rpl_sec_counter, p->lvl);

      rpl_sec_counter++;

      sec_aead(ccm_nonce, buffer_length, sec_len, p->lvl, RPL_ENCRYPT);
      uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                     ICMP6_RPL, RPL_CODE_SEC_DAO,
                     buffer_length + sec_len + mic_len);
#else
      uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                     ICMP6_RPL, RPL_CODE_DAO, buffer_length);
#endif
    }
    if(should_ack) {
      PRINTF("RPL: Sending DAO ACK\n");
      uip_clear_buf();
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
    }
  }
#endif /* RPL_WITH_STORING */
}
/*---------------------------------------------------------------------------*/
static void
dao_input_nonstoring(int sec_len, uint8_t mic_len)
{
#if RPL_WITH_NON_STORING
  uip_ipaddr_t dao_sender_addr;
  uip_ipaddr_t dao_parent_addr;
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint8_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  uip_ipaddr_t prefix;
  uint8_t buffer_length;
  int pos;
  int len;

  prefixlen = 0;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);
  memset(&dao_parent_addr, 0, 16);

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

  pos = sec_len;
  instance_id = buffer[pos++];
  instance = rpl_get_instance(instance_id);
  lifetime = instance->default_lifetime;

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  dag = instance->current_dag;
  /* Is the DAG ID present? */
  if(flags & RPL_DAO_D_FLAG) {
    if(memcmp(&dag->dag_id, &buffer[pos], sizeof(dag->dag_id))) {
      PRINTF("RPL: Ignoring a DAO for a DAG different from ours\n");
      return;
    }
    pos += 16;
  }

  /* Check if there are any RPL options present. */
  for(; pos < buffer_length + sec_len; pos += len) {
    subopt_type = buffer[pos];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* The option consists of a two-byte header and a payload. */
      len = 2 + buffer[pos + 1];
    }

    if(len + pos > buffer_length + sec_len) {
      PRINTF("RPL: Invalid DAO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      return;
    }

    switch(subopt_type) {
    case RPL_OPTION_TARGET:
      /* Handle the target option. */
      prefixlen = buffer[pos + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + pos + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_OPTION_TRANSIT:
      /* The path sequence and control are ignored. */
      /*      pathcontrol = buffer[pos + 3];
              pathsequence = buffer[pos + 4];*/
      lifetime = buffer[pos + 5];
      if(len >= 20) {
        memcpy(&dao_parent_addr, buffer + pos + 6, 16);
      }
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
         (unsigned)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF(", parent: ");
  PRINT6ADDR(&dao_parent_addr);
  PRINTF(" \n");

  if(lifetime == RPL_ZERO_LIFETIME) {
    PRINTF("RPL: No-Path DAO received\n");
    rpl_ns_expire_parent(dag, &prefix, &dao_parent_addr);
  } else {
    if(rpl_ns_update_node(dag, &prefix, &dao_parent_addr, RPL_LIFETIME(instance, lifetime)) == NULL) {
      PRINTF("RPL: failed to add link\n");
      return;
    }
  }

  if(flags & RPL_DAO_K_FLAG) {
    PRINTF("RPL: Sending DAO ACK\n");
    uip_clear_buf();
    dao_ack_output(instance, &dao_sender_addr, sequence,
                   RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
  }
#endif /* RPL_WITH_NON_STORING */
}
/*---------------------------------------------------------------------------*/
static void
dao_input(void)
{
  uint8_t *buffer;
  rpl_instance_t *instance;
  uint8_t instance_id;
  int sec_len;

#if RPL_SECURITY
  uint8_t mic_len;      /* n-byte MAC length */
  rpl_sec_section_t sec_section;
#endif

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a DAO from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;
  sec_len = 0;

#if RPL_SECURITY
  int pos;
  uint8_t buffer_length;

  uint8_t timestamp;
  uint8_t kim;
  uint8_t lvl;
  uint32_t counter;

  uint8_t key_index;

  uint8_t ccm_nonce[RPL_NONCE_LENGTH];

  pos = 0;

  rpl_remove_header();
  buffer = UIP_ICMP_PAYLOAD;

  timestamp = (buffer[pos++] & RPL_TIMESTAMP_MASK) >> RPL_TIMESTAMP_SHIFT;
  kim = (buffer[pos++] & RPL_KIM_MASK) >> RPL_KIM_SHIFT;
  lvl = (buffer[pos++] & RPL_LVL_MASK);
  ++pos;  /* Flags */

/*  Discard packets with time stamp instead of simple counters,
 *  Only accept packets with pre installed key
 *  Discard unassigned security levels
 */

  if((timestamp == 1) || (kim != 0) || (lvl > 3)) {
    goto discard;
  }

  counter = get32(buffer, pos);
  pos += 4;

  /* We are in KIM = 0, only key_index is present */
  key_index = buffer[pos++];
  /* Key_index must be zero because we use preinstalled key */
  if(key_index != 0) {
    goto discard;
  }

  sec_len = pos;
  mic_len = get_mic_len(lvl);
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

/* IP Header and ICMP header mutable fields set to 0 */
  set_ip_icmp_fields(NULL, RPL_CODE_SEC_DAO);

  set_ccm_nonce(ccm_nonce, counter, lvl);

  if(sec_aead(ccm_nonce, buffer_length, sec_len, lvl, RPL_DECRYPT) == 0) {
    PRINTF("RPL: MIC mismatch\n");
    goto discard;
  }

  sec_section.timestamp = timestamp;
  sec_section.kim = kim;
  sec_section.lvl = lvl;
  sec_section.key_source[0] = 0;  /* TODO: not implemented */
  sec_section.key_source[1] = 0;
  sec_section.key_index = key_index;
#endif /* RPL_SECURITY */

  instance_id = buffer[sec_len];
  instance = rpl_get_instance(instance_id);
  if(instance == NULL) {
    PRINTF("RPL: Ignoring a DAO for an unknown RPL instance(%u)\n",
           instance_id);
    goto discard;
  }

  if(RPL_IS_STORING(instance)) {
#if RPL_SECURITY
    dao_input_storing(sec_len, mic_len, (void *)&sec_section);
#else
    dao_input_storing(0, 0, NULL);
#endif
  } else if(RPL_IS_NON_STORING(instance)) {
#if RPL_SECURITY
    dao_input_nonstoring(sec_len, mic_len);
#else
    dao_input_nonstoring(0, 0);
#endif
  }

discard:
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_DAO_ACK
static void
handle_dao_retransmission(void *ptr)
{
  rpl_parent_t *parent;
  uip_ipaddr_t prefix;
  rpl_instance_t *instance;

  parent = ptr;
  if(parent == NULL || parent->dag == NULL || parent->dag->instance == NULL) {
    return;
  }
  instance = parent->dag->instance;

  if(instance->my_dao_transmissions >= RPL_DAO_MAX_RETRANSMISSIONS) {
    /* No more retransmissions - give up. */
    if(instance->lifetime_unit == 0xffff && instance->default_lifetime == 0xff) {
      /*
       * ContikiRPL was previously using infinite lifetime for routes
       * and no DAO_ACK configured. This probably means that the root
       * and possibly other nodes might be running an old version that
       * does not support DAO ack. Assume that everything is ok for
       * now and let the normal repair mechanisms detect any problems.
       */
      return;
    }

    if(RPL_IS_STORING(instance) && instance->of->dao_ack_callback) {
      /* Inform the objective function about the timeout. */
      instance->of->dao_ack_callback(parent, RPL_DAO_ACK_TIMEOUT);
    }

    /* Perform local repair and hope to find another parent. */
    rpl_local_repair(instance);
    return;
  }

  PRINTF("RPL: will retransmit DAO - seq:%d trans:%d\n", instance->my_dao_seqno,
         instance->my_dao_transmissions);

  if(get_global_addr(&prefix) == 0) {
    return;
  }

  ctimer_set(&instance->dao_retransmit_timer,
             RPL_DAO_RETRANSMISSION_TIMEOUT / 2 +
             (random_rand() % (RPL_DAO_RETRANSMISSION_TIMEOUT / 2)),
             handle_dao_retransmission, parent);

  instance->my_dao_transmissions++;
  dao_output_target_seq(parent, &prefix,
                        instance->default_lifetime, instance->my_dao_seqno);
}
#endif /* RPL_WITH_DAO_ACK */
/*---------------------------------------------------------------------------*/
void
dao_output(rpl_parent_t *parent, uint8_t lifetime)
{
  /* Destination Advertisement Object */
  uip_ipaddr_t prefix;

  if(get_global_addr(&prefix) == 0) {
    PRINTF("RPL: No global address set for this node - suppressing DAO\n");
    return;
  }

  if(parent == NULL || parent->dag == NULL || parent->dag->instance == NULL) {
    return;
  }

  RPL_LOLLIPOP_INCREMENT(dao_sequence);
#if RPL_WITH_DAO_ACK
  /* set up the state since this will be the first transmission of DAO */
  /* retransmissions will call directly to dao_output_target_seq */
  /* keep track of my own sending of DAO for handling ack and loss of ack */
  if(lifetime != RPL_ZERO_LIFETIME) {
    rpl_instance_t *instance;
    instance = parent->dag->instance;

    instance->my_dao_seqno = dao_sequence;
    instance->my_dao_transmissions = 1;
    ctimer_set(&instance->dao_retransmit_timer, RPL_DAO_RETRANSMISSION_TIMEOUT,
               handle_dao_retransmission, parent);
  }
#else
  /* We know that we have tried to register so now we are assuming
     that we have a down-link - unless this is a zero lifetime one */
  parent->dag->instance->has_downward_route = lifetime != RPL_ZERO_LIFETIME;
#endif /* RPL_WITH_DAO_ACK */

  /* Sending a DAO with own prefix as target */
  dao_output_target(parent, &prefix, lifetime);
}
/*---------------------------------------------------------------------------*/
void
dao_output_target(rpl_parent_t *parent, uip_ipaddr_t *prefix, uint8_t lifetime)
{
  dao_output_target_seq(parent, prefix, lifetime, dao_sequence);
}
/*---------------------------------------------------------------------------*/
static void
dao_output_target_seq(rpl_parent_t *parent, uip_ipaddr_t *prefix,
                      uint8_t lifetime, uint8_t seq_no)
{
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;
  uint8_t prefixlen;
  int pos;
  uip_ipaddr_t *parent_ipaddr = NULL;
  uip_ipaddr_t *dest_ipaddr = NULL;

#if RPL_SECURITY
  int sec_len;
  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  uint8_t mic_len;      /* n-byte MAC length */
#endif /* RPL_SECURITY */

  /* Destination Advertisement Object */

  /* If we are in feather mode, we should not send any DAOs */
  if(rpl_get_mode() == RPL_MODE_FEATHER) {
    return;
  }

  if(parent == NULL) {
    PRINTF("RPL dao_output_target error parent NULL\n");
    return;
  }

  parent_ipaddr = rpl_get_parent_ipaddr(parent);
  if(parent_ipaddr == NULL) {
    PRINTF("RPL dao_output_target error parent IP address NULL\n");
    return;
  }

  dag = parent->dag;
  if(dag == NULL) {
    PRINTF("RPL dao_output_target error dag NULL\n");
    return;
  }

  instance = dag->instance;

  if(instance == NULL) {
    PRINTF("RPL dao_output_target error instance NULL\n");
    return;
  }
  if(prefix == NULL) {
    PRINTF("RPL dao_output_target error prefix NULL\n");
    return;
  }
#ifdef RPL_DEBUG_DAO_OUTPUT
  RPL_DEBUG_DAO_OUTPUT(parent);
#endif

  buffer = UIP_ICMP_PAYLOAD;
  pos = 0;

#if RPL_SECURITY
  sec_len = add_security_section(RPL_TIMESTAMP_0, RPL_KIM_0,
                                 RPL_SEC_LVL, RPL_KEY_INDEX_0);
  pos += sec_len;
#endif /* RPL_SECURITY */

  buffer[pos++] = instance->instance_id;
  buffer[pos] = 0;
#if RPL_DAO_SPECIFY_DAG
  buffer[pos] |= RPL_DAO_D_FLAG;
#endif /* RPL_DAO_SPECIFY_DAG */
#if RPL_WITH_DAO_ACK
  if(lifetime != RPL_ZERO_LIFETIME) {
    buffer[pos] |= RPL_DAO_K_FLAG;
  }
#endif /* RPL_WITH_DAO_ACK */
  ++pos;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = seq_no;
#if RPL_DAO_SPECIFY_DAG
  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += sizeof(dag->dag_id);
#endif /* RPL_DAO_SPECIFY_DAG */

  /* create target subopt */
  prefixlen = sizeof(*prefix) * CHAR_BIT;
  buffer[pos++] = RPL_OPTION_TARGET;
  buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = prefixlen;
  memcpy(buffer + pos, prefix, (prefixlen + 7) / CHAR_BIT);
  pos += ((prefixlen + 7) / CHAR_BIT);

  /* Create a transit information sub-option. */
  buffer[pos++] = RPL_OPTION_TRANSIT;
  buffer[pos++] = (instance->mop != RPL_MOP_NON_STORING) ? 4 : 20;
  buffer[pos++] = 0; /* flags - ignored */
  buffer[pos++] = 0; /* path control - ignored */
  buffer[pos++] = 0; /* path seq - ignored */
  buffer[pos++] = lifetime;

  if(instance->mop != RPL_MOP_NON_STORING) {
    /* Send DAO to parent */
    dest_ipaddr = parent_ipaddr;
  } else {
    /* Include parent global IP address */
    memcpy(buffer + pos, &parent->dag->dag_id, 8); /* Prefix */
    pos += 8;
    memcpy(buffer + pos, ((const unsigned char *)parent_ipaddr) + 8, 8); /* Interface identifier */
    pos += 8;
    /* Send DAO to root */
    dest_ipaddr = &parent->dag->dag_id;
  }

  PRINTF("RPL: Sending a %sDAO with sequence number %u, lifetime %u, prefix ",
         lifetime == RPL_ZERO_LIFETIME ? "No-Path " : "", seq_no, lifetime);

  PRINT6ADDR(prefix);
  PRINTF(" to ");
  PRINT6ADDR(dest_ipaddr);
  PRINTF(" , parent ");
  PRINT6ADDR(parent_ipaddr);
  PRINTF("\n");

  if(dest_ipaddr != NULL) {
#if RPL_SECURITY
    set_ip_icmp_fields(dest_ipaddr, RPL_CODE_SEC_DAO);

    set_ccm_nonce(ccm_nonce, rpl_sec_counter, RPL_SEC_LVL);

    rpl_sec_counter++;

    mic_len = sec_aead(ccm_nonce, pos - sec_len, sec_len, RPL_SEC_LVL, RPL_ENCRYPT);

    pos += mic_len;

    uip_icmp6_send(dest_ipaddr, ICMP6_RPL, RPL_CODE_SEC_DAO, pos);
#else
    uip_icmp6_send(dest_ipaddr, ICMP6_RPL, RPL_CODE_DAO, pos);
#endif
  }
}
/*---------------------------------------------------------------------------*/
static void
dao_ack_input(void)
{
#if RPL_WITH_DAO_ACK

  uint8_t *buffer;
  uint8_t instance_id;
  uint8_t sequence;
  uint8_t status;
  rpl_instance_t *instance;
  rpl_parent_t *parent;
  int pos;
  int sec_len;

  buffer = UIP_ICMP_PAYLOAD;
  pos = 0;
  sec_len = 0;

#if RPL_SECURITY
  uint8_t buffer_length;

  uint8_t timestamp;
  uint8_t kim;
  uint8_t lvl;
  uint32_t counter;

  uint8_t key_index;

  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  uint8_t mic_len;      /* n-byte MAC length */

  rpl_remove_header();
  buffer = UIP_ICMP_PAYLOAD;

  timestamp = (buffer[pos++] & RPL_TIMESTAMP_MASK) >> RPL_TIMESTAMP_SHIFT;
  kim = (buffer[pos++] & RPL_KIM_MASK) >> RPL_KIM_SHIFT;
  lvl = (buffer[pos++] & RPL_LVL_MASK);
  ++pos;

/*  Discard packets with time stamp instead of simple counters,
 *  Only accept packets with pre installed key
 *  Discard unassigned security levels
 */

  if((timestamp == 1) || (kim != 0) || (lvl > 3)) {
    goto discard;
  }

  counter = get32(buffer, pos);
  pos += 4;

  /* We are in KIM = 0, only key_index is present */
  key_index = buffer[pos++];
  /* Key_index must be zero because we use preinstalled key */
  if(key_index != 0) {
    goto discard;
  }

  sec_len = pos;
  mic_len = get_mic_len(lvl);
  buffer_length = uip_len - uip_l3_icmp_hdr_len - sec_len - mic_len;

/* IP Header and ICMP header mutable fields set to 0 */
  set_ip_icmp_fields(NULL, RPL_CODE_SEC_DAO_ACK);

  set_ccm_nonce(ccm_nonce, counter, lvl);

  if(sec_aead(ccm_nonce, buffer_length, sec_len, lvl, RPL_DECRYPT) == 0) {
    PRINTF("RPL: MIC mismatch\n");
    goto discard;
  }

#endif

  instance_id = buffer[pos++];
  ++pos;
  sequence = buffer[pos++];
  status = buffer[pos++];

  instance = rpl_get_instance(instance_id);
  if(instance == NULL) {
    uip_clear_buf();
    return;
  }

  if(RPL_IS_STORING(instance)) {
    parent = rpl_find_parent(instance->current_dag, &UIP_IP_BUF->srcipaddr);
    if(parent == NULL) {
      /* not a known instance - drop the packet and ignore */
      uip_clear_buf();
      return;
    }
  } else {
    parent = NULL;
  }

  PRINTF("RPL: Received a DAO %s with sequence number %d (%d) and status %d from ",
         status < 128 ? "ACK" : "NACK",
         sequence, instance->my_dao_seqno, status);
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  if(sequence == instance->my_dao_seqno) {
    instance->has_downward_route = status < 128;

    /* always stop the retransmit timer when the ACK arrived */
    ctimer_stop(&instance->dao_retransmit_timer);

    /* Inform objective function on status of the DAO ACK */
    if(RPL_IS_STORING(instance) && instance->of->dao_ack_callback) {
      instance->of->dao_ack_callback(parent, status);
    }

#if RPL_REPAIR_ON_DAO_NACK
    if(status >= RPL_DAO_ACK_UNABLE_TO_ACCEPT) {
      /*
       * Failed the DAO transmission - need to remove the default route.
       * Trigger a local repair since we can not get our DAO in.
       */
      rpl_local_repair(instance);
    }
#endif
  } else if(RPL_IS_STORING(instance)) {
    /* this DAO ACK should be forwarded to another recently registered route */
    uip_ds6_route_t *re;
    uip_ipaddr_t *nexthop;
    if((re = find_route_entry_by_dao_ack(sequence)) != NULL) {
      /* pick the recorded seq no from that node and forward DAO ACK - and
         clear the pending flag*/
      RPL_ROUTE_CLEAR_DAO_PENDING(re);

      nexthop = uip_ds6_route_nexthop(re);
      if(nexthop == NULL) {
        PRINTF("RPL: No next hop to fwd DAO ACK to\n");
      } else {
        PRINTF("RPL: Fwd DAO ACK to:");
        PRINT6ADDR(nexthop);
        PRINTF("\n");
        buffer[sec_len + 2] = re->state.dao_seqno_in;

#if RPL_SECURITY
        add_security_section(timestamp, kim, lvl, key_index);
        set_ip_icmp_fields(nexthop, RPL_CODE_SEC_DAO_ACK);
        set_ccm_nonce(ccm_nonce, rpl_sec_counter, lvl);

        rpl_sec_counter++;

        sec_aead(ccm_nonce, buffer_length, sec_len, lvl, RPL_ENCRYPT);
        uip_icmp6_send(nexthop, ICMP6_RPL, RPL_CODE_SEC_DAO_ACK,
                       buffer_length + sec_len + mic_len);
#else
        uip_icmp6_send(nexthop, ICMP6_RPL, RPL_CODE_DAO_ACK, pos);
#endif /* RPL_SECURITY */
      }

      if(status >= RPL_DAO_ACK_UNABLE_TO_ACCEPT) {
        /* this node did not get in to the routing tables above... - remove */
        uip_ds6_route_rm(re);
      }
    } else {
      PRINTF("RPL: No route entry found to forward DAO ACK (seqno %u)\n", sequence);
    }
  }

#if RPL_SECURITY
discard:
  uip_clear_buf();
#endif
#endif /* RPL_WITH_DAO_ACK */

  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
void
dao_ack_output(rpl_instance_t *instance, uip_ipaddr_t *dest, uint8_t sequence,
               uint8_t status)
{
#if RPL_WITH_DAO_ACK
  unsigned char *buffer;
  int pos;

#if RPL_SECURITY
  int sec_len;
  uint8_t ccm_nonce[RPL_NONCE_LENGTH];
  uint8_t mic_len;      /* n-byte MAC length */
#endif /* RPL_SECURITY */

  pos = 0;
  buffer = UIP_ICMP_PAYLOAD;

#if RPL_SECURITY
  sec_len = add_security_section(RPL_TIMESTAMP_0, RPL_KIM_0,
                                 RPL_SEC_LVL, RPL_KEY_INDEX_0);

  pos += sec_len;
#endif /* RPL_SECURITY */

  PRINTF("RPL: Sending a DAO %s with sequence number %d to ", status < 128 ? "ACK" : "NACK", sequence);
  PRINT6ADDR(dest);
  PRINTF(" with status %d\n", status);

  buffer[pos++] = instance->instance_id;
  buffer[pos++] = 0;
  buffer[pos++] = sequence;
  buffer[pos++] = status;

#if RPL_SECURITY
  set_ip_icmp_fields(dest, RPL_CODE_SEC_DAO_ACK);

  set_ccm_nonce(ccm_nonce, rpl_sec_counter, RPL_SEC_LVL);

  rpl_sec_counter++;

  mic_len = sec_aead(ccm_nonce, pos - sec_len, sec_len, RPL_SEC_LVL, RPL_ENCRYPT);

  pos += mic_len;

  uip_icmp6_send(dest, ICMP6_RPL, RPL_CODE_SEC_DAO_ACK, pos);
#else
  uip_icmp6_send(dest, ICMP6_RPL, RPL_CODE_DAO_ACK, pos);
#endif /* RPL_SECURITY */
#endif /* RPL_WITH_DAO_ACK */
}
/*---------------------------------------------------------------------------*/
void
rpl_icmp6_register_handlers()
{
  uip_icmp6_register_input_handler(&dis_handler);
  uip_icmp6_register_input_handler(&dio_handler);
  uip_icmp6_register_input_handler(&dao_handler);
  uip_icmp6_register_input_handler(&dao_ack_handler);
#if RPL_SECURITY
  rpl_last_dis.responded = RPL_DIS_RESPONDED;
#endif /* RPL_SECURITY */
}
/*---------------------------------------------------------------------------*/

/** @}*/
