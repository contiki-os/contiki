/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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

#include "contiki.h"
#include "unit-test.h"
#include "net/mac/frame802154.h"

#include <stdio.h>

#define VERBOSE 0

PROCESS(test_process, "frame802154.c test");
AUTOSTART_PROCESSES(&test_process);

typedef enum {SUCCESS, FAILURE} result_t;
typedef enum {NO_ADDR, SHORT, LONG} addr_mode_t;
typedef enum {NOT_PRESENT, PRESENT} panid_mode_t;
typedef enum {OFF, ON} panid_cmpr_mode_t;

typedef struct {
  addr_mode_t dest_addr_mode;
  addr_mode_t src_addr_mode;
  panid_mode_t dest_panid_mode;
  panid_mode_t src_panid_mode;
  panid_cmpr_mode_t panid_cmpr_mode;
} panid_test_def;

/* IEEE 802.15.4-2015, the itemization in Section 7.2.1.5 */
/* This is applied to frames but Acknowledge */
static panid_test_def panid_table_0b00_0b01[] = {
  /* dest_addr, src_addr, dest_panid,  src_panid,   panid_cmp */
  //{  NO_ADDR,   NO_ADDR,  UNDEFINED,   UNDEFINED,   OFF},
  //{  NO_ADDR,   NO_ADDR,  UNDEFINED,   UNDEFINED,   ON},
  {  SHORT,     SHORT,    PRESENT,     NOT_PRESENT, ON},
  {  LONG,      SHORT,    PRESENT,     NOT_PRESENT, ON},
  {  SHORT,     LONG,     PRESENT,     NOT_PRESENT, ON},
  {  LONG,      LONG,     PRESENT,     NOT_PRESENT, ON},
  {  SHORT,     SHORT,    PRESENT,     PRESENT,     OFF},
  {  LONG,      SHORT,    PRESENT,     PRESENT,     OFF},
  {  SHORT,     LONG,     PRESENT,     PRESENT,     OFF},
  {  LONG,      LONG,     PRESENT,     PRESENT,     OFF},
  {  SHORT,     NO_ADDR,  PRESENT,     NOT_PRESENT, OFF},
  {  LONG,      NO_ADDR,  PRESENT,     NOT_PRESENT, OFF},
  {  NO_ADDR,   SHORT,    NOT_PRESENT, PRESENT,     OFF},
  {  NO_ADDR,   LONG,     NOT_PRESENT, PRESENT,     OFF},
  //{  SHORT,     NO_ADDR,  UNDEFINED,   UNDEFINED,   ON},
  //{  LONG,      NO_ADDR,  UNDEFINED,   UNDEFINED,   ON},
  //{  NO_ADDR,   SHORT,    UNDEFINED,   UNDEFINED,   ON},
  //{  NO_ADDR,   LONG,     UNDEFINED,   UNDEFINED,   ON},
};


