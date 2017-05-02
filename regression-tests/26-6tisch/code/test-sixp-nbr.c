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
 * ARISING IN ANY STEP OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "lib/assert.h"

#include "net/mac/tsch/sixtop/sixp-nbr.h"

#include "unit-test.h"
#include "common.h"

static linkaddr_t peer_addr_1;
static linkaddr_t peer_addr_2;
static linkaddr_t peer_addr_3;

PROCESS(test_process, "6P neighbor APIs test");
AUTOSTART_PROCESSES(&test_process);

static void
test_setup(void)
{
  sixp_nbr_init();

  memset(&peer_addr_1, 0, sizeof(peer_addr_1));
  peer_addr_1.u8[0] = 1;

  memset(&peer_addr_2, 0, sizeof(peer_addr_2));
  peer_addr_2.u8[0] = 2;

  memset(&peer_addr_3, 0, sizeof(peer_addr_3));
  peer_addr_3.u8[0] = 3;
}

UNIT_TEST_REGISTER(test_alloc_and_free,
                   "test sixp_nbr_{alloc,free}");
UNIT_TEST(test_alloc_and_free)
{
  sixp_nbr_t *nbr;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_1) == NULL);
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_2) == NULL);

  /* a valid allocation */
  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_1)) != NULL);
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_1) == nbr);

  /* another allocation */
  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_2)) != NULL);
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_2) == nbr);

  /* no available memory because NBR_TABLE_CONF_MAX_NEIGHBORS == 2 */
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_3) == NULL);
  UNIT_TEST_ASSERT(sixp_nbr_alloc(&peer_addr_3) == NULL);
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_3) == NULL);

  /* cannot allocate a duplicate nbr */
  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_1)) == NULL);
  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_2)) == NULL);

  /* remove allocated nbrs */
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_1) != NULL);
  sixp_nbr_free(sixp_nbr_find(&peer_addr_1));
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_1) == NULL);

  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_2) != NULL);
  sixp_nbr_free(sixp_nbr_find(&peer_addr_2));
  UNIT_TEST_ASSERT(sixp_nbr_find(&peer_addr_2) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_gtx_management,
                   "test GTX Management");
UNIT_TEST(test_gtx_management)
{
  sixp_nbr_t *nbr;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_1)) != NULL);
  UNIT_TEST_ASSERT(sixp_nbr_get_gtx(nbr) == 0);

  UNIT_TEST_ASSERT(sixp_nbr_advance_gtx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_gtx(nbr) == 0x01);

  UNIT_TEST_ASSERT(sixp_nbr_advance_gtx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_gtx(nbr) == 0x02);

  UNIT_TEST_ASSERT(sixp_nbr_advance_gtx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_gtx(nbr) == 0x01);

  UNIT_TEST_ASSERT(sixp_nbr_advance_gtx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_gtx(nbr) == 0x02);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_grx_management,
                   "test GRX Management");
UNIT_TEST(test_grx_management)
{
  sixp_nbr_t *nbr;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_1)) != NULL);
  UNIT_TEST_ASSERT(sixp_nbr_get_grx(nbr) == 0);

  UNIT_TEST_ASSERT(sixp_nbr_advance_grx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_grx(nbr) == 0x01);

  UNIT_TEST_ASSERT(sixp_nbr_advance_grx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_grx(nbr) == 0x02);

  UNIT_TEST_ASSERT(sixp_nbr_advance_grx(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_grx(nbr) == 0x01);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_next_seqno,
                   "test next_seqno operation");
UNIT_TEST(test_next_seqno)
{
  sixp_nbr_t *nbr;
  UNIT_TEST_BEGIN();

  test_setup();
  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr_1)) != NULL);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 0);

  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 1);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 2);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 3);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 4);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 5);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 6);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 7);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 8);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 9);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 10);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 11);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 12);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 13);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 14);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 15);
  UNIT_TEST_ASSERT(sixp_nbr_increment_next_seqno(nbr) == 0);
  UNIT_TEST_ASSERT(sixp_nbr_get_next_seqno(nbr) == 0);

  UNIT_TEST_END();
}


PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();
  printf("Run unit-test\n");
  printf("---\n");

  /* alloc / free */
  UNIT_TEST_RUN(test_alloc_and_free);

  /* GTX */
  UNIT_TEST_RUN(test_gtx_management);
  /* GRX */
  UNIT_TEST_RUN(test_grx_management);

  /* next sequence number */
  UNIT_TEST_RUN(test_next_seqno);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
