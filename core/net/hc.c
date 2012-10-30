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

/**
 * \file
 * TCP/IP header compression implementation
 * \author Adam Dunkels <adam@sics.se>
 *
 */

#include "net/hc.h"

#include "net/uip.h"

#include <string.h>

#define FLAGS_COMPRESSED     0x8000
#define FLAGS_BROADCASTDATA  0x4000

struct hc_hdr {
  uint16_t flagsport;
  uip_ipaddr_t srcipaddr;
};

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
  uip_ipaddr_t srcipaddr, destipaddr;
  
  /* UDP header. */
  uint16_t srcport,
    destport;
  uint16_t udplen;
  uint16_t udpchksum;
};

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/**
 * Initialize the header compression module.
 */
/*---------------------------------------------------------------------------*/
void
hc_init(void)
{

}
/*---------------------------------------------------------------------------*/
/**
 * Compress a header
 *
 * This function compresses the TCP/IP headers in a buffer and
 * should be called just before sending out data on the network. A
 * pointer to the compressed header is returned, and len is
 * adjusted.
 *
 * If the header could not be compressed, the function does nothing
 * and returns a NULL pointer.
 *
 * \return A pointer to the start of the compressed header or NULL if
 * the header could not be compressed.
 */
/*---------------------------------------------------------------------------*/
int
hc_compress(uint8_t *buf, int len)
{
  struct hc_hdr *hdr;
  struct udpip_hdr *uhdr;

  hdr = (struct hc_hdr *)buf;
  uhdr = (struct udpip_hdr *)buf;

  /* Check the original TCP/IP header to see if it matches our
     pattern, and compress if it does. */
  
  if(uhdr->vhl == 0x45 &&                      /* Only IPv4 without
						  options. */
     uhdr->len[0] == 0x00 &&                   /* Only packets < 256
						  bytes long. */
     uhdr->ipoffset[0] == 0x00 &&              /* No fragmented IP
						  packets. */
     uhdr->ipoffset[1] == 0x00 &&              /* No fragmented IP
						  packets. */
     uhdr->proto == UIP_PROTO_UDP &&           /* Only UDP packets. */
     uip_ipaddr_cmp(&uhdr->destipaddr, &uip_broadcast_addr) && /* Only
						  link-local broadcast
						  packets. */
     uhdr->destport == uhdr->srcport &&        /* Only packets with
						  the same destination
						  and source port
						  number. */
     (uhdr->destport & UIP_HTONS(0xc000)) == 0) {  /* Only packets with the two
						  highest bits in the port
						  number equal to zero. */

    hdr->flagsport = uip_htons(
			   FLAGS_COMPRESSED    | /* Compressed header. */
			   FLAGS_BROADCASTDATA | /* Broadcast data. */
			   (uip_htons(uhdr->destport) & 0x3fff));
    uip_ipaddr_copy(&hdr->srcipaddr, &uhdr->srcipaddr);

    /* Move the packet data to the end of the compressed header. */
    memcpy((char *)hdr + HC_HLEN,
	   &buf[UIP_IPUDPH_LEN],
	   len - UIP_IPUDPH_LEN);

    /* Return the new packet length. */
    return len - (UIP_IPUDPH_LEN - HC_HLEN);
  }

  /* No compression possible, return NULL pointer. */
  return len;
     
}
/*---------------------------------------------------------------------------*/
/**
 * Inflate (decompress) a header
 *
 * This function should be called to inflate a possibly compressed
 * packet header just after a packet has been received from the
 * network. The function will copy the packet data so that the
 * original header fits and adjusts uip_len.
 *
 */
/*---------------------------------------------------------------------------*/
int
hc_inflate(uint8_t *buf, int len)
{
  struct udpip_hdr *uhdr;
  struct hc_hdr *hdr;
  
  hdr = (struct hc_hdr *)buf;
  
  /* First, check if the header in buf is compressed or not. */
  if((hdr->flagsport & UIP_HTONS(FLAGS_COMPRESSED)) != 0 &&
     (hdr->flagsport & UIP_HTONS(FLAGS_BROADCASTDATA)) != 0) {
    
    /* Move packet data in memory to make room for the uncompressed header. */
    memmove(&buf[UIP_IPUDPH_LEN - HC_HLEN],
	    buf, len);
    uhdr = (struct udpip_hdr *)buf;
    hdr = (struct hc_hdr *)&buf[UIP_IPUDPH_LEN - HC_HLEN];
    
    uip_ipaddr_copy(&uhdr->srcipaddr, &hdr->srcipaddr);
    uhdr->srcport = hdr->flagsport & UIP_HTONS(0x3fff);
    uhdr->destport = hdr->flagsport & UIP_HTONS(0x3fff);
    
    uhdr->udplen = len;
    
    len += UIP_IPUDPH_LEN - HC_HLEN;

    
    uhdr->vhl = 0x45;
    uhdr->tos = 0;
    uhdr->len[0] = 0;
    uhdr->len[1] = len;
    uhdr->ipid[0] = uhdr->ipid[1] = 0xAD;
    uhdr->ipoffset[0] = uhdr->ipoffset[1] = 0;
    uhdr->ttl = 2;
    uhdr->proto = UIP_PROTO_UDP;
    uip_ipaddr_copy(&uhdr->destipaddr, &uip_broadcast_addr);
    uhdr->udpchksum = 0;

    uhdr->ipchksum = 0;
    uhdr->ipchksum = ~(uip_ipchksum());

  }

  return len;
}
/*---------------------------------------------------------------------------*/
