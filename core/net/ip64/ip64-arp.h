/*
 * Copyright (c) 2001-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip_arp.h,v 1.2 2006/08/26 23:58:45 oliverschmidt Exp $
 *
 */

#ifndef IP64_ARP_H
#define IP64_ARP_H

#include "net/ip/uip.h"

#include "ip64-eth.h"


/* The uip_arp_init() function must be called before any of the other
   ARP functions. */
void ip64_arp_init(void);

/* The uip_arp_ipin() function should be called whenever an IP packet
   arrives from the Ethernet. This function refreshes the ARP table or
   inserts a new mapping if none exists. The function assumes that an
   IP packet with an Ethernet header is present in the uip_buf buffer
   and that the length of the packet is in the uip_len variable. */
void ip64_arp_ip_input(const uint8_t *packet, uint16_t packet_len);

/* The uip_arp_arpin() should be called when an ARP packet is received
   by the Ethernet driver. This function also assumes that the
   Ethernet frame is present in the uip_buf buffer. When the
   uip_arp_arpin() function returns, the contents of the uip_buf
   buffer should be sent out on the Ethernet if the uip_len variable
   is > 0. */
uint16_t ip64_arp_arp_input(const uint8_t *packet, uint16_t packet_len);

/* The uip_arp_out() function should be called when an IP packet
   should be sent out on the Ethernet. This function creates an
   Ethernet header before the IP header in the uip_buf buffer. The
   Ethernet header will have the correct Ethernet MAC destination
   address filled in if an ARP table entry for the destination IP
   address (or the IP address of the default router) is present. If no
   such table entry is found, the IP packet is overwritten with an ARP
   request and we rely on TCP to retransmit the packet that was
   overwritten. In any case, the uip_len variable holds the length of
   the Ethernet frame that should be transmitted. */
void ip64_arp_ip_output(uint8_t *packet, uint16_t packet_len);


int ip64_arp_create_ethhdr(uint8_t *link_header,
			   const uint8_t *network_header);

int ip64_arp_create_arp_request(uint8_t *link_header,
				const uint8_t *network_header);

int ip64_arp_check_cache(const uint8_t *nlhdr);


#endif /* IP64_ARP_H */
