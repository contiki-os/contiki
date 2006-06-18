/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: dtc.c,v 1.1 2006/06/18 07:49:34 adamdunkels Exp $
 */
/**
 * \defgroup dtc Distributed TCP caching
 *
 * Distributed TCP Caching (DTC) is a mechanism that helps to improve
 * the energy efficiency of TCP in wireless sensor networks.
 *
 * @{
 */
 
/**
 * \file
 * Implementation of the distributed TCP caching mechanism for uIP.
 * \author Adam Dunkels <adam@sics.se>
 *
 */


#include "contiki.h"
#include "dtc-service.h"
#include "uip-fw.h"

#include "dtc.h"
#include "dtc-conf.h"
#include "uip.h"
#include "uip-fw.h"
#include "uip_arch.h"

#include "list.h"
#include "contiki.h"

/*------------------------------------------------------------------------------*/
/**
 * \internal
 * The TCP and IP headers.
 */
struct tcpip_hdr {
  /* IP header. */
  u8_t vhl,
    tos;          
  u16_t len;
  
  u8_t ipid[2],        
    ipoffset[2],  
    ttl,          
    proto;     
  u16_t ipchksum;
  u32_t srcipaddr, 
    destipaddr;
  
  /* TCP header. */
  u16_t srcport,
    destport;
  u32_t seqno, ackno;
  u8_t tcpoffset,
    flags;
  u16_t wnd;
  u16_t tcpchksum;
  u8_t urgp[2];
  u8_t data[4];
};

/**
 * \internal
 * Structure for keeping the state of one TCP end-point.
 */
struct dtc_tcpend {
  u32_t ipaddr;   /**< The IP address of the end-point. */
  u32_t seqno;    /**< The next expected TCP sequence number from the
		     end-point. */
  u32_t rttseq;   /**< The sequence number for which an RTT
		     measurement is currently being made, or 0 if no
		     measurement is being made. */
  u16_t port;     /**< The TCP port number of the end-point. */
  u16_t window;   /**< The receiver window for the end-point. */
  int sa;         /**< The smoothed avergage for the RTT
		     estimations. */
  int sv;         /**< The smoothed variance for the RTT
		     estimations. */
  u8_t rto;       /**< The RTO generated from the RTT estimations. */
  u8_t tmr;       /**< Timer. */
  u8_t nrtx;      /**< Number of retransmissions performed by the
		     distributed TCP snoop module. */
};

/**
 * \internal
 * The structure holding the state of a TCP connection.
 */
struct dtc_conn {
  struct dtc_conn *next;
  list_t segments;
  struct dtc_tcpend e1;   /**< First TCP end-point. */
  struct dtc_tcpend e2;   /**< Second TCP end-point. */
  u8_t tmr;                     /**< Timer. */
};

/**
 * \internal
 * The structure holding a cached TCP segment.
 */
struct dtc_segment {
  struct dtc_segment *next;
  struct tcpip_hdr hdr;        /**< The TCP and IP headers of the
				  packet. */
  struct dtc_conn *conn;       /**< Pointer to the connection that the
				  segment belongs to. */
  u16_t len;                   /**< The length of the segment,
				  measured in TCP bytes. */
  u8_t data[DTC_CONF_MAX_SEGSIZE];
                               /**< The data contained in the
				  segment. */
  u8_t tmr;                    /**< Timer - starts at 0 and counts
				  upward every time
				  dtc_periodic() is invoked. */
};

/**
 * \internal
 * The list of cached TCP segments.
 */
MEMB(segments, sizeof(struct dtc_segment), DTC_CONF_NUM_SEGMENTS);
/*static struct dtc_segment segments[DTC_CONF_NUM_SEGMENTS];*/

/**
 * \internal
 * The list of TCP connections for which the distributed TCP snoop
 * mechanism is active.
 */
MEMB(conns, sizeof(struct dtc_conn), DTC_CONF_NUM_CONNECTIONS);
/*static struct dtc_conn conns[DTC_CONF_NUM_CONNECTIONS];*/

LIST(connlist);

#define TCP_SEQ_LT(a,b)     ((s32_t)((a)-(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((s32_t)((a)-(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((s32_t)((a)-(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((s32_t)((a)-(b)) >= 0)
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f

/**
 * \internal
 * The length of the standard IP header, without IP options.
 */
#define IP_HLEN 20

/**
 * \internal
 * The start value of the retransmission time-out.
 */
#define RTO 30

static u32_t htonl(u32_t n);


