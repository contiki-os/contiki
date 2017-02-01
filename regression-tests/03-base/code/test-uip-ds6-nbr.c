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

#include "lib/simEnvChange.h"
#include "sys/cooja_mt.h"

#include "unit-test.h"
#include "common.h"

PROCESS(test_process, "uip-ds6-route.c test");
AUTOSTART_PROCESSES(&test_process);

static uip_ipaddr_t dest1;
static uip_ipaddr_t dest2;
static uip_ipaddr_t dest3;
static uip_lladdr_t dest1_lladdr;
static uip_lladdr_t dest2_lladdr;
static uip_lladdr_t dest3_lladdr;
static uip_ipaddr_t next_hop1;
static uip_ipaddr_t next_hop2;
static uip_lladdr_t next_hop1_lladdr;
static uip_lladdr_t next_hop2_lladdr;

static void
setup(void)
{
  uip_ds6_nbr_t *nbr, *next_nbr;

  uiplib_ipaddrconv("fd00::1", &dest1);
  uiplib_ipaddrconv("fd00::2", &dest2);
  uiplib_ipaddrconv("fd00::3", &dest3);
  uiplib_ipaddrconv("fe80::202:2:2:1", &next_hop1);
  uiplib_ipaddrconv("fe80::202:2:2:2", &next_hop2);
  memset(&next_hop1_lladdr, 0, sizeof(next_hop1_lladdr));
  memset(&next_hop2_lladdr, 0, sizeof(next_hop1_lladdr));
  memset(&dest1_lladdr, 0, sizeof(dest1_lladdr));
  memset(&dest2_lladdr, 0, sizeof(dest1_lladdr));
  memset(&dest3_lladdr, 0, sizeof(dest1_lladdr));
  dest1_lladdr.addr[0] = 1;
  dest2_lladdr.addr[0] = 2;
  dest3_lladdr.addr[0] = 3;
  next_hop1_lladdr.addr[0] = 4;
  next_hop2_lladdr.addr[0] = 5;

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  for(nbr = uip_ds6_nbr_list_head();
      nbr != NULL;
      nbr = next_nbr) {
    next_nbr = uip_ds6_nbr_list_item_next(nbr);
    uip_ds6_nbr_rm(nbr);
  }
#else
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = next_nbr) {
    next_nbr = nbr_table_next(ds6_neighbors, nbr);
    uip_ds6_nbr_rm(nbr);
  }
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
}

UNIT_TEST_REGISTER(test_add, "uip_ds6_nbr_add");
UNIT_TEST(test_add)
{
  uip_ds6_nbr_t *nbr;
  setup();

  UNIT_TEST_BEGIN();

  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);


  nbr = uip_ds6_nbr_add(&dest2, &dest2_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  nbr = uip_ds6_nbr_add(&dest3, &dest3_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  /*
   * The maximum number of neighbor caches is 2, which is defined by
   * UIP_DS6_NBR_CONF_MAX_NB in project-conf.h.
   */
  UNIT_TEST_ASSERT(nbr == NULL);
#else
  UNIT_TEST_ASSERT(nbr != NULL);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_add_ex, "uip_ds6_nbr_add extra");
UNIT_TEST(test_add_ex)
{
  uip_ds6_nbr_t *nbr;
  setup();

  UNIT_TEST_BEGIN();

  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);


  /* add a nbr with the same link-layer address as the previous one */
  nbr = uip_ds6_nbr_add(&dest2, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  nbr = uip_ds6_nbr_lookup(&dest1);
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  UNIT_TEST_ASSERT(nbr != NULL);
#else
  /*
   * The previous one is overwritten by the new one when
   * UIP_DS6_NBR_MULTI_IPV6_ADDRS is disabled.
   */
  UNIT_TEST_ASSERT(nbr == NULL);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  UNIT_TEST_END();
}


UNIT_TEST_REGISTER(test_rm, "uip_ds6_nbr_rm");
UNIT_TEST(test_rm)
{
  uip_ds6_nbr_t *nbr;

  setup();

  UNIT_TEST_BEGIN();

  /* initial state: there is no nbr. */
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 0);

  /* add one nbr */
  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 1);

  /* remove the nbr; we have no nbr again. */
  uip_ds6_nbr_rm(nbr);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_update_lladdr, "uip_ds6_update_lladdr");
