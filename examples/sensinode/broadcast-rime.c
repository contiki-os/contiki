/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: example-broadcast.c,v 1.2 2009/11/19 17:29:41 nifi Exp $
 */

/**
 * \file
 *         Testing the broadcast layer in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "lib/random.h"
#include "net/rime/rimestats.h"
#include "dev/leds.h"
#include "dev/models.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define BROADCAST_CHANNEL 129

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "BROADCAST example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  leds_toggle(LEDS_RED);
  PRINTF("broadcast message received from %02x.%02x\n", from->u8[0], from->u8[1]);
  PRINTF("Size=0x%02x: '0x%04x'\n", packetbuf_datalen(), *(uint16_t *) packetbuf_dataptr());
}

static void print_rime_stats()
{
  PRINTF("\nNetwork Stats\n");
  PRINTF("   TX=%lu ,      RX=%lu\n", rimestats.tx, rimestats.rx);
  PRINTF("LL-TX=%lu ,   LL-RX=%lu\n", rimestats.lltx, rimestats.llrx);
  PRINTF(" Long=%lu ,   Short=%lu\n", rimestats.toolong, rimestats.tooshort);
  PRINTF("T/Out=%lu , CCA-Err=%lu\n", rimestats.timedout,
      rimestats.contentiondrop);
}

static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;
  static uint16_t counter;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  PRINTF("Start\n");
  broadcast_open(&broadcast, BROADCAST_CHANNEL, &bc_rx);
  PRINTF("Open Broadcast Connection, channel %u\n", BROADCAST_CHANNEL);
  //    leds_off(LEDS_ALL);
  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    leds_on(LEDS_GREEN);
    packetbuf_copyfrom(&counter, sizeof(counter));
    PRINTF("Sending %u bytes: 0x%04x\n", packetbuf_datalen(), *(uint16_t *) packetbuf_dataptr());
    if (broadcast_send(&broadcast) == 0) {
      PRINTF("Error Sending\n");
    }

    print_rime_stats();
    PRINTF("===================================\n");
    counter++;
    leds_off(LEDS_GREEN);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
