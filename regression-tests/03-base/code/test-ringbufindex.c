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
#include "unit-test.h"

#include "lib/ringbufindex.h"

PROCESS(test_process, "ringbuf-index.c test");
AUTOSTART_PROCESSES(&test_process);

static struct ringbufindex ri;
static const uint8_t ri_size = 2;

static void
test_print_report(const unit_test_t *utp)
{
  printf("=check-me= ");
  if(utp->result == unit_test_failure) {
    printf("FAILED   - %s: exit at L%u\n", utp->descr, utp->exit_line);
  } else {
    printf("SUCCEEDED - %s\n", utp->descr);
  }
}

UNIT_TEST_REGISTER(test_ringbufindex_init, "Init");
UNIT_TEST(test_ringbufindex_init)
{
  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  UNIT_TEST_ASSERT(ri.mask == ri_size - 1 &&
                   ri.put_ptr == 0 &&
                   ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_put, "Put");
UNIT_TEST(test_ringbufindex_put)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /*
   * This one must fail because the ringbuf is full. The ringbuf can have one
   * item (size - 1) at most.
   */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_peek_put, "PeekPut");
UNIT_TEST(test_ringbufindex_peek_put)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Get index for the first item */
  ret = ringbufindex_peek_put(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get index for the second item; should be -1 because it's full */
  ret = ringbufindex_peek_put(&ri);
  UNIT_TEST_ASSERT(ret == -1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_get, "Get");
UNIT_TEST(test_ringbufindex_get)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Get the first item; it's been not put yet. */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == -1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get the first item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Put the second item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get the second item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_peek_get, "PeekGet");
UNIT_TEST(test_ringbufindex_peek_get)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Get the index of the first item; it's been not put yet. */
  ret = ringbufindex_peek_get(&ri);
  UNIT_TEST_ASSERT(ret == -1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get the index of the first item */
  ret = ringbufindex_peek_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get and remove the first item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Put the second item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get the index of the second item */
  ret = ringbufindex_peek_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get and remove the second item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_size, "Size");
UNIT_TEST(test_ringbufindex_size)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  ret = ringbufindex_size(&ri);
  UNIT_TEST_ASSERT(ret == ri_size);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_elements, "Elements");
UNIT_TEST(test_ringbufindex_elements)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Nothing in ringbuf */
  ret = ringbufindex_elements(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* One item in ringbuf */
  ret = ringbufindex_elements(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get the first item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Nothing in ringbuf */
  ret = ringbufindex_elements(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Put the second item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* One item in ringbuf */
  ret = ringbufindex_elements(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get the second item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Nothing in ringbuf */
  ret = ringbufindex_elements(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_full, "Full");
UNIT_TEST(test_ringbufindex_full)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Nothing in ringbuf; not full */
  ret = ringbufindex_full(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* One item in ringbuf; full */
  ret = ringbufindex_full(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get the first item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Nothing in ringbuf; not full */
  ret = ringbufindex_full(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Put the second item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* One item in ringbuf; full */
  ret = ringbufindex_full(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get the second item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Nothing in ringbuf; not full */
  ret = ringbufindex_full(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

UNIT_TEST_REGISTER(test_ringbufindex_empty, "Empty");
UNIT_TEST(test_ringbufindex_empty)
{
  int ret;

  UNIT_TEST_BEGIN();

  ringbufindex_init(&ri, ri_size);

  /* Nothing in ringbuf; empty */
  ret = ringbufindex_empty(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Put the first item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* One item in ringbuf; not empty */
  ret = ringbufindex_empty(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 0);

  /* Get the first item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Nothing in ringbuf; empty */
  ret = ringbufindex_empty(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 1 && ri.get_ptr == 1);

  /* Put the second item */
  ret = ringbufindex_put(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* One item in ringbuf; not empty */
  ret = ringbufindex_empty(&ri);
  UNIT_TEST_ASSERT(ret == 0 && ri.put_ptr == 0 && ri.get_ptr == 1);

  /* Get the second item */
  ret = ringbufindex_get(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  /* Nothing in ringbuf; empty */
  ret = ringbufindex_empty(&ri);
  UNIT_TEST_ASSERT(ret == 1 && ri.put_ptr == 0 && ri.get_ptr == 0);

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();
  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(test_ringbufindex_init);
  UNIT_TEST_RUN(test_ringbufindex_put);
  UNIT_TEST_RUN(test_ringbufindex_peek_put);
  UNIT_TEST_RUN(test_ringbufindex_get);
  UNIT_TEST_RUN(test_ringbufindex_peek_get);
  UNIT_TEST_RUN(test_ringbufindex_size);
  UNIT_TEST_RUN(test_ringbufindex_elements);
  UNIT_TEST_RUN(test_ringbufindex_full);
  UNIT_TEST_RUN(test_ringbufindex_empty);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
