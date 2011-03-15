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
#include "net/rpl/rpl-private.h"
#include "net/packetbuf.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_NONE

#include "net/uip-debug.h"

/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED                 0x80
#define RPL_DIO_MOP_SHIFT                3
#define RPL_DIO_MOP_MASK                 0x3c
#define RPL_DIO_PREFERENCE_MASK          0x07

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
  /*     |     Flags     |   Reserved    |   Option(s)...  */
  /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

  buffer = UIP_ICMP_PAYLOAD;
  buffer[0] = buffer[1] = 0;

  if(addr == NULL) {
    PRINTF("RPL: Sending a DIS\n");
    uip_create_linklocal_rplnodes_mcast(&tmpaddr);
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
      stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
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
  dio.rank = (buffer[i] << 8) | buffer[i + 1];
  i += 2;

  PRINTF("RPL: Incoming DIO rank %u\n", (unsigned)dio.rank);

  dio.grounded = buffer[i] & RPL_DIO_GROUNDED;
  dio.mop = (buffer[i]& RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT;
  dio.preference = buffer[i++] & RPL_DIO_PREFERENCE_MASK;

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

    if(len + i > buffer_length) {
      PRINTF("RPL: Invalid DIO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      return;
    }

    PRINTF("RPL: DIO suboption %u, length: %u\n", subopt_type, len - 2);

    switch(subopt_type) {
    case RPL_DIO_SUBOPT_DAG_METRIC_CONTAINER:
      if(len < 6) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      dio.mc.type = buffer[i + 2];
      dio.mc.flags = buffer[i + 3];
      dio.mc.aggr = buffer[i + 4] >> 4;
      dio.mc.prec = buffer[i + 4] & 0xf;
      dio.mc.length = buffer[i + 5];

      if(dio.mc.type == RPL_DAG_MC_ETX) {
        dio.mc.obj.etx = buffer[i + 6] << 8;
        dio.mc.obj.etx |= buffer[i + 7];

        PRINTF("RPL: DAG MC: type %u, flags %u, aggr %u, prec %u, length %u, ETX %u\n",
	       (unsigned)dio.mc.type,  
	       (unsigned)dio.mc.flags, 
	       (unsigned)dio.mc.aggr, 
	       (unsigned)dio.mc.prec, 
	       (unsigned)dio.mc.length, 
	       (unsigned)dio.mc.obj.etx);
      } else if(dio.mc.type == RPL_DAG_MC_ENERGY) {
        dio.mc.obj.energy.flags = buffer[i + 6];
        dio.mc.obj.energy.energy_est = buffer[i + 7];
      } else {
       PRINTF("RPL: Unhandled DAG MC type: %u\n", (unsigned)dio.mc.type);
       return;
      }
      break;
    case RPL_DIO_SUBOPT_ROUTE_INFO:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
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
	RPL_STAT(rpl_stats.malformed_msgs++);
	return;
      }

      break;
    case RPL_DIO_SUBOPT_DAG_CONF:
      if(len != 16) {
        PRINTF("RPL: Invalid DAG configuration option, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }

      /* Path control field not yet implemented - at i + 2 */
      dio.dag_intdoubl = buffer[i + 3];
      dio.dag_intmin = buffer[i + 4];
      dio.dag_redund = buffer[i + 5];
      dio.dag_max_rankinc = (buffer[i + 6] << 8) | buffer[i + 7];
      dio.dag_min_hoprankinc = (buffer[i + 8] << 8) | buffer[i + 9];
      dio.ocp = (buffer[i + 10] << 8) | buffer[i + 11];
      /* buffer + 12 is reserved */
      dio.default_lifetime = buffer[i + 13];
      dio.lifetime_unit = (buffer[i + 14] << 8) | buffer[i + 15];
      PRINTF("RPL: DIO Conf:dbl=%d, min=%d red=%d maxinc=%d mininc=%d ocp=%d d_l=%u l_u=%u\n",
             dio.dag_intdoubl, dio.dag_intmin, dio.dag_redund,
             dio.dag_max_rankinc, dio.dag_min_hoprankinc, dio.ocp,
             dio.default_lifetime, dio.lifetime_unit);
      break;
    case RPL_DIO_SUBOPT_PREFIX_INFO:
      if(len != 32) {
        PRINTF("RPL: DAG Prefix info not ok, len != 32\n");
	RPL_STAT(rpl_stats.malformed_msgs++);
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
    default:
      PRINTF("RPL: Unsupported suboption type in DIO: %u\n",
	(unsigned)subopt_type);
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

  /* DAG Information Object */
  pos = 0;

  buffer = UIP_ICMP_PAYLOAD;
  buffer[pos++] = dag->instance_id;
  buffer[pos++] = dag->version;
  buffer[pos++] = dag->rank >> 8;
  buffer[pos++] = dag->rank & 0xff;

  buffer[pos] = 0;
  if(dag->grounded) {
    buffer[pos] |= RPL_DIO_GROUNDED;
  }

  buffer[pos] = dag->mop << RPL_DIO_MOP_SHIFT;
  pos++;

  buffer[pos++] = ++dag->dtsn_out;

  /* reserved 2 bytes */
  buffer[pos++] = 0; /* flags */
  buffer[pos++] = 0; /* reserved */

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

  if(dag->mc.type != RPL_DAG_MC_NONE) {
    dag->of->update_metric_container(dag);

    buffer[pos++] = RPL_DIO_SUBOPT_DAG_METRIC_CONTAINER;
    buffer[pos++] = 6;
    buffer[pos++] = dag->mc.type;
    buffer[pos++] = dag->mc.flags;
    buffer[pos] = dag->mc.aggr << 4;
    buffer[pos++] |= dag->mc.prec;
    if(dag->mc.type == RPL_DAG_MC_ETX) {
      buffer[pos++] = 2;
      buffer[pos++] = dag->mc.obj.etx >> 8;
      buffer[pos++] = dag->mc.obj.etx & 0xff;
    } else if(dag->mc.type == RPL_DAG_MC_ENERGY) {
      buffer[pos++] = 2;
      buffer[pos++] = dag->mc.obj.energy.flags;
      buffer[pos++] = dag->mc.obj.energy.energy_est;
    } else {
      PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
	(unsigned)dag->mc.type);
      return;
    }
  }

  /* always add a sub-option for DAG configuration */
  buffer[pos++] = RPL_DIO_SUBOPT_DAG_CONF;
  buffer[pos++] = 14;
  buffer[pos++] = 0; /* No Auth, PCS = 0 */
  buffer[pos++] = dag->dio_intdoubl;
  buffer[pos++] = dag->dio_intmin;
  buffer[pos++] = dag->dio_redundancy;
  buffer[pos++] = dag->max_rankinc >> 8;
  buffer[pos++] = dag->max_rankinc & 0xff;
  buffer[pos++] = dag->min_hoprankinc >> 8;
  buffer[pos++] = dag->min_hoprankinc & 0xff;
  /* OCP is in the DAG_CONF option */
  buffer[pos++] = dag->of->ocp >> 8;
  buffer[pos++] = dag->of->ocp & 0xff;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = dag->default_lifetime;
  buffer[pos++] = dag->lifetime_unit >> 8;
  buffer[pos++] = dag->lifetime_unit & 0xff;

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
    memcpy(&buffer[pos], &dag->prefix_info.prefix, 16);
    pos += 16;
    PRINTF("RPL: Sending prefix info in DIO for ");
    PRINT6ADDR(&dag->prefix_info.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce (len %d)\n",
           dag->prefix_info.length);
  }

  /* buffer[len++] = RPL_DIO_SUBOPT_PAD1; */

  /* Unicast requests get unicast replies! */
  if(uc_addr == NULL) {
    PRINTF("RPL: Sending a multicast-DIO with rank %u\n",
        (unsigned)dag->rank);
    uip_create_linklocal_rplnodes_mcast(&addr);
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
  uint8_t pathcontrol;
  uint8_t pathsequence;
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  uint8_t buffer_length;
  int pos;
  int len;
  int i;
  int learned_from;
  rpl_parent_t *p;

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

  dag = rpl_get_dag(instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Ignoring a DAO for a different RPL instance (%u)\n",
           instance_id);
    return;
  }

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  /* Is the DAGID present? */
  if(flags & RPL_DAO_D_FLAG) {
    /* currently the DAG ID is ignored since we only use global
       RPL Instance IDs... */
    pos += 16;
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
      /* handle the target option */
      prefixlen = buffer[i + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_DIO_SUBOPT_TRANSIT:
      /* path sequence and control ignored */
      pathcontrol = buffer[i + 3];
      pathsequence = buffer[i + 4];
      lifetime = buffer[i + 5];
      /* parent address also ignored */
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %lu, prefix length: %u prefix: ",
         (unsigned long)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF("\n");

  if(lifetime == ZERO_LIFETIME) {
    /* No-Path DAO received; invoke the route purging routine. */
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

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    /* Check if this is a DAO forwarding loop. */
    p = rpl_find_parent(dag, &dao_sender_addr);
    /* check if this is a new DAO registration with an "illegal" rank */
    /* if we already route to this node it is likely */
    if(p != NULL && DAG_RANK(p->rank, dag) < DAG_RANK(dag->rank, dag) 
      /* && uip_ds6_route_lookup(&prefix) == NULL*/) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from a node with a lower rank! (%u < %u)\n",
          DAG_RANK(p->rank, dag), DAG_RANK(dag->rank, dag));
      p->rank = INFINITE_RANK;
      p->updated = 1;
      return;
    }
  }

  rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
  if(rep == NULL) {
    RPL_STAT(rpl_stats.mem_overflows++);
    PRINTF("RPL: Could not add a route after receiving a DAO\n");
    return;
  } else {
    rep->state.lifetime = lifetime * dag->lifetime_unit;
    rep->state.learned_from = learned_from;
  }

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    if(dag->preferred_parent) {
      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(&dag->preferred_parent->addr);
      PRINTF("\n");
      uip_icmp6_send(&dag->preferred_parent->addr,
                     ICMP6_RPL, RPL_CODE_DAO, buffer_length);
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
#if RPL_CONF_DAO_ACK
  buffer[pos++] = RPL_DAO_K_FLAG; /* DAO ACK request, no DODAGID */
#else
  buffer[pos++] = 0; /* No DAO ACK request, no DODAGID */
#endif
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

  /* create a transit information subopt (RPL-18)*/
  buffer[pos++] = RPL_DIO_SUBOPT_TRANSIT;
  buffer[pos++] = 4;
  buffer[pos++] = 0; /* flags - ignored */
  buffer[pos++] = 0; /* path control - ignored */
  buffer[pos++] = 0; /* path seq - ignored */
  buffer[pos++] = (lifetime / dag->lifetime_unit) & 0xff;

  if(n == NULL) {
    uip_create_linklocal_rplnodes_mcast(&addr);
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
