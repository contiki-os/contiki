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

#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"

#include "unit-test.h"
#include "common.h"

static uint8_t buf[TSCH_PACKET_MAX_LEN];
static uint8_t ref_data[TSCH_PACKET_MAX_LEN];

PROCESS(test_process, "6P packet APIs test");
AUTOSTART_PROCESSES(&test_process);

UNIT_TEST_REGISTER(test_set_get_metadata_add_req,
                   "test sixp_pkt_{set,get}_metadata(add_req)");
UNIT_TEST(test_set_get_metadata_add_req)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[0] = 0xAB;
  ref_data[1] = 0xCD;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0xAB);
  UNIT_TEST_ASSERT(p_result[1] == 0xCD);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_delete_req,
                   "test sixp_pkt_{set,get}_metadata(delete_req)");
UNIT_TEST(test_set_get_metadata_delete_req)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[0] = 0xAB;
  ref_data[1] = 0xCD;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0xAB);
  UNIT_TEST_ASSERT(p_result[1] == 0xCD);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_status_req,
                   "test sixp_pkt_{set,get}_metadata(status_req)");
UNIT_TEST(test_set_get_metadata_status_req)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[0] = 0xAB;
  ref_data[1] = 0xCD;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0xAB);
  UNIT_TEST_ASSERT(p_result[1] == 0xCD);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_list_req,
                   "test sixp_pkt_{set,get}_metadata(list_req)");
UNIT_TEST(test_set_get_metadata_list_req)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[0] = 0xAB;
  ref_data[1] = 0xCD;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0xAB);
  UNIT_TEST_ASSERT(p_result[1] == 0xCD);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_clear_req,
                   "test sixp_pkt_{set,get}_metadata(clear_req)");
UNIT_TEST(test_set_get_metadata_clear_req)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[0] = 0xAB;
  ref_data[1] = 0xCD;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0xAB);
  UNIT_TEST_ASSERT(p_result[1] == 0xCD);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_success_res,
                   "test sixp_pkt_{set,get}_metadata(success_res)");
UNIT_TEST(test_set_get_metadata_success_res)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_success_conf,
                   "test sixp_pkt_{set,get}_metadata(success_conf)");
UNIT_TEST(test_set_get_metadata_success_conf)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_err_res,
                   "test sixp_pkt_{set,get}_metadata(err_res)");
UNIT_TEST(test_set_get_metadata_err_res)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Err Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_metadata_err_conf,
                   "test sixp_pkt_{set,get}_metadata(err_conf)");
