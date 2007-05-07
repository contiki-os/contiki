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
 * $Id: uaodv.c,v 1.13 2007/05/07 11:51:20 bg- Exp $
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

#define NDEBUG
#include "lib/assert.h"

#ifdef CC2420_RADIO
#include "dev/cc2420.h"
#endif

/* This implementation never expires routes!!! */
#define MY_ROUTE_TIMEOUT ((u16_t)~0u)

PROCESS(uaodv_process, "uAODV");

static struct uip_udp_conn *bcastconn, *unicastconn;

/* Compare sequence numbers as per RFC 3561. */
#ifdef __GNUC__
#define SCMP32(a, b) ((int32_t)((a) - (b)))
#else /* __GNUC__ */
/*
 * The macro above works for any compiler that does int32_t arithmetic
 * in 2's complement. Machines that does not use 2's complement needs
 * to figure a different implementation.
 */
#define SCMP32(a, b) exit(1) 
#endif /* __GNUC__ */

#define NDELETED 4

/*
 * Save known seqno:s in network byte order.
 */
static
struct {
  uip_ipaddr_t orig;
  u32_t seqno;
} lseq[NDELETED];

static CC_INLINE void
save_latest_seqno(const uip_ipaddr_t *originator, const u32_t *seqno)
{
  unsigned n = (originator->u8[2] + originator->u8[3]) % NDELETED;
  uip_ipaddr_copy(&lseq[n].orig, originator);
  lseq[n].seqno = *seqno;
}

static CC_INLINE u32_t
latest_seqno(const uip_ipaddr_t *originator)
{
  unsigned n = (originator->u8[2] + originator->u8[3]) % NDELETED;
  if(uip_ipaddr_cmp(&lseq[n].orig, originator))
    return lseq[n].seqno;
  return 0;
}

/*
 * When possible, keep state across reboots.
 */
#ifdef __GNUC__
#define NOINIT __attribute__((section(".noinit")))
#else /* __GNUC__ */
#define NOINIT
#endif /* __GNUC__ */
NOINIT static u32_t rreq_id, rreq_seqno;

#define NFWCACHE 16

static struct {
  uip_ipaddr_t orig;
  u32_t id;
} fwcache[NFWCACHE];

static CC_INLINE int
fwc_lookup(const uip_ipaddr_t *orig, const u32_t *id)
{
  unsigned n = (orig->u8[2] + orig->u8[3]) % NFWCACHE;
  return fwcache[n].id == *id && uip_ipaddr_cmp(&fwcache[n].orig, orig);
}

static CC_INLINE void
fwc_add(const uip_ipaddr_t *orig, const u32_t *id)
{
  unsigned n = orig->u8[3] % NFWCACHE;
  fwcache[n].id = *id;
  fwcache[n].orig = *orig;
}

#ifdef NDEBUG
#define PRINTF(...) do {} while (0)
#define print_debug(...) do{}while(0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#ifdef __GNUC__
static void
print_debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#endif /* __GNUC__ */
static void
print_debug(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  printf("%d.%d.%d.%d: ", uip_ipaddr_to_quad(&uip_hostaddr));
  vprintf(fmt, ap);
  va_end(ap);
  return;
}
#endif

#define BUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define uip_udp_sender() (&BUF->srcipaddr)

