/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         IPv6 data structures handling functions.
 *         Comprises part of the Neighbor discovery (RFC 4861)
 *         and auto configuration (RFC 4862) state machines.
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 */
/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
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
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "lib/random.h"
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "net/uip-packetqueue.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#ifdef UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED
#define NEIGHBOR_STATE_CHANGED(n) UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED(n)
void NEIGHBOR_STATE_CHANGED(uip_ds6_nbr_t *n);
#else
#define NEIGHBOR_STATE_CHANGED(n)
#endif /* UIP_DS6_CONF_NEIGHBOR_STATE_CHANGED */

struct etimer uip_ds6_timer_periodic;                           /** \brief Timer for maintenance of data structures */

#if UIP_CONF_ROUTER
struct stimer uip_ds6_timer_ra;                                 /** \brief RA timer, to schedule RA sending */
#if UIP_ND6_SEND_RA
static uint8_t racount;                                         /** \brief number of RA already sent */
static uint16_t rand_time;                                      /** \brief random time value for timers */
#endif
#else /* UIP_CONF_ROUTER */
struct etimer uip_ds6_timer_rs;                                 /** \brief RS timer, to schedule RS sending */
static uint8_t rscount;                                         /** \brief number of rs already sent */
#endif /* UIP_CONF_ROUTER */

/** \name "DS6" Data structures */
/** @{ */
uip_ds6_netif_t uip_ds6_if;                                       /** \brief The single interface */
uip_ds6_nbr_t uip_ds6_nbr_cache[UIP_DS6_NBR_NB];                  /** \brief Neighor cache */
uip_ds6_defrt_t uip_ds6_defrt_list[UIP_DS6_DEFRT_NB];             /** \brief Default rt list */
uip_ds6_prefix_t uip_ds6_prefix_list[UIP_DS6_PREFIX_NB];          /** \brief Prefix list */
uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];          /** \brief Routing table */

/* Used by Cooja to enable extraction of addresses from memory.*/
uint8_t uip_ds6_addr_size;
uint8_t uip_ds6_netif_addr_list_offset;

/** @} */

/* "full" (as opposed to pointer) ip address used in this file,  */
static uip_ipaddr_t loc_fipaddr;

/* Pointers used in this file */
static uip_ipaddr_t *locipaddr;
static uip_ds6_addr_t *locaddr;
static uip_ds6_maddr_t *locmaddr;
static uip_ds6_aaddr_t *locaaddr;
static uip_ds6_prefix_t *locprefix;
static uip_ds6_nbr_t *locnbr;
static uip_ds6_defrt_t *locdefrt;
static uip_ds6_route_t *locroute;

/*---------------------------------------------------------------------------*/
void
uip_ds6_init(void)
{
  PRINTF("Init of IPv6 data structures\n");
  PRINTF("%u neighbors\n%u default routers\n%u prefixes\n%u routes\n%u unicast addresses\n%u multicast addresses\n%u anycast addresses\n",
     UIP_DS6_NBR_NB, UIP_DS6_DEFRT_NB, UIP_DS6_PREFIX_NB, UIP_DS6_ROUTE_NB,
     UIP_DS6_ADDR_NB, UIP_DS6_MADDR_NB, UIP_DS6_AADDR_NB);
  memset(uip_ds6_nbr_cache, 0, sizeof(uip_ds6_nbr_cache));
  memset(uip_ds6_defrt_list, 0, sizeof(uip_ds6_defrt_list));
  memset(uip_ds6_prefix_list, 0, sizeof(uip_ds6_prefix_list));
  memset(&uip_ds6_if, 0, sizeof(uip_ds6_if));
  memset(uip_ds6_routing_table, 0, sizeof(uip_ds6_routing_table));
  uip_ds6_addr_size = sizeof(struct uip_ds6_addr);
  uip_ds6_netif_addr_list_offset = offsetof(struct uip_ds6_netif, addr_list);

  /* Set interface parameters */
  uip_ds6_if.link_mtu = UIP_LINK_MTU;
  uip_ds6_if.cur_hop_limit = UIP_TTL;
  uip_ds6_if.base_reachable_time = UIP_ND6_REACHABLE_TIME;
  uip_ds6_if.reachable_time = uip_ds6_compute_reachable_time();
  uip_ds6_if.retrans_timer = UIP_ND6_RETRANS_TIMER;
  uip_ds6_if.maxdadns = UIP_ND6_DEF_MAXDADNS;

  /* Create link local address, prefix, multicast addresses, anycast addresses */
  uip_create_linklocal_prefix(&loc_fipaddr);
#if UIP_CONF_ROUTER
  uip_ds6_prefix_add(&loc_fipaddr, UIP_DEFAULT_PREFIX_LEN, 0, 0, 0, 0);
#else /* UIP_CONF_ROUTER */
  uip_ds6_prefix_add(&loc_fipaddr, UIP_DEFAULT_PREFIX_LEN, 0);
#endif /* UIP_CONF_ROUTER */
  uip_ds6_set_addr_iid(&loc_fipaddr, &uip_lladdr);
  uip_ds6_addr_add(&loc_fipaddr, 0, ADDR_AUTOCONF);

  uip_create_linklocal_allnodes_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);