/* IEEE 802.15.4-2015, Table 7-2 in Section 7.2.1.5 */
static panid_test_def panid_table_0b10[] = {
  /* dest_addr, src_addr, dest_panid,  src_panid,   panid_cmp */
  {  NO_ADDR,   NO_ADDR,  NOT_PRESENT, NOT_PRESENT, OFF}, // index 0,  row-1
  {  NO_ADDR,   NO_ADDR,  PRESENT,     NOT_PRESENT, ON},  // index 1,  row-2
  {  SHORT,     NO_ADDR,  PRESENT,     NOT_PRESENT, OFF}, // index 2,  row-3-1
  {  SHORT,     NO_ADDR,  NOT_PRESENT, NOT_PRESENT, ON},  // index 3,  row-4-1
  {  NO_ADDR,   SHORT,    NOT_PRESENT, PRESENT,     OFF}, // index 4,  row-5-1
  {  NO_ADDR,   SHORT,    NOT_PRESENT, NOT_PRESENT, ON},  // index 5   row-6-1
  {  LONG,      LONG,     PRESENT,     NOT_PRESENT, OFF}, // index 6,  row-7
  {  LONG,      LONG,     NOT_PRESENT, NOT_PRESENT, ON},  // index 7,  row-8
  {  SHORT,     SHORT,    PRESENT,     PRESENT,     OFF}, // index 8,  row-9,  *1
  {  SHORT,     LONG,     PRESENT,     PRESENT,     OFF}, // index 9,  row-10, *1
  {  LONG,      SHORT,    PRESENT,     PRESENT,     OFF}, // index 10, row-11, *1
  {  SHORT,     LONG,     PRESENT,     NOT_PRESENT, ON},  // index 11, row-12, *2
  {  LONG,      SHORT,    PRESENT,     NOT_PRESENT, ON},  // index 12, row-13, *2
  {  SHORT,     SHORT,    PRESENT,     NOT_PRESENT, ON},  // index 13, row-14, *2
  {  LONG,      NO_ADDR,  PRESENT,     NOT_PRESENT, OFF}, // index 14, row-3-2
  {  LONG,      NO_ADDR,  NOT_PRESENT, NOT_PRESENT, ON},  // index 15, row-4-2
  {  NO_ADDR,   LONG,     NOT_PRESENT, PRESENT,     OFF}, // index 16, row-5-2
  {  NO_ADDR,   LONG,     NOT_PRESENT, NOT_PRESENT, ON},  // index 17, row-6-2
  /* *1: contradict row-12 in Table 2a of IEEE 802.15.4e-2012 */
  /* *2: contradict row-13 in Table 2a of IEEE 802.15.4e-2012 */
};

typedef void (*setup_fcf_p)(const panid_test_def *, frame802154_fcf_t *);


UNIT_TEST_REGISTER(panid_frame_ver_0b00, "PAN ID Cmpr Handing (frame-ver: 0b00)");
UNIT_TEST_REGISTER(panid_frame_ver_0b01, "PAN ID Cmpr Handing (frame-ver: 0b01)");
UNIT_TEST_REGISTER(panid_frame_ver_0b10, "PAN ID Cmpr Handing (frame-ver: 0b10)");


static void
test_print_report(const unit_test_t *utp)
{
  printf("=check-me= ");
  if(utp->result == unit_test_failure) {
    printf("FAILED   - %s: at test index %d\n", utp->descr, utp->exit_line);
  } else {
    printf("SUCEEDED - %s\n", utp->descr);
  }
}

static void
setup_addr_mode(addr_mode_t mode, uint8_t *addr_mode)
{
  switch(mode) {
  case NO_ADDR:
    *addr_mode = FRAME802154_NOADDR;
    break;
  case SHORT:
    *addr_mode = FRAME802154_SHORTADDRMODE;
    break;
  case LONG:
    *addr_mode = FRAME802154_LONGADDRMODE;
    break;
  default:
    break;
  }
}

static void
setup_panid_cmpr_mode(panid_cmpr_mode_t panid_cmpr_mode, frame802154_fcf_t *fcf)
{
  switch(panid_cmpr_mode) {
  case OFF:
    fcf->panid_compression = 0;
    break;
  case ON:
    fcf->panid_compression = 1;
    break;
  default:
    break;
  }
}

static void
setup_frame802154_2003_fcf(const panid_test_def *t, frame802154_fcf_t *fcf)
{
  fcf->frame_version = FRAME802154_IEEE802154_2003;
  fcf->frame_type = FRAME802154_DATAFRAME;

  setup_addr_mode(t->dest_addr_mode, &fcf->dest_addr_mode);
  setup_addr_mode(t->src_addr_mode, &fcf->src_addr_mode);
  setup_panid_cmpr_mode(t->panid_cmpr_mode, fcf);
}

