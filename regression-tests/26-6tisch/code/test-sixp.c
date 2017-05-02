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

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/sixtop/sixp-nbr.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"

#include "unit-test.h"
#include "common.h"

#define UNKNOWN_SF_SFID 0
#define TEST_SF_SFID   0xf1

static linkaddr_t peer_addr;
static uint8_t test_sf_input_is_called = 0;

static void
input(sixp_pkt_type_t type,sixp_pkt_code_t code, const uint8_t *body,
      uint16_t body_len, const linkaddr_t *peer_addr)
{
  test_sf_input_is_called = 1;
}

static const sixtop_sf_t test_sf = {
  TEST_SF_SFID,
  0,
  NULL,
  input,
  NULL
};

PROCESS(test_process, "6top protocol APIs test");
AUTOSTART_PROCESSES(&test_process);

static void
test_setup(void)
{
  test_mac_driver.init();
  sixtop_init();
  packetbuf_clear();
  memset(&peer_addr, 0, sizeof(peer_addr));
  sixtop_add_sf(&test_sf);
  test_sf_input_is_called = 0;
}

UNIT_TEST_REGISTER(test_input_no_sf,
                   "sixp_input(no_sf)");
UNIT_TEST(test_input_no_sf)
{
  uint32_t body;
  uint8_t *p;

  UNIT_TEST_BEGIN();
  test_setup();

  memset(&body, 0, sizeof(body));
  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   UNKNOWN_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   NULL) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);

  p = packetbuf_hdrptr();

  /* length */
  UNIT_TEST_ASSERT(packetbuf_totlen() == 11);

  /* Termination 1 IE */
  UNIT_TEST_ASSERT(p[0] == 0x00);
  UNIT_TEST_ASSERT(p[1] == 0x3f);

  /* IETF IE */
  UNIT_TEST_ASSERT(p[2] == 0x05);
  UNIT_TEST_ASSERT(p[3] == 0x98);

  /* 6top IE */
  UNIT_TEST_ASSERT(p[4] == 0x00);
  UNIT_TEST_ASSERT(p[5] == 0x11);
  UNIT_TEST_ASSERT(p[6] == 0x08);
  UNIT_TEST_ASSERT(p[7] == UNKNOWN_SF_SFID);
  UNIT_TEST_ASSERT(p[8] == 0x0a);

  /* Payload Termination IE */
  UNIT_TEST_ASSERT(p[9] == 0x00);
  UNIT_TEST_ASSERT(p[10] == 0xf8);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);
  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_input_busy,
                   "sixp_input(busy)");
UNIT_TEST(test_input_busy)
{
  uint8_t *p;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  /* send a request to the peer first */
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_trans_find(&peer_addr) == NULL);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                               TEST_SF_SFID, NULL, 0, &peer_addr,
                               NULL, NULL, 0) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);
  UNIT_TEST_ASSERT(sixp_trans_find(&peer_addr) != NULL);

  test_mac_driver.init(); /* clear test_mac_send_is_called status */

  memset(&body, 0, sizeof(body));
  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   NULL) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);

  p = packetbuf_hdrptr();

  /* length */
  UNIT_TEST_ASSERT(packetbuf_totlen() == 11);

  /* Termination 1 IE */
  UNIT_TEST_ASSERT(p[0] == 0x00);
  UNIT_TEST_ASSERT(p[1] == 0x3f);

  /* IETF IE */
  UNIT_TEST_ASSERT(p[2] == 0x05);
  UNIT_TEST_ASSERT(p[3] == 0x98);

  /* 6top IE */
  UNIT_TEST_ASSERT(p[4] == 0x00);
  UNIT_TEST_ASSERT(p[5] == 0x11);
  UNIT_TEST_ASSERT(p[6] == 0x0a);
  UNIT_TEST_ASSERT(p[7] == TEST_SF_SFID);
  UNIT_TEST_ASSERT(p[8] == 0x0a);

  /* Payload Termination IE */
  UNIT_TEST_ASSERT(p[9] == 0x00);
  UNIT_TEST_ASSERT(p[10] == 0xf8);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);
  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_input_no_memory,
                   "sixp_input(no_memory)");
UNIT_TEST(test_input_no_memory)
{
  uint32_t body;
  sixp_pkt_t pkt;
  uint8_t *p;
  linkaddr_t addr;

  UNIT_TEST_BEGIN();
  test_setup();

  memset(&body, 0, sizeof(body));
  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                    &pkt) == 0);
  memset(&addr, 0, sizeof(addr));
  addr.u8[0] = 1;
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &addr) != NULL);
  addr.u8[0] = 2;
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &addr) != NULL);
  addr.u8[0] = 3;
  /* no memory left for another transaction */
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &addr) == NULL);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);

  p = packetbuf_hdrptr();

  /* length */
  UNIT_TEST_ASSERT(packetbuf_totlen() == 11);

  /* Termination 1 IE */
  UNIT_TEST_ASSERT(p[0] == 0x00);
  UNIT_TEST_ASSERT(p[1] == 0x3f);

  /* IETF IE */
  UNIT_TEST_ASSERT(p[2] == 0x05);
  UNIT_TEST_ASSERT(p[3] == 0x98);

  /* 6top IE */
  UNIT_TEST_ASSERT(p[4] == 0x00);
  UNIT_TEST_ASSERT(p[5] == 0x11);
  UNIT_TEST_ASSERT(p[6] == 0x0b);
  UNIT_TEST_ASSERT(p[7] == TEST_SF_SFID);
  UNIT_TEST_ASSERT(p[8] == 0x0a);

  /* Payload Termination IE */
  UNIT_TEST_ASSERT(p[9] == 0x00);
  UNIT_TEST_ASSERT(p[10] == 0xf8);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);
  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_input_schedule_generation,
                   "sixp_input(schedule_generation)");