#if UIP_CONF_ROUTER
  uip_create_linklocal_allrouters_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);
#if UIP_ND6_SEND_RA
  stimer_set(&uip_ds6_timer_ra, 2);     /* wait to have a link local IP address */
#endif /* UIP_ND6_SEND_RA */
#else /* UIP_CONF_ROUTER */
  etimer_set(&uip_ds6_timer_rs,
             random_rand() % (UIP_ND6_MAX_RTR_SOLICITATION_DELAY *
                              CLOCK_SECOND));
#endif /* UIP_CONF_ROUTER */
  etimer_set(&uip_ds6_timer_periodic, UIP_DS6_PERIOD);

  return;
}


/*---------------------------------------------------------------------------*/
void
uip_ds6_periodic(void)
{

  /* Periodic processing on unicast addresses */
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused) {
      if((!locaddr->isinfinite) && (stimer_expired(&locaddr->vlifetime))) {
        uip_ds6_addr_rm(locaddr);
#if UIP_ND6_DEF_MAXDADNS > 0
      } else if((locaddr->state == ADDR_TENTATIVE)
                && (locaddr->dadnscount <= uip_ds6_if.maxdadns)
                && (timer_expired(&locaddr->dadtimer))
                && (uip_len == 0)) {
        uip_ds6_dad(locaddr);
#endif /* UIP_ND6_DEF_MAXDADNS > 0 */
      }
    }
  }

  /* Periodic processing on default routers */
  for(locdefrt = uip_ds6_defrt_list;
      locdefrt < uip_ds6_defrt_list + UIP_DS6_DEFRT_NB; locdefrt++) {
    if((locdefrt->isused) && (!locdefrt->isinfinite) &&
       (stimer_expired(&(locdefrt->lifetime)))) {
      uip_ds6_defrt_rm(locdefrt);
    }
  }

#if !UIP_CONF_ROUTER
  /* Periodic processing on prefixes */
  for(locprefix = uip_ds6_prefix_list;
      locprefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB;
      locprefix++) {
    if(locprefix->isused && !locprefix->isinfinite
       && stimer_expired(&(locprefix->vlifetime))) {
      uip_ds6_prefix_rm(locprefix);
    }
  }