UNIT_TEST(test_set_get_metadata_err_conf)
{
  sixp_pkt_metadata_t testdata, result;
  uint8_t *ptr = (uint8_t *)&testdata;
  uint8_t *p_result = (uint8_t *)&result;

  ptr[0] = 0xAB;
  ptr[1] = 0xCD;

  UNIT_TEST_BEGIN();

  /* Err Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_metadata(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_metadata(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(p_result[0] == 0x00);
  UNIT_TEST_ASSERT(p_result[1] == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_add_req,
                   "test sixp_pkt_{set,get}_cell_options(add_req)");
UNIT_TEST(test_set_get_cell_options_add_req)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                              testdata,
                              buf, sizeof(buf)) == 0);
  ref_data[2] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                              &result,
                              buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_delete_req,
                   "test sixp_pkt_{set,get}_cell_options(delete_req)");
UNIT_TEST(test_set_get_cell_options_delete_req)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                              testdata,
                              buf, sizeof(buf)) == 0);
  ref_data[2] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                              &result,
                              buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_status_req,
                   "test sixp_pkt_{set,get}_cell_options(status_req)");
UNIT_TEST(test_set_get_cell_options_status_req)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                              testdata,
                              buf, sizeof(buf)) == 0);
  ref_data[2] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                              &result,
                              buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_list_req,
                   "test sixp_pkt_{set,get}_cell_options(list_req)");
UNIT_TEST(test_set_get_cell_options_list_req)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                              testdata,
                              buf, sizeof(buf)) == 0);
  ref_data[2] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                              &result,
                              buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_clear_req,
                   "test sixp_pkt_{set,get}_cell_options(clear_req)");
UNIT_TEST(test_set_get_cell_options_clear_req)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                              testdata,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_REQUEST,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                              &result,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_success_res,
                   "test sixp_pkt_{set,get}_cell_options(success_res)");
UNIT_TEST(test_set_get_cell_options_success_res)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_RESPONSE,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                              testdata,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_RESPONSE,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                              &result,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_success_conf,
                   "test sixp_pkt_{set,get}_cell_options(success_conf)");
UNIT_TEST(test_set_get_cell_options_success_conf)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_CONFIRMATION,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                              testdata,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_CONFIRMATION,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                              &result,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_err_res,
                   "test sixp_pkt_{set,get}_cell_options(err_res)");
UNIT_TEST(test_set_get_cell_options_err_res)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Err Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_RESPONSE,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                              testdata,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_RESPONSE,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                              &result,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_options_err_conf,
                   "test sixp_pkt_{set,get}_cell_options(err_conf)");
UNIT_TEST(test_set_get_cell_options_err_conf)
{
  sixp_pkt_cell_options_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Err Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_options(SIXP_PKT_TYPE_CONFIRMATION,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                              testdata,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_options(SIXP_PKT_TYPE_CONFIRMATION,
                              (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                              &result,
                              buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_add_req,
                   "test sixp_pkt_{set,get}_num_cells(add_req)");
UNIT_TEST(test_set_get_num_cells_add_req)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           testdata,
                           buf, sizeof(buf)) == 0);
  ref_data[3] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           &result,
                           buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_delete_req,
                   "test sixp_pkt_{set,get}_num_cells(delete_req)");
UNIT_TEST(test_set_get_num_cells_delete_req)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                           testdata,
                           buf, sizeof(buf)) == 0);
  ref_data[3] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                           &result,
                           buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_status_req,
                   "test sixp_pkt_{set,get}_num_cells(status_req)");
UNIT_TEST(test_set_get_num_cells_status_req)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                           &result,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_list_req,
                   "test sixp_pkt_{set,get}_num_cells(list_req)");
UNIT_TEST(test_set_get_num_cells_list_req)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                           &result,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_clear_req,
                   "test sixp_pkt_{set,get}_num_cells(clear_req)");
UNIT_TEST(test_set_get_num_cells_clear_req)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                           &result,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_success_res,
                   "test sixp_pkt_{set,get}_num_cells(success_res)");
UNIT_TEST(test_set_get_num_cells_success_res)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           testdata,
                           buf, sizeof(buf)) == 0);
  ref_data[0] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           &result,
                           buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_success_conf,
                   "test sixp_pkt_{set,get}_num_cells(success_conf)");
UNIT_TEST(test_set_get_num_cells_success_conf)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           &result,
                           buf, sizeof(buf)) == -1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_err_res,
                   "test sixp_pkt_{set,get}_num_cells(err_res)");
UNIT_TEST(test_set_get_num_cells_err_res)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Err Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           &result,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_num_cells_err_conf,
                   "test sixp_pkt_{set,get}_num_cells(err_conf)");
UNIT_TEST(test_set_get_num_cells_err_conf)
{
  sixp_pkt_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Err Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           testdata,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           &result,
                           buf, sizeof(buf)) == -1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_add_req,
                   "test sixp_pkt_{set,get}_reserved(add_req)");
UNIT_TEST(test_set_get_reserved_add_req)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_delete_req,
                   "test sixp_pkt_{set,get}_reserved(delete_req)");
UNIT_TEST(test_set_get_reserved_delete_req)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_status_req,
                   "test sixp_pkt_{set,get}_reserved(status_req)");
UNIT_TEST(test_set_get_reserved_status_req)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_list_req,
                   "test sixp_pkt_{set,get}_reserved(list_req)");
UNIT_TEST(test_set_get_reserved_list_req)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                          testdata,
                          buf, sizeof(buf)) == 0);
  ref_data[3] = 0x07;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                          &result,
                          buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x07);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_clear_req,
                   "test sixp_pkt_{set,get}_reserved(clear_req)");
UNIT_TEST(test_set_get_reserved_clear_req)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_REQUEST,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_success_res,
                   "test sixp_pkt_{set,get}_reserved(success_res)");
UNIT_TEST(test_set_get_reserved_success_res)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_success_conf,
                   "test sixp_pkt_{set,get}_reserved(success_conf)");
UNIT_TEST(test_set_get_reserved_success_conf)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_err_res,
                   "test sixp_pkt_{set,get}_reserved(err_res)");
UNIT_TEST(test_set_get_reserved_err_res)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_RESPONSE,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_reserved_err_conf,
                   "test sixp_pkt_{set,get}_reserved(err_conf)");
UNIT_TEST(test_set_get_reserved_err_conf)
{
  sixp_pkt_reserved_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_reserved(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          testdata,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_reserved(SIXP_PKT_TYPE_CONFIRMATION,
                          (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                          &result,
                          buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_add_req,
                   "test sixp_pkt_{set,get}_offset(add_req)");
UNIT_TEST(test_set_get_offset_add_req)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_delete_req,
                   "test sixp_pkt_{set,get}_offset(delete_req)");
UNIT_TEST(test_set_get_offset_delete_req)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_status_req,
                   "test sixp_pkt_{set,get}_offset(status_req)");
UNIT_TEST(test_set_get_offset_status_req)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_list_req,
                   "test sixp_pkt_{set,get}_offset(list_req)");
UNIT_TEST(test_set_get_offset_list_req)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0xabcd;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                        testdata,
                        buf, sizeof(buf)) == 0);
  ref_data[4] = 0xcd;
  ref_data[5] = 0xab;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                        &result,
                        buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0xabcd);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_clear_req,
                   "test sixp_pkt_{set,get}_offset(clear_req)");
UNIT_TEST(test_set_get_offset_clear_req)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_REQUEST,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_success_res,
                   "test sixp_pkt_{set,get}_offset(success_res)");
UNIT_TEST(test_set_get_offset_success_res)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_RESPONSE,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_RESPONSE,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_success_conf,
                   "test sixp_pkt_{set,get}_offset(success_conf)");
UNIT_TEST(test_set_get_offset_success_conf)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_CONFIRMATION,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_CONFIRMATION,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_err_res,
                   "test sixp_pkt_{set,get}_offset(err_res)");
UNIT_TEST(test_set_get_offset_err_res)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_RESPONSE,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_RESPONSE,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_offset_err_conf,
                   "test sixp_pkt_{set,get}_offset(err_conf)");
UNIT_TEST(test_set_get_offset_err_conf)
{
  sixp_pkt_offset_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_offset(SIXP_PKT_TYPE_CONFIRMATION,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                        testdata,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_offset(SIXP_PKT_TYPE_CONFIRMATION,
                        (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                        &result,
                        buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}


UNIT_TEST_REGISTER(test_set_get_max_num_cells_add_req,
                   "test sixp_pkt_{set,get}_max_num_cells(add_req)");
UNIT_TEST(test_set_get_max_num_cells_add_req)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_delete_req,
                   "test sixp_pkt_{set,get}_max_num_cells(delete_req)");
UNIT_TEST(test_set_get_max_num_cells_delete_req)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_status_req,
                   "test sixp_pkt_{set,get}_max_num_cells(status_req)");
UNIT_TEST(test_set_get_max_num_cells_status_req)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_list_req,
                   "test sixp_pkt_{set,get}_max_num_cells(list_req)");
UNIT_TEST(test_set_get_max_num_cells_list_req)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x1234;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                               testdata,
                               buf, sizeof(buf)) == 0);
  ref_data[6] = 0x34;
  ref_data[7] = 0x12;
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                               &result,
                               buf, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x1234);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_clear_req,
                   "test sixp_pkt_{set,get}_max_num_cells(clear_req)");
UNIT_TEST(test_set_get_max_num_cells_clear_req)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_REQUEST,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_success_res,
                   "test sixp_pkt_{set,get}_max_num_cells(success_res)");
UNIT_TEST(test_set_get_max_num_cells_success_res)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_success_conf,
                   "test sixp_pkt_{set,get}_max_num_cells(success_conf)");
UNIT_TEST(test_set_get_max_num_cells_success_conf)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_err_res,
                   "test sixp_pkt_{set,get}_max_num_cells(err_res)");
UNIT_TEST(test_set_get_max_num_cells_err_res)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_RESPONSE,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_max_num_cells_err_conf,
                   "test sixp_pkt_{set,get}_max_num_cells(err_conf)");
UNIT_TEST(test_set_get_max_num_cells_err_conf)
{
  sixp_pkt_max_num_cells_t testdata, result;
  testdata = 0x07;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_max_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                               testdata,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_max_num_cells(SIXP_PKT_TYPE_CONFIRMATION,
                               (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                               &result,
                               buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result == 0x00);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_add_req,
                   "test sixp_pkt_{set,get}_cell_list(add_req)");
UNIT_TEST(test_set_get_cell_list_add_req)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Add Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           testdata, sizeof(testdata), 0, // offset 0
                           buf, sizeof(buf)) == 0);
  memcpy(ref_data + 4, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           testdata, sizeof(testdata), 4,
                           buf, sizeof(buf)) == 0);
  /* 4 of cell_offset means 16-octet offset  */
  memcpy(ref_data + 4 + 16, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           &result, &result_len,
                           buf, sizeof(testdata) * 2 + 4) == 0);
  UNIT_TEST_ASSERT(result_len == sizeof(testdata) * 2);
  UNIT_TEST_ASSERT(memcmp(result, testdata, sizeof(testdata)) == 0);
  UNIT_TEST_ASSERT(memcmp(result + 16, testdata, sizeof(testdata)) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_delete_req,
                   "test sixp_pkt_{set,get}_cell_list(delete_req)");
