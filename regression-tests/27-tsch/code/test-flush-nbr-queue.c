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

#include "net/linkaddr.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"

#include "unit-test.h"
#include "common.h"

PROCESS(test_process, "tsch_queue_flush_nbr_queue() test");
AUTOSTART_PROCESSES(&test_process);

static linkaddr_t test_nbr_addr = {{ 0x01 }};
#define TEST_PEER_ADDR &test_nbr_addr

UNIT_TEST_REGISTER(test,
                   "flush_nbr_queue() should delete all the packet in the queue");
UNIT_TEST(test)
{
  struct tsch_packet *packet;
  struct tsch_neighbor *nbr;

  UNIT_TEST_BEGIN();

  packet = tsch_queue_add_packet(TEST_PEER_ADDR, NULL, NULL);
  UNIT_TEST_ASSERT(packet != NULL);

  nbr = tsch_queue_get_nbr(TEST_PEER_ADDR);
  UNIT_TEST_ASSERT(nbr != NULL);

  /*
   * QUEUEBUF_CONF_NUM is set with 1; so another addition should fail due to
   * lack of memory.
   */
  packet = tsch_queue_add_packet(TEST_PEER_ADDR, NULL, NULL);
  UNIT_TEST_ASSERT(packet == NULL);

  /* tsch_queue_flush_nbr_queue() is called inside of tsch_queue_reset(). */
  tsch_queue_reset();

  /* After flushing the nbr queue, we should be able to add a new packet */
  packet = tsch_queue_add_packet(TEST_PEER_ADDR, NULL, NULL);
  UNIT_TEST_ASSERT(packet != NULL);

  UNIT_TEST_END();
}

PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  tsch_set_coordinator(1);

  etimer_set(&et, CLOCK_SECOND);
  while(tsch_is_associated == 0) {
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  printf("Run unit-test\n");
  printf("---\n");

  UNIT_TEST_RUN(test);

  printf("=check-me= DONE\n");
  PROCESS_END();
}
