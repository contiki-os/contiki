/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    IPv6 Neighbor cache (link-layer/IPv6 address mapping)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Simon Duquennoy <simonduq@sics.se>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "lib/list.h"
#include "net/link-stats.h"
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/ipv6/uip-ds6-nbr.h"

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
#include "lib/memb.h"
#endif /* UIP_DS6_NBR_MULT_IPV6_ADDRS */

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#ifdef UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED
#define NEIGHBOR_STATE_CHANGED(n) UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED(n)
void NEIGHBOR_STATE_CHANGED(uip_ds6_nbr_t *n);
#else
#define NEIGHBOR_STATE_CHANGED(n)
#endif /* UIP_DS6_CONF_NEIGHBOR_STATE_CHANGED */

#ifdef UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx) UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
void LINK_NEIGHBOR_CALLBACK(const linkaddr_t *addr, int status, int numtx);
#else
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
#endif /* UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK */

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
/* A configurable function called after adding a new neighbor as next hop */
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK
void NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK(const linkaddr_t *addr);
#endif /* NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK */

/* A configurable function called after removing a next hop neighbor */
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK
void NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK(const linkaddr_t *addr);
#endif /* NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK */
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
static void nbr_entry_rm(uip_ds6_nbr_entry_t *nbr_entry);
NBR_TABLE(uip_ds6_nbr_entry_t, ds6_nbr_entries);
MEMB(nbr_memb, uip_ds6_nbr_t, UIP_DS6_NBR_MAX_NB);
#else
NBR_TABLE_GLOBAL(uip_ds6_nbr_t, ds6_neighbors);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

