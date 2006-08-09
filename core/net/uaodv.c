/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * $Id: uaodv.c,v 1.2 2006/08/09 16:13:39 bg- Exp $
 */

/**
 * \file
 *         Micro implementation of the AODV ad hoc routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


#include <stdio.h>
#include <stdarg.h>

#include "contiki.h"
#include "net/uaodv-def.h"
#include "net/uaodv-rt.h"

#ifdef TMOTE_SKY
#include "dev/cc2420.h"
#endif

#define MY_ROUTE_TIMEOUT 0

PROCESS(uaodv_process, "uAODV");

static struct uip_udp_conn *aodvconn, *unicastconn;

static u32_t rreq_id, rreq_seqno;

static uip_ipaddr_t last_rreq_originator;
static u32_t last_rreq_id;


#define ip2quad(p) uip_ipaddr1(p),uip_ipaddr2(p),uip_ipaddr3(p),uip_ipaddr4(p)

#if 1
#define print_debug(...) do{}while(0)
#else
static void
print_debug(const char *fmt, ...)
{
#if 1
  va_list ap;

  va_start(ap, fmt);
  printf("%d.%d.%d.%d: ", ip2quad(&uip_hostaddr));
  vprintf(fmt, ap);
  va_end(ap);
#endif
  return;
}
#endif

#define uip_udp_sender() (&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->srcipaddr)

/*---------------------------------------------------------------------------*/
static void
sendto(const uip_ipaddr_t *dest, char *buf, int len)
{
  /* XXX: this is a HACK! We're updating the uIP UDP connection
     "unicastconn" so that the destination address is the next-hop,
     and we're patching the "uip_udp_conn" variable so that it points
     the this connection instead. THIS IS NOT A NICE WAY TO DO THIS,
     but it is currently nicer than the alternative (requesting a new
     poll, and remembering the state, etc.). */
  
  uip_ipaddr_copy(&unicastconn->ripaddr, dest);
  uip_udp_conn = unicastconn;
  uip_send(buf, len);
}
/*---------------------------------------------------------------------------*/
static void
send_rreq(uip_ipaddr_t *addr)
{
  struct uaodv_msg_rreq *rm = (struct uaodv_msg_rreq *)uip_appdata;
  
  rm->type = UAODV_RREQ_TYPE;
  rm->flags = UAODV_RREQ_UNKSEQNO;
  rm->reserved = 0;
  rm->hop_count = 0;
  rm->rreq_id = rreq_id;
  uip_ipaddr_copy(&rm->dest_addr, addr);
  rm->dest_seqno = 0;
  uip_gethostaddr(&rm->src_addr);
  rm->src_seqno = rreq_seqno;
  uip_send((char *)rm, sizeof(struct uaodv_msg_rreq));
  
  ++rreq_seqno;
  ++rreq_id;

  uip_ipaddr_copy(&last_rreq_originator, &rm->src_addr);
  last_rreq_id = rm->rreq_id;
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(uip_ipaddr_t *dst, uip_ipaddr_t *nexthop, uip_ipaddr_t *src, u32_t seqno)
{
  struct uaodv_msg_rrep *rm = (struct uaodv_msg_rrep *)uip_appdata;
  
  rm->type = UAODV_RREP_TYPE;
  rm->flags = 0;
  rm->reserved = 0;
  rm->hop_count = 0;
  uip_ipaddr_copy(&rm->src_addr, src);
  rm->dest_seqno = seqno;
  uip_ipaddr_copy(&rm->dest_addr, dst);
  rm->lifetime = MY_ROUTE_TIMEOUT;
  sendto(nexthop, (char *)rm, sizeof(struct uaodv_msg_rrep));

  print_debug("Sending RREP to %d.%d.%d.%d\n",
	      uip_ipaddr1(nexthop),
	      uip_ipaddr2(nexthop),
	      uip_ipaddr3(nexthop),
	      uip_ipaddr4(nexthop));
    
  
}
/*---------------------------------------------------------------------------*/
#ifdef UAODV_BAD_ROUTE
static void
send_rerr(uip_ipaddr_t *addr, u32_t seqno)
{
  struct uaodv_msg_rerr *rm = (struct uaodv_msg_rerr *)uip_appdata;
  
  rm->type = UAODV_RERR_TYPE;
  rm->flags = 0;
  rm->reserved = 0;
  rm->dest_count = 1;
  uip_ipaddr_copy(&rm->unreach[0].addr, addr);
  rm->unreach[0].seqno = seqno;

  sendto(&uip_broadcast_addr, (char *)rm, sizeof(struct uaodv_msg_rerr));

  print_debug("Broadcasting initial RERR for %d.%d.%d.%d\n", ip2quad(addr));  
}
#endif
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rreq(void)
{
  struct uaodv_msg_rreq *rm = (struct uaodv_msg_rreq *)uip_appdata;
  uip_ipaddr_t dest_addr, src_addr;
  struct uaodv_rt_entry *rt;

#ifdef TMOTE_SKY
  if(cc2420_last_rssi <= -38 || cc2420_last_correlation < 100) {
    printf("RREQ drop from %d.%d.%d.%d %d %d \n",
	   uip_ipaddr1(uip_udp_sender()),
	   uip_ipaddr2(uip_udp_sender()),
	   uip_ipaddr3(uip_udp_sender()),
	   uip_ipaddr4(uip_udp_sender()),
	   cc2420_last_rssi,
	   cc2420_last_correlation);
    return;
  }
#endif

  /* Check if it is for our address. */
  if(uip_ipaddr_cmp(&rm->dest_addr, &uip_hostaddr)) {
    print_debug("RREQ for our address!\n");
    rt = uaodv_rt_lookup(&rm->src_addr);
    if(rt == NULL || rm->hop_count <= rt->hop_count) {
      /* Insert originator into routing table. */
      print_debug("Inserting1 %d.%d.%d.%d into routing table, next hop %d.%d.%d.%d., hop_count %d\n",
		  uip_ipaddr1(&rm->src_addr),
		  uip_ipaddr2(&rm->src_addr),
		  uip_ipaddr3(&rm->src_addr),
		  uip_ipaddr4(&rm->src_addr),
		  uip_ipaddr1(uip_udp_sender()),
		  uip_ipaddr2(uip_udp_sender()),
		  uip_ipaddr3(uip_udp_sender()),
		  uip_ipaddr4(uip_udp_sender()),
		  rm->hop_count);
      uaodv_rt_add(&rm->src_addr, uip_udp_sender(), rm->hop_count, 0);
    }
    
    /* Send an RREP back to the source of the RREQ. */
    uip_ipaddr_copy( &dest_addr, &rm->dest_addr );
    uip_ipaddr_copy( &src_addr, &rm->src_addr );
    send_rrep(&dest_addr, uip_udp_sender(), &src_addr, rm->src_seqno);
  } else if(!uip_ipaddr_cmp(&last_rreq_originator, &rm->src_addr) ||
	    last_rreq_id != rm->rreq_id) {
    print_debug("RREQ for %d.%d.%d.%d!\n",
		uip_ipaddr1(&rm->dest_addr),
		uip_ipaddr2(&rm->dest_addr),
		uip_ipaddr3(&rm->dest_addr),
		uip_ipaddr4(&rm->dest_addr));
    rt = uaodv_rt_lookup(&rm->src_addr);
    if(rt == NULL || rm->hop_count <= rt->hop_count) {
      /* Insert originator into routing table. */
      print_debug("Inserting2 %d.%d.%d.%d into routing table, next hop %d.%d.%d.%d., hop_count %d\n",
		  uip_ipaddr1(&rm->src_addr),
		  uip_ipaddr2(&rm->src_addr),
		  uip_ipaddr3(&rm->src_addr),
		  uip_ipaddr4(&rm->src_addr),
		  uip_ipaddr1(uip_udp_sender()),
		  uip_ipaddr2(uip_udp_sender()),
		  uip_ipaddr3(uip_udp_sender()),
		  uip_ipaddr4(uip_udp_sender()),
		  rm->hop_count);
      uaodv_rt_add(&rm->src_addr, uip_udp_sender(), rm->hop_count, 0);
    }

    /* Forward RREQ. XXX to be compliant, we should check the TTL
       of the incoming packet and forward only if TTL > 1. We
       should also set the TTL in our outgoing IP header too, but
       there currently is no way of doing this in Contiki so we
       don't do it at the moment. */
	
    print_debug("Forwarding RREQ to %d.%d.%d.%d ttl=%d\n",
		uip_ipaddr1(&rm->src_addr),
		uip_ipaddr2(&rm->src_addr),
		uip_ipaddr3(&rm->src_addr),
		uip_ipaddr4(&rm->src_addr),
		uip_udp_conn->ttl);

    uip_ipaddr_copy(&last_rreq_originator, &rm->src_addr);
    last_rreq_id = rm->rreq_id;
    rm->hop_count++;
    uip_send((char *)rm, sizeof(struct uaodv_msg_rreq));
  } else {
    print_debug("Not forwarding rreq last_rreq_originator %d.%d.%d.%d src_addr %d.%d.%d.%d, last_rreq_id %d rreq_id %d\n",
		uip_ipaddr1(&last_rreq_originator),
		uip_ipaddr2(&last_rreq_originator),
		uip_ipaddr3(&last_rreq_originator),
		uip_ipaddr4(&last_rreq_originator),
		uip_ipaddr1(&rm->src_addr),
		uip_ipaddr2(&rm->src_addr),
		uip_ipaddr3(&rm->src_addr),
		uip_ipaddr4(&rm->src_addr),
		last_rreq_id, rm->rreq_id);
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rrep(void)
{
  struct uaodv_msg_rrep *rm = (struct uaodv_msg_rrep *)uip_appdata;
  struct uaodv_rt_entry *rt;
  
  print_debug("RREP received\n");

  /* Check if we have another route to this destination. If so, we'll
     use the new one if the hop count is lower, and if the sequence
     number say we should. */
  rt = uaodv_rt_lookup(&rm->dest_addr);
  if(rt == NULL ||
     (rt->hop_count > rm->hop_count /* && seqno < seqno */)) {
    
    /* Insert originator into routing table. */
    print_debug("Inserting3 %d.%d.%d.%d into routing table, next hop %d.%d.%d.%d, hop_count %d.\n",
		uip_ipaddr1(&rm->dest_addr),
		uip_ipaddr2(&rm->dest_addr),
		uip_ipaddr3(&rm->dest_addr),
		uip_ipaddr4(&rm->dest_addr),
		uip_ipaddr1(uip_udp_sender()),
		uip_ipaddr2(uip_udp_sender()),
		uip_ipaddr3(uip_udp_sender()),
		uip_ipaddr4(uip_udp_sender()), rm->hop_count
		);
    uaodv_rt_add(&rm->dest_addr, uip_udp_sender(), rm->hop_count, 0);
#ifdef TMOTE_SKY
    /* This link is ok since he is unicasting back to us! */
    cc2420_recv_ok(uip_udp_sender());
    printf("RREP recv ok from %d.%d.%d.%d %d %d \n",
	   uip_ipaddr1(uip_udp_sender()),
	   uip_ipaddr2(uip_udp_sender()),
	   uip_ipaddr3(uip_udp_sender()),
	   uip_ipaddr4(uip_udp_sender()),
	   cc2420_last_rssi,
	   cc2420_last_correlation);
#endif

  } else {
    print_debug("Not inserting %d.%d.%d.%d into routing table, next hop %d.%d.%d.%d, hop_count %d.\n",
		uip_ipaddr1(&rm->dest_addr),
		uip_ipaddr2(&rm->dest_addr),
		uip_ipaddr3(&rm->dest_addr),
		uip_ipaddr4(&rm->dest_addr),
		uip_ipaddr1(uip_udp_sender()),
		uip_ipaddr2(uip_udp_sender()),
		uip_ipaddr3(uip_udp_sender()),
		uip_ipaddr4(uip_udp_sender()), rm->hop_count
		);

  }
  if(uip_ipaddr_cmp(&rm->src_addr, &uip_hostaddr)) {
    print_debug("------- COMPLETE ROUTE FOUND\n");
  } else {
  
    /*    print_debug("Sending back to originator\n");

    print_debug("Route lookup for %d.%d.%d.%d\n",
	  uip_ipaddr1(&rm->src_addr),
	  uip_ipaddr2(&rm->src_addr),
	  uip_ipaddr3(&rm->src_addr),
	  uip_ipaddr4(&rm->src_addr));*/
	
    rt = uaodv_rt_lookup(&rm->src_addr);

    if(rt == NULL) {
      print_debug("RREP received, but no route back to originator... :-( \n");
      return;
    }

    rm->hop_count++;

    print_debug("Sending RREP to %d.%d.%d.%d\n",
		uip_ipaddr1(&rt->nexthop),
		uip_ipaddr2(&rt->nexthop),
		uip_ipaddr3(&rt->nexthop),
		uip_ipaddr4(&rt->nexthop));
    
    sendto(&rt->nexthop, (char *)rm, sizeof(struct uaodv_msg_rrep));
    
    /*    print_debug("RREP forwarded\n");*/
  }
}
/*---------------------------------------------------------------------------*/
#ifdef UAODV_BAD_ROUTE
static void
handle_incoming_rerr(void)
{
  struct uaodv_msg_rerr *rm = (struct uaodv_msg_rerr *)uip_appdata;
  struct uaodv_rt_entry *rt;

  print_debug("RERR received from %d.%d.%d.%d route to %d.%d.%d.%d seq=%d\n",
	      ip2quad(uip_udp_sender()),
	      ip2quad(&rm->unreach[0].addr), rm->unreach[0].seqno);

  rt = uaodv_rt_lookup(&rm->unreach[0].addr);
  if(rt != NULL && uip_ipaddr_cmp(&rt->nexthop, uip_udp_sender())) {
    uaodv_rt_remove(rt);
    print_debug("RERR rebroadcast\n");
    sendto(&uip_broadcast_addr, (char *)rm, sizeof(struct uaodv_msg_rerr));
  }
}
#endif
/*---------------------------------------------------------------------------*/
static void
handle_incoming_packet(void)
{
  struct uaodv_msg *m = (struct uaodv_msg *)uip_appdata;

  /*  print_debug("New UDP data, AODV packet type %d\n", m->type);*/
  switch(m->type) {
  case UAODV_RREQ_TYPE:
    handle_incoming_rreq();
    break;

  case UAODV_RREP_TYPE:
    handle_incoming_rrep();
    break;

#ifdef UAODV_BAD_ROUTE
  case UAODV_RERR_TYPE:
    handle_incoming_rerr();
    break;
#endif
  }

}
/*---------------------------------------------------------------------------*/
static enum {
  COMMAND_NONE,
  COMMAND_SEND_RREQ,
  COMMAND_SEND_RERR,
} command;

#ifdef UAODV_BAD_ROUTE
static uip_ipaddr_t bad_nexthop;
u32_t bad_seqno;

void
uaodv_bad_route(struct uaodv_rt_entry *rt)
{
  uip_ipaddr_copy(&bad_nexthop, &rt->nexthop);
  bad_seqno = rt->seqno;
  command = COMMAND_SEND_RERR;
  process_post(&uaodv_process, PROCESS_EVENT_MSG, NULL);
}
#endif

static uip_ipaddr_t rreq_addr;
static struct timer next_time;

struct uaodv_rt_entry *
uaodv_request_route_to(uip_ipaddr_t *host)
{
  struct uaodv_rt_entry *route = uaodv_rt_lookup(host);

  if(route != NULL) {
    return route;
  }

  /*
   * Broadcast protocols must be rate-limited!
   */
  if(!timer_expired(&next_time)) {
    return NULL;
  }

  if(command != COMMAND_NONE) {
    return NULL;
  }

  uip_ipaddr_copy(&rreq_addr, host);
  command = COMMAND_SEND_RREQ;
  process_post(&uaodv_process, PROCESS_EVENT_MSG, NULL);
  timer_set(&next_time, CLOCK_SECOND/4); /* Max 10/s per RFC3561. */
  return NULL;
}

PROCESS_THREAD(uaodv_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit);

  PROCESS_BEGIN();

  printf("uaodv_process starting\n");

  aodvconn = udp_broadcast_new(HTONS(UAODV_UDPPORT), NULL);
  aodvconn->ttl = 1;
  unicastconn = udp_broadcast_new(HTONS(UAODV_UDPPORT), NULL);
  unicastconn->ttl = 1;
  
  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == tcpip_event) {
      if(uip_newdata()) {
	handle_incoming_packet();
	continue;
      }
      if(uip_poll()) {
	if(command == COMMAND_SEND_RREQ)
	  send_rreq(&rreq_addr);
#ifdef UAODV_BAD_ROUTE
	else if (command == COMMAND_SEND_RERR)
	  send_rerr(&bad_nexthop, bad_seqno);
#endif
	command = COMMAND_NONE;
	continue;
      }
    }

    if(ev == PROCESS_EVENT_MSG) {
      tcpip_poll_udp(aodvconn);
    }
  }

 exit:
  command = COMMAND_NONE;
  uaodv_rt_flush_all();
  uip_udp_remove(aodvconn);
  aodvconn = NULL;
  uip_udp_remove(unicastconn);
  unicastconn = NULL;
  printf("uaodv_process exiting\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
