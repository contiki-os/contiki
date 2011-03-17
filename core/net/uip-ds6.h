/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         Network interface and stateless autoconfiguration (RFC 4862)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 *
 */
/*
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

#ifndef __UIP_DS6_H__
#define __UIP_DS6_H__

#include "net/uip.h"
#include "sys/stimer.h"

/*--------------------------------------------------*/
/** Configuration. For all tables (Neighbor cache, Prefix List, Routing Table,
 * Default Router List, Unicast address list, multicast address list, anycast address list),
 * we define:
 * - the number of elements requested by the user in contiki configuration (name suffixed by _NBU)
 * - the number of elements assigned by the system (name suffixed by _NBS)
 * - the total number of elements is the sum (name suffixed by _NB)
*/
/* Neighbor cache */
#define UIP_DS6_NBR_NBS 0
#ifndef UIP_CONF_DS6_NBR_NBU
#define UIP_DS6_NBR_NBU  4
#else
#define UIP_DS6_NBR_NBU UIP_CONF_DS6_NBR_NBU
#endif
#define UIP_DS6_NBR_NB UIP_DS6_NBR_NBS + UIP_DS6_NBR_NBU

/* Default router list */
#define UIP_DS6_DEFRT_NBS 0
#ifndef UIP_CONF_DS6_DEFRT_NBU
#define UIP_DS6_DEFRT_NBU 2
#else
#define UIP_DS6_DEFRT_NBU UIP_CONF_DS6_DEFRT_NBU
#endif
#define UIP_DS6_DEFRT_NB UIP_DS6_DEFRT_NBS + UIP_DS6_DEFRT_NBU

/* Prefix list */
#define UIP_DS6_PREFIX_NBS  1
#ifndef UIP_CONF_DS6_PREFIX_NBU
#define UIP_DS6_PREFIX_NBU  2
#else
#define UIP_DS6_PREFIX_NBU UIP_CONF_DS6_PREFIX_NBU
#endif
#define UIP_DS6_PREFIX_NB UIP_DS6_PREFIX_NBS + UIP_DS6_PREFIX_NBU

/* Routing table */
#define UIP_DS6_ROUTE_NBS 0
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_DS6_ROUTE_NBU 4
#else
#define UIP_DS6_ROUTE_NBU UIP_CONF_DS6_ROUTE_NBU
#endif
#define UIP_DS6_ROUTE_NB UIP_DS6_ROUTE_NBS + UIP_DS6_ROUTE_NBU

/* Unicast address list*/
#define UIP_DS6_ADDR_NBS 1
#ifndef UIP_CONF_DS6_ADDR_NBU
#define UIP_DS6_ADDR_NBU 2
#else
#define UIP_DS6_ADDR_NBU UIP_CONF_DS6_ADDR_NBU
#endif
#define UIP_DS6_ADDR_NB UIP_DS6_ADDR_NBS + UIP_DS6_ADDR_NBU

/* Multicast address list */
#if UIP_CONF_ROUTER
#define UIP_DS6_MADDR_NBS 2 + UIP_DS6_ADDR_NB   /* all routers + all nodes + one solicited per unicast */
#else
#define UIP_DS6_MADDR_NBS 1 + UIP_DS6_ADDR_NB   /* all nodes + one solicited per unicast */
#endif
#ifndef UIP_CONF_DS6_MADDR_NBU
#define UIP_DS6_MADDR_NBU 0
#else
#define UIP_DS6_MADDR_NBU UIP_CONF_DS6_MADDR_NBU
#endif
#define UIP_DS6_MADDR_NB UIP_DS6_MADDR_NBS + UIP_DS6_MADDR_NBU

