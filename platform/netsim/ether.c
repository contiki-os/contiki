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
 * $Id: ether.c,v 1.17 2010/10/19 18:29:05 adamdunkels Exp $
 */
/**
 * \file
 * This module implements a simple "ether", into which datapackets can
 * be injected. The packets are delivered to all nodes that are in
 * transmission range.
 *
 * \author Adam Dunkels <adam@sics.se>
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include "ether.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "nodes.h"

#include "dev/radio-sensor.h"

#include "dev/serial-line.h"

#include "sensor.h"

#include "node.h"
#include "net/uip.h"
#include "net/uip-fw.h"

#ifndef NULL
#define NULL 0
#endif /* NULL */

MEMB(packets, struct ether_packet, 20000);
LIST(active_packets);

static u8_t rxbuffer[2048];
static clock_time_t timer;

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define PRINTF(...)
/*#define PRINTF(x) printf x*/

static int s, sc;

#define PTYPE_NONE   0
#define PTYPE_CLOCK  1
#define PTYPE_DATA   2
#define PTYPE_SENSOR 3
#define PTYPE_LEDS   4
#define PTYPE_TEXT   5
#define PTYPE_DONE   6
#define PTYPE_SERIAL 7
#define PTYPE_RADIO_STATUS 8

#define SERIAL_LEN 80

struct ether_hdr {
  int type;
  struct sensor_data sensor_data;
  clock_time_t clock;
  int linex, liney;
  int signal;
  int srcx, srcy;
  int srcpid;
  int srcid;
  int srcnodetype;
  int leds;
  int radio_status;
  char text[NODES_TEXTLEN + SERIAL_LEN];
};

static int strength;

static int collisions = 1;
static int num_collisions = 0;
static int num_sent = 0;
static int num_received = 0;
static int drop_probability = 0;
static int num_drops = 0;

#include <sys/time.h>

static struct timeval t1;

static int linex, liney;

