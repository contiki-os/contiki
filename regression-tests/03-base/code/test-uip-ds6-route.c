/*
 * Copyright (c) 2017, Yasuyuki Tanaka
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
 */

#include <stdio.h>

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "lib/assert.h"
#include "net/ipv6/uip-ds6-nbr.h"

#include "unit-test.h"
#include "common.h"

PROCESS(test_process, "uip-ds6-route.c test");
AUTOSTART_PROCESSES(&test_process);

static uip_ipaddr_t dest1;
static uip_ipaddr_t dest2;
static uip_ipaddr_t dest3;
static uip_ipaddr_t next_hop1;
static uip_ipaddr_t next_hop2;
static uip_ipaddr_t next_hop3;
static uip_lladdr_t next_hop1_lladdr;
static uip_lladdr_t next_hop2_lladdr;
static uip_lladdr_t next_hop3_lladdr;

static void
setup(void)
{
  uip_ipaddr_t prefix;
  uip_ds6_nbr_t *nbr;
  uip_ds6_route_t *route, *next_route;

  reset_callback_status();

  /* remove all the installed routes */
  for(route = uip_ds6_route_head();
      route != NULL;
      route = next_route) {
    next_route = uip_ds6_route_next(route);
    uip_ds6_route_rm(route);
  }
  assert(uip_ds6_route_num_routes() == 0);

  /* need to install prefix for on-link determination */
  uiplib_ip6addrconv("fd00::", &prefix);
  if(uip_ds6_prefix_lookup(&prefix, 64)  == NULL) {
    uip_ds6_prefix_add((uip_ipaddr_t *)&prefix, 64,
                       0, 0, 0, 0);
  }

  uiplib_ipaddrconv("fd00::1", &dest1);
  uiplib_ipaddrconv("fd00::2", &dest2);
  uiplib_ipaddrconv("fd00::3", &dest3);
  uiplib_ipaddrconv("fe80::202:2:2:1", &next_hop1);
  uiplib_ipaddrconv("fe80::202:2:2:2", &next_hop2);
  uiplib_ipaddrconv("fe80::202:2:2:3", &next_hop3);
  memset(&next_hop1_lladdr, 0, sizeof(next_hop1_lladdr));
  memset(&next_hop2_lladdr, 0, sizeof(next_hop1_lladdr));
  memset(&next_hop3_lladdr, 0, sizeof(next_hop1_lladdr));
  next_hop1_lladdr.addr[0] = 1;
  next_hop2_lladdr.addr[0] = 2;
  next_hop3_lladdr.addr[0] = 3;

  /*
   * Contiki requires next_hop1 to be in the neighbor cache table in order to use
   * it for a route.
   */
  if((nbr = uip_ds6_nbr_lookup((const uip_ipaddr_t *)&next_hop1)) == NULL) {
    nbr = uip_ds6_nbr_add(&next_hop1, &next_hop1_lladdr,
                          1, NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  }
  assert(nbr != NULL);

  if((nbr = uip_ds6_nbr_lookup((const uip_ipaddr_t *)&next_hop2)) == NULL) {
    nbr = uip_ds6_nbr_add(&next_hop2, &next_hop2_lladdr,
                          1, NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  }
  assert(nbr != NULL);
}

UNIT_TEST_REGISTER(test_add, "uip_ds6_route_add");
UNIT_TEST(test_add)
{
  uip_ds6_route_t *ret;

  setup();

  UNIT_TEST_BEGIN();

  /* before adding a route, the nbr_table item for "next_hop1" is not locked */
  UNIT_TEST_ASSERT(nbr_table_is_locked((const linkaddr_t *)&next_hop1_lladdr)
                   == 0);

  /* add a route with a link-local address for the next-hop */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(ret != NULL);
  UNIT_TEST_ASSERT(nbr_table_is_locked((const linkaddr_t *)&next_hop1_lladdr)
                   == 1);
  UNIT_TEST_ASSERT(test_callback_state == ADDED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  /* add the same route as before; should succeed */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(ret != NULL);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  /* add another route */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest2, 128, &next_hop2);
  UNIT_TEST_ASSERT(ret != NULL);
  UNIT_TEST_ASSERT(test_callback_state == ADDED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop2_lladdr));

  /* this addition should fail because of no memory */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest3, 128, &next_hop3);
  UNIT_TEST_ASSERT(ret == NULL);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_add_ex, "uip_ds6_route_add extra");