/*---------------------------------------------------------------------------*/
static void
sendto(const uip_ipaddr_t *dest, const void *buf, int len)
{
  /* XXX: this is a HACK! We're updating the uIP UDP connection
     "unicastconn" so that the destination address is the next-hop,
     and we're patching the "uip_udp_conn" variable so that it points
     the this connection instead. THIS IS NOT A NICE WAY TO DO THIS,
     but it is currently nicer than the alternative (requesting a new
     poll, and remembering the state, etc.). */
  
  uip_ipaddr_copy(&unicastconn->ripaddr, dest);
  uip_udp_conn = unicastconn;
  uip_udp_packet_send(unicastconn, buf, len);
}
/*---------------------------------------------------------------------------*/
static void
send_rreq(uip_ipaddr_t *addr)
{
  struct uaodv_msg_rreq *rm = (struct uaodv_msg_rreq *)uip_appdata;
  
  print_debug("send RREQ for %d.%d.%d.%d\n", uip_ipaddr_to_quad(addr));

  rm->type = UAODV_RREQ_TYPE;
  rm->dest_seqno = latest_seqno(addr);
  if(rm->dest_seqno == 0)
    rm->flags = UAODV_RREQ_UNKSEQNO;
  else
    rm->flags = 0;
  rm->reserved = 0;
  rm->hop_count = 0;
  rm->rreq_id = htonl(rreq_id++);
  uip_ipaddr_copy(&rm->dest_addr, addr);
  uip_gethostaddr(&rm->orig_addr);
  rreq_seqno++;			/* Always */
  rm->orig_seqno = htonl(rreq_seqno);
  uip_udp_packet_send(bcastconn, rm, sizeof(struct uaodv_msg_rreq));
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(uip_ipaddr_t *dest, uip_ipaddr_t *nexthop, uip_ipaddr_t *orig,
	  u32_t *seqno, unsigned hop_count)
{
  struct uaodv_msg_rrep *rm = (struct uaodv_msg_rrep *)uip_appdata;
  
  print_debug("send RREP orig=%d.%d.%d.%d hops=%d\n",
	      uip_ipaddr_to_quad(orig), hop_count);

  rm->type = UAODV_RREP_TYPE;
  rm->flags = 0;
  rm->reserved = 0;
  rm->hop_count = hop_count;
  uip_ipaddr_copy(&rm->orig_addr, orig);
  rm->dest_seqno = *seqno;
  uip_ipaddr_copy(&rm->dest_addr, dest);
  rm->lifetime = HTONL(MY_ROUTE_TIMEOUT);
  sendto(nexthop, rm, sizeof(struct uaodv_msg_rrep));
}
/*---------------------------------------------------------------------------*/
static void
send_rerr(uip_ipaddr_t *addr, u32_t *seqno)
{
  struct uaodv_msg_rerr *rm = (struct uaodv_msg_rerr *)uip_appdata;
  
  print_debug("send RERR for %d.%d.%d.%d\n", uip_ipaddr_to_quad(addr));  

  rm->type = UAODV_RERR_TYPE;
  rm->flags = 0;
  rm->reserved = 0;
  rm->dest_count = 1;
  uip_ipaddr_copy(&rm->unreach[0].addr, addr);
  rm->unreach[0].seqno = *seqno;

  uip_udp_packet_send(bcastconn, rm, sizeof(struct uaodv_msg_rerr));
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rreq(void)
{
  struct uaodv_msg_rreq *rm = (struct uaodv_msg_rreq *)uip_appdata;
  uip_ipaddr_t dest_addr, orig_addr;
  struct uaodv_rt_entry *rt, *fw = NULL;
  
  print_debug("RREQ %d.%d.%d.%d -> %d.%d.%d.%d ttl=%u"
	      " orig=%d.%d.%d.%d seq=%lu hops=%u dest=%d.%d.%d.%d seq=%lu\n",
	      uip_ipaddr_to_quad(&BUF->srcipaddr),
	      uip_ipaddr_to_quad(&BUF->destipaddr),
	      BUF->ttl,
	      uip_ipaddr_to_quad(&rm->orig_addr), ntohl(rm->orig_seqno),
	      rm->hop_count,
	      uip_ipaddr_to_quad(&rm->dest_addr), ntohl(rm->dest_seqno));

  if(uip_ipaddr_cmp(&rm->orig_addr, &uip_hostaddr)) {
    return;			/* RREQ looped back! */
  }

#ifdef CC2420_RADIO
 {
   int ret = cc2420_check_remote(uip_udp_sender()->u16[1]);

   if(ret == REMOTE_YES) {
     print_debug("RREQ drop is remote\n");
     return;
   } else if (ret == REMOTE_NO) {
     /* Is neigbour, accept it. */
   } else if(cc2420_last_rssi <= -38 || cc2420_last_correlation < 100) {
     print_debug("RREQ drop %d %d\n",
		 cc2420_last_rssi,cc2420_last_correlation);
     return;
   }
 }
#endif

  /* New reverse route? */
  rt = uaodv_rt_lookup(&rm->orig_addr);
  if(rt == NULL
     || (SCMP32(ntohl(rm->orig_seqno), rt->seqno) > 0) /* New route. */
     || (SCMP32(ntohl(rm->orig_seqno), rt->seqno) == 0
	 && rm->hop_count < rt->hop_count)) { /* Better route. */
    print_debug("Inserting1\n");
    rt = uaodv_rt_add(&rm->orig_addr, uip_udp_sender(),
		      rm->hop_count, ntohl(rm->orig_seqno));
  }
    
  /* Check if it is for our address or a fresh route. */
  if(uip_ipaddr_cmp(&rm->dest_addr, &uip_hostaddr)) {
    fw = NULL;
  } else {
    fw = uaodv_rt_lookup(&rm->dest_addr);
    if(!(rm->flags & UAODV_RREQ_UNKSEQNO)
       && fw != NULL
       && SCMP32(ntohl(rm->dest_seqno), fw->seqno) >= 0) {
      fw = NULL;
    }
  }

  if (fw != NULL) {
    u32_t net_seqno;

    print_debug("RREQ for known route\n");
    uip_ipaddr_copy(&dest_addr, &rm->dest_addr);
    uip_ipaddr_copy(&orig_addr, &rm->orig_addr);
    net_seqno = htonl(fw->seqno);
    send_rrep(&dest_addr, &rt->nexthop, &orig_addr, &net_seqno,
	      fw->hop_count + 1);
  } else if(uip_ipaddr_cmp(&rm->dest_addr, &uip_hostaddr)) {
    u32_t net_seqno;

    print_debug("RREQ for our address\n");
    uip_ipaddr_copy(&dest_addr, &rm->dest_addr);
    uip_ipaddr_copy(&orig_addr, &rm->orig_addr);

    if(!(rm->flags & UAODV_RREQ_UNKSEQNO)
       && SCMP32(rreq_seqno, ntohl(rm->dest_seqno)) < 0) { 
      print_debug("New rreq_seqno\n"); /* We must have rebooted. */
      rreq_seqno = ntohl(rm->dest_seqno) + 1;
    } else {
      rreq_seqno++;
    }
    net_seqno = htonl(rreq_seqno);
    send_rrep(&dest_addr, &rt->nexthop, &orig_addr, &net_seqno, 0);
  } else {
    /* Have we seen this RREQ before? */
    if(fwc_lookup(&rm->orig_addr, &rm->rreq_id)) {
      print_debug("RREQ cached, not fwd\n");
      return;
    }
    fwc_add(&rm->orig_addr, &rm->rreq_id);

    print_debug("RREQ fwd\n");
    rm->hop_count++;
    uip_udp_packet_send(bcastconn, rm, sizeof(struct uaodv_msg_rreq));
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rrep(void)
{
  struct uaodv_msg_rrep *rm = (struct uaodv_msg_rrep *)uip_appdata;
  struct uaodv_rt_entry *rt;

  print_debug("RREP %d.%d.%d.%d -> %d.%d.%d.%d"
	      " dest=%d.%d.%d.%d seq=%lu hops=%u orig=%d.%d.%d.%d\n",
	      uip_ipaddr_to_quad(&BUF->srcipaddr),
	      uip_ipaddr_to_quad(&BUF->destipaddr),
	      uip_ipaddr_to_quad(&rm->dest_addr), ntohl(rm->dest_seqno),
	      rm->hop_count,
	      uip_ipaddr_to_quad(&rm->orig_addr));

  rt = uaodv_rt_lookup(&rm->dest_addr);

  print_debug("RREP received dst=%d.%d.%d.%d rt=%p\n",
	      uip_ipaddr_to_quad(&rm->dest_addr), rt);

  /* New forward route? */
  if(rt == NULL || (SCMP32(ntohl(rm->dest_seqno), rt->seqno) > 0)) {
    print_debug("Inserting3\n");
    rt = uaodv_rt_add(&rm->dest_addr, uip_udp_sender(),
		      rm->hop_count, ntohl(rm->dest_seqno));
#ifdef CC2420_RADIO
    /* This link is ok since he is unicasting back to us! */
    cc2420_recv_ok(uip_udp_sender());
    print_debug("RREP recv ok %d %d\n",
		cc2420_last_rssi, cc2420_last_correlation);
#endif
  } else {
    print_debug("Not inserting\n");
  }

  /* Forward RREP towards originator? */
  if(uip_ipaddr_cmp(&rm->orig_addr, &uip_hostaddr)) {
    print_debug("------- COMPLETE ROUTE FOUND\n");
  } else {
    if(uip_ipaddr_cmp(&BUF->destipaddr, &uip_broadcast_addr)) {
      print_debug("RREP hello received?\n");
      return;
    }

    rt = uaodv_rt_lookup(&rm->orig_addr);

    if(rt == NULL) {
      print_debug("RREP received, but no route back to originator... :-( \n");
      return;
    }

    rm->hop_count++;

    print_debug("Fwd RREP to %d.%d.%d.%d\n", uip_ipaddr_to_quad(&rt->nexthop));

    sendto(&rt->nexthop, rm, sizeof(struct uaodv_msg_rrep));
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rerr(void)
{
  struct uaodv_msg_rerr *rm = (struct uaodv_msg_rerr *)uip_appdata;
  struct uaodv_rt_entry *rt;

  print_debug("RERR %d.%d.%d.%d -> %d.%d.%d.%d"
	      " unreach=%d.%d.%d.%d seq=%lu\n",
	      uip_ipaddr_to_quad(&BUF->srcipaddr),
	      uip_ipaddr_to_quad(&BUF->destipaddr),
	      uip_ipaddr_to_quad((uip_ipaddr_t *)&rm->unreach[0]),
	      ntohl(rm->unreach[0].seqno));

  rt = uaodv_rt_lookup(&rm->unreach[0].addr);
  if(rt != NULL
     && uip_ipaddr_cmp(&rt->nexthop, uip_udp_sender())
     && SCMP32(rt->seqno, ntohl(rm->unreach[0].seqno)) <= 0) {
    save_latest_seqno(&rm->unreach[0].addr, &rm->unreach[0].seqno);
    uaodv_rt_remove(rt);
    print_debug("RERR rebroadcast\n");
    uip_udp_packet_send(bcastconn, rm, sizeof(struct uaodv_msg_rerr));
  }
}
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

  case UAODV_RERR_TYPE:
    handle_incoming_rerr();
    break;
  }

}
/*---------------------------------------------------------------------------*/
static enum {
  COMMAND_NONE,
  COMMAND_SEND_RREQ,
  COMMAND_SEND_RERR,
} command;

static uip_ipaddr_t bad_dest;
static u32_t bad_seqno;		/* In network byte order! */

void
uaodv_bad_route(struct uaodv_rt_entry *rt)
{
  u32_t net_seqno = htonl(rt->seqno);
  save_latest_seqno(&rt->dest, &net_seqno);
  uip_ipaddr_copy(&bad_dest, &rt->dest);
  bad_seqno = htonl(rt->seqno);
  command = COMMAND_SEND_RERR;
  process_post(&uaodv_process, PROCESS_EVENT_MSG, NULL);
}

static uip_ipaddr_t rreq_addr;
static struct timer next_time;

struct uaodv_rt_entry *
uaodv_request_route_to(uip_ipaddr_t *host)
{
  struct uaodv_rt_entry *route = uaodv_rt_lookup(host);

  if(route != NULL) {
    uaodv_rt_lru(route);
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
  timer_set(&next_time, CLOCK_SECOND/8); /* Max 10/s per RFC3561. */
  return NULL;
}

PROCESS_THREAD(uaodv_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit);

  PROCESS_BEGIN();

  printf("uaodv_process starting\n");

  bcastconn = udp_broadcast_new(HTONS(UAODV_UDPPORT), NULL);
#ifdef AODV_COMPLIANCE
  bcastconn->ttl = 6;
#else
  bcastconn->ttl = 1;
#endif
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
	if(command == COMMAND_SEND_RREQ) {
	  if(uaodv_rt_lookup(&rreq_addr) == NULL)
	    send_rreq(&rreq_addr);
	} else if (command == COMMAND_SEND_RERR) {
	  send_rerr(&bad_dest, &bad_seqno);
	}
	command = COMMAND_NONE;
	continue;
      }
    }

    if(ev == PROCESS_EVENT_MSG) {
#if 1
      static struct etimer etimer;
      etimer_set(&etimer, 2);
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
#endif
      tcpip_poll_udp(bcastconn);
    }
  }

 exit:
  command = COMMAND_NONE;
  uaodv_rt_flush_all();
  uip_udp_remove(bcastconn);
  bcastconn = NULL;
  uip_udp_remove(unicastconn);
  unicastconn = NULL;
  printf("uaodv_process exiting\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