UNIT_TEST(test_set_get_cell_list_delete_req)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Delete Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == 0);
  memcpy(ref_data + 4, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD,
                           testdata, sizeof(testdata), 4,
                           buf, sizeof(buf)) == 0);
  /* 4 of cell_offset means 16-octet offset  */
  memcpy(ref_data + 4 + 16, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                           &result, &result_len,
                           buf, sizeof(testdata) + 4) == 0);
  UNIT_TEST_ASSERT(result_len == sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);
  UNIT_TEST_ASSERT(memcmp(result + 16, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_status_req,
                   "test sixp_pkt_{set,get}_cell_list(status_req)");
UNIT_TEST(test_set_get_cell_list_status_req)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Status Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_STATUS,
                           &result, &result_len,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result_len == 0);
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_list_req,
                   "test sixp_pkt_{set,get}_cell_list(list_req)");
UNIT_TEST(test_set_get_cell_list_list_req)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* List Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_LIST,
                           &result, &result_len,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result_len == 0);
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_clear_req,
                   "test sixp_pkt_{set,get}_cell_list(clear_req)");
UNIT_TEST(test_set_get_cell_list_clear_req)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Clear Request */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_REQUEST,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_CLEAR,
                           &result, &result_len,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result_len == 0);
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_success_res,
                   "test sixp_pkt_{set,get}_cell_list(success_res)");
