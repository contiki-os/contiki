/*
 * Copyright (c) 2012, Thingsquare, www.thingsquare.com.
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
 */
#include "contiki.h"
#include "net/rime/trickle.h"

#include "sys/node-id.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(trickle_node_process, "Trickle node");
AUTOSTART_PROCESSES(&trickle_node_process);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{
  printf("%d.%d: trickle message received '%s'\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 (char *)packetbuf_dataptr());
}
const static struct trickle_callbacks trickle_callback = {trickle_recv};
static struct trickle_conn trickle;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(trickle_node_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  trickle_open(&trickle, CLOCK_SECOND, 145, &trickle_callback);

  etimer_set(&et, CLOCK_SECOND * 4);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));
  if(node_id == 1) {
    packetbuf_copyfrom("Hello, world", 13);
    trickle_send(&trickle);
  }
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
