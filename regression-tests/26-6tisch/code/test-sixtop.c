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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "lib/assert.h"

#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixtop-conf.h"

#include "unit-test.h"
#include "common.h"

#if SIXTOP_MAX_SCHEDULING_FUNCTIONS != 2
#error SIXTOP_CONF_MAX_SCHEDULING_FUNCTIONS must be set with 2 for this test
#endif

#define TEST_SF1_SFID 0xf1
#define TEST_SF2_SFID 0xf2
#define TEST_SF3_SFID 0xf3

static uint8_t sf_init_vector;
static void test_sf1_init(void);
static void test_sf2_init(void);
static void test_sf3_init(void);
static void reset_sf_init_vector(void);

static const sixtop_sf_t test_sf1 = {
  TEST_SF1_SFID,
  0,
  test_sf1_init,
  NULL,
  NULL
};

static const sixtop_sf_t test_sf2 = {
  TEST_SF2_SFID,
  0,
  test_sf2_init,
  NULL,
  NULL
};

static const sixtop_sf_t test_sf3 = {
  TEST_SF3_SFID,
  0,
  test_sf3_init,
  NULL,
  NULL
};

static void
test_sf1_init(void)
{
  sf_init_vector |= TEST_SF1_SFID;
}

static void
test_sf2_init(void)
{
  sf_init_vector |= TEST_SF2_SFID;
}

static void
test_sf3_init(void)
{
  sf_init_vector |= TEST_SF3_SFID;
}

void
reset_sf_init_vector(void)
{
  sf_init_vector = 0;
}

PROCESS(test_process, "6top module APIs test");
AUTOSTART_PROCESSES(&test_process);

UNIT_TEST_REGISTER(test_add_sf,
                   "sixtop_add_sf() should add a SF");
UNIT_TEST(test_add_sf)
{
  UNIT_TEST_BEGIN();

  /* confirm any of test_sf{1,2,3} is not added yet. */
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF1_SFID) == NULL);
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF2_SFID) == NULL);

  /* add test_sf1 */
  UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf1) == 0);
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF1_SFID) == &test_sf1);

  /* add test_sf2 */
  UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf2) == 0);
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF2_SFID) == &test_sf2);

  /*
   * add test_sf3; should fail because of lack of memory.
   * SIXTOP_MAX_SCHEDULING_FUNCTIONS is set with two.
   */
  UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf3) == -1);
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF3_SFID) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_find_sf,
                   "sixtop_find_sf() should find a SF which has been added");
UNIT_TEST(test_find_sf)
{
  UNIT_TEST_BEGIN();

  /*
   * See test_add_sf(); sixtop_find_sf() is tested together with sixtop_add_sf()
   * in it.
   */

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_init,
                   "sixtop_init() should remove all SFs in the 6top module");
UNIT_TEST(test_init)
{
  UNIT_TEST_BEGIN();

  /* set up a precondition where some SFs has been added */
  if(sixtop_find_sf(TEST_SF1_SFID) == NULL) {
    UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf1) == 0);
    UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF1_SFID) != NULL);
  }

  if(sixtop_find_sf(TEST_SF2_SFID) == NULL) {
    UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf2) == 0);
    UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF2_SFID) != NULL);
  }

  /* call sixtop_init() which removes all the SFs in the module */
  sixtop_init();
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF1_SFID) == NULL);
  UNIT_TEST_ASSERT(sixtop_find_sf(TEST_SF2_SFID) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output,
                   "test sixtop_output()");
UNIT_TEST(test_output)
{
  linkaddr_t peer_addr;
  uint8_t *p;
  UNIT_TEST_BEGIN();

  packetbuf_clear();
  UNIT_TEST_ASSERT(packetbuf_hdralloc(1) > 0);
  p = packetbuf_hdrptr();
  p[0] = 0xca; // header
  p = packetbuf_dataptr();
  p[0] = 0xfe; // data
  packetbuf_set_datalen(1);

  sixtop_output(&peer_addr, NULL, NULL);

  p = packetbuf_hdrptr();

  /* Length */
  UNIT_TEST_ASSERT(packetbuf_totlen() == 9);

  /* Termination 1 IE */
  UNIT_TEST_ASSERT(p[0] == 0x00);
  UNIT_TEST_ASSERT(p[1] == 0x3f);

  /* IETF IE */
  UNIT_TEST_ASSERT(p[2] == 0x03);
  UNIT_TEST_ASSERT(p[3] == 0x98);

  /* 6top IE Sub-ID */
  UNIT_TEST_ASSERT(p[4] == 0x00);
  /* test data set above */
  UNIT_TEST_ASSERT(p[5] == 0xca);
  UNIT_TEST_ASSERT(p[6] == 0xfe);

  /* Payload Termination IE */
  UNIT_TEST_ASSERT(p[7] == 0x00);
  UNIT_TEST_ASSERT(p[8] == 0xf8);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_init_sf,
                   "sixtop_init_sf() should invoke every SF->init()");
UNIT_TEST(test_init_sf)
{
  UNIT_TEST_BEGIN();

  /* remove all the SFs in the 6top module */
  sixtop_init();

  /* add test_sf1 */
  UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf1) == 0);
  reset_sf_init_vector();
  sixtop_init_sf();
  UNIT_TEST_ASSERT(sf_init_vector == TEST_SF1_SFID);

  /* add test_sf2 */
  UNIT_TEST_ASSERT(sixtop_add_sf(&test_sf2) == 0);
  reset_sf_init_vector();
  sixtop_init_sf();
  UNIT_TEST_ASSERT(sf_init_vector == (TEST_SF1_SFID | TEST_SF2_SFID));

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(test_add_sf);
  UNIT_TEST_RUN(test_find_sf);
  UNIT_TEST_RUN(test_init);
  UNIT_TEST_RUN(test_init_sf);
  UNIT_TEST_RUN(test_output);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
