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
 *
 * $Id: tcpip.c,v 1.24 2010/02/09 12:58:53 adamdunkels Exp $
 */
/**
 * \file
 *         Code for tunnelling uIP packets over the Rime mesh routing module
 *
 * \author  Adam Dunkels <adam@sics.se>\author
 * \author  Mathilde Durvy <mdurvy@cisco.com> (IPv6 related code)
 * \author  Julien Abeille <jabeille@cisco.com> (IPv6 related code)
 */
#include "contiki-net.h"

#include "net/uip-split.h"

#include <string.h>

#if UIP_CONF_IPV6
#include "net/uip-nd6.h"
#include "net/uip-netif.h"
#endif

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif

#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN + uip_ext_len])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

process_event_t tcpip_event;
#if UIP_CONF_ICMP6
process_event_t tcpip_icmp6_event;
#endif /* UIP_CONF_ICMP6 */

/*static struct tcpip_event_args ev_args;*/

/*periodic check of active connections*/
static struct etimer periodic;

#if UIP_CONF_IPV6 && UIP_CONF_IPV6_REASSEMBLY
/*timer for reassembly*/
extern struct etimer uip_reass_timer;
#endif

#if UIP_TCP
/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  u16_t port;
  struct process *p;
};

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  struct process *p;
} s;
#endif

enum {
  TCP_POLL,
  UDP_POLL,
  PACKET_INPUT
};

/* Called on IP packet output. */
#if UIP_CONF_IPV6

static u8_t (* outputfunc)(uip_lladdr_t *a);

u8_t
tcpip_output(uip_lladdr_t *a)
{
  int ret;
  if(outputfunc != NULL) {
    ret = outputfunc(a);
    return ret;
  }
  UIP_LOG("tcpip_output: Use tcpip_set_outputfunc() to set an output function");
  return 0;
}

void
tcpip_set_outputfunc(u8_t (*f)(uip_lladdr_t *))
{
  outputfunc = f;
}
#else

static u8_t (* outputfunc)(void);
u8_t
tcpip_output(void)
{
  if(outputfunc != NULL) {
    return outputfunc();
  }
  UIP_LOG("tcpip_output: Use tcpip_set_outputfunc() to set an output function");
  return 0;
}

void
tcpip_set_outputfunc(u8_t (*f)(void))
{
  outputfunc = f;
}
#endif

#if UIP_CONF_IP_FORWARD
unsigned char tcpip_is_forwarding; /* Forwarding right now? */
#endif /* UIP_CONF_IP_FORWARD */

PROCESS(tcpip_process, "TCP/IP stack");

/*---------------------------------------------------------------------------*/
static void
start_periodic_tcp_timer(void)
{
  if(etimer_expired(&periodic)) {
    etimer_restart(&periodic);
  }
}
/*---------------------------------------------------------------------------*/
static void
check_for_tcp_syn(void)
{
  /* This is a hack that is needed to start the periodic TCP timer if
     an incoming packet contains a SYN: since uIP does not inform the
     application if a SYN arrives, we have no other way of starting
     this timer.  This function is called for every incoming IP packet
     to check for such SYNs. */
#define TCP_SYN 0x02
  if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
     (UIP_TCP_BUF->flags & TCP_SYN) == TCP_SYN) {
    start_periodic_tcp_timer();
  }
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
#if UIP_CONF_IP_FORWARD
  if(uip_len > 0) {
    tcpip_is_forwarding = 1;
    if(uip_fw_forward() == UIP_FW_LOCAL) {
      tcpip_is_forwarding = 0;
      check_for_tcp_syn();
      uip_input();
      if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
        uip_split_output();
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
        tcpip_ipv6_output();
#else
	PRINTF("tcpip packet_input forward output len %d\n", uip_len);
        tcpip_output();
#endif
#endif /* UIP_CONF_TCP_SPLIT */
      }
    }
    tcpip_is_forwarding = 0;
  }
