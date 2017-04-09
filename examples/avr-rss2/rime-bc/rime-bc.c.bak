/*
  Contiki Rime BRD demo.
  Broadcast Temp, Bat Voltage, RSSI, LQI DEMO. Robert Olsson <robert@herjulf.se>
  Heavily based on code from:
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
  See Contiki for full copyright.
*/

#include "contiki.h"
#include "contiki-net.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
#include "rf230bb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "rss2.h"
#include <avr/io.h>
#include <avr/wdt.h>

#define MAX_NEIGHBORS 64
#define SIZE          40

unsigned char charbuf[SIZE];
#define MAX_BCAST_SIZE 99

struct broadcast_message {
  uint8_t head; /* version << 4 + ttl */
  uint8_t seqno;
  uint8_t buf[MAX_BCAST_SIZE+20];  /* Check for max payload 20 extra to be able to test */
};

#define DEF_CHAN 26

struct neighbor {
  struct neighbor *next;
  linkaddr_t addr;

  /* The ->last_rssi and ->last_lqi fields hold the Received Signal
     Strength Indicator (RSSI) and CC2420 Link Quality Indicator (LQI)
     values that are received for the incoming broadcast packets. */
  uint16_t last_rssi, last_lqi;
  uint8_t last_seqno;

  /* The ->avg_gap contains the average seqno gap that we have seen
     from this neighbor. */
  uint32_t avg_seqno_gap;
};

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);

static struct broadcast_conn broadcast;

#define SEQNO_EWMA_UNITY 0x100 /* Moving average */
#define SEQNO_EWMA_ALPHA 0x040

PROCESS(broadcast_process, "Broadcast process");
AUTOSTART_PROCESSES(&broadcast_process);

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct neighbor *n;
  struct broadcast_message *msg;
  uint8_t seqno_gap;

  leds_off(4); // - RED
  msg = packetbuf_dataptr();
  /* From our own address. Can happen if we receive own pkt via relay
     Ignore
  */

  if(linkaddr_cmp(&linkaddr_node_addr, from)) {
    //do nothing
    goto out;
  }

  /* Check if we already know this neighbor. */
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {

    if(linkaddr_cmp(&n->addr, from)) 
      break;
  }

  if(n == NULL) {
    n = memb_alloc(&neighbors_memb); /* New neighbor */

    if(! n ) 
      goto out;
    
    linkaddr_copy(&n->addr, from);
    n->last_seqno = msg->seqno - 1;
    n->avg_seqno_gap = SEQNO_EWMA_UNITY;
    list_add(neighbors_list, n);
  }

  n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

  /* Compute the average sequence number gap from this neighbor. */
  seqno_gap = msg->seqno - n->last_seqno;

  
  n->avg_seqno_gap = (((uint32_t)seqno_gap * SEQNO_EWMA_UNITY) *
                      SEQNO_EWMA_ALPHA) / SEQNO_EWMA_UNITY +
                      ((uint32_t)n->avg_seqno_gap * (SEQNO_EWMA_UNITY -
                                                     SEQNO_EWMA_ALPHA)) / SEQNO_EWMA_UNITY;

  n->last_seqno = msg->seqno;

  printf("&: %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u DRP=%-d.%02d]\n",
	 msg->buf,
	 from->u8[0], from->u8[1], msg->seqno, msg->head & 0xF,
	 n->last_rssi,
	 n->last_lqi, 
	 (int)(n->avg_seqno_gap / SEQNO_EWMA_UNITY),
	 (int)(((100UL * n->avg_seqno_gap) / SEQNO_EWMA_UNITY) % 100));

out:
  leds_off(4); //0b00010000 - PE4
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

PROCESS_THREAD(broadcast_process, ev, data)
{
  PROCESS_BEGIN();
  broadcast_open(&broadcast, 129, &broadcast_call);
  wdt_disable();
  leds_init();
  rf230_set_channel(26);  
  while(1) 
  {
	  //no transmissions. just wait to receive
  }
  PROCESS_END();
}
