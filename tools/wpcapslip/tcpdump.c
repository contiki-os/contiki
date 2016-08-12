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
 */

#include "contiki-net.h"

#include <string.h>
#include <stdio.h>

 struct ip_hdr {
  /* IP header. */
   uint8_t vhl,
    tos,
     len[2],
     ipid[2],
     ipoffset[2],
     ttl,
     proto;
   uint16_t ipchksum;
   uint8_t srcipaddr[4],
     destipaddr[4];
 };

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f

struct tcpip_hdr {
  /* IP header. */
   uint8_t vhl,
    tos,
     len[2],
     ipid[2],
     ipoffset[2],
     ttl,
     proto;
   uint16_t ipchksum;
   uint8_t srcipaddr[4],
     destipaddr[4];
  /* TCP header. */
  uint16_t srcport,
    destport;
  uint8_t seqno[4],
    ackno[4],
    tcpoffset,
    flags,
    wnd[2];
  uint16_t tcpchksum;
  uint8_t urgp[2];
  uint8_t optdata[4];
};

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8

struct icmpip_hdr {
  /* IP header. */
   uint8_t vhl,
    tos,
     len[2],
     ipid[2],
     ipoffset[2],
     ttl,
     proto;
  uint16_t ipchksum;
  uint8_t srcipaddr[4],
    destipaddr[4];
  /* The ICMP and IP headers. */
  /* ICMP (echo) header. */
  uint8_t type, icode;
  uint16_t icmpchksum;
  uint16_t id, seqno;
};


/* The UDP and IP headers. */
struct udpip_hdr {
  /* IP header. */
   uint8_t vhl,
    tos,
     len[2],
     ipid[2],
     ipoffset[2],
     ttl,
     proto;
   uint16_t ipchksum;
  uint8_t srcipaddr[4],
    destipaddr[4];
  
  /* UDP header. */
  uint16_t srcport,
    destport;
  uint16_t udplen;
  uint16_t udpchksum;
};

#define ETHBUF    ((struct eth_hdr *)&packet[0])
#define IPBUF     ((struct ip_hdr *)&packet[0])
#define UDPBUF  ((struct udpip_hdr *)&packet[0])
#define ICMPBUF ((struct icmpip_hdr *)&packet[0])
#define TCPBUF  ((struct tcpip_hdr *)&packet[0])