UNIT_TEST(test_set_get_cell_list_success_res)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Success Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == 0);
  memcpy(ref_data, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           &result, &result_len,
                           buf, sizeof(testdata)) == 0);
  UNIT_TEST_ASSERT(result_len == sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_success_conf,
                   "test sixp_pkt_{set,get}_cell_list(success_conf)");
UNIT_TEST(test_set_get_cell_list_success_conf)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Success Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == 0);
  memcpy(ref_data, testdata, sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                           &result, &result_len,
                           buf, sizeof(testdata)) == 0);
  UNIT_TEST_ASSERT(result_len == sizeof(testdata));
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_err_res,
                   "test sixp_pkt_{set,get}_cell_list(err_res)");
UNIT_TEST(test_set_get_cell_list_err_res)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Err Response */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_RESPONSE,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           &result, &result_len,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(result_len == 0);
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_set_get_cell_list_err_conf,
                   "test sixp_pkt_{set,get}_cell_list(err_conf)");
UNIT_TEST(test_set_get_cell_list_err_conf)
{
  /* make a cell list having four cells as test data */
  const uint8_t testdata[] = {0x01, 0x23, 0x45, 0x67,
                              0x89, 0xab, 0xcd, 0xef,
                              0xde, 0xad, 0xbe, 0xef,
                              0xca, 0xfe, 0xca, 0xfe};
  const uint8_t *result;
  sixp_pkt_offset_t result_len;

  UNIT_TEST_BEGIN();

  /* Err Confirmation */
  memset(buf, 0, sizeof(buf));
  memset(ref_data, 0, sizeof(ref_data));
  memset(&result, 0, sizeof(result));
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(result == 0x00);
  UNIT_TEST_ASSERT(
    sixp_pkt_set_cell_list(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           testdata, sizeof(testdata), 0,
                           buf, sizeof(buf)) == -1);
  UNIT_TEST_ASSERT(memcmp(buf, ref_data, sizeof(buf)) == 0);
  UNIT_TEST_ASSERT(
    sixp_pkt_get_cell_list(SIXP_PKT_TYPE_CONFIRMATION,
                           (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR,
                           &result, &result_len,
                           buf, sizeof(testdata)) == -1);
  UNIT_TEST_ASSERT(result_len == 0);
  UNIT_TEST_ASSERT(memcmp(result, testdata, result_len) == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_valid_version,
                   "test sixp_pkt_parse(valid_version)");
UNIT_TEST(test_parse_valid_version)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_buf[] = {0x01, 0x02, 0xfe, 0x6a,
                              0xde, 0xad, 0xbe, 0xef};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) == 0);
  UNIT_TEST_ASSERT(pkt.type == SIXP_PKT_TYPE_REQUEST);
  UNIT_TEST_ASSERT(pkt.code.cmd == SIXP_PKT_CMD_DELETE);
  UNIT_TEST_ASSERT(pkt.sfid == 0xfe);
  UNIT_TEST_ASSERT(pkt.seqno == 10);
  UNIT_TEST_ASSERT(pkt.gab == 2);
  UNIT_TEST_ASSERT(pkt.gba == 1);
  UNIT_TEST_ASSERT(pkt.body == &test_buf[4]);
  UNIT_TEST_ASSERT(pkt.body_len == 4);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_invalid_version,
                   "test sixp_pkt_parse(invalid_version)");
