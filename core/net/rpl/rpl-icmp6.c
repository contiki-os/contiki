
/**
 * \addtogroup uip6
 * @{
 */
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
 * $Id: rpl-icmp6.c,v 1.18 2010/06/06 21:42:50 nvt-se Exp $
 */
/**
 * \file
 *         ICMP6 I/O for RPL control messages.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Joel Hoglund <joel@sics.se>,
 *               Mathieu Pouillot <m.pouillot@watteco.com>
 */

#include "net/tcpip.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"
#include "net/uip-icmp6.h"
#include "net/rpl/rpl.h"
#include "net/rime/packetbuf.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_ANNOTATE

#include "net/uip-debug.h"

/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED            0x80
#define RPL_DIO_DEST_ADV_SUPPORTED  0x40
#define RPL_DIO_DEST_ADV_TRIGGER    0x20
#define RPL_DIO_MOP_MASK            0x18
#define RPL_DIO_MOP_NON_STORING     0x00
#define RPL_DIO_MOP_STORING         0x10
#define RPL_DIO_DAG_PREFERENCE_MASK 0x07


#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/*---------------------------------------------------------------------------*/
static void dis_input(void);
static void dio_input(void);
static void dao_input(void);
static void dao_ack_input(void);

static uint8_t dao_sequence;
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
      if(!uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
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
         (uint32_t)buffer[pos + 2] << 8  | buffer[pos + 3];
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
static void
dis_input(void)
{
  rpl_dag_t *dag;

  /* DAG Information Solicitation */
  PRINTF("RPL: Received a DIS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  dag = rpl_get_dag(RPL_ANY_INSTANCE);
  if(dag != NULL) {
    if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
      PRINTF("RPL: Multicast DIS => reset DIO timer\n");
      rpl_reset_dio_timer(dag, 0);
    } else {
      PRINTF("RPL: Unicast DIS, reply to sender\n");
      dio_output(dag, &UIP_IP_BUF->srcipaddr);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
dis_output(uip_ipaddr_t *addr)
{
  unsigned char *buffer;
  uip_ipaddr_t tmpaddr;

  /* DAG Information Solicitation  - 2 bytes reserved */
  /*      0                   1                   2        */
  /*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  */
  /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
  /*     |           Reserved            |   Option(s)...  */
  /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

  buffer = UIP_ICMP_PAYLOAD;
  buffer[0] = buffer[1] = 0;

  if(addr == NULL) {
    PRINTF("RPL: Sending a DIS\n");
    uip_create_linklocal_allrouters_mcast(&tmpaddr);
    addr = &tmpaddr;
  } else {
    PRINTF("RPL: Sending a unicast DIS\n");
  }
  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_DIS, 2);

}
/*---------------------------------------------------------------------------*/
static void
dio_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  rpl_dio_t dio;
  uint8_t subopt_type;
  int i;
  int len;
  uip_ipaddr_t from;
  uip_ds6_nbr_t *nbr;

  dio.dag_intdoubl = DEFAULT_DIO_INTERVAL_DOUBLINGS;
  dio.dag_intmin = DEFAULT_DIO_INTERVAL_MIN;
  dio.dag_redund = DEFAULT_DIO_REDUNDANCY;

  uip_ipaddr_copy(&from, &UIP_IP_BUF->srcipaddr);

  /* DAG Information Object */
  PRINTF("RPL: Received a DIO from ");
  PRINT6ADDR(&from);
  PRINTF("\n");

  if((nbr = uip_ds6_nbr_lookup(&from)) == NULL) {
    if((nbr = uip_ds6_nbr_add(&from, (uip_lladdr_t *)
                              packetbuf_addr(PACKETBUF_ADDR_SENDER),
                              0, NBR_REACHABLE)) != NULL) {
      /* set reachable timer */
      stimer_set(&(nbr->reachable), UIP_ND6_REACHABLE_TIME);
      PRINTF("RPL: Neighbor added to neighbor cache ");
      PRINT6ADDR(&from);
      PRINTF(", ");
      PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      PRINTF("\n");
    }
  } else {
    PRINTF("RPL: Neighbor already in neighbor cache\n");
  }


  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

  /* Process the DIO base option. */
  i = 0;
  buffer = UIP_ICMP_PAYLOAD;

  dio.instance_id = buffer[i++];
  dio.version = buffer[i++];
  dio.dag_rank = (buffer[i] << 8) + buffer[i + 1];
  i += 2;

  dio.grounded = buffer[i] & RPL_DIO_GROUNDED;
  dio.dst_adv_trigger = buffer[i] & RPL_DIO_DEST_ADV_TRIGGER;
  dio.dst_adv_supported = buffer[i] & RPL_DIO_DEST_ADV_SUPPORTED;
  dio.preference = buffer[i++] & RPL_DIO_DAG_PREFERENCE_MASK;

  dio.dtsn = buffer[i++];
  /* two reserved bytes */
  i += 2;

  memcpy(&dio.dag_id, buffer + i, sizeof(dio.dag_id));
  i += sizeof(dio.dag_id);

  /* Check if there are any DIO suboptions. */
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_DIO_SUBOPT_PAD1) {
      len = 1;
    } else {
      /* Suboption with a two-byte header + payload */
      len = 2 + buffer[i + 1];
    }

    switch(subopt_type) {
    case RPL_DIO_SUBOPT_DAG_MC:
      if(len < 7) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
        return;
      }
      break;
    case RPL_DIO_SUBOPT_ROUTE_INFO:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
        return;
      }
      /* flags is both preference and flags for now */
      dio.destination_prefix.length = buffer[i + 2];
      dio.destination_prefix.flags = buffer[i + 3];
      dio.destination_prefix.lifetime = get32(buffer, i + 4);

      if(((dio.destination_prefix.length + 7)/ 8) + 8 <= len &&
         dio.destination_prefix.length <= 128) {
        PRINTF("RPL: Copying destination prefix\n");
        memcpy(&dio.destination_prefix.prefix, &buffer[i + 8],
               (dio.destination_prefix.length + 7) / 8);
      } else {
        PRINTF("RPL: Invalid route infoprefix option, len = %d\n", len);
      }

      break;
    case RPL_DIO_SUBOPT_DAG_CONF:
      /* Path control field not yet implemented - at i + 2 */
      dio.dag_intdoubl = buffer[i + 3];
      dio.dag_intmin = buffer[i + 4];
      dio.dag_redund = buffer[i + 5];
      dio.dag_max_rankinc = (buffer[i + 6] << 8) | buffer[i + 7];
      dio.dag_min_hoprankinc = (buffer[i + 8] << 8) | buffer[i + 9];
      PRINTF("RPL: DIO trickle timer:dbl=%d, min=%d red=%d maxinc=%d mininc=%d\n",
             dio.dag_intdoubl,
             dio.dag_intmin, dio.dag_redund,
             dio.dag_max_rankinc, dio.dag_min_hoprankinc);
      break;
    case RPL_DIO_SUBOPT_OCP:
      dio.ocp = buffer[i + 2] << 8 | buffer[i + 3];
      PRINTF("RPL: DAG OCP Sub-opt received OCP = %u\n", dio.ocp);
      break;
    case RPL_DIO_SUBOPT_PREFIX_INFO:
      if(len != 32) {
        PRINTF("RPL: DAG Prefix info not ok, len != 32\n");
        return;
      }
      dio.prefix_info.length = buffer[i + 2];
      dio.prefix_info.flags = buffer[i + 3];
      /* valid lifetime is ingnored for now - at i + 4 */
      /* preferred lifetime stored in lifetime */
      dio.prefix_info.lifetime = get32(buffer, i + 8);
      /* 32-bit reserved at i + 12 */
      PRINTF("RPL: Copying prefix information\n");
      memcpy(&dio.prefix_info.prefix, &buffer[i + 16], 16);
      break;
    }
  }

  rpl_process_dio(&from, &dio);
}
/*---------------------------------------------------------------------------*/
void
dio_output(rpl_dag_t *dag, uip_ipaddr_t *uc_addr)
{
  unsigned char *buffer;
  int pos;
  uip_ipaddr_t addr;

  /* DAG Information Solicitation */
  PRINTF("RPL: Sending a DIO with rank: %u\n", (unsigned)dag->rank);
  pos = 0;

  buffer = UIP_ICMP_PAYLOAD;
  buffer[pos++] = dag->instance_id;
  buffer[pos++] = dag->version;
  buffer[pos++] = dag->rank >> 8;
  buffer[pos++] = dag->rank & 0xff;

  if(dag->grounded) {
    buffer[pos] |= RPL_DIO_GROUNDED;
  }
  /* Set dst_adv_trigger and dst_adv_supported. */
  buffer[pos] |= RPL_DIO_DEST_ADV_SUPPORTED | RPL_DIO_DEST_ADV_TRIGGER;
  pos++;
  buffer[pos++] = dag->dtsn;
  /* reserved 2 bytes */
  pos += 2;

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

  /* The objective function object must appear first. */
  buffer[pos++] = RPL_DIO_SUBOPT_OCP;
  buffer[pos++] = 2;
  buffer[pos++] = dag->of->ocp >> 8;
  buffer[pos++] = dag->of->ocp & 0xff;

  /* always add a sub-option for DAG configuration */
  buffer[pos++] = RPL_DIO_SUBOPT_DAG_CONF;
  buffer[pos++] = 8;
  buffer[pos++] = 0; /* PCS */
  buffer[pos++] = dag->dio_intdoubl;
  buffer[pos++] = dag->dio_intmin;
  buffer[pos++] = dag->dio_redundancy;
  buffer[pos++] = dag->max_rankinc >> 8;
  buffer[pos++] = dag->max_rankinc & 0xff;
  buffer[pos++] = dag->min_hoprankinc >> 8;
  buffer[pos++] = dag->min_hoprankinc & 0xff;

  /* if prefix info length > 0 then we have a prefix to send! */
  if(dag->prefix_info.length > 0) {
    buffer[pos++] = RPL_DIO_SUBOPT_PREFIX_INFO;
    buffer[pos++] = 30; /* always 30 bytes + 2 long */
    buffer[pos++] = dag->prefix_info.length;
    buffer[pos++] = dag->prefix_info.flags;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    memset(&buffer[pos], 0, 4);
    pos += 4;
    memcpy(&buffer[pos], &(dag->prefix_info.prefix), 16);
    pos += 16;
    PRINTF("RPL: Sending prefix info in DIO ");
    PRINT6ADDR(&dag->prefix_info.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce. len:%d\n",
           dag->prefix_info.length);
  }

  /* buffer[len++] = RPL_DIO_SUBOPT_PAD1; */

  /* Unicast requests get unicast replies! */
  if(uc_addr == NULL) {
    PRINTF("RPL: Sending a multicast-DIO with rank %u\n",
        (unsigned)dag->rank);
    uip_create_linklocal_allrouters_mcast(&addr);
    uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DIO, pos);
  } else {
    PRINTF("RPL: Sending unicast-DIO with rank %u to ",
        (unsigned)dag->rank);
    PRINT6ADDR(uc_addr);
    PRINTF("\n");
    uip_icmp6_send(uc_addr, ICMP6_RPL, RPL_CODE_DIO, pos);
  }
}
/*---------------------------------------------------------------------------*/
static void
dao_input(void)
{
  uip_ipaddr_t dao_sender_addr;
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint32_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  rpl_parent_t *n;
  uint8_t buffer_length;
  int pos;
  int len;
  int i;
  int learned_from;

  lifetime = 0;
  prefixlen = 0;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a DAO from ");
  PRINT6ADDR(&dao_sender_addr);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

  pos = 0;
  instance_id = buffer[pos++];
  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  /* is the DAGID present ? */
  if(flags & RPL_DAO_D_FLAG) {
    /* currently the DAG ID is ignored since we only use global
       RPL Instance IDs... */
    pos += 16;
  }

  /* handle the target option */
  dag = rpl_get_dag(instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Ignoring a DAO for a different RPL instance (%u)\n",
           instance_id);
    return;
  }

  /* Check if there are any DIO suboptions. */
  i = pos;
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_DIO_SUBOPT_PAD1) {
      len = 1;
    } else {
      /* Suboption with a two-byte header + payload */
      len = 2 + buffer[i + 1];
    }

    switch(subopt_type) {
    case RPL_DIO_SUBOPT_TARGET:
      prefixlen = buffer[i + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_DIO_SUBOPT_TRANSIT:
      /* path sequence and control ignored */
      lifetime = get32(buffer, i + 4);
      /* parent address also ignored */
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %lu, prefix length: %u prefix: ",
         (unsigned long)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF("\n");

  if(lifetime == ZERO_LIFETIME) {
    /* No-DAO received; invoke the route purging routine. */
    rep = uip_ds6_route_lookup(&prefix);
    if(rep != NULL && rep->state.saved_lifetime == 0) {
      PRINTF("RPL: Setting expiration timer for prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      rep->state.saved_lifetime = rep->state.lifetime;
      rep->state.lifetime = DAO_EXPIRATION_TIMEOUT;
    }
    return;
  }

  learned_from = uip_is_addr_mcast(&dao_sender_addr) ?
                 RPL_ROUTE_FROM_MULTICAST_DAO : RPL_ROUTE_FROM_UNICAST_DAO;

  rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
  if(rep == NULL) {
    PRINTF("RPL: Could not add a route after receiving a DAO\n");
    return;
  } else {
    rep->state.lifetime = lifetime;
    rep->state.learned_from = learned_from;
  }

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    if((n = rpl_preferred_parent(dag)) != NULL) {
      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(&n->addr);
      PRINTF("\n");
      uip_icmp6_send(&n->addr, ICMP6_RPL, RPL_CODE_DAO, buffer_length);
    } else if(flags & RPL_DAO_K_FLAG) {
      dao_ack_output(dag, &dao_sender_addr, sequence);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
dao_output(rpl_parent_t *n, uint32_t lifetime)
{
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint8_t prefixlen;
  uip_ipaddr_t addr;
  uip_ipaddr_t prefix;
  int pos;

  /* Destination Advertisement Object */
  if(get_global_addr(&prefix) == 0) {
    PRINTF("RPL: No global address set for this node - suppressing DAO\n");
    return;
  }

  if(n == NULL) {
    dag = rpl_get_dag(RPL_ANY_INSTANCE);
    if(dag == NULL) {
      PRINTF("RPL: Did not join a DAG before sending DAO\n");
      return;
    }
  } else {
    dag = n->dag;
  }

  buffer = UIP_ICMP_PAYLOAD;

  ++dao_sequence;
  pos = 0;

  buffer[pos++] = dag->instance_id;
  buffer[pos++] = RPL_DAO_K_FLAG; /* no ack request, no DODAGID */
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = dao_sequence & 0xff;

  /* create target subopt */
  prefixlen = sizeof(prefix) * CHAR_BIT;
  buffer[pos++] = RPL_DIO_SUBOPT_TARGET;
  buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = prefixlen;
  memcpy(buffer + pos, &prefix, (prefixlen + 7) / CHAR_BIT);
  pos += ((prefixlen + 7) / CHAR_BIT);

  /* create a transit information subopt */
  buffer[pos++] = RPL_DIO_SUBOPT_TRANSIT;
  buffer[pos++] = 6;
  buffer[pos++] = 0; /* path seq - ignored */
  buffer[pos++] = 0; /* path control - ignored */
  set32(buffer, pos, lifetime);
  pos += 4;

  if(n == NULL) {
    uip_create_linklocal_allnodes_mcast(&addr);
  } else {
    uip_ipaddr_copy(&addr, &n->addr);
  }

  PRINTF("RPL: Sending DAO with prefix ");
  PRINT6ADDR(&prefix);
  PRINTF(" to ");
  if(n != NULL) {
    PRINT6ADDR(&n->addr);
  } else {
    PRINTF("multicast address");
  }
  PRINTF("\n");

  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DAO, pos);
}
/*---------------------------------------------------------------------------*/
static void
dao_ack_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  uint8_t instance_id;
  uint8_t sequence;
  uint8_t status;

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

  instance_id = buffer[0];
  sequence = buffer[2];
  status = buffer[3];

  PRINTF("RPL: Received a DAO ACK with sequence number %d and status %d from ",
    sequence, status);
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
void
dao_ack_output(rpl_dag_t *dag, uip_ipaddr_t *dest, uint8_t sequence)
{
  unsigned char *buffer;

  PRINTF("RPL: Sending a DAO ACK with sequence number %d to ", sequence);
  PRINT6ADDR(dest);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;

  buffer[0] = dag->instance_id;
  buffer[1] = 0;
  buffer[2] = sequence;
  buffer[3] = 0;

  uip_icmp6_send(dest, ICMP6_RPL, RPL_CODE_DAO_ACK, 4);
}
/*---------------------------------------------------------------------------*/
void
uip_rpl_input(void)
{
  PRINTF("Received an RPL control message\n");
  switch(UIP_ICMP_BUF->icode) {
  case RPL_CODE_DIO:
    dio_input();
    break;
  case RPL_CODE_DIS:
    dis_input();
    break;
  case RPL_CODE_DAO:
    dao_input();
    break;
  case RPL_CODE_DAO_ACK:
    dao_ack_input();
    break;
  default:
    PRINTF("RPL: received an unknown ICMP6 code (%u)\n", UIP_ICMP_BUF->icode);
    break;
  }

  uip_len = 0;
}