static void
setup_frame802154_2006_fcf(const panid_test_def *t, frame802154_fcf_t *fcf)
{
  fcf->frame_version = FRAME802154_IEEE802154_2006;
  fcf->frame_type = FRAME802154_DATAFRAME;

  setup_addr_mode(t->dest_addr_mode, &fcf->dest_addr_mode);
  setup_addr_mode(t->src_addr_mode, &fcf->src_addr_mode);
  setup_panid_cmpr_mode(t->panid_cmpr_mode, fcf);
}


static void
setup_frame802154_2015_fcf(const panid_test_def *t, frame802154_fcf_t *fcf)
{
  fcf->frame_version = FRAME802154_IEEE802154E_2012;

  setup_addr_mode(t->dest_addr_mode, &fcf->dest_addr_mode);
  setup_addr_mode(t->src_addr_mode, &fcf->src_addr_mode);
  setup_panid_cmpr_mode(t->panid_cmpr_mode, fcf);
}

static int
panid_run_test(const panid_test_def table[], size_t table_size,
               setup_fcf_p setup_fcf)
{
  int i;
  int num_of_tests = table_size / sizeof(panid_test_def);
  frame802154_fcf_t fcf;
  int has_src_pan_id, has_dest_pan_id;
  const panid_test_def *test;
  result_t result;

  for(i = 0; i < num_of_tests; i++) {
    test = &table[i];
    setup_fcf(test, &fcf);
    has_src_pan_id = 0;
    has_dest_pan_id = 0;

    frame802154_has_panid(&fcf, &has_src_pan_id, &has_dest_pan_id);

    result = FAILURE;
    switch(test->dest_panid_mode) {
    case NOT_PRESENT:
      if(has_dest_pan_id == 0) {
        result = SUCCESS;
      }
      break;
    case PRESENT:
      if(has_dest_pan_id == 1) {
        result = SUCCESS;
      }
      break;
    }

    if(result == SUCCESS) {
      result = FAILURE;
      switch(test->src_panid_mode) {
      case NOT_PRESENT:
        if(has_src_pan_id == 0) {
          result = SUCCESS;
        }
        break;
      case PRESENT:
        if(has_src_pan_id == 1) {
          result = SUCCESS;
        }
        break;
      }
    }

#if VERBOSE == 1
    printf("%d, %d, %d, %d, %d\n",
           test->dest_addr_mode,
           test->src_addr_mode,
           has_dest_pan_id,
           has_src_pan_id,
           test->panid_cmpr_mode);
#else
    printf("%s", result == SUCCESS ? "." : "E");
#endif
    if(result == FAILURE) {
      break;
    }
  }
  printf("\n");
  return i;
}

UNIT_TEST(panid_frame_ver_0b00)
{
  int index;
  int num_of_tests = sizeof(panid_table_0b00_0b01) / sizeof(panid_test_def);

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT((index = panid_run_test(panid_table_0b00_0b01,
                                           sizeof(panid_table_0b00_0b01),
                                           setup_frame802154_2003_fcf)) ==
                   num_of_tests);

  UNIT_TEST_END();
  utp->exit_line = index;
}

UNIT_TEST(panid_frame_ver_0b01)
{
  int index;
  int num_of_tests = sizeof(panid_table_0b00_0b01) / sizeof(panid_test_def);

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT((index = panid_run_test(panid_table_0b00_0b01,
                                           sizeof(panid_table_0b00_0b01),
                                           setup_frame802154_2006_fcf)) ==
                   num_of_tests);

  UNIT_TEST_END();
  utp->exit_line = index;
}

UNIT_TEST(panid_frame_ver_0b10)
{
  int index;
  int num_of_tests = sizeof(panid_table_0b10) / sizeof(panid_test_def);

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT((index = panid_run_test(panid_table_0b10,
                                           sizeof(panid_table_0b10),
                                           setup_frame802154_2015_fcf)) ==
                   num_of_tests);

  UNIT_TEST_END();
  utp->exit_line = index;
}

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(panid_frame_ver_0b00);
  UNIT_TEST_RUN(panid_frame_ver_0b01);
  UNIT_TEST_RUN(panid_frame_ver_0b10);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
