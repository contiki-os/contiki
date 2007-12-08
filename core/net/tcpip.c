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
 * $Id: tcpip.c,v 1.10 2007/12/08 23:06:02 oliverschmidt Exp $
 */

#include "contiki-net.h"

#include "net/uip-split.h"

#include <string.h>

process_event_t tcpip_event;

/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  u16_t port;
  struct process *p;
};

/*static struct tcpip_event_args ev_args;*/

static struct etimer periodic;

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  struct process *p;
} s;

enum {
  TCP_POLL,
  UDP_POLL,
  PACKET_INPUT
};

u8_t (* tcpip_output)(void); /* Called on IP packet output. */

unsigned char tcpip_do_forwarding; /* Forwarding enabled.   */
unsigned char tcpip_is_forwarding; /* Forwarding right now? */

PROCESS(tcpip_process, "TCP/IP stack");

/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  if(uip_len > 0) {
    if(tcpip_do_forwarding) {
      tcpip_is_forwarding = 1;
      if(uip_fw_forward() == UIP_FW_LOCAL) {
	tcpip_is_forwarding = 0;
	uip_input();
	if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
	  uip_split_output();
#else
	  tcpip_output();
#endif
	}
      }
      tcpip_is_forwarding = 0;
    } else {
      uip_input();
      if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
	uip_split_output();
#else
	tcpip_output();
#endif
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
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
  
  uip_ipaddr(&addr, 255,255,255,255);
  conn = udp_new(&addr, port, appstate);
  if(conn != NULL) {
    udp_bind(conn, port);
  }
  return conn;
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
static void
eventhandler(process_event_t ev, process_data_t data)
{
  static unsigned char i;
  register struct listenport *l;
  struct process *p;
  
  switch(ev) {
  case PROCESS_EVENT_EXITED:
    /* This is the event we get if a process has exited. We go through
       the TCP/IP tables to see if this process had any open
       connections or listening TCP ports. If so, we'll close those
       connections. */
    p = (struct process *)data;
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
      static unsigned char i;

      /* Check the clock so see if we should call the periodic uIP
	 processing. */
      if(etimer_expired(&periodic)) {
	for(i = 0; i < UIP_CONNS; ++i) {
	  if(uip_conn_active(i)) {
	    /* Only restart the timer if there are active
	       connections. */
	    etimer_restart(&periodic);
	    uip_periodic(i);
	    if(uip_len > 0) {
	      tcpip_output();
	    }
	  }
	}
	
	/*	for(i = 0; i < UIP_UDP_CONNS; i++) {
	  uip_udp_periodic(i);
	  if(uip_len > 0) {
	    tcpip_output();
	  }
	  }*/
	uip_fw_periodic();
      }
    }
    break;
    
  case TCP_POLL:
    if(data != NULL) {
      uip_poll_conn(data);
      if(uip_len > 0) {
	tcpip_output();
      }

      /* Start the periodic polling, if it isn't already active. */
      if(etimer_expired(&periodic)) {
	etimer_restart(&periodic);
      }

    }
    break;
#if UIP_UDP
  case UDP_POLL:
    if(data != NULL) {
      uip_udp_periodic_conn(data);
      if(uip_len > 0) {
	tcpip_output();
      }
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
}
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
tcpip_poll_udp(struct uip_udp_conn *conn)
{
  process_post(&tcpip_process, UDP_POLL, conn);
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
void
tcpip_poll_tcp(struct uip_conn *conn)
{
  process_post(&tcpip_process, TCP_POLL, conn);
}
/*---------------------------------------------------------------------------*/
void
tcpip_uipcall(void)
{
  register uip_udp_appstate_t *ts;
  static unsigned char i;
  register struct listenport *l;

#if UIP_UDP
  if(uip_conn != NULL) {
    ts = &uip_conn->appstate;
  } else {
    ts = &uip_udp_conn->appstate;
  }
#else /* UIP_UDP */
  ts = &uip_conn->appstate;
#endif /* UIP_UDP */

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
    if(etimer_expired(&periodic)) {
      etimer_restart(&periodic);
    }
  }

  if(ts->p != NULL) {
    process_post_synch(ts->p, tcpip_event, ts->state);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcpip_process, ev, data)
{
  int i;
  
  PROCESS_BEGIN();
  
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    s.listenports[i].port = 0;
  }
  s.p = PROCESS_CURRENT();
  tcpip_event = process_alloc_event();
  etimer_set(&periodic, CLOCK_SECOND/2);

  uip_init();
  
  while(1) {
    PROCESS_YIELD();
    eventhandler(ev, data);
  }
  
  PROCESS_END();
}
