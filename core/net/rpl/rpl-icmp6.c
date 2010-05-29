
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
 * $Id: rpl-icmp6.c,v 1.12 2010/05/29 22:23:21 nvt-se Exp $
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

#define RPL_DIO_GROUNDED            0x80
#define RPL_DIO_DEST_ADV_SUPPORTED  0x40
#define RPL_DIO_DEST_ADV_TRIGGER    0x20
#define RPL_DIO_DEST_ADV_STORED     0x10
#define RPL_DIO_DAG_PREFERENCE_MASK 0x07


#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])

static void dis_input(void);
static void dio_input(void);
static void dao_input(void);

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

  /* DAG Information Solicitation */

  /* Add a padding to be compliant with ICMPv6 */
  buffer = UIP_ICMP_PAYLOAD;
  buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

  if(addr == NULL) {
    PRINTF("RPL: Sending a DIS\n");
    uip_create_linklocal_allrouters_mcast(&tmpaddr);
    addr = &tmpaddr;
  } else {
    PRINTF("RPL: Sending a unicast DIS\n");
  }
  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_DIS, 4);

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
  dio.grounded = buffer[i] & RPL_DIO_GROUNDED;
  dio.dst_adv_trigger = buffer[i] & RPL_DIO_DEST_ADV_TRIGGER;
  dio.dst_adv_supported = buffer[i] & RPL_DIO_DEST_ADV_SUPPORTED;
  dio.preference = buffer[i++] & RPL_DIO_DAG_PREFERENCE_MASK;
  dio.sequence_number = buffer[i++];

  dio.dag_rank = (buffer[i] << 8) + buffer[i + 1];
  i += 2;
  dio.instance_id = buffer[i++];
  dio.dtsn = buffer[i++];

  memcpy(&dio.dag_id, buffer + i, sizeof(dio.dag_id));
  i += sizeof(dio.dag_id);

  /* Check if there are any DIO suboptions. */
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_DIO_SUBOPT_PAD1) {
      len = 1;
    } else {
      /* Suboption with a three-byte header. */
      len = (buffer[i + 1] << 8) + buffer[i + 2] + 3;
    }

    switch(subopt_type) {
    case RPL_DIO_SUBOPT_DAG_MC:
      if(len < 7) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
        return;
      }
      break;
    case RPL_DIO_SUBOPT_DEST_PREFIX:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
        return;
      }
      /* prebference is both preference and flags for now */
      dio.destination_prefix.preference = buffer[i + 3];
      dio.destination_prefix.lifetime = get32(buffer, i + 4);
      dio.destination_prefix.length = buffer[i + 8];
      if(((dio.destination_prefix.length + 7)/ 8) + 9 <= len &&
         dio.destination_prefix.length <= 128) {
        PRINTF("RPL: Copying destination prefix\n");
        memcpy(&dio.destination_prefix.prefix, &buffer[i + 9],
               (dio.destination_prefix.length + 7) / 8);

        /* NOTE: This is a test for adding autoconf prefix in RPL via  */
        /* a destination prefix - this use one reserved flag bit       */
        /* This should not be made until RPL decides to join the DAG   */
        /* And not if there already is a global address configured for */
        /* the specific DAG */
        if((dio.destination_prefix.preference & UIP_ND6_RA_FLAG_AUTONOMOUS)) {
          uip_ipaddr_t ipaddr;
          memcpy(&ipaddr, &dio.destination_prefix.prefix,
                 (dio.destination_prefix.length + 7) / 8);
          uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
          if(uip_ds6_addr_lookup(&ipaddr) == NULL) {
            PRINTF("RPL: adding global IP address ");
            PRINT6ADDR(&ipaddr);
            PRINTF("\n");
            uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
          }
        }
      } else {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
      }

      break;
    case RPL_DIO_SUBOPT_DAG_CONF:
      dio.dag_intdoubl = buffer[i + 3];
      dio.dag_intmin = buffer[i + 4];
      dio.dag_redund = buffer[i + 5];
      dio.dag_max_rankinc = buffer[i + 6];
      dio.dag_min_hoprankinc = buffer[i + 7];
      PRINTF("RPL: DIO trickle timer:dbl=%d, min=%d red=%d maxinc=%d mininc=%d\n",
	     dio.dag_intdoubl,
             dio.dag_intmin, dio.dag_redund,
             dio.dag_max_rankinc, dio.dag_min_hoprankinc);
      break;
    case RPL_DIO_SUBOPT_OCP:
      dio.ocp = buffer[i + 3] << 8 | buffer[i + 4];
      PRINTF("RPL: DAG OCP Sub-opt received OCP = %u\n", dio.ocp);
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
  buffer[0] = dag->preference;
  if(dag->grounded) {
    buffer[0] |= RPL_DIO_GROUNDED;
  }
  /* Set dst_adv_trigger and dst_adv_supported. */
  buffer[0] |= RPL_DIO_DEST_ADV_SUPPORTED | RPL_DIO_DEST_ADV_TRIGGER;

  buffer[1] = dag->sequence_number;

  pos = 2;
  buffer[pos++] = dag->rank >> 8;
  buffer[pos++] = dag->rank & 0xff;
  buffer[pos++] = dag->instance_id;
  buffer[pos++] = dag->dtsn;

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

  /* The objective function object must appear first. */
  buffer[pos++] = RPL_DIO_SUBOPT_OCP;
  buffer[pos++] = 0;
  buffer[pos++] = 2;
  buffer[pos++] = dag->of->ocp >> 8;
  buffer[pos++] = dag->of->ocp & 0xff;

  /* alignment here ??? */

  /* always add a sub-option for DAG configuration */
  buffer[pos++] = RPL_DIO_SUBOPT_DAG_CONF;
  buffer[pos++] = 0;
  buffer[pos++] = 5;
  buffer[pos++] = dag->dio_intdoubl;
  buffer[pos++] = dag->dio_intmin;
  buffer[pos++] = dag->dio_redundancy;
  buffer[pos++] = dag->max_rankinc;
  buffer[pos++] = dag->min_hoprankinc;

  /* if prefix length > 0 then we have a prefix to send! */
  if(dag->destination_prefix.length > 0) {
    buffer[pos++] = RPL_DIO_SUBOPT_DEST_PREFIX;
    buffer[pos++] = 0;
    buffer[pos++] = (dag->destination_prefix.length + 7)/ 8 + 9;
    buffer[pos++] = dag->destination_prefix.preference;
    set32(buffer, pos, dag->destination_prefix.lifetime);
    pos += 4;
    buffer[pos++] = dag->destination_prefix.length;
    memcpy(&buffer[pos], &(dag->destination_prefix.prefix),
           (dag->destination_prefix.length + 7)/ 8);
    pos += (dag->destination_prefix.length + 7)/ 8;
    PRINTF("RPL: Sending prefix info in DIO ");
    PRINT6ADDR(&dag->destination_prefix.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce. len:%d\n",
           dag->destination_prefix.length);
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
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint32_t lifetime;
  uint8_t rank;
  uint8_t prefixlen;
  uint32_t route_tag;
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  rpl_parent_t *n;
  int pos;

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a DAO from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;

  pos = 0;
  sequence = (buffer[pos] << 8) | buffer[pos + 1];
  pos += 2;

  rank = (buffer[pos] << 8) | buffer[pos + 1];
  pos += 2;

  /* pos = 4 */
  instance_id = buffer[pos++];

  route_tag = buffer[pos++];
  prefixlen = buffer[pos++];
  /* ignore RRCount for now: rrcount = buffer[pos++]; */
  pos++;
  /* pos = 8 */
  lifetime = get32(buffer, pos);
  pos += 4;

  dag = rpl_get_dag(instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Ignoring a DAO for a different DAG instance (%u)\n",
        instance_id);
    return;
  }

  if(rank < dag->rank) {
    PRINTF("RPL: Incoming DAO rank is %u, my rank is %u\n",
	   (unsigned)rank, (unsigned)dag->rank);
    return;
  }

  memset(&prefix, 0, sizeof(prefix));
  memcpy(&prefix, buffer + pos, prefixlen / CHAR_BIT);

  PRINTF("RPL: DAO rank: %u, lifetime: %lu, prefix length: %u",
	(unsigned)rank, (unsigned long)lifetime, (unsigned)prefixlen);
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

  rep = rpl_add_route(dag, &prefix, prefixlen,
                      &UIP_IP_BUF->srcipaddr);
  if(rep == NULL) {
    PRINTF("RPL: Could not add a route while receiving a DAO\n");
    return;
  }

  rep->state.lifetime = lifetime;

  if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
    rep->state.learned_from = RPL_ROUTE_FROM_MULTICAST_DAO;
  } else {
    rep->state.learned_from = RPL_ROUTE_FROM_UNICAST_DAO;
    if((n = rpl_preferred_parent(dag)) != NULL) {
      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(&n->addr);
      PRINTF("\n");
      uip_icmp6_send(&n->addr, ICMP6_RPL, RPL_CODE_DAO,
        14 + (prefixlen / CHAR_BIT));
    }
  }
}
/*---------------------------------------------------------------------------*/
void
dao_output(rpl_parent_t *n, uint32_t lifetime)
{
  rpl_dag_t *dag;
  unsigned char *buffer;
  static uint16_t sequence;
  uint8_t prefixlen;
  uip_ipaddr_t addr;
  uip_ipaddr_t prefix;
  int pos;

  /* Destination Advertisement Object */

  if(n == NULL) {
    dag = rpl_get_dag(RPL_ANY_INSTANCE);
    if(dag == NULL) {
      PRINTF("RPL: Did not join a DAG before receiving DAO\n");
      return;
    }
  } else {
    dag = n->dag;
  }

  buffer = UIP_ICMP_PAYLOAD;

  ++sequence;
  pos = 0;
  buffer[pos++] = sequence >> 8;
  buffer[pos++] = sequence & 0xff;

  buffer[pos++] = dag->rank >> 8;
  buffer[pos++] = dag->rank & 0xff;

  /* pos = 4 */
  buffer[pos++] = dag->instance_id;
  /* Route tag. Unspecified in draft-ietf-roll-rpl-06. */
  buffer[pos++] = 0;

  prefixlen = sizeof(prefix) * CHAR_BIT;
  buffer[pos++] = prefixlen;
  /* Reverse route count. Not used because reverse routing is unscalable
     beyond a few hops. */
  buffer[pos++] = 0;
  /* pos = 8 */
  /* DAO Lifetime. */
  set32(buffer, pos, lifetime);
  pos += 4;

  /* pos = 12 or 14 */
  if(get_global_addr(&prefix) == 0) {
    PRINTF("RPL: No global address set for this node - suppressing DAO\n");
    return;
  }
  memcpy(buffer + pos, &prefix, prefixlen / CHAR_BIT);
  pos += (prefixlen / CHAR_BIT);

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
  default:
    PRINTF("RPL: received an unknown ICMP6 code (%u)\n", UIP_ICMP_BUF->icode);
    break;
  }

  uip_len = 0;
}
