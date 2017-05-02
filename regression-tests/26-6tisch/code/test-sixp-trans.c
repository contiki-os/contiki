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

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"

#include "unit-test.h"
#include "common.h"

#define TEST_SF_SFID          0xfe
#define TEST_SF_TIMEOUT_VALUE (5 * CLOCK_SECOND)

static void test_setup(void);
static void timeout_handler(sixp_pkt_cmd_t cmd, const linkaddr_t *peer_addr);
static void sent_callback(void *arg, uint16_t arg_len,
                          const linkaddr_t *dest_addr,
                          sixp_output_status_t status);
static uint8_t timeout_handler_is_called;
static uint8_t sent_callback_is_called;


PROCESS(test_process, "6P transaction APIs test");
AUTOSTART_PROCESSES(&test_process);

static const sixtop_sf_t test_sf = {
  TEST_SF_SFID,
  TEST_SF_TIMEOUT_VALUE,
  NULL,
  NULL,
  timeout_handler
};

static void
test_setup(void)
{
  sixtop_init();
  assert(sixtop_add_sf(&test_sf) == 0);
  timeout_handler_is_called = 0;
  sent_callback_is_called = 0;
}

static void
timeout_handler(sixp_pkt_cmd_t cmd, const linkaddr_t *peer_addr)
{
  timeout_handler_is_called = 1;
}

static void
sent_callback(void *arg, uint16_t arg_len, const linkaddr_t *dest_addr,
              sixp_output_status_t status)
{
  sent_callback_is_called = 1;
}

UNIT_TEST_REGISTER(test_alloc_2_step_trans,
                   "test sixp_trans_alloc(2_step_transaction)");
UNIT_TEST(test_alloc_2_step_trans)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 8; /* Metadata, CellOptions, NumCells, and CellList */
  peer_addr.u8[0] = 0;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_cmd(trans) == SIXP_PKT_CMD_ADD);
  UNIT_TEST_ASSERT(sixp_trans_get_seqno(trans) == 10);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_2_STEP);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  /* allocate a transaction for the clear request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR;
  pkt.seqno = 0;
  pkt.body = req_body;
  pkt.body_len = 2; /* only Metadata */
  peer_addr.u8[0] = 1;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_seqno(trans) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_cmd(trans) == SIXP_PKT_CMD_CLEAR);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_2_STEP);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_alloc_3_step_trans,
                   "test sixp_trans_alloc(3_step_transaction)");
UNIT_TEST(test_alloc_3_step_trans)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */
  peer_addr.u8[0] = 0;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_seqno(trans) == 7);
  UNIT_TEST_ASSERT(sixp_trans_get_cmd(trans) == SIXP_PKT_CMD_ADD);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_3_STEP);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE;
  pkt.seqno = 5;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */
  peer_addr.u8[0] = 1;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_seqno(trans) == 5);
  UNIT_TEST_ASSERT(sixp_trans_get_cmd(trans) == SIXP_PKT_CMD_DELETE);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_3_STEP);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_alloc_duplicate_peer,
                   "test sixp_trans_alloc(duplicate_peer)");
UNIT_TEST(test_alloc_duplicate_peer)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */
  peer_addr.u8[0] = 0;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(sixp_trans_get_seqno(trans) == 7);
  UNIT_TEST_ASSERT(sixp_trans_get_cmd(trans) == SIXP_PKT_CMD_ADD);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_3_STEP);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE;
  pkt.seqno = 5;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */
  peer_addr.u8[0] = 0;  /* the same address used above to allocate a trans */

  /* a new transaction should not be allocated */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_alloc_lack_of_memory,
                   "test sixp_trans_alloc(lack_of_memory)");