UNIT_TEST(test_parse_invalid_version)
{
  /*
   * - Version (4-bit) : 2 (invalid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_buf[] = {0x02, 0x08, 0xfe, 0x6a,
                              0xde, 0xad, 0xbe, 0xef};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) < 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_invalid_length,
                   "test sixp_pkt_parse(invalid_length)");
UNIT_TEST(test_parse_invalid_length)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   */
  const uint8_t test_buf[] = {0x02, 0x08, 0xfe};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) < 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_invalid_cmd,
                   "test sixp_pkt_parse(invalid_cmd)");
UNIT_TEST(test_parse_invalid_cmd)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 10 (undefined)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_buf[] = {0x01, 0x0a, 0xfe, 0x6a,
                              0xde, 0xad, 0xbe, 0xef};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) == -1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_invalid_rc,
                   "test sixp_pkt_parse(invalid_rc)");
UNIT_TEST(test_parse_invalid_rc)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 1 (response)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 15 (undefined)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_buf[] = {0x11, 0x0f, 0xfe, 0x6a};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) == -1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_parse_invalid_type,
                   "test sixp_pkt_parse(invalid_type)");
UNIT_TEST(test_parse_invalid_type)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 3 (undefined)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 0 (undefined)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_buf[] = {0x31, 0x00, 0xfe, 0x6a};
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(sixp_pkt_parse(test_buf, sizeof(test_buf), &pkt) == -1);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_create_valid_packet,
                   "test sixp_pkt_create(valid_packet)");
