/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef UIP_DS6_ROUTE_H
#define UIP_DS6_ROUTE_H

void uip_ds6_route_init(void);


/* Event constants for the uip-ds6 route notification interface. The
   notification interface allows for a user program to be notified via
   a callback when a route has been added or removed and when the
   system has added or removed a default route. */
#define UIP_DS6_NOTIFICATION_DEFRT_ADD 0
#define UIP_DS6_NOTIFICATION_DEFRT_RM  1
#define UIP_DS6_NOTIFICATION_ROUTE_ADD 2
#define UIP_DS6_NOTIFICATION_ROUTE_RM  3

typedef void (* uip_ds6_notification_callback)(int event,
					       uip_ipaddr_t *route,
					       uip_ipaddr_t *nexthop,
					       int num_routes);
struct uip_ds6_notification {
  struct uip_ds6_notification *next;
  uip_ds6_notification_callback callback;
};

void uip_ds6_notification_add(struct uip_ds6_notification *n,
			      uip_ds6_notification_callback c);
/*--------------------------------------------------*/


/* Routing table */
#define UIP_DS6_ROUTE_NBS 0
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_DS6_ROUTE_NBU 4
#else
#define UIP_DS6_ROUTE_NBU UIP_CONF_DS6_ROUTE_NBU
#endif
#define UIP_DS6_ROUTE_NB UIP_DS6_ROUTE_NBS + UIP_DS6_ROUTE_NBU

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



/** \brief An entry in the routing table */
typedef struct uip_ds6_route {
  struct uip_ds6_route *next;
  uip_ipaddr_t ipaddr;
  uip_ipaddr_t nexthop;
  uint8_t length;
  uint8_t metric;
#ifdef UIP_DS6_ROUTE_STATE_TYPE
  UIP_DS6_ROUTE_STATE_TYPE state;
#endif
} uip_ds6_route_t;



/** \brief An entry in the default router list */
typedef struct uip_ds6_defrt {
  struct uip_ds6_defrt *next;
  uip_ipaddr_t ipaddr;
  struct stimer lifetime;
  uint8_t isinfinite;
} uip_ds6_defrt_t;

/** \name Default router list basic routines */
/** @{ */
uip_ds6_defrt_t *uip_ds6_defrt_add(uip_ipaddr_t *ipaddr,
                                   unsigned long interval);
void uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt);
uip_ds6_defrt_t *uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr);
uip_ipaddr_t *uip_ds6_defrt_choose(void);

void uip_ds6_defrt_periodic(void);
/** @} */


/** \name Routing Table basic routines */
/** @{ */
uip_ds6_route_t *uip_ds6_route_lookup(uip_ipaddr_t *destipaddr);
uip_ds6_route_t *uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint8_t length,
                                   uip_ipaddr_t *next_hop, uint8_t metric);
void uip_ds6_route_rm(uip_ds6_route_t *route);
void uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop);

int uip_ds6_route_num_routes(void);
uip_ds6_route_t *uip_ds6_route_list_head(void);

/** @} */

#endif /* UIP_DS6_ROUTE_H */