#endif /* !UIP_CONF_ROUTER */

  /* Periodic processing on neighbors */
  for(locnbr = uip_ds6_nbr_cache;
      locnbr < uip_ds6_nbr_cache + UIP_DS6_NBR_NB;
      locnbr++) {
    if(locnbr->isused) {
      switch(locnbr->state) {
      case NBR_INCOMPLETE:
        if(locnbr->nscount >= UIP_ND6_MAX_MULTICAST_SOLICIT) {
          uip_ds6_nbr_rm(locnbr);
        } else if(stimer_expired(&locnbr->sendns) && (uip_len == 0)) {
          locnbr->nscount++;
          PRINTF("NBR_INCOMPLETE: NS %u\n", locnbr->nscount);
          uip_nd6_ns_output(NULL, NULL, &locnbr->ipaddr);
          stimer_set(&locnbr->sendns, uip_ds6_if.retrans_timer / 1000);
        }
        break;
      case NBR_REACHABLE:
        if(stimer_expired(&locnbr->reachable)) {
          PRINTF("REACHABLE: moving to STALE (");
          PRINT6ADDR(&locnbr->ipaddr);
          PRINTF(")\n");
          locnbr->state = NBR_STALE;
        }
        break;
      case NBR_DELAY:
        if(stimer_expired(&locnbr->reachable)) {
          locnbr->state = NBR_PROBE;
          locnbr->nscount = 0;
          PRINTF("DELAY: moving to PROBE\n");
          stimer_set(&locnbr->sendns, 0);
        }
        break;
      case NBR_PROBE:
        if(locnbr->nscount >= UIP_ND6_MAX_UNICAST_SOLICIT) {
          PRINTF("PROBE END\n");
          if((locdefrt = uip_ds6_defrt_lookup(&locnbr->ipaddr)) != NULL) {
            if (!locdefrt->isinfinite) {
              uip_ds6_defrt_rm(locdefrt);
            }
          }
          uip_ds6_nbr_rm(locnbr);
        } else if(stimer_expired(&locnbr->sendns) && (uip_len == 0)) {
          locnbr->nscount++;
          PRINTF("PROBE: NS %u\n", locnbr->nscount);
          uip_nd6_ns_output(NULL, &locnbr->ipaddr, &locnbr->ipaddr);
          stimer_set(&locnbr->sendns, uip_ds6_if.retrans_timer / 1000);
        }
        break;
      default:
        break;
      }
    }
  }

#if UIP_CONF_ROUTER & UIP_ND6_SEND_RA
  /* Periodic RA sending */
  if(stimer_expired(&uip_ds6_timer_ra) && (uip_len == 0)) {
    uip_ds6_send_ra_periodic();
  }
#endif /* UIP_CONF_ROUTER & UIP_ND6_SEND_RA */
  etimer_reset(&uip_ds6_timer_periodic);
  return;
}

/*---------------------------------------------------------------------------*/
uint8_t
uip_ds6_list_loop(uip_ds6_element_t *list, uint8_t size,
                  uint16_t elementsize, uip_ipaddr_t *ipaddr,
                  uint8_t ipaddrlen, uip_ds6_element_t **out_element)
{
  uip_ds6_element_t *element;

  *out_element = NULL;

  for(element = list;
      element <
      (uip_ds6_element_t *)((uint8_t *)list + (size * elementsize));
      element = (uip_ds6_element_t *)((uint8_t *)element + elementsize)) {
    if(element->isused) {
      if(uip_ipaddr_prefixcmp(&element->ipaddr, ipaddr, ipaddrlen)) {
        *out_element = element;
        return FOUND;
      }
    } else {
      *out_element = element;
    }
  }

  return *out_element != NULL ? FREESPACE : NOSPACE;
}