UNIT_TEST(test_create_valid_packet)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_body[] = {0xde, 0xad, 0xbe, 0xef};
  uint8_t *data_ptr, *hdr_ptr;
  sixp_pkt_t pkt;

  UNIT_TEST_BEGIN();

  packetbuf_clear();
  memset(&pkt, 0, sizeof(pkt));
  UNIT_TEST_ASSERT(
    sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                    (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                    0xfe, 10, 2, 1,
                    test_body, sizeof(test_body), &pkt) == 0);

  hdr_ptr = packetbuf_hdrptr();
  data_ptr = packetbuf_dataptr();

  /* Test lengths */
  UNIT_TEST_ASSERT(packetbuf_hdrlen() == 4);
  UNIT_TEST_ASSERT(packetbuf_datalen() == 4);
  UNIT_TEST_ASSERT(packetbuf_totlen() == 8);

  /* 6top IE Content */
  UNIT_TEST_ASSERT(hdr_ptr[0] == 0x01);
  UNIT_TEST_ASSERT(hdr_ptr[1] == 0x02);
  UNIT_TEST_ASSERT(hdr_ptr[2] == 0xfe);
  UNIT_TEST_ASSERT(hdr_ptr[3] == 0x6a);
  UNIT_TEST_ASSERT(data_ptr[0] == 0xde);
  UNIT_TEST_ASSERT(data_ptr[1] == 0xad);
  UNIT_TEST_ASSERT(data_ptr[2] == 0xbe);
  UNIT_TEST_ASSERT(data_ptr[3] == 0xef);

  /* Test returned pkt */
  UNIT_TEST_ASSERT(pkt.type == SIXP_PKT_TYPE_REQUEST);
  UNIT_TEST_ASSERT(pkt.code.cmd == SIXP_PKT_CMD_DELETE);
  UNIT_TEST_ASSERT(pkt.sfid == 0xfe);
  UNIT_TEST_ASSERT(pkt.seqno == 10);
  UNIT_TEST_ASSERT(pkt.gab == 2);
  UNIT_TEST_ASSERT(pkt.gba == 1);
  UNIT_TEST_ASSERT(memcmp(pkt.body, test_body, pkt.body_len) == 0);
  UNIT_TEST_ASSERT(pkt.body_len == 4);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_create_valid_packet_with_null_pkt,
                   "test sixp_pkt_create(valid_packet_with_null_pkt)");