#else /* UIP_CONF_IP_FORWARD */
  if(uip_len > 0) {
    check_for_tcp_syn();
    uip_input();
    if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
      uip_split_output();
#else /* UIP_CONF_TCP_SPLIT */
#if UIP_CONF_IPV6
      tcpip_ipv6_output();
#else
      PRINTF("tcpip packet_input output len %d\n", uip_len);
      tcpip_output();
#endif
#endif /* UIP_CONF_TCP_SPLIT */
    }
  }
#endif /* UIP_CONF_IP_FORWARD */
}
/*---------------------------------------------------------------------------*/
#if UIP_TCP
#if UIP_ACTIVE_OPEN
struct uip_conn *
tcp_connect(uip_ipaddr_t *ripaddr, u16_t port, void *appstate)
{
  struct uip_conn *c;
  
  c = uip_connect(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  c->appstate.p = PROCESS_CURRENT();
  c->appstate.state = appstate;
  
  tcpip_poll_tcp(c);
  
  return c;
}
#endif /* UIP_ACTIVE_OPEN */
/*---------------------------------------------------------------------------*/
void
tcp_unlisten(u16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == port &&
       l->p == PROCESS_CURRENT()) {
      l->port = 0;
      uip_unlisten(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_listen(u16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = port;
      l->p = PROCESS_CURRENT();
      uip_listen(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_attach(struct uip_conn *conn,
	   void *appstate)
{
  register uip_tcp_appstate_t *s;

  s = &conn->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;
}

#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
udp_attach(struct uip_udp_conn *conn,
	   void *appstate)
{
  register uip_udp_appstate_t *s;

  s = &conn->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, u16_t port, void *appstate)
{
  struct uip_udp_conn *c;
  uip_udp_appstate_t *s;
  
  c = uip_udp_new(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  s = &c->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;

  return c;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_broadcast_new(u16_t port, void *appstate)
{
  uip_ipaddr_t addr;
  struct uip_udp_conn *conn;

#if UIP_CONF_IPV6
  uip_create_linklocal_allnodes_mcast(&addr);
#else
  uip_ipaddr(&addr, 255,255,255,255);
#endif /* UIP_CONF_IPV6 */
  conn = udp_new(&addr, port, appstate);
  if(conn != NULL) {
    udp_bind(conn, port);
  }
  return conn;
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_CONF_ICMP6
u8_t
icmp6_new(void *appstate) {
  if(uip_icmp6_conns.appstate.p == PROCESS_NONE) {
    uip_icmp6_conns.appstate.p = PROCESS_CURRENT();
    uip_icmp6_conns.appstate.state = appstate;
    return 0;
  }
  return 1;
}

void
tcpip_icmp6_call(u8_t type)
{
  if(uip_icmp6_conns.appstate.p != PROCESS_NONE) {
    /* XXX: This is a hack that needs to be updated. Passing a pointer (&type)
       like this only works with process_post_synch. */
    process_post_synch(uip_icmp6_conns.appstate.p, tcpip_icmp6_event, &type);
  }
  return;
}
#endif /* UIP_CONF_ICMP6 */
/*---------------------------------------------------------------------------*/
static void
eventhandler(process_event_t ev, process_data_t data)
{
#if UIP_TCP
  static unsigned char i;
  register struct listenport *l;
#endif /*UIP_TCP*/
  struct process *p;
   
  switch(ev) {
    case PROCESS_EVENT_EXITED:
      /* This is the event we get if a process has exited. We go through
         the TCP/IP tables to see if this process had any open
         connections or listening TCP ports. If so, we'll close those
         connections. */

      p = (struct process *)data;
#if UIP_TCP
      l = s.listenports;
      for(i = 0; i < UIP_LISTENPORTS; ++i) {
        if(l->p == p) {
          uip_unlisten(l->port);
          l->port = 0;
          l->p = PROCESS_NONE;
        }
        ++l;
      }
	 
      {
        register struct uip_conn *cptr;
	    
        for(cptr = &uip_conns[0]; cptr < &uip_conns[UIP_CONNS]; ++cptr) {
          if(cptr->appstate.p == p) {
            cptr->appstate.p = PROCESS_NONE;
            cptr->tcpstateflags = UIP_CLOSED;
          }
	       
        }
	    
      }
#endif /* UIP_TCP */
#if UIP_UDP
      {
        register struct uip_udp_conn *cptr;
        for(cptr = &uip_udp_conns[0];
            cptr < &uip_udp_conns[UIP_UDP_CONNS]; ++cptr) {
          if(cptr->appstate.p == p) {
            cptr->lport = 0;
          }
        }
      
      }
#endif /* UIP_UDP */
      break;

    case PROCESS_EVENT_TIMER:
      /* We get this event if one of our timers have expired. */
      {
        /* Check the clock so see if we should call the periodic uIP
           processing. */
        if(data == &periodic &&
           etimer_expired(&periodic)) {
#if UIP_TCP
          for(i = 0; i < UIP_CONNS; ++i) {
            if(uip_conn_active(i)) {
              /* Only restart the timer if there are active
                 connections. */
              etimer_restart(&periodic);
              uip_periodic(i);
#if UIP_CONF_IPV6
              tcpip_ipv6_output();
#else
              if(uip_len > 0) {
		PRINTF("tcpip_output from periodic len %d\n", uip_len);
                tcpip_output();
		PRINTF("tcpip_output after periodic len %d\n", uip_len);
              }
#endif /* UIP_CONF_IPV6 */
            }
          }
#endif /* UIP_TCP */
#if UIP_CONF_IP_FORWARD
          uip_fw_periodic();
#endif /* UIP_CONF_IP_FORWARD */
        }
        
#if UIP_CONF_IPV6
#if UIP_CONF_IPV6_REASSEMBLY
        /*
         * check the timer for reassembly
         */
        if(data == &uip_reass_timer &&
           etimer_expired(&uip_reass_timer)) {
          uip_reass_over();
          tcpip_ipv6_output();
        }
#endif /* UIP_CONF_IPV6_REASSEMBLY */
        /*
         * check the different timers for neighbor discovery and
         * stateless autoconfiguration
         */
        if(data == &uip_nd6_timer_periodic &&
           etimer_expired(&uip_nd6_timer_periodic)) {
          uip_nd6_periodic();
          tcpip_ipv6_output();
        }
	    
        if(data == &uip_netif_timer_dad &&
           etimer_expired(&uip_netif_timer_dad)){
          uip_netif_dad();
          tcpip_ipv6_output();
        }
	    
        if(data == &uip_netif_timer_rs &&
           etimer_expired(&uip_netif_timer_rs)){
          uip_netif_send_rs();
          tcpip_ipv6_output();
        }

        if(data == &uip_netif_timer_periodic &&
           etimer_expired(&uip_netif_timer_periodic)){
          uip_netif_periodic();
        }
#endif /* UIP_CONF_IPV6 */
      }
      break;
	 
#if UIP_TCP
    case TCP_POLL:
      if(data != NULL) {
        uip_poll_conn(data);
#if UIP_CONF_IPV6
        tcpip_ipv6_output();
#else /* UIP_CONF_IPV6 */
        if(uip_len > 0) {
	  PRINTF("tcpip_output from tcp poll len %d\n", uip_len);
          tcpip_output();
        }
#endif /* UIP_CONF_IPV6 */
        /* Start the periodic polling, if it isn't already active. */
        start_periodic_tcp_timer();
      }
      break;
#endif /* UIP_TCP */
#if UIP_UDP
    case UDP_POLL:
      if(data != NULL) {
        uip_udp_periodic_conn(data);
#if UIP_CONF_IPV6
        tcpip_ipv6_output();
#else
        if(uip_len > 0) {
          tcpip_output();
        }
#endif /* UIP_UDP */
      }
      break;
#endif /* UIP_UDP */

    case PACKET_INPUT:
      packet_input();
      break;
  };
}
/*---------------------------------------------------------------------------*/
void
tcpip_input(void)
{
  process_post_synch(&tcpip_process, PACKET_INPUT, NULL);
  uip_len = 0;
#if UIP_CONF_IPV6
  uip_ext_len = 0;
#endif /*UIP_CONF_IPV6*/
}
/*---------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
void
tcpip_ipv6_output(void)
{
  struct uip_nd6_neighbor *nbc = NULL;
  struct uip_nd6_defrouter *dr = NULL;
  
  if(uip_len == 0)
    return;

  if(uip_len > UIP_LINK_MTU){
    UIP_LOG("tcpip_ipv6_output: Packet to big");
    uip_len = 0;
    return;
  }
  if(uip_is_addr_unspecified(&UIP_IP_BUF->destipaddr)){
    UIP_LOG("tcpip_ipv6_output: Destination address unspecified");
    uip_len = 0;
    return;
  }
  if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
    /*If destination is on link */
    nbc = NULL;
    if(uip_nd6_is_addr_onlink(&UIP_IP_BUF->destipaddr)){
      nbc = uip_nd6_nbrcache_lookup(&UIP_IP_BUF->destipaddr);
    } else {
#if UIP_CONF_ROUTER
      /*destination is not on link*/
      uip_ipaddr_t ipaddr;
      uip_ipaddr_t *next_hop;

      /* Try to find the next hop address in the local routing table. */
      next_hop = uip_router != NULL ?
        uip_router->lookup(&UIP_IP_BUF->destipaddr, &ipaddr) : NULL;
      if(next_hop != NULL) {
        /* Look for the next hop of the route in the neighbor cache.
           Add a cache entry if we can't find it. */
        nbc = uip_nd6_nbrcache_lookup(next_hop);
        if(nbc == NULL) {
          nbc = uip_nd6_nbrcache_add(next_hop, NULL, 1, NO_STATE);
        }
      } else {
#endif /* UIP_CONF_ROUTER */
        /* No route found, check if a default router exists and use it then. */
        dr = uip_nd6_choose_defrouter();
        if(dr != NULL){
          nbc = dr->nb;
        } else {
          /* shall we send a icmp error message destination unreachable ?*/
          UIP_LOG("tcpip_ipv6_output: Destination off-link but no router");
          uip_len = 0;
          return;
        }
#if UIP_CONF_ROUTER
      }
#endif /* UIP_CONF_ROUTER */
    }
    /* there are two cases where the entry logically does not exist:
     * 1 it really does not exist. 2 it is in the NO_STATE state */
    if (nbc == NULL || nbc->state == NO_STATE) {
      if (nbc == NULL) {
        /* create neighbor cache entry, original packet is replaced by NS*/
        nbc = uip_nd6_nbrcache_add(&UIP_IP_BUF->destipaddr, NULL, 0, INCOMPLETE);
      } else {
        nbc->state = INCOMPLETE;
      }
#if UIP_CONF_IPV6_QUEUE_PKT
      /* copy outgoing pkt in the queuing buffer for later transmmit */
      memcpy(nbc->queue_buf, UIP_IP_BUF, uip_len);
      nbc->queue_buf_len = uip_len;
#endif
      /* RFC4861, 7.2.2:
       * "If the source address of the packet prompting the solicitation is the
       * same as one of the addresses assigned to the outgoing interface, that
       * address SHOULD be placed in the IP Source Address of the outgoing
       * solicitation.  Otherwise, any one of the addresses assigned to the
       * interface should be used."*/
      if(uip_netif_is_addr_my_unicast(&UIP_IP_BUF->srcipaddr)){
        uip_nd6_io_ns_output(&UIP_IP_BUF->srcipaddr, NULL, &nbc->ipaddr);
      } else {
        uip_nd6_io_ns_output(NULL, NULL, &nbc->ipaddr);
      }

      stimer_set(&(nbc->last_send),
                uip_netif_physical_if.retrans_timer / 1000);
      nbc->count_send = 1;
    } else {
      if (nbc->state == INCOMPLETE){
        PRINTF("tcpip_ipv6_output: neighbor cache entry incomplete\n");
#if UIP_CONF_IPV6_QUEUE_PKT
        /* copy outgoing pkt in the queuing buffer for later transmmit and set
           the destination neighbor to nbc */
        memcpy(nbc->queue_buf, UIP_IP_BUF, uip_len);
        nbc->queue_buf_len = uip_len;
        uip_len = 0;
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
        return;
      }
      /* if running NUD (nbc->state == STALE, DELAY, or PROBE ) keep
         sending in parallel see rfc 4861 Node behavior in section 7.7.3*/
	 
      if (nbc->state == STALE){
        nbc->state = DELAY;
        stimer_set(&(nbc->reachable),
                  UIP_ND6_DELAY_FIRST_PROBE_TIME);
        PRINTF("tcpip_ipv6_output: neighbor cache entry stale moving to delay\n");
      }
      
      stimer_set(&(nbc->last_send),
                uip_netif_physical_if.retrans_timer / 1000);
      
      tcpip_output(&(nbc->lladdr));


#if UIP_CONF_IPV6_QUEUE_PKT
      /* Send the queued packets from here, may not be 100% perfect though.
       * This happens in a few cases, for example when instead of receiving a
       * NA after sendiong a NS, you receive a NS with SLLAO: the entry moves
       *to STALE, and you must both send a NA and the queued packet
       */
      if(nbc->queue_buf_len != 0) {
        uip_len = nbc->queue_buf_len;
        memcpy(UIP_IP_BUF, nbc->queue_buf, uip_len);
        nbc->queue_buf_len = 0;
        tcpip_output(&(nbc->lladdr));
      }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/

      uip_len = 0;
      return;
    }
  }
   
  /*multicast IP destination address */
  tcpip_output(NULL);
  uip_len = 0;
  uip_ext_len = 0;
   
}
#endif
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
tcpip_poll_udp(struct uip_udp_conn *conn)
{
  process_post(&tcpip_process, UDP_POLL, conn);
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_TCP
void
tcpip_poll_tcp(struct uip_conn *conn)
{
  process_post(&tcpip_process, TCP_POLL, conn);
}
#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
void
tcpip_uipcall(void)
{
  register uip_udp_appstate_t *ts;
  
#if UIP_UDP
  if(uip_conn != NULL) {
    ts = &uip_conn->appstate;
  } else {
    ts = &uip_udp_conn->appstate;
  }
#else /* UIP_UDP */
  ts = &uip_conn->appstate;
#endif /* UIP_UDP */

#if UIP_TCP
 {
   static unsigned char i;
   register struct listenport *l;
   
   /* If this is a connection request for a listening port, we must
      mark the connection with the right process ID. */
   if(uip_connected()) {
     l = &s.listenports[0];
     for(i = 0; i < UIP_LISTENPORTS; ++i) {
       if(l->port == uip_conn->lport &&
	  l->p != PROCESS_NONE) {
	 ts->p = l->p;
	 ts->state = NULL;
	 break;
       }
       ++l;
     }
     
     /* Start the periodic polling, if it isn't already active. */
     start_periodic_tcp_timer();
   }
 }
#endif /* UIP_TCP */
  
  if(ts->p != NULL) {
    process_post_synch(ts->p, tcpip_event, ts->state);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcpip_process, ev, data)
{
  PROCESS_BEGIN();
  
#if UIP_TCP
 {
   static unsigned char i;
   
   for(i = 0; i < UIP_LISTENPORTS; ++i) {
     s.listenports[i].port = 0;
   }
   s.p = PROCESS_CURRENT();
 }
#endif

  tcpip_event = process_alloc_event();
#if UIP_CONF_ICMP6
  tcpip_icmp6_event = process_alloc_event();
#endif /* UIP_CONF_ICMP6 */
  etimer_set(&periodic, CLOCK_SECOND / 2);

  uip_init();
  
  while(1) {
    PROCESS_YIELD();
    eventhandler(ev, data);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