/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_add(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr,
                uint8_t isrouter, uint8_t state)
{
  int r;

  r = uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_nbr_cache, UIP_DS6_NBR_NB,
      sizeof(uip_ds6_nbr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locnbr);

  if(r == FREESPACE) {
    locnbr->isused = 1;
    uip_ipaddr_copy(&locnbr->ipaddr, ipaddr);
    if(lladdr != NULL) {
      memcpy(&locnbr->lladdr, lladdr, UIP_LLADDR_LEN);
    } else {
      memset(&locnbr->lladdr, 0, UIP_LLADDR_LEN);
    }
    locnbr->isrouter = isrouter;
    locnbr->state = state;
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_new(&locnbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
    /* timers are set separately, for now we put them in expired state */
    stimer_set(&locnbr->reachable, 0);
    stimer_set(&locnbr->sendns, 0);
    locnbr->nscount = 0;
    PRINTF("Adding neighbor with ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF("link addr ");
    PRINTLLADDR((&(locnbr->lladdr)));
    PRINTF("state %u\n", state);
    NEIGHBOR_STATE_CHANGED(locnbr);

    locnbr->last_lookup = clock_time();
    return locnbr;
  } else if(r == NOSPACE) {
    /* We did not find any empty slot on the neighbor list, so we need
       to remove one old entry to make room. */
    uip_ds6_nbr_t *n, *oldest;
    clock_time_t oldest_time;

    oldest = NULL;
    oldest_time = clock_time();

    for(n = uip_ds6_nbr_cache;
        n < &uip_ds6_nbr_cache[UIP_DS6_NBR_NB];
        n++) {
      if(n->isused) {
        if(n->last_lookup < oldest_time) {
          oldest = n;
          oldest_time = n->last_lookup;
        }
      }
    }
    if(oldest != NULL) {
      uip_ds6_nbr_rm(oldest);
      return uip_ds6_nbr_add(ipaddr, lladdr, isrouter, state);
    }
  }
  PRINTF("uip_ds6_nbr_add drop\n");
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr)
{
  if(nbr != NULL) {
    nbr->isused = 0;
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_free(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
    NEIGHBOR_STATE_CHANGED(nbr);
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_nbr_cache, UIP_DS6_NBR_NB,
      sizeof(uip_ds6_nbr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locnbr) == FOUND) {
    locnbr->last_lookup = clock_time();
    return locnbr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_ll_lookup(uip_lladdr_t *lladdr)
{
  uip_ds6_nbr_t *fin;

  for(locnbr = uip_ds6_nbr_cache, fin = locnbr + UIP_DS6_NBR_NB;
       locnbr < fin;
       ++locnbr) {
    if(locnbr->isused) {
      if(!memcmp(lladdr, &locnbr->lladdr, UIP_LLADDR_LEN)) {
        return locnbr;
      }
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_add(uip_ipaddr_t *ipaddr, unsigned long interval)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_defrt_list, UIP_DS6_DEFRT_NB,
      sizeof(uip_ds6_defrt_t), ipaddr, 128,
      (uip_ds6_element_t **)&locdefrt) == FREESPACE) {
    locdefrt->isused = 1;
    uip_ipaddr_copy(&locdefrt->ipaddr, ipaddr);
    if(interval != 0) {
      stimer_set(&locdefrt->lifetime, interval);
      locdefrt->isinfinite = 0;
    } else {
      locdefrt->isinfinite = 1;
    }

    PRINTF("Adding defrouter with ip addr ");
    PRINT6ADDR(&locdefrt->ipaddr);
    PRINTF("\n");

    ANNOTATE("#L %u 1\n", ipaddr->u8[sizeof(uip_ipaddr_t) - 1]);

    return locdefrt;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt)
{
  if(defrt != NULL) {
    defrt->isused = 0;
    ANNOTATE("#L %u 0\n", defrt->ipaddr.u8[sizeof(uip_ipaddr_t) - 1]);
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop((uip_ds6_element_t *)uip_ds6_defrt_list,
		       UIP_DS6_DEFRT_NB, sizeof(uip_ds6_defrt_t), ipaddr, 128,
		       (uip_ds6_element_t **)&locdefrt) == FOUND) {
    return locdefrt;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_defrt_choose(void)
{
  uip_ds6_nbr_t *bestnbr;

  locipaddr = NULL;
  for(locdefrt = uip_ds6_defrt_list;
      locdefrt < uip_ds6_defrt_list + UIP_DS6_DEFRT_NB; locdefrt++) {
    if(locdefrt->isused) {
      PRINTF("Defrt, IP address ");
      PRINT6ADDR(&locdefrt->ipaddr);
      PRINTF("\n");
      bestnbr = uip_ds6_nbr_lookup(&locdefrt->ipaddr);
      if(bestnbr != NULL && bestnbr->state != NBR_INCOMPLETE) {
        PRINTF("Defrt found, IP address ");
        PRINT6ADDR(&locdefrt->ipaddr);
        PRINTF("\n");
        return &locdefrt->ipaddr;
      } else {
        locipaddr = &locdefrt->ipaddr;
        PRINTF("Defrt INCOMPLETE found, IP address ");
        PRINT6ADDR(&locdefrt->ipaddr);
        PRINTF("\n");
      }
    }
  }
  return locipaddr;
}

#if UIP_CONF_ROUTER
/*---------------------------------------------------------------------------*/
uip_ds6_prefix_t *
uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen,
                   uint8_t advertise, uint8_t flags, unsigned long vtime,
                   unsigned long ptime)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_prefix_list, UIP_DS6_PREFIX_NB,
      sizeof(uip_ds6_prefix_t), ipaddr, ipaddrlen,
      (uip_ds6_element_t **)&locprefix) == FREESPACE) {
    locprefix->isused = 1;
    uip_ipaddr_copy(&locprefix->ipaddr, ipaddr);
    locprefix->length = ipaddrlen;
    locprefix->advertise = advertise;
    locprefix->l_a_reserved = flags;
    locprefix->vlifetime = vtime;
    locprefix->plifetime = ptime;
    PRINTF("Adding prefix ");
    PRINT6ADDR(&locprefix->ipaddr);
    PRINTF("length %u, flags %x, Valid lifetime %lx, Preffered lifetime %lx\n",
       ipaddrlen, flags, vtime, ptime);
    return locprefix;
  } else {
    PRINTF("No more space in Prefix list\n");
  }
  return NULL;
}


#else /* UIP_CONF_ROUTER */
uip_ds6_prefix_t *
uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen,
                   unsigned long interval)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_prefix_list, UIP_DS6_PREFIX_NB,
      sizeof(uip_ds6_prefix_t), ipaddr, ipaddrlen,
      (uip_ds6_element_t **)&locprefix) == FREESPACE) {
    locprefix->isused = 1;
    uip_ipaddr_copy(&locprefix->ipaddr, ipaddr);
    locprefix->length = ipaddrlen;
    if(interval != 0) {
      stimer_set(&(locprefix->vlifetime), interval);
      locprefix->isinfinite = 0;
    } else {
      locprefix->isinfinite = 1;
    }
    PRINTF("Adding prefix ");
    PRINT6ADDR(&locprefix->ipaddr);
    PRINTF("length %u, vlifetime%lu\n", ipaddrlen, interval);
  }
  return NULL;
}
#endif /* UIP_CONF_ROUTER */