/* Anycast address list */
#if UIP_CONF_ROUTER
#define UIP_DS6_AADDR_NBS UIP_DS6_PREFIX_NB - 1 /* One per non link local prefix (subnet prefix anycast address) */
#else
#define UIP_DS6_AADDR_NBS 0
#endif
#ifndef UIP_CONF_DS6_AADDR_NBU
#define UIP_DS6_AADDR_NBU 0
#else
#define UIP_DS6_AADDR_NBU UIP_CONF_DS6_AADDR_NBU
#endif
#define UIP_DS6_AADDR_NB UIP_DS6_AADDR_NBS + UIP_DS6_AADDR_NBU


/*--------------------------------------------------*/
/** \brief Possible states for the nbr cache entries */
#define  NBR_INCOMPLETE 0
#define  NBR_REACHABLE 1
#define  NBR_STALE 2
#define  NBR_DELAY 3
#define  NBR_PROBE 4

/** \brief Possible states for the an address  (RFC 4862) */
#define ADDR_TENTATIVE 0
#define ADDR_PREFERRED 1
#define ADDR_DEPRECATED 2

/** \brief How the address was acquired: Autoconf, DHCP or manually */
#define  ADDR_ANYTYPE 0
#define  ADDR_AUTOCONF 1
#define  ADDR_DHCP 2
#define  ADDR_MANUAL 3

/** \brief General DS6 definitions */
#define UIP_DS6_PERIOD   (CLOCK_SECOND/10)  /** Period for uip-ds6 periodic task*/
#define FOUND 0
#define FREESPACE 1
#define NOSPACE 2


/*--------------------------------------------------*/
#if UIP_CONF_IPV6_QUEUE_PKT
#include "net/uip-packetqueue.h"
#endif                          /*UIP_CONF_QUEUE_PKT */
/** \brief An entry in the nbr cache */
typedef struct uip_ds6_nbr {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  uip_lladdr_t lladdr;
  struct stimer reachable;
  struct stimer sendns;
  clock_time_t last_lookup;
  uint8_t nscount;
  uint8_t isrouter;
  uint8_t state;
#if UIP_CONF_IPV6_QUEUE_PKT
  struct uip_packetqueue_handle packethandle;
#define UIP_DS6_NBR_PACKET_LIFETIME CLOCK_SECOND * 4
#endif                          /*UIP_CONF_QUEUE_PKT */
} uip_ds6_nbr_t;

/** \brief An entry in the default router list */
typedef struct uip_ds6_defrt {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  struct stimer lifetime;
  uint8_t isinfinite;
} uip_ds6_defrt_t;

/** \brief A prefix list entry */
#if UIP_CONF_ROUTER
typedef struct uip_ds6_prefix {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  uint8_t length;
  uint8_t advertise;
  u32_t vlifetime;
  u32_t plifetime;
  uint8_t l_a_reserved; /**< on-link and autonomous flags + 6 reserved bits */
} uip_ds6_prefix_t;
#else /* UIP_CONF_ROUTER */
typedef struct uip_ds6_prefix {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  uint8_t length;
  struct stimer vlifetime;
  uint8_t isinfinite;
} uip_ds6_prefix_t;
#endif /*UIP_CONF_ROUTER */

/** * \brief Unicast address structure */
typedef struct uip_ds6_addr {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  uint8_t state;
  uint8_t type;
  uint8_t isinfinite;
  struct stimer vlifetime;
  struct timer dadtimer;
  uint8_t dadnscount;
} uip_ds6_addr_t;

/** \brief Anycast address  */
typedef struct uip_ds6_aaddr {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
} uip_ds6_aaddr_t;

/** \brief A multicast address */
typedef struct uip_ds6_maddr {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
} uip_ds6_maddr_t;

/** \brief define some additional RPL related route state and
 *  neighbor callback for RPL - if not a DS6_ROUTE_STATE is already set */
