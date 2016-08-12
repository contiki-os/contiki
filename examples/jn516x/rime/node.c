/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */
/**
 * \author Atis Elsts <atis.elsts@sics.se>
 * \file
 *         ContikiMAC + Rime stack test for JN516x platform.
 */

#include "contiki-conf.h"
#include "net/rime/rime.h"

#if 0
#define RX_ADDR1 140
#define RX_ADDR2 228
#else
#define RX_ADDR1 7
#define RX_ADDR2 0
#endif

#define MAX_RETRANSMISSIONS 4

/*---------------------------------------------------------------------------*/
PROCESS(unicast_test_process, "ContikiMAC Node");
AUTOSTART_PROCESSES(&unicast_test_process);

static void
recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno)
{
  printf("runicast message received from %d.%d, seqno %d, len %d: '%s'\n",
         from->u8[0], from->u8[1], seqno, packetbuf_datalen(), (char *)packetbuf_dataptr());
}
static void
sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions)
{
  printf("runicast message sent to %d.%d, retransmissions %d\n",
         to->u8[0], to->u8[1], retransmissions);
}
static void
timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions)
{
  printf("runicast message timed out when sending to %d.%d, retransmissions %d\n",
         to->u8[0], to->u8[1], retransmissions);
}
static const struct runicast_callbacks runicast_callbacks = { recv_runicast,
                                                              sent_runicast,
                                                              timedout_runicast };
static struct runicast_conn runicast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_test_process, ev, data)
{
  PROCESS_BEGIN();

  puts("unicast test start");

  runicast_open(&runicast, 144, &runicast_callbacks);

  /* Receiver node: do nothing */
  if(linkaddr_node_addr.u8[0] == RX_ADDR1 &&
     linkaddr_node_addr.u8[1] == RX_ADDR2) {
    puts("wait forever");
  }
  while(1) {
    static struct etimer et;
    static int seqno;

    etimer_set(&et, CLOCK_SECOND * 5);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(linkaddr_node_addr.u8[0] == RX_ADDR1 &&
       linkaddr_node_addr.u8[1] == RX_ADDR2) {
      puts("tick...");
      continue;
    }

    if(!runicast_is_transmitting(&runicast)) {
      static char buffer[100] = "hello";
      linkaddr_t recv;

      memset(&recv, 0, LINKADDR_SIZE);
      packetbuf_copyfrom(buffer, sizeof(buffer));
      recv.u8[0] = RX_ADDR1;
      recv.u8[1] = RX_ADDR2;

      printf("%u.%u: sending runicast to address %u.%u\n",
             linkaddr_node_addr.u8[0],
             linkaddr_node_addr.u8[1],
             recv.u8[0],
             recv.u8[1]);

      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, ++seqno);
      runicast_send(&runicast, &recv, MAX_RETRANSMISSIONS);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