UNIT_TEST(test_update_lladdr)
{
  uip_ds6_nbr_t *nbr1, *nbr2;
  int ret;

  setup();

  UNIT_TEST_BEGIN();

  /* add two nbrs */
  nbr1 = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr1 != NULL);

  nbr2 = uip_ds6_nbr_add(&dest2, &dest2_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr2 != NULL);

  /* change the link-layer address of the first nbr to dest2_lladdr */
  ret = uip_ds6_nbr_update_lladdr(&nbr1, &dest2_lladdr);
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  UNIT_TEST_ASSERT(ret == 1); /* succeeded */
#else
  /*
   * We can associate a link-layer address with a single IPv6 address without
   * UIP_DS6_NBR_MULTI_ADDRS enabled.
   */
  UNIT_TEST_ASSERT(ret == 0); /* failed */
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

  UNIT_TEST_END();
}

#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
UNIT_TEST_REGISTER(test_rm_ex, "uip_ds6_nbr_rm extra");
UNIT_TEST(test_rm_ex)
{
  uip_ds6_nbr_t *nbr1, *nbr2;
  uip_ds6_route_t *route;

  setup();

  UNIT_TEST_BEGIN();

  /* initial state: there is no nbr. */
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 0);

  /* add one nbr */
  nbr1 = uip_ds6_nbr_add(&next_hop1, &next_hop1_lladdr, 1,
                         NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr1 != NULL);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 1);

  /* add another nbr */
  nbr2 = uip_ds6_nbr_add(&next_hop2, &next_hop2_lladdr, 1,
                         NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr2 != NULL);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 2);

  /* add route with nbr1 */
  reset_callback_status();
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);
  UNIT_TEST_ASSERT(test_callback_state == ADDED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  /* remove nbr1 which has a route */
  reset_callback_status();
  uip_ds6_nbr_rm(nbr1);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 1);
  UNIT_TEST_ASSERT(test_callback_state == REMOVED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  /* remove nbr2 which doesn't have any route */
  reset_callback_status();
  uip_ds6_nbr_rm(nbr2);
  UNIT_TEST_ASSERT(uip_ds6_nbr_num() == 0);
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_update_lladdr_ex, "uip_ds6_update_lladdr extra");
UNIT_TEST(test_update_lladdr_ex)
{
  uip_ds6_nbr_t *nbr;
  uip_ds6_route_t *route;
  int ret;

  setup();

  UNIT_TEST_BEGIN();

  /* add a nbr with null_linkaddr */
  nbr = uip_ds6_nbr_add(&next_hop1, (const uip_lladdr_t *)&linkaddr_null, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  /* add a route using next_hop1 */
  reset_callback_status();
  route = uip_ds6_route_add(&dest1, 128, &next_hop1);
  UNIT_TEST_ASSERT(route != NULL);
  /* callback is not invoked because the next_hop2 has liknaddr_null */
  UNIT_TEST_ASSERT(test_callback_state == NOT_INVOKED);

  /* update linkaddr of next_hop2 with next_hop1_lladdr */
  reset_callback_status();
  ret = uip_ds6_nbr_update_lladdr(&nbr, &next_hop1_lladdr);
  UNIT_TEST_ASSERT(ret == 1); /* succeeds */
  UNIT_TEST_ASSERT(test_callback_state == ADDED_INVOKED);
  UNIT_TEST_ASSERT(linkaddr_cmp(&test_addr_in_callback,
                                (const linkaddr_t *)&next_hop1_lladdr));

  UNIT_TEST_END();
}
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */

UNIT_TEST_REGISTER(test_get_ll, "uip_ds6_nbr_get_ll");
UNIT_TEST(test_get_ll)
{
  uip_ds6_nbr_t *nbr;
  const uip_lladdr_t *lladdr;

  setup();

  UNIT_TEST_BEGIN();

  /* add one nbr */
  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  /* retrieve lladdr of the previous nbr */
  lladdr = uip_ds6_nbr_get_ll(nbr);
  UNIT_TEST_ASSERT(linkaddr_cmp((const linkaddr_t *)lladdr,
                               (const linkaddr_t *)&dest1_lladdr));

  /* request lladdr with NULL for nbr */
  lladdr = uip_ds6_nbr_get_ll(NULL);
  UNIT_TEST_ASSERT(lladdr == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_get_ipaddr, "uip_ds6_nbr_get_ipaddr");
UNIT_TEST(test_get_ipaddr)
{
  uip_ds6_nbr_t *nbr;
  const uip_ipaddr_t *ipaddr;
  setup();

  UNIT_TEST_BEGIN();

  /* add one nbr */
  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  /* retrieve ipaddr of the previous nbr */
  ipaddr = uip_ds6_nbr_get_ipaddr(nbr);
  UNIT_TEST_ASSERT(uip_ipaddr_cmp(ipaddr, &dest1));

  /* request ipaddr with NULL for nbr */
  ipaddr = uip_ds6_nbr_get_ipaddr(NULL);
  UNIT_TEST_ASSERT(ipaddr == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_lookup, "uip_ds6_nbr_lookup");
UNIT_TEST(test_lookup)
{
  uip_ds6_nbr_t *nbr, *ret;

  setup();

  UNIT_TEST_BEGIN();

  /* add one nbr */
  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  /* retrieve nbr by dest1 */
  ret = uip_ds6_nbr_lookup(&dest1);
  UNIT_TEST_ASSERT(ret == nbr);

  /* retrieve nbr by dest2 that is not added */
  ret = uip_ds6_nbr_lookup(&dest2);
  UNIT_TEST_ASSERT(ret == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_lladdr_from_ipaddr, "uip_ds6_nbr_lladdr_from_ipaddr");
UNIT_TEST(test_lladdr_from_ipaddr)
{
  uip_ds6_nbr_t *nbr;
  const uip_lladdr_t *lladdr;

  setup();

  UNIT_TEST_BEGIN();

  /* add one nbr */
  nbr = uip_ds6_nbr_add(&dest1, &dest1_lladdr, 1,
                        NBR_INCOMPLETE, NBR_TABLE_REASON_UNDEFINED, NULL);
  UNIT_TEST_ASSERT(nbr != NULL);

  /* retrieve lladdr by dest1 */
  lladdr = uip_ds6_nbr_lladdr_from_ipaddr(&dest1);
  UNIT_TEST_ASSERT(linkaddr_cmp((const linkaddr_t *)lladdr,
                                (const linkaddr_t *)&dest1_lladdr));

  /* retrieve lladdr by dest2 that is not added */
  lladdr = uip_ds6_nbr_lladdr_from_ipaddr(&dest2);
  UNIT_TEST_ASSERT(lladdr == NULL);

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
  UNIT_TEST_RUN(test_update_lladdr);
#if UIP_DS6_NBR_MULTI_IPV6_ADDRS
  UNIT_TEST_RUN(test_rm_ex);
  UNIT_TEST_RUN(test_update_lladdr_ex);
#endif /* UIP_DS6_NBR_MULTI_IPV6_ADDRS */
  UNIT_TEST_RUN(test_get_ll);
  UNIT_TEST_RUN(test_get_ipaddr);
  UNIT_TEST_RUN(test_lookup);
  UNIT_TEST_RUN(test_lladdr_from_ipaddr);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