UNIT_TEST(test_add_ex)
{
  uip_ds6_route_t *ret;

  setup();

  UNIT_TEST_BEGIN();

  /* before adding a route, the nbr_table item for "next_hop1" is not locked */
  UNIT_TEST_ASSERT(nbr_table_is_locked((const linkaddr_t *)&next_hop1_lladdr)
                   == 0);

  /* add a route with a link-local address for the next-hop */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(ret != NULL);
  UNIT_TEST_ASSERT(nbr_table_is_locked((const linkaddr_t *)&next_hop1_lladdr)
                   == 1);
  UNIT_TEST_ASSERT(test_callback_state == ADDED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));


  /* add another route with the same next-hop as the previous one */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest2, 128, &next_hop1);
  UNIT_TEST_ASSERT(ret != NULL);
  ret = uip_ds6_route_lookup(&dest1);
  UNIT_TEST_ASSERT(ret != NULL);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  /* this addition should fail because of lack of memory */
  reset_callback_status();
  ret = uip_ds6_route_add(&dest3, 128, &next_hop3);
  UNIT_TEST_ASSERT(ret == NULL);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_rm, "uip_ds6_route_rm");
UNIT_TEST(test_rm)
{
  uip_ds6_route_t *route;

  setup();

  UNIT_TEST_BEGIN();

  /* add a route with a link-local address for the next-hop */
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);


  /* remove the route */
  reset_callback_status();
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 1);
  uip_ds6_route_rm(route);
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 0);
  UNIT_TEST_ASSERT(test_callback_state == REMOVED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_rm_ex, "uip_ds6_route_rm extra");
UNIT_TEST(test_rm_ex)
{
  uip_ds6_route_t *route1, *route2;

  setup();

  UNIT_TEST_BEGIN();

  /* add two routes with an identical link-local address for the next-hop */
  route1 = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route1 != NULL);
  route2 = uip_ds6_route_add(&dest2, 128, &next_hop1);
  UNIT_TEST_ASSERT(route2 != NULL);

  /* remove the first route */
  reset_callback_status();
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 2);
  uip_ds6_route_rm(route1);
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 1);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  /* remove the second route */
  reset_callback_status();
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 1);
  uip_ds6_route_rm(route2);
  UNIT_TEST_ASSERT(uip_ds6_route_num_routes() == 0);
  UNIT_TEST_ASSERT(test_callback_state == REMOVED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_lookup, "uip_ds6_route_lookup");
UNIT_TEST(test_lookup)
{
  uip_ds6_route_t *route, *ret;

  setup();

  UNIT_TEST_BEGIN();

  /* lookup a route which is not installed */
  ret = uip_ds6_route_lookup(&dest1);
  UNIT_TEST_ASSERT(ret == NULL);

  /* add a route with a link-local address for the next-hop */
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);

  /* lookup a route which is not installed */
  ret = uip_ds6_route_lookup(&dest1);
  UNIT_TEST_ASSERT(ret == route);

  /* lookup a route for another destination */
  ret = uip_ds6_route_lookup(&dest2);
  UNIT_TEST_ASSERT(ret == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_is_nexthop, "uip_ds6_route_is_nexthop");
UNIT_TEST(test_is_nexthop)
{
  int ret;
  uip_ds6_route_t *route;

  setup();

  UNIT_TEST_BEGIN();

  /* "next_hop1" is not a next-hop address unless it is used for a route */
  ret = uip_ds6_route_is_nexthop(&next_hop1);
  UNIT_TEST_ASSERT(ret == 0);

  /* add a route with a link-local address for the next-hop */
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);

  /* now, *nexthop* should be a next-hop address */
  ret = uip_ds6_route_is_nexthop(&next_hop1);
  UNIT_TEST_ASSERT(ret == 1);

  /* *dest* should not be a next-hop address */
  ret = uip_ds6_route_is_nexthop(&dest1);
  UNIT_TEST_ASSERT(ret == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_nexthop, "uip_ds6_route_nexthop");
UNIT_TEST(test_nexthop)
{
  const uip_ipaddr_t *ret;
  uip_ds6_route_t *route;

  setup();

  UNIT_TEST_BEGIN();

  /* add a route with a link-local address for the next-hop */
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);

  /* the next-hop of the route for dest should be "next_hop1" */
  ret = uip_ds6_route_nexthop(route);
  UNIT_TEST_ASSERT(ret != NULL && uip_ipaddr_cmp(ret, &next_hop1));

  /* no next-hop should be found for NULL */
  ret = uip_ds6_route_nexthop(NULL);
  UNIT_TEST_ASSERT(ret == NULL);

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  printf("UIP_DS6_NBR_MULTI_IPV6_ADDRS is enabled\n");
#else
  printf("UIP_DS6_NBR_MULTI_IPV6_ADDRS is disabled\n");
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(test_add);
  UNIT_TEST_RUN(test_add_ex);
  UNIT_TEST_RUN(test_rm);
  UNIT_TEST_RUN(test_rm_ex);
  UNIT_TEST_RUN(test_lookup);
  UNIT_TEST_RUN(test_is_nexthop);
  UNIT_TEST_RUN(test_nexthop);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
