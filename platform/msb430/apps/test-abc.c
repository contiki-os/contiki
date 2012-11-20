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
 */

/**
 * \file
 *         Testing the abc layer in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "sys/node-id.h"

#include <stdio.h>

#ifndef SENDER_ID
#define SENDER_ID	1
#endif

#ifndef SEND_INTERVAL
#define SEND_INTERVAL	(CLOCK_SECOND * 3)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(test_abc_process, "ABC test");
AUTOSTART_PROCESSES(&test_abc_process);
/*---------------------------------------------------------------------------*/
static void
abc_recv(struct abc_conn *c)
{
  char *s;
  int i;

  s = packetbuf_dataptr();

  printf("abc message received '%s'\n", (char *)packetbuf_dataptr());
  printf("message length: %u\n", packetbuf_datalen());
  for (i = 0; i < packetbuf_datalen(); i++) {
    printf("0x%x ", s[i]);
  }
  printf("\n");
}
const static struct abc_callbacks abc_call = {abc_recv};
static struct abc_conn abc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_abc_process, ev, data)
{
  static unsigned i;
  static struct etimer et;
  static int len;
  static char buf[32];

  PROCESS_EXITHANDLER(abc_close(&abc);)
    
  PROCESS_BEGIN();

  abc_open(&abc, 128, &abc_call);

  while(1) {
    etimer_set(&et, SEND_INTERVAL);
    PROCESS_WAIT_EVENT();
    if (etimer_expired(&et) && node_id == SENDER_ID) {
      len = snprintf(buf, sizeof (buf), "%u", ++i);
      printf("Sending packet %d\n", i);
      packetbuf_copyfrom(buf, len + 1);
      abc_send(&abc);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