UNIT_TEST(test_input_schedule_generation)
{
  sixp_nbr_t *nbr;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();
  memset(&body, 0, sizeof(body));

  UNIT_TEST_ASSERT((nbr = sixp_nbr_alloc(&peer_addr)) != NULL);
  /* nbr has GTX 0 and GRX 0 now */

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 1, 1,
                                   (const uint8_t *)&body, sizeof(body),
                                   NULL) == 0);

  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);
  UNIT_TEST_ASSERT(test_sf_input_is_called == 0);

  UNIT_TEST_ASSERT(sixp_nbr_advance_gtx(nbr) == 0);
  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);
  UNIT_TEST_ASSERT(test_sf_input_is_called == 0);

  UNIT_TEST_ASSERT(sixp_nbr_advance_grx(nbr) == 0);
  sixp_input(packetbuf_hdrptr(), packetbuf_totlen(), &peer_addr);
  UNIT_TEST_ASSERT(test_sf_input_is_called == 1);
  UNIT_TEST_ASSERT((trans = sixp_trans_find(&peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_request_1,
                   "sixp_output(request_1)");
UNIT_TEST(test_output_request_1)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();

  test_setup();
  memset(&body, 0, sizeof(body));

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  memset(&body, 0, sizeof(body));

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                               TEST_SF_SFID, (uint8_t *)&body, sizeof(body),
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_request_2,
                   "sixp_output(request_2)");
UNIT_TEST(test_output_request_2)
{
  uint32_t body;
  UNIT_TEST_BEGIN();

  test_setup();
  memset(&body, 0, sizeof(body));

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                               TEST_SF_SFID, (uint8_t *)&body, sizeof(body),
                               &peer_addr, NULL, NULL, 0) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_response_1,
                   "sixp_output(response_1)");
UNIT_TEST(test_output_response_1)
{
  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_response_2,
                   "sixp_output(response_2)");
UNIT_TEST(test_output_response_2)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_response_3,
                   "sixp_output(response_3)");
UNIT_TEST(test_output_response_3)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_RECEIVED) == 0);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_response_4,
                   "sixp_output(response_4)");
UNIT_TEST(test_output_response_4)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_confirmation_1,
                   "sixp_output(confirmation_1)");
UNIT_TEST(test_output_confirmation_1)
{
  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_confirmation_2,
                   "sixp_output(confirmation_2)");
UNIT_TEST(test_output_confirmation_2)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_confirmation_3,
                   "sixp_output(confirmation_3)");
UNIT_TEST(test_output_confirmation_3)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == 0);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_output_confirmation_4,
                   "sixp_output(confirmation_4)");
UNIT_TEST(test_output_confirmation_4)
{
  sixp_pkt_t pkt;
  sixp_trans_t *trans;
  uint32_t body;

  UNIT_TEST_BEGIN();
  test_setup();

  UNIT_TEST_ASSERT(sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                                   (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                                   TEST_SF_SFID, 10, 0, 0,
                                   (const uint8_t *)&body, sizeof(body),
                                   &pkt) == 0);
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == 0);

  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);
  UNIT_TEST_ASSERT(sixp_output(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               TEST_SF_SFID, NULL, 0,
                               &peer_addr, NULL, NULL, 0) == -1);
  UNIT_TEST_ASSERT(test_mac_send_function_is_called() == 0);

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  /* wait until the sixtop module gets ready */
  etimer_set(&et, CLOCK_SECOND);
  tschmac_driver.init();
  tschmac_driver.on();
  tsch_set_coordinator(1);
  while(tsch_is_associated == 0) {
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(test_input_no_sf);
  UNIT_TEST_RUN(test_input_busy);
  UNIT_TEST_RUN(test_input_no_memory);
  UNIT_TEST_RUN(test_input_schedule_generation);

  UNIT_TEST_RUN(test_output_request_1);
  UNIT_TEST_RUN(test_output_request_2);
  UNIT_TEST_RUN(test_output_response_1);
  UNIT_TEST_RUN(test_output_response_2);
  UNIT_TEST_RUN(test_output_response_3);
  UNIT_TEST_RUN(test_output_response_4);
  UNIT_TEST_RUN(test_output_confirmation_1);
  UNIT_TEST_RUN(test_output_confirmation_2);
  UNIT_TEST_RUN(test_output_confirmation_3);
  UNIT_TEST_RUN(test_output_confirmation_4);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