UNIT_TEST(test_create_valid_packet_with_null_pkt)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_body[] = {0xde, 0xad, 0xbe, 0xef};
  uint8_t *data_ptr, *hdr_ptr;

  UNIT_TEST_BEGIN();

  packetbuf_clear();
  UNIT_TEST_ASSERT(
    sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                    (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                    0xfe, 10, 2, 1,
                    test_body, sizeof(test_body), NULL) == 0);

  hdr_ptr = packetbuf_hdrptr();
  data_ptr = packetbuf_dataptr();

  /* Test lengths */
  UNIT_TEST_ASSERT(packetbuf_hdrlen() == 4);
  UNIT_TEST_ASSERT(packetbuf_datalen() == 4);
  UNIT_TEST_ASSERT(packetbuf_totlen() == 8);

  /* 6top IE Content */
  UNIT_TEST_ASSERT(hdr_ptr[0] == 0x01);
  UNIT_TEST_ASSERT(hdr_ptr[1] == 0x02);
  UNIT_TEST_ASSERT(hdr_ptr[2] == 0xfe);
  UNIT_TEST_ASSERT(hdr_ptr[3] == 0x6a);
  UNIT_TEST_ASSERT(data_ptr[0] == 0xde);
  UNIT_TEST_ASSERT(data_ptr[1] == 0xad);
  UNIT_TEST_ASSERT(data_ptr[2] == 0xbe);
  UNIT_TEST_ASSERT(data_ptr[3] == 0xef);

  /* No need to test returned because pkt is NULL */

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_create_too_long_body,
                   "test sixp_pkt_create(too_long_body)");