/*---------------------------------------------------------------------------*/
void
uip_ds6_prefix_rm(uip_ds6_prefix_t *prefix)
{
  if(prefix != NULL) {
    prefix->isused = 0;
  }
  return;
}
/*---------------------------------------------------------------------------*/
uip_ds6_prefix_t *
uip_ds6_prefix_lookup(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen)
{
  if(uip_ds6_list_loop((uip_ds6_element_t *)uip_ds6_prefix_list,
		       UIP_DS6_PREFIX_NB, sizeof(uip_ds6_prefix_t),
		       ipaddr, ipaddrlen,
		       (uip_ds6_element_t **)&locprefix) == FOUND) {
    return locprefix;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uint8_t
uip_ds6_is_addr_onlink(uip_ipaddr_t *ipaddr)
{
  for(locprefix = uip_ds6_prefix_list;
      locprefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB; locprefix++) {
    if(locprefix->isused &&
       uip_ipaddr_prefixcmp(&locprefix->ipaddr, ipaddr, locprefix->length)) {
      return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
uip_ds6_addr_t *
uip_ds6_addr_add(uip_ipaddr_t *ipaddr, unsigned long vlifetime, uint8_t type)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.addr_list, UIP_DS6_ADDR_NB,
      sizeof(uip_ds6_addr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaddr) == FREESPACE) {
    locaddr->isused = 1;
    uip_ipaddr_copy(&locaddr->ipaddr, ipaddr);
    locaddr->type = type;
    if(vlifetime == 0) {
      locaddr->isinfinite = 1;
    } else {
      locaddr->isinfinite = 0;
      stimer_set(&(locaddr->vlifetime), vlifetime);
    }
#if UIP_ND6_DEF_MAXDADNS > 0
    locaddr->state = ADDR_TENTATIVE;
    timer_set(&locaddr->dadtimer,
              random_rand() % (UIP_ND6_MAX_RTR_SOLICITATION_DELAY *
                               CLOCK_SECOND));
    locaddr->dadnscount = 0;
#else /* UIP_ND6_DEF_MAXDADNS > 0 */
    locaddr->state = ADDR_PREFERRED;
#endif /* UIP_ND6_DEF_MAXDADNS > 0 */
    uip_create_solicited_node(ipaddr, &loc_fipaddr);
    uip_ds6_maddr_add(&loc_fipaddr);
    return locaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_addr_rm(uip_ds6_addr_t *addr)
{
  if(addr != NULL) {
    uip_create_solicited_node(&addr->ipaddr, &loc_fipaddr);
    if((locmaddr = uip_ds6_maddr_lookup(&loc_fipaddr)) != NULL) {
      uip_ds6_maddr_rm(locmaddr);
    }
    addr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_addr_t *
uip_ds6_addr_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.addr_list, UIP_DS6_ADDR_NB,
      sizeof(uip_ds6_addr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaddr) == FOUND) {
    return locaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
/*
 * get a link local address -
 * state = -1 => any address is ok. Otherwise state = desired state of addr.
 * (TENTATIVE, PREFERRED, DEPRECATED)
 */
uip_ds6_addr_t *
uip_ds6_get_link_local(int8_t state)
{
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused && (state == -1 || locaddr->state == state)
       && (uip_is_addr_link_local(&locaddr->ipaddr))) {
      return locaddr;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
/*
 * get a global address -
 * state = -1 => any address is ok. Otherwise state = desired state of addr.
 * (TENTATIVE, PREFERRED, DEPRECATED)
 */
uip_ds6_addr_t *
uip_ds6_get_global(int8_t state)
{
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused && (state == -1 || locaddr->state == state)
       && !(uip_is_addr_link_local(&locaddr->ipaddr))) {
      return locaddr;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *
uip_ds6_maddr_add(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.maddr_list, UIP_DS6_MADDR_NB,
      sizeof(uip_ds6_maddr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locmaddr) == FREESPACE) {
    locmaddr->isused = 1;
    uip_ipaddr_copy(&locmaddr->ipaddr, ipaddr);
    return locmaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_maddr_rm(uip_ds6_maddr_t *maddr)
{
  if(maddr != NULL) {
    maddr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *
uip_ds6_maddr_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.maddr_list, UIP_DS6_MADDR_NB,
      sizeof(uip_ds6_maddr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locmaddr) == FOUND) {
    return locmaddr;
  }
  return NULL;
}


/*---------------------------------------------------------------------------*/
uip_ds6_aaddr_t *
uip_ds6_aaddr_add(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.aaddr_list, UIP_DS6_AADDR_NB,
      sizeof(uip_ds6_aaddr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaaddr) == FREESPACE) {
    locaaddr->isused = 1;
    uip_ipaddr_copy(&locaaddr->ipaddr, ipaddr);
    return locaaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_aaddr_rm(uip_ds6_aaddr_t *aaddr)
{
  if(aaddr != NULL) {
    aaddr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_aaddr_t *
uip_ds6_aaddr_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop((uip_ds6_element_t *)uip_ds6_if.aaddr_list,
		       UIP_DS6_AADDR_NB, sizeof(uip_ds6_aaddr_t), ipaddr, 128,
		       (uip_ds6_element_t **)&locaaddr) == FOUND) {
    return locaaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_lookup(uip_ipaddr_t *destipaddr)
{
  uip_ds6_route_t *locrt = NULL;
  uint8_t longestmatch = 0;

  PRINTF("DS6: Looking up route for ");
  PRINT6ADDR(destipaddr);
  PRINTF("\n");

  for(locroute = uip_ds6_routing_table;
      locroute < uip_ds6_routing_table + UIP_DS6_ROUTE_NB; locroute++) {
    if((locroute->isused) && (locroute->length >= longestmatch)
       &&
       (uip_ipaddr_prefixcmp
        (destipaddr, &locroute->ipaddr, locroute->length))) {
      longestmatch = locroute->length;
      locrt = locroute;
    }
  }

  if(locrt != NULL) {
    PRINTF("DS6: Found route:");
    PRINT6ADDR(destipaddr);
    PRINTF(" via ");
    PRINT6ADDR(&locrt->nexthop);
    PRINTF("\n");
  } else {
    PRINTF("DS6: No route found\n");
  }

  return locrt;
}

/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint8_t length, uip_ipaddr_t *nexthop,
                  uint8_t metric)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_routing_table, UIP_DS6_ROUTE_NB,
      sizeof(uip_ds6_route_t), ipaddr, length,
      (uip_ds6_element_t **)&locroute) == FREESPACE) {
    locroute->isused = 1;
    uip_ipaddr_copy(&(locroute->ipaddr), ipaddr);
    locroute->length = length;
    uip_ipaddr_copy(&(locroute->nexthop), nexthop);
    locroute->metric = metric;

#ifdef UIP_DS6_ROUTE_STATE_TYPE
    memset(&locroute->state, 0, sizeof(UIP_DS6_ROUTE_STATE_TYPE));
#endif

    PRINTF("DS6: adding route: ");
    PRINT6ADDR(ipaddr);
    PRINTF(" via ");
    PRINT6ADDR(nexthop);
    PRINTF("\n");
    ANNOTATE("#L %u 1;blue\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
  }

  return locroute;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_route_rm(uip_ds6_route_t *route)
{
  route->isused = 0;
#if (DEBUG & DEBUG_ANNOTATE) == DEBUG_ANNOTATE
  /* we need to check if this was the last route towards "nexthop" */
  /* if so - remove that link (annotation) */
  for(locroute = uip_ds6_routing_table;
      locroute < uip_ds6_routing_table + UIP_DS6_ROUTE_NB;
      locroute++) {
    if(locroute->isused && uip_ipaddr_cmp(&locroute->nexthop, &route->nexthop))      {
      /* we found another link using the specific nexthop, so keep the #L */
      return;
    }
  }
  ANNOTATE("#L %u 0\n",route->nexthop.u8[sizeof(uip_ipaddr_t) - 1]);
#endif
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop)
{
  for(locroute = uip_ds6_routing_table;
      locroute < uip_ds6_routing_table + UIP_DS6_ROUTE_NB;
      locroute++) {
    if(locroute->isused && uip_ipaddr_cmp(&locroute->nexthop, nexthop)) {
      locroute->isused = 0;
    }
  }
  ANNOTATE("#L %u 0\n",nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_select_src(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{
  uint8_t best = 0;             /* number of bit in common with best match */
  uint8_t n = 0;
  uip_ds6_addr_t *matchaddr = NULL;

  if(!uip_is_addr_link_local(dst) && !uip_is_addr_mcast(dst)) {
    /* find longest match */
    for(locaddr = uip_ds6_if.addr_list;
        locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
      /* Only preferred global (not link-local) addresses */
      if(locaddr->isused && locaddr->state == ADDR_PREFERRED &&
         !uip_is_addr_link_local(&locaddr->ipaddr)) {
        n = get_match_length(dst, &locaddr->ipaddr);
        if(n >= best) {
          best = n;
          matchaddr = locaddr;
        }
      }
    }
  } else {
    matchaddr = uip_ds6_get_link_local(ADDR_PREFERRED);
  }

  /* use the :: (unspecified address) as source if no match found */
  if(matchaddr == NULL) {
    uip_create_unspecified(src);
  } else {
    uip_ipaddr_copy(src, &matchaddr->ipaddr);
  }
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_set_addr_iid(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  /* We consider only links with IEEE EUI-64 identifier or
   * IEEE 48-bit MAC addresses */
#if (UIP_LLADDR_LEN == 8)
  memcpy(ipaddr->u8 + 8, lladdr, UIP_LLADDR_LEN);
  ipaddr->u8[8] ^= 0x02;
#elif (UIP_LLADDR_LEN == 6)
  memcpy(ipaddr->u8 + 8, lladdr, 3);
  ipaddr->u8[11] = 0xff;
  ipaddr->u8[12] = 0xfe;
  memcpy(ipaddr->u8 + 13, (uint8_t *)lladdr + 3, 3);
  ipaddr->u8[8] ^= 0x02;
#else
#error uip-ds6.c cannot build interface address when UIP_LLADDR_LEN is not 6 or 8
#endif
}

/*---------------------------------------------------------------------------*/
uint8_t
get_match_length(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{
  uint8_t j, k, x_or;
  uint8_t len = 0;

  for(j = 0; j < 16; j++) {
    if(src->u8[j] == dst->u8[j]) {
      len += 8;
    } else {
      x_or = src->u8[j] ^ dst->u8[j];
      for(k = 0; k < 8; k++) {
        if((x_or & 0x80) == 0) {
          len++;
          x_or <<= 1;
        } else {
          break;
        }
      }
      break;
    }
  }
  return len;
}

/*---------------------------------------------------------------------------*/
#if UIP_ND6_DEF_MAXDADNS > 0
void
uip_ds6_dad(uip_ds6_addr_t *addr)
{
  /* send maxdadns NS for DAD  */
  if(addr->dadnscount < uip_ds6_if.maxdadns) {
    uip_nd6_ns_output(NULL, NULL, &addr->ipaddr);
    addr->dadnscount++;
    timer_set(&addr->dadtimer,
              uip_ds6_if.retrans_timer / 1000 * CLOCK_SECOND);
    return;
  }
  /*
   * If we arrive here it means DAD succeeded, otherwise the dad process
   * would have been interrupted in ds6_dad_ns/na_input
   */
  PRINTF("DAD succeeded, ipaddr:");
  PRINT6ADDR(&addr->ipaddr);
  PRINTF("\n");

  addr->state = ADDR_PREFERRED;
  return;
}

/*---------------------------------------------------------------------------*/
/*
 * Calling code must handle when this returns 0 (e.g. link local
 * address can not be used).
 */
int
uip_ds6_dad_failed(uip_ds6_addr_t *addr)
{
  if(uip_is_addr_link_local(&addr->ipaddr)) {
    PRINTF("Contiki shutdown, DAD for link local address failed\n");
    return 0;
  }
  uip_ds6_addr_rm(addr);
  return 1;
}
#endif /*UIP_ND6_DEF_MAXDADNS > 0 */

/*---------------------------------------------------------------------------*/
#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
void
uip_ds6_send_ra_sollicited(void)
{
  /* We have a pb here: RA timer max possible value is 1800s,
   * hence we have to use stimers. However, when receiving a RS, we
   * should delay the reply by a random value between 0 and 500ms timers.
   * stimers are in seconds, hence we cannot do this. Therefore we just send
   * the RA (setting the timer to 0 below). We keep the code logic for
   * the days contiki will support appropriate timers */
  rand_time = 0;
  PRINTF("Solicited RA, random time %u\n", rand_time);

  if(stimer_remaining(&uip_ds6_timer_ra) > rand_time) {
    if(stimer_elapsed(&uip_ds6_timer_ra) < UIP_ND6_MIN_DELAY_BETWEEN_RAS) {
      /* Ensure that the RAs are rate limited */
/*      stimer_set(&uip_ds6_timer_ra, rand_time +
                 UIP_ND6_MIN_DELAY_BETWEEN_RAS -
                 stimer_elapsed(&uip_ds6_timer_ra));
  */ } else {
      stimer_set(&uip_ds6_timer_ra, rand_time);
    }
  }
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_send_ra_periodic(void)
{
  if(racount > 0) {
    /* send previously scheduled RA */
    uip_nd6_ra_output(NULL);
    PRINTF("Sending periodic RA\n");
  }

  rand_time = UIP_ND6_MIN_RA_INTERVAL + random_rand() %
    (uint16_t) (UIP_ND6_MAX_RA_INTERVAL - UIP_ND6_MIN_RA_INTERVAL);
  PRINTF("Random time 1 = %u\n", rand_time);

  if(racount < UIP_ND6_MAX_INITIAL_RAS) {
    if(rand_time > UIP_ND6_MAX_INITIAL_RA_INTERVAL) {
      rand_time = UIP_ND6_MAX_INITIAL_RA_INTERVAL;
      PRINTF("Random time 2 = %u\n", rand_time);
    }
    racount++;
  }
  PRINTF("Random time 3 = %u\n", rand_time);
  stimer_set(&uip_ds6_timer_ra, rand_time);
}

#endif /* UIP_ND6_SEND_RA */
#else /* UIP_CONF_ROUTER */
/*---------------------------------------------------------------------------*/
void
uip_ds6_send_rs(void)
{
  if((uip_ds6_defrt_choose() == NULL)
     && (rscount < UIP_ND6_MAX_RTR_SOLICITATIONS)) {
    PRINTF("Sending RS %u\n", rscount);
    uip_nd6_rs_output();
    rscount++;
    etimer_set(&uip_ds6_timer_rs,
               UIP_ND6_RTR_SOLICITATION_INTERVAL * CLOCK_SECOND);
  } else {
    PRINTF("Router found ? (boolean): %u\n",
           (uip_ds6_defrt_choose() != NULL));
    etimer_stop(&uip_ds6_timer_rs);
  }
  return;
}

#endif /* UIP_CONF_ROUTER */
/*---------------------------------------------------------------------------*/
uint32_t
uip_ds6_compute_reachable_time(void)
{
  return (uint32_t) (UIP_ND6_MIN_RANDOM_FACTOR
                     (uip_ds6_if.base_reachable_time)) +
    ((uint16_t) (random_rand() << 8) +
     (uint16_t) random_rand()) %
    (uint32_t) (UIP_ND6_MAX_RANDOM_FACTOR(uip_ds6_if.base_reachable_time) -
                UIP_ND6_MIN_RANDOM_FACTOR(uip_ds6_if.base_reachable_time));
}


/** @} */