UNIT_TEST(test_alloc_lack_of_memory)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  uint8_t req_body[8];

  /* SIXTOP_CONF_MAX_TRANSACTIONS is set with 2; see project-conf.h */

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */

  peer_addr.u8[0] = 0;
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &peer_addr) != NULL);

  peer_addr.u8[0] = 1;
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &peer_addr) != NULL);

  /* allocation should fail because of lack of memory */
  peer_addr.u8[0] = 2;
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, &peer_addr) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_alloc_null_pkt,
                   "test sixp_trans_alloc(null_pkt)");
UNIT_TEST(test_alloc_null_pkt)
{
  linkaddr_t peer_addr;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  peer_addr.u8[0] = 0;
  /* allocation should fail because of NULL specified as the 1st arg */
  UNIT_TEST_ASSERT(sixp_trans_alloc(NULL, &peer_addr) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_alloc_null_peer_addr,
                   "test sixp_trans_alloc(null_peer_addr)");
UNIT_TEST(test_alloc_null_peer_addr)
{
  sixp_pkt_t pkt;
  uint8_t req_body[8];

  /* SIXTOP_CONF_6P_MAX_TRANSACTIONS is set with 2; see project-conf.h */

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, NumCells */

  /* allocation should fail because of NULL specified as the 2nd arg */
  UNIT_TEST_ASSERT(sixp_trans_alloc(&pkt, NULL) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_find,
                   "test sixp_trans_find()");
UNIT_TEST(test_find)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans, *returned_trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 2; /* Metadata */
  peer_addr.u8[0] = 0;

  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  /* find the trans */
  UNIT_TEST_ASSERT((returned_trans = sixp_trans_find(&peer_addr)) == trans);

  /* no trans by another peer_addr */
  peer_addr.u8[0] = 1;
  UNIT_TEST_ASSERT(sixp_trans_find(&peer_addr) == NULL);

  /* no trans by null peer_addr */
  UNIT_TEST_ASSERT(sixp_trans_find(NULL) == NULL);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_callback,
                   "test sixp_trans_{set,invoke}_callback()");
UNIT_TEST(test_callback)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR;
  pkt.seqno = 7;
  pkt.body = req_body;
  pkt.body_len = 2; /* Metadata */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(sent_callback_is_called == 0);
  sixp_trans_invoke_callback(trans, SIXP_OUTPUT_STATUS_SUCCESS);
  UNIT_TEST_ASSERT(sent_callback_is_called == 0);

  sixp_trans_set_callback(trans, sent_callback, NULL, 0);
  sixp_trans_invoke_callback(trans, SIXP_OUTPUT_STATUS_SUCCESS);
  UNIT_TEST_ASSERT(sent_callback_is_called == 1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_2_step_initiator,
                   "test sixp_trans_{transit,get}_state(2-step-initiator)");
UNIT_TEST(test_state_transition_2_step_initiator)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 8; /* Metadata, CellOptions, NumCells, and CellList */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_RECEIVED);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_TERMINATING);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_2_step_responder,
                   "test sixp_trans_{transit,get}_state(2-step-responder)");
UNIT_TEST(test_state_transition_2_step_responder)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 8; /* Metadata, CellOptions, NumCells, and CellList */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_TERMINATING);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_3_step_initiator,
                   "test sixp_trans_{transit,get}_state(3-step-initiator)");
UNIT_TEST(test_state_transition_3_step_initiator)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, and NumCells */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_3_STEP);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_RECEIVED);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_CONFIRMATION_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_CONFIRMATION_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_TERMINATING);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_3_step_responder,
                   "test sixp_trans_{transit,get}_state(3-step-responder)");
UNIT_TEST(test_state_transition_3_step_responder)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, and NumCells */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(sixp_trans_get_mode(trans) == SIXP_TRANS_MODE_3_STEP);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_REQUEST_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_RESPONSE_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_TERMINATING);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_invalid_transition_initiator,
                   "test sixp_trans_{transit,get}_state(invalid_transition_i)");