/*---------------------------------------------------------------------------*/
static void
tcpflags(unsigned char flags, char *flagsstr)
{
  if(flags & TCP_FIN) {
    *flagsstr++ = 'F';
  }
  if(flags & TCP_SYN) {
    *flagsstr++ = 'S';
  }
  if(flags & TCP_RST) {
    *flagsstr++ = 'R';
  }
  if(flags & TCP_ACK) {
    *flagsstr++ = 'A';
  }
  if(flags & TCP_URG) {
    *flagsstr++ = 'U';
  }

  *flagsstr = 0;
}
/*---------------------------------------------------------------------------*/
static char *
n(uint16_t num, char *ptr)
{
  uint16_t d;
  uint8_t a, f;

  if(num == 0) {
    *ptr = '0';
    return ptr + 1;
  } else {
    f = 0;
    for(d = 10000; d >= 1; d /= 10) {
      a = (num / d) % 10;
      if(f == 1 || a > 0) {
	*ptr = a + '0';
	++ptr;
	f = 1;
      }
    }
  }
  return ptr;
}
/*---------------------------------------------------------------------------*/
static char *
d(char *ptr)
{
  *ptr = '.';
  return ptr + 1;
}
/*---------------------------------------------------------------------------*/
static char *
s(char *str, char *ptr)
{
  strcpy(ptr, str);
  return ptr + strlen(str);
}
/*---------------------------------------------------------------------------*/
int
tcpdump_format(uint8_t *packet, uint16_t packetlen,
	       char *buf, uint16_t buflen)
{
  char flags[8];
  if(IPBUF->proto == UIP_PROTO_ICMP) {
    if(ICMPBUF->type == ICMP_ECHO) {
      return s(" ping",
	     n(IPBUF->destipaddr[3], d(
	     n(IPBUF->destipaddr[2], d(
	     n(IPBUF->destipaddr[1], d(
	     n(IPBUF->destipaddr[0],
             s(" ",
	     n(IPBUF->srcipaddr[3], d(
	     n(IPBUF->srcipaddr[2], d(
	     n(IPBUF->srcipaddr[1], d(
             n(IPBUF->srcipaddr[0],
	     buf)))))))))))))))) - buf;
	     
      /*      return sprintf(buf, "%d.%d.%d.%d %d.%d.%d.%d ping",
		     IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		     IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		     IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		     IPBUF->destipaddr[2], IPBUF->destipaddr[3]);*/
    } else if(ICMPBUF->type == ICMP_ECHO_REPLY) {
      return s(" pong",
	     n(IPBUF->destipaddr[3], d(
	     n(IPBUF->destipaddr[2], d(
	     n(IPBUF->destipaddr[1], d(
	     n(IPBUF->destipaddr[0],
             s(" ",
	     n(IPBUF->srcipaddr[3], d(
	     n(IPBUF->srcipaddr[2], d(
	     n(IPBUF->srcipaddr[1], d(
             n(IPBUF->srcipaddr[0],
	     buf)))))))))))))))) - buf;
      /*      return sprintf(buf, "%d.%d.%d.%d %d.%d.%d.%d pong",
		     IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		     IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		     IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		     IPBUF->destipaddr[2], IPBUF->destipaddr[3]);*/
    }
  } else if(IPBUF->proto == UIP_PROTO_UDP) {
      return s(" UDP",
	     n(uip_htons(UDPBUF->destport), d(
	     n(IPBUF->destipaddr[3], d(
	     n(IPBUF->destipaddr[2], d(
	     n(IPBUF->destipaddr[1], d(
	     n(IPBUF->destipaddr[0],
             s(" ",
	     n(uip_htons(UDPBUF->srcport), d(
	     n(IPBUF->srcipaddr[3], d(
	     n(IPBUF->srcipaddr[2], d(
	     n(IPBUF->srcipaddr[1], d(
             n(IPBUF->srcipaddr[0],
	     buf)))))))))))))))))))) - buf;
      /*    return sprintf(buf, "%d.%d.%d.%d.%d %d.%d.%d.%d.%d UDP",
		   IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		   IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		   uip_htons(UDPBUF->srcport),
		   IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		   IPBUF->destipaddr[2], IPBUF->destipaddr[3],
		   uip_htons(UDPBUF->destport));*/
  } else if(IPBUF->proto == UIP_PROTO_TCP) {
    tcpflags(TCPBUF->flags, flags);
      return s(flags,
             s(" ",
	     n(uip_htons(TCPBUF->destport), d(
	     n(IPBUF->destipaddr[3], d(
	     n(IPBUF->destipaddr[2], d(
	     n(IPBUF->destipaddr[1], d(
	     n(IPBUF->destipaddr[0],
             s(" ",
	     n(uip_htons(TCPBUF->srcport), d(
	     n(IPBUF->srcipaddr[3], d(
	     n(IPBUF->srcipaddr[2], d(
	     n(IPBUF->srcipaddr[1], d(
             n(IPBUF->srcipaddr[0],
	     buf))))))))))))))))))))) - buf;
    /*    return sprintf(buf, "%d.%d.%d.%d.%d %d.%d.%d.%d.%d %s",
		   IPBUF->srcipaddr[0], IPBUF->srcipaddr[1],
		   IPBUF->srcipaddr[2], IPBUF->srcipaddr[3],
		   uip_htons(TCPBUF->srcport),
		   IPBUF->destipaddr[0], IPBUF->destipaddr[1],
		   IPBUF->destipaddr[2], IPBUF->destipaddr[3],
		   uip_htons(TCPBUF->destport),
		   flags);  */
  } else {
    strcpy(buf, "Unrecognized protocol");
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
