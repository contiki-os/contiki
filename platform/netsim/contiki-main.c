/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: contiki-main.c,v 1.4 2006/10/23 09:01:06 adamdunkels Exp $
 */

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "net/tapdev.h"
#include "net/tapdev-drv.h"
#include "net/tapdev-service.h"
#include "net/ethernode-drv.h"
#include "net/ethernode.h"
#include "ether.h"

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>*/

#include "dev/button-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/vib-sensor.h"
#include "dev/radio-sensor.h"
#include "dev/leds.h"

static struct uip_fw_netif tapif =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, tapdev_send)};
static struct uip_fw_netif ethernodeif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, ethernode_drv_send)};

static const struct uip_eth_addr ethaddr = {{0x00,0x06,0x98,0x01,0x02,0x12}};

/*PROCESS(test_send_process, "Test send stuff");
  PROCESS(test_tcp_process, "Test TCP");*/

SENSORS(&button_sensor, &pir_sensor, &vib_sensor, &radio_sensor);

PROCINIT(&sensors_process, &etimer_process, &tcpip_process,
	 &ethernode_drv_process,
	 &uip_fw_process);

#if 0
static
PT_THREAD(send_packet(struct pt *pt,
		      struct uip_udp_conn *c, process_event_t ev,
		      process_data_t data))
{
  PT_BEGIN(pt);

  tcpip_poll_udp(c);
  
  PT_YIELD_UNTIL(pt, ev == tcpip_event);
  
  uip_send("hej", 3);
  
  PT_END(pt);
}

PROCESS_THREAD(test_send_process, ev, data)
{
  static struct uip_udp_conn *conn;
  static struct etimer etimer;
  static struct pt send_pt;
  
  PROCESS_BEGIN();

  conn = udp_broadcast_new(HTONS(3737), NULL);

  etimer_set(&etimer, CLOCK_SECOND * 2);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etimer) || uip_newdata());

    if(uip_newdata()) {
      /*      printf("Got a packet!\n");*/
    }

    if(etimer_expired(&etimer)) {
      PROCESS_SPAWN(&send_pt, send_packet(&send_pt, conn, ev, data));
      etimer_set(&etimer, CLOCK_SECOND * 2);
    }
    
  }
  
  PROCESS_END();
}

PROCESS_THREAD(test_tcp_process, ev, data)
{
  uip_ipaddr_t server;
  
  PROCESS_BEGIN();

  uip_ipaddr(server, 255,255,255,255);
  tcp_connect(server, HTONS(1000), NULL);

  while(1) {
    PROCESS_WAIT_EVENT();
    printf("test_tcp_process: event %d\n", ev);
  }
  
  PROCESS_END();
}

#endif /* 0 */
/*---------------------------------------------------------------------------*/
/*static void
idle(void)
{
  ether_server_poll();
  display_tick();
  display_redraw();
  ether_tick();
  ek_run();
}*/
/*---------------------------------------------------------------------------*/
void
contiki_main(int flag)
{
  random_init(getpid());

  leds_init();
  
  process_init();

  procinit_init();
 
  if(flag == 1) {
    process_start(&tapdev_drv_process, NULL);
    uip_fw_register(&ethernodeif);
    uip_fw_default(&tapif);
    printf("uip_hostaddr %02x%02x\n", uip_hostaddr.u16[0], uip_hostaddr.u16[1]);
  } else {
    uip_fw_default(&ethernodeif);
  }
  leds_green(LEDS_ON);

  autostart_start(autostart_processes);

  while(1) {
    int n;
    n = process_run();
    /*    if(n > 0) {
      printf("%d processes in queue\n");
      }*/
    usleep(1);
    etimer_request_poll();
  }

}
/*---------------------------------------------------------------------------*/
process_event_t codeprop_event_quit;