#ifndef UIP_DS6_ROUTE_STATE_TYPE
#define UIP_DS6_ROUTE_STATE_TYPE rpl_route_entry_t
/* Needed for the extended route entry state when using ContikiRPL */
typedef struct rpl_route_entry {
  uint32_t lifetime;
  uint32_t saved_lifetime;
  void *dag;
  uint8_t learned_from;
} rpl_route_entry_t;
#endif /* UIP_DS6_ROUTE_STATE_TYPE */

/* only define the callback if RPL is active */
#if UIP_CONF_IPV6_RPL
#ifndef UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED
#define UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED rpl_ipv6_neighbor_callback
#endif /* UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED */
#endif /* UIP_CONF_IPV6_RPL */



/** \brief An entry in the routing table */
typedef struct uip_ds6_route {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  uint8_t length;
  uint8_t metric;
  uip_ipaddr_t nexthop;
#ifdef UIP_DS6_ROUTE_STATE_TYPE
  UIP_DS6_ROUTE_STATE_TYPE state;
#endif
} uip_ds6_route_t;

/** \brief  Interface structure (contains all the interface variables) */
typedef struct uip_ds6_netif {
  uint32_t link_mtu;
  uint8_t cur_hop_limit;
  uint32_t base_reachable_time; /* in msec */
  uint32_t reachable_time;      /* in msec */
  uint32_t retrans_timer;       /* in msec */
  uint8_t maxdadns;
  uip_ds6_addr_t addr_list[UIP_DS6_ADDR_NB];
  uip_ds6_aaddr_t aaddr_list[UIP_DS6_AADDR_NB];
  uip_ds6_maddr_t maddr_list[UIP_DS6_MADDR_NB];
} uip_ds6_netif_t;

/** \brief Generic type for a DS6, to use a common loop though all DS */
typedef struct uip_ds6_element {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
} uip_ds6_element_t;


/*---------------------------------------------------------------------------*/
extern uip_ds6_netif_t uip_ds6_if;
extern struct etimer uip_ds6_timer_periodic;

#if UIP_CONF_ROUTER
extern uip_ds6_prefix_t uip_ds6_prefix_list[UIP_DS6_PREFIX_NB];
#else /* UIP_CONF_ROUTER */
extern struct etimer uip_ds6_timer_rs;
#endif /* UIP_CONF_ROUTER */


/*---------------------------------------------------------------------------*/
/** \brief Initialize data structures */
void uip_ds6_init(void);

/** \brief Periodic processing of data structures */
void uip_ds6_periodic(void);

/** \brief Generic loop routine on an abstract data structure, which generalizes
 * all data structures used in DS6 */
uint8_t uip_ds6_list_loop(uip_ds6_element_t *list, uint8_t size,
                          uint16_t elementsize, uip_ipaddr_t *ipaddr,
                          uint8_t ipaddrlen,
                          uip_ds6_element_t **out_element);

/** \name Neighbor Cache basic routines */
/** @{ */
uip_ds6_nbr_t *uip_ds6_nbr_add(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr,
                               uint8_t isrouter, uint8_t state);
void uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr);
uip_ds6_nbr_t *uip_ds6_nbr_lookup(uip_ipaddr_t *ipaddr);

/** @} */

/** \name Default router list basic routines */
/** @{ */
uip_ds6_defrt_t *uip_ds6_defrt_add(uip_ipaddr_t *ipaddr,
                                   unsigned long interval);
void uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt);
uip_ds6_defrt_t *uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr);
uip_ipaddr_t *uip_ds6_defrt_choose(void);

/** @} */

/** \name Prefix list basic routines */
/** @{ */
#if UIP_CONF_ROUTER
uip_ds6_prefix_t *uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t length,
                                     uint8_t advertise, uint8_t flags,
                                     unsigned long vtime,
                                     unsigned long ptime);
#else /* UIP_CONF_ROUTER */
uip_ds6_prefix_t *uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t length,
                                     unsigned long interval);
#endif /* UIP_CONF_ROUTER */
void uip_ds6_prefix_rm(uip_ds6_prefix_t *prefix);
uip_ds6_prefix_t *uip_ds6_prefix_lookup(uip_ipaddr_t *ipaddr,
                                        uint8_t ipaddrlen);
