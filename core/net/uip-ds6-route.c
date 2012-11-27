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
#include "net/uip-ds6.h"
#include "net/uip.h"

#include "lib/list.h"
#include "lib/memb.h"


#include <string.h>

LIST(routelist);
MEMB(routememb, uip_ds6_route_t, UIP_DS6_ROUTE_NB);

LIST(defaultrouterlist);
MEMB(defaultroutermemb, uip_ds6_defrt_t, UIP_DS6_DEFRT_NB);

LIST(notificationlist);

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

/*---------------------------------------------------------------------------*/
static void
call_route_callback(int event, uip_ipaddr_t *route,
		    uip_ipaddr_t *nexthop)
{
  int num;
  struct uip_ds6_notification *n;
  for(n = list_head(notificationlist);
      n != NULL;
      n = list_item_next(n)) {
    if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD ||
       event == UIP_DS6_NOTIFICATION_DEFRT_RM) {
      num = list_length(defaultrouterlist);
    } else {
      num = list_length(routelist);
    }
    n->callback(event, route, nexthop, num);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_notification_add(struct uip_ds6_notification *n,
			 uip_ds6_notification_callback c)
{
  if(n != NULL && c != NULL) {
    n->callback = c;
    list_add(notificationlist, n);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_notification_rm(struct uip_ds6_notification *n)
{
  list_remove(notificationlist, n);
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_route_init(void)
{
  memb_init(&routememb);
  list_init(routelist);

  memb_init(&defaultroutermemb);
  list_init(defaultrouterlist);

  list_init(notificationlist);
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_list_head(void)
{
  return list_head(routelist);
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_route_num_routes(void)
{
  return list_length(routelist);
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_lookup(uip_ipaddr_t *addr)
{
  uip_ds6_route_t *r;
  uip_ds6_route_t *found_route;
  uint8_t longestmatch;

  PRINTF("uip-ds6-route: Looking up route for ");
  PRINT6ADDR(addr);
  PRINTF("\n");


  found_route = NULL;
  longestmatch = 0;
  for(r = list_head(routelist);
      r != NULL;
      r = list_item_next(r)) {
    if(r->length >= longestmatch &&
       uip_ipaddr_prefixcmp(addr, &r->ipaddr, r->length)) {
      longestmatch = r->length;
      found_route = r;
    }

  }

  if(found_route != NULL) {
    PRINTF("uip-ds6-route: Found route:");
    PRINT6ADDR(addr);
    PRINTF(" via ");
    PRINT6ADDR(&found_route->nexthop);
    PRINTF("\n");
  } else {
    PRINTF("uip-ds6-route: No route found\n");
  }

  return found_route;
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
uip_ds6_route_add(uip_ipaddr_t *ipaddr, uint8_t length,
		  uip_ipaddr_t *nexthop, uint8_t metric)
{
  uip_ds6_route_t *r;

  /* First make sure that we don't add a route twice. If we find an
     existing route for our destination, we'll just update the old
     one. */
  r = uip_ds6_route_lookup(ipaddr);
  if(r != NULL) {
    PRINTF("uip_ds6_route_add: old route already found, updating this one instead: ");
    PRINT6ADDR(ipaddr);
    PRINTF("\n");
  } else {
    /* Allocate a routing entry and add the route to the list */
    r = memb_alloc(&routememb);
    if(r == NULL) {
      PRINTF("uip_ds6_route_add: could not allocate memory for new route to ");
      PRINT6ADDR(ipaddr);
      PRINTF(", dropping it\n");
      return NULL;
    }
    list_add(routelist, r);

    PRINTF("uip_ds6_route_add num %d\n", list_length(routelist));
  }

  uip_ipaddr_copy(&(r->ipaddr), ipaddr);
  r->length = length;
  uip_ipaddr_copy(&(r->nexthop), nexthop);
  r->metric = metric;

#ifdef UIP_DS6_ROUTE_STATE_TYPE
  memset(&r->state, 0, sizeof(UIP_DS6_ROUTE_STATE_TYPE));
#endif

  PRINTF("uip_ds6_route_add: adding route: ");
  PRINT6ADDR(ipaddr);
  PRINTF(" via ");
  PRINT6ADDR(nexthop);
  PRINTF("\n");
  ANNOTATE("#L %u 1;blue\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);

  call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_ADD, ipaddr, nexthop);

  return r;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_route_rm(uip_ds6_route_t *route)
{
  uip_ds6_route_t *r;
  /* Make sure that the route is in the list before removing it. */
  for(r = list_head(routelist);
      r != NULL;
      r = list_item_next(r)) {
    if(r == route) {
      list_remove(routelist, route);
      memb_free(&routememb, route);

      PRINTF("uip_ds6_route_rm num %d\n", list_length(routelist));

      call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_RM,
			  &route->ipaddr, &route->nexthop);
#if (DEBUG & DEBUG_ANNOTATE) == DEBUG_ANNOTATE
      /* we need to check if this was the last route towards "nexthop" */
      /* if so - remove that link (annotation) */
      for(r = list_head(routelist);
	  r != NULL;
	  r = list_item_next(r)) {
	if(uip_ipaddr_cmp(&r->nexthop, &route->nexthop)) {
	  /* we found another link using the specific nexthop, so keep the #L */
	  return;
	}
      }
      ANNOTATE("#L %u 0\n", route->nexthop.u8[sizeof(uip_ipaddr_t) - 1]);
#endif
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_route_rm_by_nexthop(uip_ipaddr_t *nexthop)
{
  uip_ds6_route_t *r;

  r = list_head(routelist);
  while(r != NULL) {
    if(uip_ipaddr_cmp(&r->nexthop, nexthop)) {
      list_remove(routelist, r);
      call_route_callback(UIP_DS6_NOTIFICATION_ROUTE_RM,
			  &r->ipaddr, &r->nexthop);
      r = list_head(routelist);
    } else {
      r = list_item_next(r);
    }
  }
}
/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_add(uip_ipaddr_t *ipaddr, unsigned long interval)
{
  uip_ds6_defrt_t *d;

  d = uip_ds6_defrt_lookup(ipaddr);
  if(d == NULL) {
    d = memb_alloc(&defaultroutermemb);
    if(d == NULL) {
      PRINTF("uip_ds6_defrt_add: could not add default route to ");
      PRINT6ADDR(ipaddr);
      PRINTF(", out of memory\n");
      return NULL;
    } else {
      PRINTF("uip_ds6_defrt_add: adding default route to ");
      PRINT6ADDR(ipaddr);
      PRINTF("\n");
    }

    list_push(defaultrouterlist, d);
  }

  uip_ipaddr_copy(&d->ipaddr, ipaddr);
  if(interval != 0) {
    stimer_set(&d->lifetime, interval);
    d->isinfinite = 0;
  } else {
    d->isinfinite = 1;
  }

  ANNOTATE("#L %u 1\n", ipaddr->u8[sizeof(uip_ipaddr_t) - 1]);

  call_route_callback(UIP_DS6_NOTIFICATION_DEFRT_ADD, ipaddr, ipaddr);

  return d;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_defrt_rm(uip_ds6_defrt_t *defrt)
{
  uip_ds6_defrt_t *d;
  /* Make sure that the defrt is in the list before we remove it. */
  for(d = list_head(defaultrouterlist);
      d != NULL;
      d = list_item_next(d)) {
    if(d == defrt) {
      PRINTF("Removing default route\n");
      list_remove(defaultrouterlist, defrt);
      memb_free(&defaultroutermemb, defrt);
      ANNOTATE("#L %u 0\n", defrt->ipaddr.u8[sizeof(uip_ipaddr_t) - 1]);
      call_route_callback(UIP_DS6_NOTIFICATION_DEFRT_RM,
			  &defrt->ipaddr, &defrt->ipaddr);
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t *
uip_ds6_defrt_lookup(uip_ipaddr_t *ipaddr)
{
  uip_ds6_defrt_t *d;
  for(d = list_head(defaultrouterlist);
      d != NULL;
      d = list_item_next(d)) {
    if(uip_ipaddr_cmp(&d->ipaddr, ipaddr)) {
      return d;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_defrt_choose(void)
{
  uip_ds6_defrt_t *d;
  uip_ds6_nbr_t *bestnbr;
  uip_ipaddr_t *addr;

  addr = NULL;
  for(d = list_head(defaultrouterlist);
      d != NULL;
      d = list_item_next(d)) {
    PRINTF("Defrt, IP address ");
    PRINT6ADDR(&d->ipaddr);
    PRINTF("\n");
    bestnbr = uip_ds6_nbr_lookup(&d->ipaddr);
    if(bestnbr != NULL && bestnbr->state != NBR_INCOMPLETE) {
      PRINTF("Defrt found, IP address ");
      PRINT6ADDR(&d->ipaddr);
      PRINTF("\n");
      return &d->ipaddr;
    } else {
      addr = &d->ipaddr;
      PRINTF("Defrt INCOMPLETE found, IP address ");
      PRINT6ADDR(&d->ipaddr);
      PRINTF("\n");
    }
  }
  return addr;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_defrt_periodic(void)
{
  uip_ds6_defrt_t *d;
  d = list_head(defaultrouterlist);
  while(d != NULL) {
    if(!d->isinfinite &&
       stimer_expired(&d->lifetime)) {
      PRINTF("uip_ds6_defrt_periodic: defrt lifetime expired\n");
      uip_ds6_defrt_rm(d);
      d = list_head(defaultrouterlist);
    } else {
      d = list_item_next(d);
    }
  }
}
/*---------------------------------------------------------------------------*/


