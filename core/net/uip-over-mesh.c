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
 * $Id: uip-over-mesh.c,v 1.5 2007/08/30 14:39:17 matsutsuka Exp $
 */

/**
 * \file
 *         Code for tunnelling uIP packets over the mesh routing module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stdio.h>

#include "net/hc.h"
#include "net/uip-fw.h"
#include "net/rime/route-discovery.h"
#include "net/rime/route.h"

#define ROUTE_TIMEOUT CLOCK_SECOND * 4

static struct queuebuf *queued_packet;
static rimeaddr_t queued_receiver;
static struct route_discovery_conn route_discovery;
static struct uc_conn dataconn;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
recv_data(struct uc_conn *c, rimeaddr_t *from)
{
  uip_len = rimebuf_copyto(&uip_buf[UIP_LLH_LEN]);

  /*  uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);*/

  PRINTF("uip-over-mesh: %d.%d: recv_data with len %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1], uip_len);
  tcpip_input();
}
/*---------------------------------------------------------------------------*/
static void
send_data(rimeaddr_t *next)
{
  PRINTF("uip-over-mesh: %d.%d: send_data with len %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 rimebuf_totlen());
  uc_send(&dataconn, next);
}
/*---------------------------------------------------------------------------*/
static void
new_route(struct route_discovery_conn *c, rimeaddr_t *to)
{
  struct route_entry *rt;
  
  if(queued_packet) {
    PRINTF("uip-over-mesh: new route, sending queued packet\n");
    
    queuebuf_to_rimebuf(queued_packet);
    queuebuf_free(queued_packet);
    queued_packet = NULL;

    rt = route_lookup(&queued_receiver);
    if(rt) {
      send_data(&queued_receiver);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
timedout(struct route_discovery_conn *c)
{
  PRINTF("uip-over-mesh: packet timed out\n");
  if(queued_packet) {
    PRINTF("uip-over-mesh: freeing queued packet\n");
    queuebuf_free(queued_packet);
    queued_packet = NULL;
  }
}
/*---------------------------------------------------------------------------*/
static const struct uc_callbacks data_callbacks = { recv_data };
static const struct route_discovery_callbacks rdc = { new_route, timedout };
/*---------------------------------------------------------------------------*/
void
uip_over_mesh_init(u16_t channels)
{

  /*  printf("Our address is %d.%d (%d.%d.%d.%d)\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 uip_hostaddr.u8[0], uip_hostaddr.u8[1],
	 uip_hostaddr.u8[2], uip_hostaddr.u8[3]); */

  uc_open(&dataconn, channels, &data_callbacks);
  route_discovery_open(&route_discovery, CLOCK_SECOND / 4,
		       channels + 1, &rdc);
  tcpip_set_forwarding(1);

}
/*---------------------------------------------------------------------------*/
u8_t
uip_over_mesh_send(void)
{
  rimeaddr_t receiver;
  struct route_entry *rt;

  /* This function is called by the uip-fw module to send out an IP
     packet. We try to send the IP packet to the next hop route, or we
     queue the packet and send out a route request for the final
     receiver of the packet. */
  
  receiver.u16[0] = ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr.u16[1];

  PRINTF("uIP over mesh send to %d.%d with len %d\n",
	 receiver.u8[0], receiver.u8[1],
	 uip_len);
  
  /*  uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);*/
  
  rimebuf_copyfrom(&uip_buf[UIP_LLH_LEN], uip_len);
  
  rt = route_lookup(&receiver);
  if(rt == NULL) {
    PRINTF("uIP over mesh no route to %d.%d\n", receiver.u8[0], receiver.u8[1]);
    if(queued_packet == NULL) {
      queued_packet = queuebuf_new_from_rimebuf();
      rimeaddr_copy(&queued_receiver, &receiver);
      route_discovery_discover(&route_discovery, &receiver, ROUTE_TIMEOUT);
    } else if(!rimeaddr_cmp(&queued_receiver, &receiver)) {
      route_discovery_discover(&route_discovery, &receiver, ROUTE_TIMEOUT);
    }
  } else {
    send_data(&rt->nexthop);
  }
  return UIP_FW_OK;
}
/*---------------------------------------------------------------------------*/