/*-----------------------------------------------------------------------------------*/
void
ether_print_stats(void)
{
  unsigned long time;
  struct timeval t2;
  gettimeofday(&t2, NULL);

  time = (t2.tv_sec * 1000 + t2.tv_usec / 1000) -
    (t1.tv_sec * 1000 + t1.tv_usec / 1000);
  /*  printf("%d, %d, %f\n", num_packets, num_collisions, time/1000.0);*/
  printf("Time: %f\n", time/1000.0);
  printf("Total packets sent: %d\n", num_sent);
  printf("Total collisions: %d\n", num_collisions);
  printf("Total packets receptions: %d\n", num_received);
  printf("Total randomly dropped packets: %d\n", num_drops);
}
/*-----------------------------------------------------------------------------------*/
void
ether_set_drop_probability(double p)
{
  drop_probability = p * 65536;
}
/*-----------------------------------------------------------------------------------*/
void
ether_set_collisions(int c)
{
  collisions = c;
}
/*-----------------------------------------------------------------------------------*/
void
ether_set_strength(int s)
{
  strength = s;
}
/*-----------------------------------------------------------------------------------*/
int
ether_strength(void)
{
  return strength;
}
/*-----------------------------------------------------------------------------------*/
void
ether_server_init(void)
{
  struct sockaddr_in sa;

  gettimeofday(&t1, NULL);
  
  memb_init(&packets);
  list_init(active_packets);

  timer = 0;

  s = socket(AF_INET,SOCK_DGRAM,0);

  if(s < 0) {
    perror("ether_server_init: socket");
  }
  
  memset((char *)&sa, 0, sizeof(sa));
  
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");/*uip_htonl(INADDR_ANY);*/

  sa.sin_port = uip_htons(ETHER_PORT);

  /*  printf("Binding to port %d\n", ETHER_PORT);*/
  
  if(bind(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    printf("Bind to port %d\n", ETHER_PORT);
    perror("bind");
    exit(1);
  }

}
/*-----------------------------------------------------------------------------------*/
void
ether_client_init(int port)
{
  struct sockaddr_in sa;
    
  sc = socket(AF_INET,SOCK_DGRAM,0);
  
  if(sc < 0) {
    perror("socket");
  }
  
  memset((char *)&sa, 0, sizeof(sa));
  
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");/*uip_htonl(INADDR_ANY);*/

  sa.sin_port = uip_htons(port);

  /*  printf("ether_client_init: binding to port %d\n", port);*/
  if(bind(sc, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    printf("Bind to port %d\n", port);
    perror("bind");
    exit(1);
  }
}
/*-----------------------------------------------------------------------------------*/
int
ether_client_poll(void)
{
  fd_set fdset;
  struct timeval tv;
  int ret;

  FD_ZERO(&fdset);
  FD_SET(sc, &fdset);

  tv.tv_sec = 0;
  tv.tv_usec = 1000;
  
  ret = select(sc + 1, &fdset, NULL, NULL, &tv);

  if(ret < 0) {
    perror("ether_client_poll: select");
  }
  return ret == 1;
}
/*-----------------------------------------------------------------------------------*/
u16_t
ether_client_read(u8_t *buf, int bufsize)
{
  int ret, len;
  fd_set fdset;
  struct timeval tv;
  struct ether_hdr *hdr = (struct ether_hdr *)rxbuffer;

  FD_ZERO(&fdset);
  FD_SET(sc, &fdset);

  tv.tv_sec = 0;
  tv.tv_usec = 10000;

  ret = select(sc + 1, &fdset, NULL, NULL, &tv);
  
  if(ret == 0) {
    /*    printf("ret 0\n");*/
    return 0;
  }
  if(FD_ISSET(sc, &fdset)) {
    ret = recv(sc, &rxbuffer[0], sizeof(rxbuffer), 0);
    if(ret == -1) {
      perror("ether_client_poll: recv");
      return 0;
    }
    len = ret;

    if(len > bufsize) {
      PRINTF("ether_client_read: packet truncated from %d to %d\n",
	     len, bufsize);
      len = bufsize;
    }

    /*    printf("Incoming len %d\n", len);*/
    memcpy(buf, &rxbuffer[sizeof(struct ether_hdr)], len);
    radio_sensor_signal = hdr->signal;

    if(hdr->type == PTYPE_DATA && hdr->srcid != node.id) {
      return len - sizeof(struct ether_hdr);
    } else if(hdr->type == PTYPE_CLOCK) {
      node_set_time(hdr->clock);
    } else if(hdr->type == PTYPE_SENSOR) {
      int strength = sensor_strength() -
	((hdr->srcx - node_x()) * (hdr->srcx - node_x()) +
	 (hdr->srcy - node_y()) * (hdr->srcy - node_y())) / sensor_strength();
      /*      printf("Dist %d \n", strength);*/
      if(strength > 0) {
	sensor_input(&hdr->sensor_data, strength);
      }
    } else if(hdr->type == PTYPE_SERIAL) {
      char *ptr = hdr->text;
      printf("serial input %s\n", ptr);
      for(ptr = hdr->text; *ptr != 0; ++ptr) {
	serial_line_input_byte(*ptr);
      }
    }
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
ether_server_poll(void)
{
  int ret;
  fd_set fdset;
  struct timeval tv;
  struct ether_hdr *hdr = (struct ether_hdr *)rxbuffer;
  /*  struct timeval rtime1, rtime2;
  struct timespec ts;
  struct timezone tz;*/

  
  tv.tv_sec = 0;
  tv.tv_usec = 100;

  
  do {
    FD_ZERO(&fdset);
    FD_SET(s, &fdset);

    ret = select(s + 1, &fdset, NULL, NULL, &tv);
    if(ret == 0) {
      return;
    }
    if(FD_ISSET(s, &fdset)) {
      ret = recv(s, &rxbuffer[0], sizeof(rxbuffer), 0);
      if(ret == -1) {
	perror("ether_poll: read");
	return;
      }
      nodes_set_line(hdr->srcx, hdr->srcy, hdr->linex, hdr->liney);
      switch(hdr->type) {
      case PTYPE_DATA:
	PRINTF("ether_poll: read %d bytes from (%d, %d)\n",
	       ret, hdr->srcx, hdr->srcy);
	ether_put((char *)rxbuffer, ret, hdr->srcx, hdr->srcy);
	break;
      case PTYPE_LEDS:
	nodes_set_leds(hdr->srcx, hdr->srcy, hdr->leds);
	break;
      case PTYPE_TEXT:
	nodes_set_text(hdr->srcx, hdr->srcy, hdr->text);
	break;
      case PTYPE_DONE:
	nodes_done(hdr->srcid);
	break;
      case PTYPE_SERIAL:
	break;
      case PTYPE_RADIO_STATUS:
	nodes_set_radio_status(hdr->srcx, hdr->srcy, hdr->radio_status);
	break;
      }
    }
    /*    tv.tv_sec = 0;
	  tv.tv_usec = 1;*/

  } while(1/*ret > 0*/);
}
/*-----------------------------------------------------------------------------------*/
void
ether_put(char *data, int len, int x, int y)
{
  struct ether_packet *p;

  /*  printf("ether_put: packet len %d at (%d, %d)\n", len, x, y);*/
  
  p = (struct ether_packet *)memb_alloc(&packets);

  if(p != NULL) {
    if(len > 1500) {
      len = 1500;
    }
    memcpy(p->data, data, len);
    p->len = len;
    p->x = x;
    p->y = y;
    list_push(active_packets, p);


  }
}
/*-----------------------------------------------------------------------------------*/
static void
send_packet(char *data, int len, int port)
{
  struct sockaddr_in sa;
  
  memset((char *)&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  sa.sin_port = uip_htons(port);
  
  if(sendto(s, data, len, 0, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    perror("ether: send_packet: sendto");
  }
}
/*-----------------------------------------------------------------------------------*/
void
ether_tick(void)
{
  struct ether_packet *p, *q;
  struct ether_hdr *hdr;
  int port;
  int x, y;
  int i;
  int interference;

  /* Go through every node and see if there are any packets destined
     to them. If two or more packets are sent in the vicinity of the
     node, they interfere with each otehr and none reaches the
     node. */
  for(i = 0; i < nodes_num(); ++i) {

    x = nodes_node(i)->x;
    y = nodes_node(i)->y;
    port = nodes_node(i)->port;

    /* Go through all active packets to see if anyone is sent within
       range of this node. */
    for(p = list_head(active_packets); p != NULL; p = p->next) {
      
      num_sent++;
      
      /* Update the node type. */
      hdr = (struct ether_hdr *)p->data;
      /*      nodes_node(hdr->srcid)->type = hdr->srcnodetype;*/
      
      if(!(p->x == x && p->y == y) && /* Don't send packets back to
					 the sender. */
	 (p->x - x) * (p->x - x) +
	 (p->y - y) * (p->y - y) <=
	 ether_strength() * ether_strength()) {

	hdr->signal = ether_strength() * ether_strength() -
	  (p->x - x) * (p->x - x) -
	  (p->y - y) * (p->y - y);
	/* This packet was sent in the reception range of this node,
	   so we check against all other packets to see if there is
	   more than one packet sent towards this node. If so, we have
	   interference and the node will not be able to receive any
	   data. */
	interference = 0;
	if(collisions) {
	  for(q = list_head(active_packets); q != NULL; q = q->next) {
	    
	    /* Compute the distance^2 and check against signal strength. */
	    if(p != q &&
	       ((q->x - x) * (q->x - x) +
		(q->y - y) * (q->y - y) <=
		ether_strength() * ether_strength())) {

	      /* If the potentially interfering packets were sent from
		 the same node, then they don't interfere with each
		 other. Otherwise they interfere and we sent the
		 interference flag to 1. */
	      if(p->x != q->x ||
		 p->y != q->y) {
		interference = 1;
	      }
	      break;
	    }
	  }
	}

	if(interference) {
	  num_collisions++;
	  /*	  printf("Collisions %d\n", num_collisions);*/
	}
	
	if(!interference) {
	  /*	  printf("ether: delivering packet from %d to %d\n",
		  hdr->srcid, port);*/
	  if((unsigned int)((rand() * 17) % 65536) >= drop_probability) {
	    send_packet(p->data, p->len, port);
	    num_received++;
	  } else {
 	    num_drops++;
 	  }
	}
      }


    }
  }

  /* Remove all packets from the active packets list. */
  while((p = list_pop(active_packets)) != NULL) {
    memb_free(&packets, (void *) p);
  }

  ++timer;
}
/*-----------------------------------------------------------------------------------*/
struct ether_packet *
ether_packets(void)
{
  return list_head(active_packets);
}
/*-----------------------------------------------------------------------------------*/
clock_time_t
ether_time(void)
{
  return timer;
}
/*-----------------------------------------------------------------------------------*/
static void
node_send_packet(char *data, int len)
{
  struct sockaddr_in sa;
  
  memset((char *)&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  sa.sin_port = uip_htons(ETHER_PORT);
        
  if(sendto(sc, data, len, 0,
	    (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    perror("ether.c node_send_packet: sendto");
  }
}
/*-----------------------------------------------------------------------------------*/
u8_t
ether_send(char *data, int len)
{
  char tmpbuf[2048];
  struct ether_hdr *hdr = (struct ether_hdr *)tmpbuf;
   

  memcpy(&tmpbuf[sizeof(struct ether_hdr)], data, len);
  
  hdr->srcx = node.x;
  hdr->srcy = node.y;
  hdr->type = PTYPE_DATA;
  /*  hdr->srcnodetype = node.type;*/
  hdr->srcid = node.id;

  hdr->linex = linex;
  hdr->liney = liney;
  node_send_packet(tmpbuf, len + sizeof(struct ether_hdr));
  
  return UIP_FW_OK;
}
/*-----------------------------------------------------------------------------------*/
void
ether_set_leds(int leds)
{
  struct ether_hdr hdr;

  memset(&hdr, 0, sizeof (hdr));  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_LEDS;
  hdr.leds = leds;
  /*  hdr.srcnodetype = node.type;*/
  hdr.srcid = node.id;
  hdr.linex = linex;
  hdr.liney = liney;

  node_send_packet((char *)&hdr, sizeof(struct ether_hdr));

}
/*-----------------------------------------------------------------------------------*/
void
ether_set_text(char *text)
{
  struct ether_hdr hdr;
  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_TEXT;
  strncpy(hdr.text, text, NODES_TEXTLEN);
  /*  hdr.srcnodetype = node.type;*/
  hdr.srcid = node.id;
  hdr.linex = linex;
  hdr.liney = liney;

  node_send_packet((char *)&hdr, sizeof(struct ether_hdr));

}
/*-----------------------------------------------------------------------------------*/
void
ether_set_radio_status(int onoroff)
{
  struct ether_hdr hdr;
  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_RADIO_STATUS;
  hdr.radio_status = onoroff;
  hdr.srcid = node.id;
  hdr.linex = linex;
  hdr.liney = liney;

  node_send_packet((char *)&hdr, sizeof(struct ether_hdr));

}
/*-----------------------------------------------------------------------------------*/
void
ether_send_sensor_data(struct sensor_data *d, int srcx, int srcy, int strength)
{
  int port;
  int x, y;
  int i;
  struct ether_hdr hdr;

  /*  printf("Sensor data at (%d, %d)\n", srcx, srcy);*/
  
  for(i = 0; i < nodes_num(); ++i) {

    x = nodes_node(i)->x;
    y = nodes_node(i)->y;
    port = nodes_node(i)->port;

    if((srcx - x) * (srcx - x) +
       (srcy - y) * (srcy - y) <=
       strength * strength) {
      
      hdr.srcx = srcx;
      hdr.srcy = srcy;
      hdr.type = PTYPE_SENSOR;
      hdr.sensor_data = *d;
      send_packet((char *)&hdr, sizeof(hdr), port);
    }
  }

}
/*-----------------------------------------------------------------------------------*/
void
ether_send_done(void)
{
  struct ether_hdr hdr;
  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_DONE;
  hdr.srcid = node.id;

  node_send_packet((char *)&hdr, sizeof(struct ether_hdr));

}
/*-----------------------------------------------------------------------------------*/
void
ether_send_serial(char *str)
{
  struct ether_hdr hdr;
  int len;

  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_SERIAL;
  hdr.srcid = node.id;
  len = strlen(str) + 1;
  if(len > sizeof(hdr.text)) {
    len = sizeof(hdr.text);
  }
  memcpy(&hdr.text, str, len);
  hdr.text[len] = 0;

  /*  printf("ether_send_serial '%s' to %d len %d\n", str, nodes_base_node_port, sizeof(struct ether_hdr));*/
  
  send_packet((char *)&hdr, sizeof(struct ether_hdr), nodes_base_node_port);
}
/*-----------------------------------------------------------------------------------*/
void
ether_set_line(int x, int y)
{
  struct ether_hdr hdr;
  
  linex = x;
  liney = y;

  
  hdr.srcx = node.x;
  hdr.srcy = node.y;
  hdr.type = PTYPE_NONE;
  hdr.srcid = node.id;
  hdr.linex = linex;
  hdr.liney = liney;

  node_send_packet((char *)&hdr, sizeof(struct ether_hdr));
}
/*-----------------------------------------------------------------------------------*/