UNIT_TEST(test_state_transition_invalid_transition_initiator)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, and NumCells */
  /* 3-step */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  /* from INIT */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  /* from REQUEST_SENT */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  /* from RESPONSE_RECEIVE */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  /* from CONFIRMATION_SENT */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_SENT);

  pkt.body_len = 8; /* Metadata, CellOptions, and NumCells CellList */
  peer_addr.u8[0] = 1;
  /* 2-step */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                       SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                       SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_TERMINATING);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_state_transition_invalid_transition_responder,
                   "test sixp_trans_{transit,get}_state(invalid_transition_r)");
UNIT_TEST(test_state_transition_invalid_transition_responder)
{
  sixp_pkt_t pkt;
  linkaddr_t peer_addr;
  sixp_trans_t *trans;
  uint8_t req_body[8];

  UNIT_TEST_BEGIN();

  test_setup();

  /* allocate a transaction for the add request */
  memset(&pkt, 0, sizeof(pkt));
  memset(&peer_addr, 0, sizeof(peer_addr));
  memset(req_body, 0, sizeof(req_body));

  pkt.sfid = TEST_SF_SFID;
  pkt.type = SIXP_PKT_TYPE_REQUEST;
  pkt.code = (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD;
  pkt.seqno = 10;
  pkt.body = req_body;
  pkt.body_len = 4; /* Metadata, CellOptions, and NumCells CellList */
  /* 3-step */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);

  /* from INIT */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  /* from REQUEST_RECEIVED */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  /* from RESPONSE_SENT */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == 0);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(sixp_trans_get_state(trans) ==
                   SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  /* from CONFIRMATION_RECEIVED */
  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_UNAVAILABLE) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_INIT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_REQUEST_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_RESPONSE_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_SENT) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                             SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);

  pkt.body_len = 8; /* Metadata, CellOptions, and NumCells CellList */
  peer_addr.u8[0] = 1;
  /* 2-step */
  UNIT_TEST_ASSERT((trans = sixp_trans_alloc(&pkt, &peer_addr)) != NULL);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_INIT);

  UNIT_TEST_ASSERT(sixp_trans_transit_state(trans,
                                      SIXP_TRANS_STATE_REQUEST_RECEIVED) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_REQUEST_RECEIVED);

  UNIT_TEST_ASSERT(sixp_trans_transit_state(trans,
                                      SIXP_TRANS_STATE_RESPONSE_SENT) == 0);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                       SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) == -1);
  UNIT_TEST_ASSERT(
    sixp_trans_get_state(trans) == SIXP_TRANS_STATE_RESPONSE_SENT);

  UNIT_TEST_ASSERT(
    sixp_trans_transit_state(trans,
                       SIXP_TRANS_STATE_TERMINATING) == 0);
  UNIT_TEST_ASSERT(
      sixp_trans_get_state(trans) == SIXP_TRANS_STATE_TERMINATING);

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

  /* sixp_trans_alloc() */
  UNIT_TEST_RUN(test_alloc_2_step_trans);
  UNIT_TEST_RUN(test_alloc_3_step_trans);
  UNIT_TEST_RUN(test_alloc_duplicate_peer);
  UNIT_TEST_RUN(test_alloc_lack_of_memory);
  UNIT_TEST_RUN(test_alloc_null_pkt);
  UNIT_TEST_RUN(test_alloc_null_peer_addr);

  /* sixp_trans_find() */
  UNIT_TEST_RUN(test_find);

  /* sixp_set_callback() & sixp_invoke_callback() */
  UNIT_TEST_RUN(test_callback);

  /* sixp_trans_transit_state() and sixp_trans_get_state() */
  UNIT_TEST_RUN(test_state_transition_2_step_initiator);
  UNIT_TEST_RUN(test_state_transition_2_step_responder);
  UNIT_TEST_RUN(test_state_transition_3_step_initiator);
  UNIT_TEST_RUN(test_state_transition_3_step_responder);
  UNIT_TEST_RUN(test_state_transition_invalid_transition_initiator);
  UNIT_TEST_RUN(test_state_transition_invalid_transition_responder);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
