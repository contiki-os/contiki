/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: announce-blink.c,v 1.3 2010/05/03 22:02:59 nifi Exp $
 */

/**
 * \file
 *         Example program that displays the number of neighbors on the JCreate LEDs
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "shell.h"

#include "dev/acc-sensor.h"
#include "net/mac/xmac.h"
#include "dev/leds.h"

#include <stdio.h>

#define MAX_NEIGHBORS_SEEN 8
#define NEIGHBOR_TIMEOUT 4 * CLOCK_SECOND

static struct announcement announcement;

struct neighbor_entry {
  struct neighbor_entry *next;
  rimeaddr_t addr;
  struct ctimer ctimer;
};

LIST(neighbor_table);
MEMB(neighbor_mem, struct neighbor_entry, MAX_NEIGHBORS_SEEN);

/*---------------------------------------------------------------------------*/
PROCESS(announce_blink_process, "Annouce blink");
AUTOSTART_PROCESSES(&announce_blink_process);
/*---------------------------------------------------------------------------*/
/*
 * This function takes the length of the neighbor table list and
 * displays it on the on-board LEDs.
 */
static void
show_leds(void)
{
  int val, i;
  int num;

  num = list_length(neighbor_table);
  
  val = 1;
  for(i = 0; i < num; ++i) {
    val |= (val << 1);
  }
  val >>= 1;

  leds_on(val & 0xff);
  leds_off(~(val & 0xff));
}
/*---------------------------------------------------------------------------*/
static void
remove_neighbor(void *dummy)
{
  struct neighbor_entry *e = dummy;

  list_remove(neighbor_table, e);
  memb_free(&neighbor_mem, e);

  show_leds();
  
  printf("%d.%d: removed %d.%d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 e->addr.u8[0], e->addr.u8[1]);
}
/*---------------------------------------------------------------------------*/
static void
received_announcement(struct announcement *a, const rimeaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct neighbor_entry *e;

  printf("%d.%d: announcement from neighbor %d.%d, id %d, value %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1],
	 id, value);
  
  
  for(e = list_head(neighbor_table); e != NULL; e = e->next) {
    if(rimeaddr_cmp(from, &e->addr)) {
      ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
      show_leds();
      return;
    }
  }

  
  e = memb_alloc(&neighbor_mem);
  if(e != NULL) {
    rimeaddr_copy(&e->addr, from);
    list_add(neighbor_table, e);
    ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
  } else {
  }
  show_leds();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(announce_blink_process, ev, data)
{
  PROCESS_BEGIN();

  announcement_register(&announcement, 80, received_announcement);

  list_init(neighbor_table);
  memb_init(&neighbor_mem);

  {
    int i;
    for(i = 0; i < 10; ++i) {
      rimeaddr_t r;
      r.u8[0] = r.u8[1] = i;
      received_announcement(NULL, &r, 0, 0);
      received_announcement(NULL, &r, 0, 0);
    }
  }
  
  SENSORS_ACTIVATE(acc_sensor);

  /* Lower the transmission power for the announcements so that only
     close-range neighbors are noticed. (Makes for a nicer visual
     effect.) */
  xmac_set_announcement_radio_txpower(1);
  
  while(1) {
    static struct etimer e;
    static uint16_t last_value;
    
    etimer_set(&e, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT();
    if(acc_sensor.value(1) / 256 != last_value) {
      last_value = acc_sensor.value(1) / 256;
      announcement_listen(1);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