uint8_t uip_ds6_is_addr_onlink(uip_ipaddr_t *ipaddr);

/** @} */

/** \name Unicast address list basic routines */
/** @{ */
uip_ds6_addr_t *uip_ds6_addr_add(uip_ipaddr_t *ipaddr,
                                 unsigned long vlifetime, uint8_t type);
void uip_ds6_addr_rm(uip_ds6_addr_t *addr);
uip_ds6_addr_t *uip_ds6_addr_lookup(uip_ipaddr_t *ipaddr);
uip_ds6_addr_t *uip_ds6_get_link_local(int8_t state);
uip_ds6_addr_t *uip_ds6_get_global(int8_t state);

/** @} */

/** \name Multicast address list basic routines */
/** @{ */
uip_ds6_maddr_t *uip_ds6_maddr_add(uip_ipaddr_t *ipaddr);
void uip_ds6_maddr_rm(uip_ds6_maddr_t *maddr);
uip_ds6_maddr_t *uip_ds6_maddr_lookup(uip_ipaddr_t *ipaddr);

/** @} */

/** \name Anycast address list basic routines */
/** @{ */
uip_ds6_aaddr_t *uip_ds6_aaddr_add(uip_ipaddr_t *ipaddr);
void uip_ds6_aaddr_rm(uip_ds6_aaddr_t *aaddr);
uip_ds6_aaddr_t *uip_ds6_aaddr_lookup(uip_ipaddr_t *ipaddr);

/** @} */


/** \name Routing Table basic routines */
/** @{ */
uip_ds6_route_t *uip_ds6_route_lookup(uip_ipaddr_t *destipaddr);
uip_ds6_route_t *uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint8_t length,
                                   uip_ipaddr_t *next_hop, uint8_t metric);
void uip_ds6_route_rm(uip_ds6_route_t *route);
void uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop);

/** @} */

/** \brief set the last 64 bits of an IP address based on the MAC address */
void uip_ds6_set_addr_iid(uip_ipaddr_t * ipaddr, uip_lladdr_t * lladdr);

/** \brief Get the number of matching bits of two addresses */
uint8_t get_match_length(uip_ipaddr_t * src, uip_ipaddr_t * dst);

/** \brief Perform Duplicate Address Selection on one address */
void uip_ds6_dad(uip_ds6_addr_t * ifaddr);

/** \brief Callback when DAD failed */
int uip_ds6_dad_failed(uip_ds6_addr_t * ifaddr);

/** \brief Source address selection, see RFC 3484 */
void uip_ds6_select_src(uip_ipaddr_t * src, uip_ipaddr_t * dst);

#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
/** \brief Send a RA as an asnwer to a RS */
void uip_ds6_send_ra_sollicited(void);

/** \brief Send a periodic RA */
void uip_ds6_send_ra_periodic(void);
#endif /* UIP_ND6_SEND_RA */
#else /* UIP_CONF_ROUTER */
/** \brief Send periodic RS to find router */
void uip_ds6_send_rs(void);
#endif /* UIP_CONF_ROUTER */

/** \brief Compute the reachable time based on base reachable time, see RFC 4861*/
uint32_t uip_ds6_compute_reachable_time(void); /** \brief compute random reachable timer */

/** \name Macros to check if an IP address (unicast, multicast or anycast) is mine */
/** @{ */
#define uip_ds6_is_my_addr(addr)  (uip_ds6_addr_lookup(addr) != NULL)
#define uip_ds6_is_my_maddr(addr) (uip_ds6_maddr_lookup(addr) != NULL)
#define uip_ds6_is_my_aaddr(addr) (uip_ds6_aaddr_lookup(addr) != NULL)
/** @} */
/** @} */

#endif /* __UIP_DS6_H__ */