UNIT_TEST(test_create_too_long_body)
{
  /*
   * - Version (4-bit) : 1 (valid version)
   * - Type (2-bit)    : 0 (request)
   * - Reserved (2-bit): 0
   * - Code (8-bit)    : 2 (delete)
   * - SFID (8-bit)    : 0xfe
   * - SeqNum (4-bit)  : 10
   * - GAB (2-bit)     : 2
   * - GBA (1-bit)     : 1
   * - Other Field     : (4-octet payload)
   */
  const uint8_t test_body[PACKETBUF_SIZE + 1];

  UNIT_TEST_BEGIN();

  packetbuf_clear();
  UNIT_TEST_ASSERT(
    sixp_pkt_create(SIXP_PKT_TYPE_REQUEST,
                    (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE,
                    0xfe, 10, 2, 1,
                    test_body, sizeof(test_body), NULL) == -1);
  UNIT_TEST_ASSERT(packetbuf_hdrlen() == 0);
  UNIT_TEST_ASSERT(packetbuf_datalen() == 0);
  UNIT_TEST_ASSERT(packetbuf_totlen() == 0);

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  printf("reference: draft-ietf-6tisch-6top-protocol-03");
  printf("Run unit-test\n");
  printf("---\n");

  /* metadata */
  UNIT_TEST_RUN(test_set_get_metadata_add_req);
  UNIT_TEST_RUN(test_set_get_metadata_delete_req);
  UNIT_TEST_RUN(test_set_get_metadata_status_req);
  UNIT_TEST_RUN(test_set_get_metadata_list_req);
  UNIT_TEST_RUN(test_set_get_metadata_clear_req);
  UNIT_TEST_RUN(test_set_get_metadata_success_res);
  UNIT_TEST_RUN(test_set_get_metadata_success_conf);
  UNIT_TEST_RUN(test_set_get_metadata_err_res);
  UNIT_TEST_RUN(test_set_get_metadata_err_conf);

  /* cell_options */
  UNIT_TEST_RUN(test_set_get_cell_options_add_req);
  UNIT_TEST_RUN(test_set_get_cell_options_delete_req);
  UNIT_TEST_RUN(test_set_get_cell_options_status_req);
  UNIT_TEST_RUN(test_set_get_cell_options_list_req);
  UNIT_TEST_RUN(test_set_get_cell_options_clear_req);
  UNIT_TEST_RUN(test_set_get_cell_options_success_res);
  UNIT_TEST_RUN(test_set_get_cell_options_success_conf);
  UNIT_TEST_RUN(test_set_get_cell_options_err_res);
  UNIT_TEST_RUN(test_set_get_cell_options_err_conf);

  /* num_cells */
  UNIT_TEST_RUN(test_set_get_num_cells_add_req);
  UNIT_TEST_RUN(test_set_get_num_cells_delete_req);
  UNIT_TEST_RUN(test_set_get_num_cells_status_req);
  UNIT_TEST_RUN(test_set_get_num_cells_list_req);
  UNIT_TEST_RUN(test_set_get_num_cells_clear_req);
  UNIT_TEST_RUN(test_set_get_num_cells_success_res);
  UNIT_TEST_RUN(test_set_get_num_cells_success_conf);
  UNIT_TEST_RUN(test_set_get_num_cells_err_res);
  UNIT_TEST_RUN(test_set_get_num_cells_err_conf);

  /* reserved */
  UNIT_TEST_RUN(test_set_get_reserved_add_req);
  UNIT_TEST_RUN(test_set_get_reserved_delete_req);
  UNIT_TEST_RUN(test_set_get_reserved_status_req);
  UNIT_TEST_RUN(test_set_get_reserved_list_req);
  UNIT_TEST_RUN(test_set_get_reserved_clear_req);
  UNIT_TEST_RUN(test_set_get_reserved_success_res);
  UNIT_TEST_RUN(test_set_get_reserved_success_conf);
  UNIT_TEST_RUN(test_set_get_reserved_err_res);
  UNIT_TEST_RUN(test_set_get_reserved_err_conf);

  /* offset */
  UNIT_TEST_RUN(test_set_get_offset_add_req);
  UNIT_TEST_RUN(test_set_get_offset_delete_req);
  UNIT_TEST_RUN(test_set_get_offset_status_req);
  UNIT_TEST_RUN(test_set_get_offset_list_req);
  UNIT_TEST_RUN(test_set_get_offset_clear_req);
  UNIT_TEST_RUN(test_set_get_offset_success_res);
  UNIT_TEST_RUN(test_set_get_offset_success_conf);
  UNIT_TEST_RUN(test_set_get_offset_err_res);
  UNIT_TEST_RUN(test_set_get_offset_err_conf);

  /* offset */
  UNIT_TEST_RUN(test_set_get_max_num_cells_add_req);
  UNIT_TEST_RUN(test_set_get_max_num_cells_delete_req);
  UNIT_TEST_RUN(test_set_get_max_num_cells_status_req);
  UNIT_TEST_RUN(test_set_get_max_num_cells_list_req);
  UNIT_TEST_RUN(test_set_get_max_num_cells_clear_req);
  UNIT_TEST_RUN(test_set_get_max_num_cells_success_res);
  UNIT_TEST_RUN(test_set_get_max_num_cells_success_conf);
  UNIT_TEST_RUN(test_set_get_max_num_cells_err_res);
  UNIT_TEST_RUN(test_set_get_max_num_cells_err_conf);

  /* cell_list */
  UNIT_TEST_RUN(test_set_get_cell_list_add_req);
  UNIT_TEST_RUN(test_set_get_cell_list_delete_req);
  UNIT_TEST_RUN(test_set_get_cell_list_status_req);
  UNIT_TEST_RUN(test_set_get_cell_list_list_req);
  UNIT_TEST_RUN(test_set_get_cell_list_clear_req);
  UNIT_TEST_RUN(test_set_get_cell_list_success_res);
  UNIT_TEST_RUN(test_set_get_cell_list_success_conf);
  UNIT_TEST_RUN(test_set_get_cell_list_err_res);
  UNIT_TEST_RUN(test_set_get_cell_list_err_conf);

  /* parse */
  UNIT_TEST_RUN(test_parse_valid_version);
  UNIT_TEST_RUN(test_parse_invalid_version);
  UNIT_TEST_RUN(test_parse_invalid_length);
  UNIT_TEST_RUN(test_parse_invalid_cmd);
  UNIT_TEST_RUN(test_parse_invalid_rc);
  UNIT_TEST_RUN(test_parse_invalid_type);

  /* create */
  UNIT_TEST_RUN(test_create_valid_packet);
  UNIT_TEST_RUN(test_create_valid_packet_with_null_pkt);
  UNIT_TEST_RUN(test_create_too_long_body);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