static u8_t input(u8_t *packetptr, u16_t len);
static u8_t output(void) {return UIP_FW_OK;}

SERVICE(dtc_service, dtc_service, { input, output });

PROCESS(dtc_process, "Distributed TCP Caching");

/*------------------------------------------------------------------------------*/
/**
 * Initialize the distributed snoop module.
 *
 * This function initializes the distributed snoop module and must be
 * called before any of the other functions are used.
 */
/*------------------------------------------------------------------------------*/
static void
init_dtc(void)
{
  memb_init(&conns);
  memb_init(&segments);

  list_init(connlist);
  /*  list_init(segmentlist);*/
}  
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Save a segment in the segment cache.
 *
 * \param hdr Pointer to the header of the segment.
 * \param len The length of the segment.
 *
 * \return A pointer to the cached segment, or NULL if the cache was full.
 *
 */
/*------------------------------------------------------------------------------*/
static struct dtc_segment *
cache_segment(struct dtc_conn *conn,
	      struct tcpip_hdr *hdr, u16_t len)
{
  struct dtc_segment *s;

  /* Check if there are any unused slots in the segment cache. */
  s = (struct dtc_segment *)memb_alloc(&segments);

  if(s == NULL) {
    /* Here we might try to make room in the segment cache, but for
       now we simply return. */
    return NULL;
  }

  /* Do some sanity checks on the segment to be cached. */
  if(len < 40 ||
     len - 40 > DTC_CONF_MAX_SEGSIZE) {
    return NULL;
  }
     
  /* Calculate the length of the segment in TCP bytes. We do this by
     taking the length of the IP packet, and subtract the size of the
     IP and TCP headers. Since we only allow IP packets without
     options, we know that the IP header is 20 bytes large, and we use
     the header offset field of the TCP header to calculate the length
     of the TCP header + options.

     Finally, if the segment has the SYN or FIN flags set, we add one
     to the segment length.  */
  
  s->len = htons(hdr->len) - IP_HLEN - ((hdr->tcpoffset >> 4) << 2);
  if((hdr->flags & (TCP_SYN | TCP_FIN)) != 0) {
    ++s->len;
  }
  

  /* If there is no TCP data in the segment, there is no need to cache
     it. */
  if(s->len == 0) {
    return NULL;
  }
  
  s->tmr = 0;  
  
  /* Copy the TCP/IP headers. */
  memcpy(&s->hdr, hdr, 40);

  /* Copy the segment data. */
  memcpy(&s->data, &hdr->data[0], len - 40);

  /* Add segment to list of segments on the connection's list. */
  list_add(conn->segments, s);