/*---------------------------------------------------------------------------*/
void
uip_ds6_neighbors_init(void)
{
  link_stats_init();
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  memb_init(&nbr_memb);
  nbr_table_register(ds6_nbr_entries, (nbr_table_callback *)nbr_entry_rm);
#else
  nbr_table_register(ds6_neighbors, (nbr_table_callback *)uip_ds6_nbr_rm);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr, const uip_lladdr_t *lladdr,
                uint8_t isrouter, uint8_t state, nbr_table_reason_t reason,
                void *data)
{
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  uip_ds6_nbr_entry_t *nbr_entry;
  uip_ds6_nbr_t *nbr;

  nbr = uip_ds6_nbr_lookup(ipaddr);
  if(nbr != NULL) {
    if(linkaddr_cmp((linkaddr_t *)uip_ds6_nbr_get_ll(nbr),
                    (linkaddr_t *)lladdr)) {
      PRINTF("uip-ds6-nbr: nbr already exists\n");
      return NULL;
    } else {
      PRINTF("uip-ds6-nbr: ipaddr has a different lladdr\n");
      return NULL;
    }
  } else {
    nbr = (uip_ds6_nbr_t *)memb_alloc(&nbr_memb);
    if(nbr == NULL) {
      PRINTF("uip-ds6-nbr: cannot allocate a new nbr\n");
      return NULL;
    }
    LIST_STRUCT_INIT(nbr, route_list);
  }

  if((nbr_entry =
      (uip_ds6_nbr_entry_t *)nbr_table_get_from_lladdr(ds6_nbr_entries,
                                                       (linkaddr_t *)lladdr))
     == NULL) {
    nbr_entry = nbr_table_add_lladdr(ds6_nbr_entries, (linkaddr_t*)lladdr,
                                     reason, data);
    if(nbr_entry == NULL) {
      PRINTF("uip-ds6-nbr: cannot allocate a new nbr_entry\n");
      return NULL;
    }
    LIST_STRUCT_INIT(nbr_entry, nbr_list);
  }
  list_push(nbr_entry->nbr_list, nbr);
  nbr->nbr_entry = nbr_entry;
#else
  uip_ds6_nbr_t *nbr = nbr_table_add_lladdr(ds6_neighbors, (linkaddr_t*)lladdr,
                                            reason, data);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
  if(nbr) {
    uip_ipaddr_copy(&nbr->ipaddr, ipaddr);
#if UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
    nbr->isrouter = isrouter;
#endif /* UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */
    nbr->state = state;
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_new(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#if UIP_ND6_SEND_NS
    if(nbr->state == NBR_REACHABLE) {
      stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
    } else {
      /* We set the timer in expired state */
      stimer_set(&nbr->reachable, 0);
    }
    stimer_set(&nbr->sendns, 0);
    nbr->nscount = 0;
#endif /* UIP_ND6_SEND_NS */
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS && UIP_CONF_MAX_ROUTES != 0
    LIST_STRUCT_INIT(nbr, route_list);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS && UIP_CONF_MAX_ROUTES != 0 */
    PRINTF("Adding neighbor with ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr ");
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    NEIGHBOR_STATE_CHANGED(nbr);
    return nbr;
  } else {
    PRINTF("uip_ds6_nbr_add drop ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr (%p) ", lladdr);
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr)
{
  if(nbr != NULL) {
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_free(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
    NEIGHBOR_STATE_CHANGED(nbr);
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK
    if(nbr->route_list != NULL && list_length(nbr->route_list) > 0) {
      NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK(
        (const linkaddr_t *)uip_ds6_nbr_get_ll(nbr));
    }
#endif /* NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK */
    uip_ds6_nbr_rm_all_routes(nbr);
    if(nbr->nbr_entry != NULL) {
      list_remove(nbr->nbr_entry->nbr_list, nbr);
    }
    memb_free(&nbr_memb, nbr);
#else
    return nbr_table_remove(ds6_neighbors, nbr);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Update the link-layer address associated with a specified 'nbr'
 * \retval 0 Failure
 * \retval 1 Success
 */
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
int
uip_ds6_nbr_update_lladdr(uip_ds6_nbr_t **nbr, const uip_lladdr_t *new_ll_addr)
{
  uip_ds6_nbr_entry_t *nbr_entry;

  if(nbr == NULL || *nbr == NULL || new_ll_addr == NULL) {
    return 0;
  }

  nbr_entry = nbr_table_get_from_lladdr(ds6_nbr_entries,
                                        (const linkaddr_t *)new_ll_addr);
  if(nbr_entry == NULL) {
    nbr_entry = nbr_table_add_lladdr(ds6_nbr_entries, (linkaddr_t*)new_ll_addr,
                                     NBR_TABLE_REASON_IPV6_ND, NULL);
    if(nbr_entry == NULL) {
      PRINTF("uip_ds6_nbr_update_lladdr(): ");
      PRINTF("cannot allocate a new nbr_entry\n");
      return 0;
    }
    LIST_STRUCT_INIT(nbr_entry, nbr_list);
  }

  if(nbr_entry == (*nbr)->nbr_entry) {
    /* nbr has the same link-layer address as new_ll_addr */
    return 1;
  }

  if((*nbr)->nbr_entry != NULL) {
    list_remove((*nbr)->nbr_entry->nbr_list, *nbr);
  }
  list_push(nbr_entry->nbr_list, *nbr);
  (*nbr)->nbr_entry = nbr_entry;

#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK
  if(linkaddr_cmp(new_ll_addr, &linkaddr_null) == 0) {
    NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK(new_ll_addr);
  }
#endif /* NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK */

  return 1;
}
#else /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
int
uip_ds6_nbr_update_lladdr(uip_ds6_nbr_t **nbr, const uip_lladdr_t *new_ll_addr)
{
  uip_ds6_nbr_t *new_nbr;
  uip_ds6_nbr_t backup_nbr;

  if(nbr == NULL || *nbr == NULL || new_ll_addr == NULL) {
    return 0;
  }

  if(uip_ds6_nbr_ll_lookup(new_ll_addr) != NULL) {
    /*
     * It seems new_ll_addr is associated with another IPv6 address. Currently,
     * we have a single 'nbr' entry per link-layer address; give up the update.
     */
    PRINTF("uip_ds6_nbr_update_lladdr(): ");
    PRINTF("new_ll_addr is associated with another IPv6 address");
    return 0;
  }

  /* make room for a newly allocated nbr first */
  memcpy(&backup_nbr, *nbr, sizeof(uip_ds6_nbr_t));
  if(uip_ds6_nbr_rm(*nbr) == 0) {
    /* Unexpectedly failed to remove 'nbr'. */
    return 0;
  }

  new_nbr = uip_ds6_nbr_add(&backup_nbr.ipaddr, new_ll_addr,
                            backup_nbr.isrouter, backup_nbr.state,
                            NBR_TABLE_REASON_IPV6_ND, NULL);
  if(new_nbr == NULL) {
    /* Failed to allocate a new 'nbr', and *nbr has already removed  */
    PRINTF("uip_ds6_nbr_update_lladdr(): ");
    PRINTF("failed to allocate a new 'nbr'");
    *nbr = NULL;
    return 0;
  }
  memcpy(new_nbr, &backup_nbr, sizeof(uip_ds6_nbr_t));
  *nbr = new_nbr; /* make '*nbr' point to 'new_nbr' */

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  if(list_length((*nbr)->route_list) > 0) {
    nbr_table_lock(ds6_neighbors, *nbr);
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK
    if(linkaddr_cmp((const linkaddr_t *)uip_ds6_nbr_get_ll(*nbr),
                    (const linkaddr_t *)&linkaddr_null) == 0) {
      NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK(
        (const linkaddr_t *)uip_ds6_nbr_get_ll(*nbr));
    }
#endif /* NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK */
  }
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  return 1;
}
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
/*---------------------------------------------------------------------------*/
const uip_ipaddr_t *
uip_ds6_nbr_get_ipaddr(const uip_ds6_nbr_t *nbr)
{
  return (nbr != NULL) ? &nbr->ipaddr : NULL;
}
/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_get_ll(const uip_ds6_nbr_t *nbr)
{
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  if(nbr == NULL) {
    return NULL;
  }
  return (const uip_lladdr_t *)nbr_table_get_lladdr(ds6_nbr_entries,
                                                    nbr->nbr_entry);
#else
  return (const uip_lladdr_t *)nbr_table_get_lladdr(ds6_neighbors, nbr);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_num(void)
{
  int num = 0;

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  uip_ds6_nbr_entry_t *nbr_entry;

  for(nbr_entry = nbr_table_head(ds6_nbr_entries);
      nbr_entry != NULL;
      nbr_entry = nbr_table_next(ds6_nbr_entries, nbr_entry)) {
    num += list_length(nbr_entry->nbr_list);
  }
#else
  uip_ds6_nbr_t *nbr;

  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
    num++;
  }
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
  return num;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_lookup(const uip_ipaddr_t *ipaddr)
{
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  uip_ds6_nbr_t *nbr;
  uip_ds6_nbr_entry_t *nbr_entry;

  if(ipaddr == NULL) {
    return NULL;
  }

  for(nbr_entry = nbr_table_head(ds6_nbr_entries);
      nbr_entry != NULL;
      nbr_entry = nbr_table_next(ds6_nbr_entries, nbr_entry)) {
    for(nbr = list_head(nbr_entry->nbr_list);
        nbr != NULL;
        nbr = list_item_next(nbr)) {
      if(uip_ipaddr_cmp(&nbr->ipaddr, ipaddr)) {
        return nbr;
      }
    }
  }
#else
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  if(ipaddr != NULL) {
    while(nbr != NULL) {
      if(uip_ipaddr_cmp(&nbr->ipaddr, ipaddr)) {
        return nbr;
      }
      nbr = nbr_table_next(ds6_neighbors, nbr);
    }
  }
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  return NULL;
}
/*---------------------------------------------------------------------------*/
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS == 0
uip_ds6_nbr_t *
uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr)
{
  return nbr_table_get_from_lladdr(ds6_neighbors, (linkaddr_t*)lladdr);
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_nbr_ipaddr_from_lladdr(const uip_lladdr_t *lladdr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_ll_lookup(lladdr);
  return nbr ? &nbr->ipaddr : NULL;
}
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS == 0 */
/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_lladdr_from_ipaddr(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_lookup(ipaddr);
  return nbr ? uip_ds6_nbr_get_ll(nbr) : NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_link_neighbor_callback(int status, int numtx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(linkaddr_cmp(dest, &linkaddr_null)) {
    return;
  }

  /* Update neighbor link statistics */
  link_stats_packet_sent(dest, status, numtx);
  /* Call upper-layer callback (e.g. RPL) */
  LINK_NEIGHBOR_CALLBACK(dest, status, numtx);

#if UIP_DS6_LL_NUD && (UIP_DS6_NBR_MULTI_IPV6_ADDRS == 0)
  /* From RFC4861, page 72, last paragraph of section 7.3.3:
   *
   *         "In some cases, link-specific information may indicate that a path to
   *         a neighbor has failed (e.g., the resetting of a virtual circuit). In
   *         such cases, link-specific information may be used to purge Neighbor
   *         Cache entries before the Neighbor Unreachability Detection would do
   *         so. However, link-specific information MUST NOT be used to confirm
   *         the reachability of a neighbor; such information does not provide
   *         end-to-end confirmation between neighboring IP layers."
   *
   * However, we assume that receiving a link layer ack ensures the delivery
   * of the transmitted packed to the IP stack of the neighbour. This is a
   * fair assumption and allows battery powered nodes save some battery by
   * not re-testing the state of a neighbour periodically if it
   * acknowledges link packets.
   *
   * When UIP_DS6_NBR_MULTI_IPV6_ADDRS is enabled, there could be more than one
   * neighbor cache which are associated with the destination link-layer
   * address. Because of that, UIP_DS6_LL_NUD cannot be used with
   * UIP_DS6_NBR_MULTI_IPV6_ADDRS enabled. Technically, connectivity to a
   * particular link-layer address does not always guarantee reachability to the
   * corresponding IPv6 address.
   */
  if(status == MAC_TX_OK) {
    uip_ds6_nbr_t *nbr;
    nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)dest);
    if(nbr != NULL && nbr->state != NBR_INCOMPLETE) {
      nbr->state = NBR_REACHABLE;
      stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
      PRINTF("uip-ds6-neighbor : received a link layer ACK : ");
      PRINTLLADDR((uip_lladdr_t *)dest);
      PRINTF(" is reachable.\n");
    }
  }
#endif /* UIP_DS6_LL_NUD && (UIP_DS6_NBR_MULTI_IPV6_ADDRS == 0) */

}
#if UIP_ND6_SEND_NS
/*---------------------------------------------------------------------------*/
/** Periodic processing on neighbors */
void
uip_ds6_neighbor_periodic(void)
{
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_list_head();
#else
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
#endif
  while(nbr != NULL) {
    switch(nbr->state) {
      case NBR_REACHABLE:
        if(stimer_expired(&nbr->reachable)) {
#if UIP_CONF_IPV6_RPL
          /* when a neighbor leave its REACHABLE state and is a default router,
             instead of going to STALE state it enters DELAY state in order to
             force a NUD on it. Otherwise, if there is no upward traffic, the
             node never knows if the default router is still reachable. This
             mimics the 6LoWPAN-ND behavior.
          */
          if(uip_ds6_defrt_lookup(&nbr->ipaddr) != NULL) {
            PRINTF("REACHABLE: defrt moving to DELAY (");
            PRINT6ADDR(&nbr->ipaddr);
            PRINTF(")\n");
            nbr->state = NBR_DELAY;
            stimer_set(&nbr->reachable, UIP_ND6_DELAY_FIRST_PROBE_TIME);
            nbr->nscount = 0;
          } else {
            PRINTF("REACHABLE: moving to STALE (");
            PRINT6ADDR(&nbr->ipaddr);
            PRINTF(")\n");
            nbr->state = NBR_STALE;
          }
#else /* UIP_CONF_IPV6_RPL */
          PRINTF("REACHABLE: moving to STALE (");
          PRINT6ADDR(&nbr->ipaddr);
          PRINTF(")\n");
          nbr->state = NBR_STALE;
#endif /* UIP_CONF_IPV6_RPL */
        }
        break;
      case NBR_INCOMPLETE:
        if(nbr->nscount >= UIP_ND6_MAX_MULTICAST_SOLICIT) {
          uip_ds6_nbr_rm(nbr);
        } else if(stimer_expired(&nbr->sendns) && (uip_len == 0)) {
          nbr->nscount++;
          PRINTF("NBR_INCOMPLETE: NS %u\n", nbr->nscount);
          uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
          stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
        }
        break;
      case NBR_DELAY:
        if(stimer_expired(&nbr->reachable)) {
          nbr->state = NBR_PROBE;
          nbr->nscount = 0;
          PRINTF("DELAY: moving to PROBE (");
          PRINT6ADDR(&nbr->ipaddr);
          PRINTF(")\n");
          stimer_set(&nbr->sendns, 0);
        }
        break;
      case NBR_PROBE:
        if(nbr->nscount >= UIP_ND6_MAX_UNICAST_SOLICIT) {
          uip_ds6_defrt_t *locdefrt;
          PRINTF("PROBE END\n");
          if((locdefrt = uip_ds6_defrt_lookup(&nbr->ipaddr)) != NULL) {
            if (!locdefrt->isinfinite) {
              uip_ds6_defrt_rm(locdefrt);
            }
          }
          uip_ds6_nbr_rm(nbr);
        } else if(stimer_expired(&nbr->sendns) && (uip_len == 0)) {
          nbr->nscount++;
          PRINTF("PROBE: NS %u (", nbr->nscount);
          PRINT6ADDR(&nbr->ipaddr);
          PRINTF(")\n");
          uip_nd6_ns_output(NULL, &nbr->ipaddr, &nbr->ipaddr);
          stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
        }
        break;
      default:
        break;
    }
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
    nbr = uip_ds6_nbr_list_item_next(nbr);
#else
    nbr = nbr_table_next(ds6_neighbors, nbr);
#endif
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_nbr_refresh_reachable_state(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr;
  nbr = uip_ds6_nbr_lookup(ipaddr);
  if(nbr != NULL && nbr->state != NBR_INCOMPLETE) {
    nbr->state = NBR_REACHABLE;
    nbr->nscount = 0;
    stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
  }
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_get_least_lifetime_neighbor(void)
{
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_list_head();
#else
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
#endif
  uip_ds6_nbr_t *nbr_expiring = NULL;
  while(nbr != NULL) {
    if(nbr_expiring != NULL) {
      clock_time_t curr = stimer_remaining(&nbr->reachable);
      if(curr < stimer_remaining(&nbr->reachable)) {
        nbr_expiring = nbr;
      }
    } else {
      nbr_expiring = nbr;
    }
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
    nbr = uip_ds6_nbr_list_next(nbr);
#else
    nbr = nbr_table_next(ds6_neighbors, nbr);
#endif
  }
  return nbr_expiring;
}
#endif /* UIP_ND6_SEND_NS */
/*---------------------------------------------------------------------------*/
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
#if UIP_CONF_MAX_ROUTES != 0
void
uip_ds6_nbr_add_route(uip_ds6_nbr_t *nbr, uip_ds6_route_t *route)
{
  if(nbr == NULL || route == NULL) {
    return;
  }
  list_push(nbr->route_list, route);
  if(list_length(nbr->route_list) == 1) {
    /*
     * This is the first time to add a route to this nbr. We're going to lock
     * this nbr so that the nbr is not removed as long as it has some route.
     */
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
    if(nbr->nbr_entry != NULL) {
      nbr_table_lock(ds6_nbr_entries, nbr->nbr_entry);
    }
#else
    nbr_table_lock(ds6_neighbors, nbr);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK
    if(linkaddr_cmp((const linkaddr_t *)uip_ds6_nbr_get_ll(nbr),
                    (const linkaddr_t *)&linkaddr_null) == 0) {
      NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK(
        (const linkaddr_t *)uip_ds6_nbr_get_ll(nbr));
    }
#endif
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_nbr_rm_route(uip_ds6_nbr_t *nbr, uip_ds6_route_t *route)
{
  if(nbr == NULL || route == NULL) {
    return;
  }
  list_remove(nbr->route_list, route);
  if(list_length(nbr->route_list) == 0) {
    /* no route is associated with the nbr; unlock it */
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
    if(nbr->nbr_entry != NULL) {
      nbr_table_unlock(ds6_nbr_entries, nbr->nbr_entry);
    }
#else
    nbr_table_unlock(ds6_neighbors, nbr);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
#ifdef NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK
    if(linkaddr_cmp((const linkaddr_t *)uip_ds6_nbr_get_ll(nbr),
                    (const linkaddr_t *)&linkaddr_null) == 0) {
      NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK(
        (const linkaddr_t *)uip_ds6_nbr_get_ll(nbr));
    }
#endif
  }
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_has_routes(uip_ds6_nbr_t *nbr)
{
  if(nbr == NULL) {
    return 0;
  }

  return list_length(nbr->route_list) > 0;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_nbr_rm_all_routes(uip_ds6_nbr_t *nbr)
{
  uip_ds6_route_t *route;
  uip_ds6_route_t *next_route;
  if(nbr == NULL || nbr->route_list == NULL) {
    return;
  }
  for(route = list_head(nbr->route_list);
      route != NULL; route = next_route) {
    next_route = list_item_next(route);
    uip_ds6_route_rm(route);
  }
}
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
/*---------------------------------------------------------------------------*/
static void
nbr_entry_rm(uip_ds6_nbr_entry_t *nbr_entry)
{
  uip_ds6_nbr_t *nbr;
  uip_ds6_nbr_t *next_nbr;
  if(nbr_entry == NULL) {
    return;
  }
  for(nbr = list_head(nbr_entry->nbr_list);
      nbr != NULL; nbr = next_nbr) {
    next_nbr = list_item_next(nbr);
    uip_ds6_nbr_rm(nbr);
  }
  memb_free(&nbr_memb, nbr_entry);
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_list_head(void)
{
  uip_ds6_nbr_entry_t *nbr_entry;
  uip_ds6_nbr_t *nbr;

  for(nbr_entry = nbr_table_head(ds6_nbr_entries);
      nbr_entry != NULL;
      nbr_entry = nbr_table_next(ds6_nbr_entries, nbr_entry)) {
    nbr = list_head(nbr_entry->nbr_list);
    if(nbr != NULL) {
      return nbr;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_list_item_next(uip_ds6_nbr_t *nbr)
{
  uip_ds6_nbr_entry_t *nbr_entry;
  uip_ds6_nbr_t *next_nbr;

  if(nbr == NULL || nbr->nbr_entry == NULL) {
    return NULL;
  }

  next_nbr = list_item_next(nbr);
  if(next_nbr == NULL) {
    for(nbr_entry = nbr_table_next(ds6_nbr_entries, nbr->nbr_entry);
        nbr_entry != NULL && next_nbr == NULL;
        nbr_entry = nbr_table_next(ds6_nbr_entries, nbr_entry)) {
      next_nbr = list_head(nbr_entry->nbr_list);
    }
  }

  return next_nbr;
}
/*---------------------------------------------------------------------------*/
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
/** @} */