  return s;
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Process an incoming TCP segment for a connection.
 */
/*------------------------------------------------------------------------------*/
static u8_t
process_segment(struct dtc_conn *conn,
		struct dtc_tcpend *src,
		struct dtc_tcpend *dst,
		u8_t *packetptr, u16_t len)
{
  struct tcpip_hdr *hdr;
  struct dtc_segment *s;
  int m;

  hdr = (struct tcpip_hdr *)packetptr;

  /* Check if the incoming segment acknowledges one in the segment
     list of the connection. */
  for(s = list_head(conn->segments); s != NULL; s = s->next) {
    
    printf("process_segment: for this conn, ack %d, ack %lu, seq %lu\n",
	   (hdr->flags & TCP_ACK), htonl(hdr->ackno), 
	   htonl(s->hdr.seqno) + s->len);

      
    /* Check if the incoming segment acknowledges this one. If so,
       we should drop the cached segment and possibly do RTT
       measurements. */
    
    if((s->hdr.srcipaddr == hdr->destipaddr) &&
       (hdr->flags & TCP_ACK) != 0 &&
       TCP_SEQ_GEQ(htonl(hdr->ackno), htonl(s->hdr.seqno) + s->len)) {
      
      /* Do RTT estimation, unless we have done retransmissions. */
      if(src->rttseq != 0 &&
	 TCP_SEQ_GEQ(htonl(hdr->ackno), src->rttseq) &&
	 src->nrtx == 0) {
	
	printf("Performing RTT estimation for seqno %lu (timer %d)\n",
	       src->rttseq, src->tmr);
	
	m = src->tmr;
	/* This is taken directly from VJs original code in his paper */
	m = m - (src->sa >> 3);
	src->sa += m;
	if(m < 0) {
	  m = -m;
	}
	m = m - (src->sv >> 2);
	src->sv += m;
	src->rto = (src->sa >> 3) + src->sv;
	src->rttseq = 0;
	printf("Performing RTO stuff, new rto %d\n", src->rto);
      }
      src->tmr = 0;
      
      printf("process_segment: bropping acked segment.\n");
      /*      s->state = STATE_FREE;*/
      list_remove(conn->segments, s);
      memb_free(&segments, s);
      
      /* Next, check if the incoming packet has the same sequence
	 number as one of the segments in the cache. If so, it is
	 either a retransmission or a duplicated packet. In any
	 case, the RTT estimation would be thwarted if we made
	 estimations, so we cancel any running RTT measurements. */
    } else if(s->hdr.seqno == hdr->seqno) {
      if(s->hdr.srcipaddr == hdr->srcipaddr) {
	src->rttseq = 0;
      } else {
	dst->rttseq = 0;
      }
    }
  }
  
  s = cache_segment(conn, hdr, len);

  if(s != NULL) {
    s->conn = conn;
    if(dst->rttseq == 0) {
      dst->rttseq = htonl(hdr->seqno);
      dst->tmr = 0;
      printf("Starting RTT est for %lu\n", dst->rttseq);
    }
    printf("Process_segment: segment cached\n");
  } else {
    printf("Process_segment: segment not cached \n");
  }

  return uip_fw_forward();
}
/*------------------------------------------------------------------------------*/
/**
 * Process an incoming packet by running it through the distributed
 * TCP snooping mechanism, and possibly by forwarding it.
 *
 * This function processes an incoming packet, and should be called
 * from the network device driver for every packet that arrives. The
 * function might call uip_fw_output() to send out packets. 
 *
 * \param packetptr A pointer to the first byte of the packet header
 * of the incoming packet. The packet must follow contiguously the
 * header in memory.
 *
 * \param len The length of the packet, including the packet header.
 *
 * \retval UIP_FW_OK If the packet should be further processed by the
 * caller.
 *  
 * \retval UIP_FW_FORWARDED If the packet was forwarded and therefor
 * should not be further processed.
 */
/*------------------------------------------------------------------------------*/
static u8_t
input(u8_t *packetptr, u16_t len)
{
  struct tcpip_hdr *hdr;
  struct dtc_conn *c;

  hdr = (struct tcpip_hdr *)packetptr;
  
  /* XXX: when testing, only run dtc for connections on port 12347. */
  if(hdr->proto != UIP_PROTO_TCP ||
     (hdr->destport != HTONS(12347) && hdr->srcport != HTONS(12347))) {
    return uip_fw_forward();
  }
 

  printf("dtc_input (%d, %d): vhl 0x%x proto %d len %d ipchk 0x%x\n",
	 node_x(), node_y(),
	 hdr->vhl, hdr->proto, HTONS(hdr->len),
	 uip_ipchksum());
  
  /* First, do some initial checking on the packet to make sure that
     it is an undamaged TCP packet. Note that the checks are placed in
     an order with the most computationally expensive at the end. */

  if(hdr->vhl == 0x45 && /* IP version and header length must be normal. */

     /* It must be a TCP packet. */
     hdr->proto == UIP_PROTO_TCP &&
     
     /* We do not allow IP fragments. */
     hdr->ipoffset[1] == 0 &&
     (hdr->ipoffset[0] & 0x3f) == 0 && 

     /* We do not process segments destined for ourselves. */
     hdr->destipaddr != *(u32_t *)uip_hostaddr &&
     
     /* The packet length must not exceed the size of the segment
	cache. */
     (DTC_CONF_MAX_SEGSIZE + 40) >= HTONS(hdr->len) &&
     
     /* The IP checksum must be OK. */
     uip_ipchksum() == 0xffff &&

     /* The TCP checksum must be OK. */
     uip_tcpchksum() == 0xffff) {

    /* Check if this segment belongs to a connection for which DTC
       currently is active. Also keep track of the first unused
       connection, in case we need to allocated it further down. */
    for(c = list_head(connlist); c != NULL; c = c->next) {
      if(c->e1.ipaddr == hdr->destipaddr &&
	 c->e1.port == htons(hdr->destport) &&
	 c->e2.ipaddr == hdr->srcipaddr &&
	 c->e2.port == htons(hdr->srcport)) {
	printf("dtc_input %d, %d: process segment 1\n",
	       node_x(), node_y());
	  return process_segment(c, &c->e2, &c->e1,
				 packetptr, len);
      } else if(c->e2.ipaddr == hdr->destipaddr &&
		  c->e2.port == htons(hdr->destport) &&
		  c->e1.ipaddr == hdr->srcipaddr &&
		  c->e1.port == htons(hdr->srcport)) {
	printf("dtc_input %d, %d: process segment 2\n", node_x(), node_y());
	return process_segment(c, &c->e1, &c->e2,
			       packetptr, len);

	/*      } else {
	printf("dtc_input %d, %d: not processing segment\n",
	       node_x(), node_y());
	       return UIP_FW_LOCAL;*/
      }
    }
    
    
    /* The incoming segment did not belong to any active connections,
       so we allocate a new connection if there is a free one that we
       can use. */
    printf("dtc_input %d, %d: creating new connection\n", node_x(), node_y());

    c = (struct dtc_conn *)memb_alloc(&conns);
    list_add(connlist, c);
    /* Fill in the fields from the incoming header. */
    c->e1.ipaddr = hdr->srcipaddr;
    c->e1.seqno = htonl(hdr->seqno);
    c->e1.rttseq = 0;
    c->e1.port = htons(hdr->srcport);
    c->e1.window = htons(hdr->wnd);
    c->e1.sa = 16;
    c->e1.sv = 0;
    c->e1.rto = RTO;
    c->e1.tmr = 0;
    c->e1.nrtx = 0;
    
    c->e2.ipaddr = hdr->destipaddr;
    c->e2.seqno = htonl(hdr->ackno);
    c->e2.rttseq = 0;
    c->e2.port = htons(hdr->destport);
    c->e2.window = 0;
    c->e2.sa = 16;
    c->e2.sv = 0;
    c->e2.rto = RTO;
    c->e2.tmr = 0;
    c->e2.nrtx = 0;
    
    return process_segment(c, &c->e1, &c->e2, packetptr, len);
  }
  return uip_fw_forward();
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Perform periodic processing for a specific connection end-point.
 *
 * The periodic processing does two things: update timers and
 * retransmit segments, where the timers drive the
 * retransmissions. This must be done on a per-TCP-endpoint basis,
 * rather than on a per-TCP-connection basis, because we maintain an
 * RTT estimate for each endpoint. The reason for not having a single
 * RTT for each TCP connection is that this node may be located on a
 * path with a very long delay to one end and a very short delay to
 * the other.
 */
/*------------------------------------------------------------------------------*/
static void
periodic_endpoint(struct dtc_tcpend *c)
{
  ++c->tmr;
  /* If the timer has reached its threshold value, we should
     retransmit the segment with the lowest sequence number, if
     cached. */
  if(c->tmr == c->rto) {
    printf("periodic_endpoint: should do retransmission.\n");
    c->tmr = 0;
  }
}
/*------------------------------------------------------------------------------*/
/**
 * Perform periodic processing.
 *
 * This function does the periodic processing in the distributed snoop
 * module. It should be called as often as the uIP periodic function
 * is called, typically twice a second.
 *
 */
/*------------------------------------------------------------------------------*/
static void
periodic(void)
{
  struct dtc_segment *s;
  struct dtc_conn *c;
  
  /*  for(i = 0; i < DTC_CONF_NUM_CONNECTIONS; ++i) {
    c = &conns[i];
    if(c->state == STATE_USED) {
      periodic_endpoint(&c->e1);
      periodic_endpoint(&c->e2);
    }
  }
  
  for(i = 0; i < DTC_CONF_NUM_SEGMENTS; ++i) {
    s = &segments[i];
    if(s->state == STATE_USED) {
      ++s->tmr;
    }
    }*/
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Convert a 32-bit unsigned value from host to network byte order.
 *
 * \param n The 32-bit value to be converted.
 *
 * \return The converted 32-bit value.
 *
 */
/*------------------------------------------------------------------------------*/
static u32_t
htonl(u32_t n)
{
#if BYTE_ORDER == BIG_ENDIAN
  return n;
#else /* BYTE_ORDER == BIG_ENDIAN */
  return (((u32_t)n & 0xff) << 24) |
    (((u32_t)n & 0xff00) << 8) |
    (((u32_t)n & 0xff0000) >> 8) |
    (((u32_t)n & 0xff000000) >> 24);
#endif /* BYTE_ORDER == BIG_ENDIAN */

}
/*------------------------------------------------------------------------------*/
PROCESS_THREAD(dtc_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();

  init_dtc();
  
  SERVICE_REGISTER(dtc_service);

  while(1) {
    etimer_set(&et, CLOCK_SECOND / 2);
    PROCESS_YIELD_UNTIL(etimer_expired(&et));

    periodic();
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
