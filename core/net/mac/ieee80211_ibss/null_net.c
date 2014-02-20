/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "contiki-conf.h"
#include "string.h"
#include "compiler.h"
#include "null_net.h"
#include "stdio.h"
#include "tcpip.h"
#include "null_net.h"
#include "uipopt.h"
#include "uip.h"
#include "packetbuf.h"
#include "uip_arp.h"
#include "ieee80211_ibss.h"
#include "null_net.h"
#include "string.h"
 

#define NULL_NET_DEBUG	1
#define NULL_NET_DEBUG_DEEP		0

#define DEBUG DEBUG_PRINT
#include "uip-debug.h"

#if UIP_CONF_IPV6

/* A callback function to declare whether the MAC actually sent the network packet. */
static void packet_sent(void* ptr, int status, int retransmissions) {
	
	UNUSED(ptr);

	if(status) {
		#if NULL_NET_DEBUG_DEEP
		PRINTF("Nullnet: Packet was sent/schedulled successfully.\n");
		#endif
	}	
	else {
		
		if (retransmissions == 0) {
			#if NULL_NET_DEBUG_DEEP
			PRINTF("Nullnet: Packet was NOT attempted.\n");
			#endif
		} else {
			PRINTF("Nullnet: Packet was NOT sent/schedulled successfully.\n");
		}
	}	
	
	/* Think about it: Shall we inform the UIP or not?*/	
}


/*--------------------------------------------------------------------*/
/** \brief Process a received 802.11 packet.
 *  \param r The MAC layer
 *
 * The 802.11 packet payload is put in the "packetbuf" by the underlying
 * MAC. It is actually an IP packet. The IP packet must be complete and,
 * thus, it is copied to the "uip_buf" and the IP layer is called. The 
 * module, actually, does nothing important.
 */
static void input(void) {		
	#if NULL_NET_DEBUG
	PRINTF("NULL_NET: Packet received [%u].\n",packetbuf_datalen());
	#endif
	/* First, copy the packet to the "uip_buf" buffer. */
	memcpy((uint8_t *)UIP_IP_BUF, (uint8_t*)(packetbuf_dataptr()), packetbuf_datalen());
	//memcpy(uip_buf, (uint8_t*)(packetbuf_dataptr()), packetbuf_datalen());
	/* Update the length of the UIP buffer. */
	uip_len = packetbuf_datalen();
		
	/* Deliver the packet to the "uip" layer. */
	tcpip_input();		
}


/*--------------------------------------------------------------------*/
/** \brief Take an IP packet and format it to be sent on an 802.11
 *  network using the underlying WiFi implementation.
 *  \param localdest The MAC address of the destination
 *
 *  The IP packet is initially in uip_buf. The resulting packet is 
 *  put in the "packetbuf" and delivered to the 802.11 MAC module.
 */
static uint8_t output(uip_lladdr_t *localdest) {
	
	/* A variable to hold the destination Ethernet address [6 bytes]. */
	uip_lladdr_t local_dest_address; 
	
	/* A temporary stack variable holding the broadcast destination address.*/
	uint8_t local_broadcast_eth_address[UIP_LLADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		
	/* Clear the packet buffer. */
	packetbuf_clear();
	
	/* Assign a reference to the beginning of the payload. */
	uint8_t* pkt_data_ptr = packetbuf_dataptr();
	
	if (localdest == NULL) {
		/* If the local address is NULL, we are sending a broadcast packet. */
		memcpy(local_dest_address.addr, local_broadcast_eth_address, UIP_LLADDR_LEN);
	
	} else {
		/* Otherwise we are sending to the specified Ethernet address. */
		memcpy(local_dest_address.addr, localdest->addr, UIP_LLADDR_LEN);
	}	
	#if NULL_NET_DEBUG
	PRINTF("NULL_NET: Sending packet [%u] to %02x:%02x:%02x:%02x:%02x:%02x.\n",
		uip_len,
		local_dest_address.addr[0],
		local_dest_address.addr[1],
		local_dest_address.addr[2],
		local_dest_address.addr[3],
		local_dest_address.addr[4],
		local_dest_address.addr[5]);
	#endif
	
	if (uip_len <= 0) {
		PRINTF("WARNING: NULL_NET; Attempt to send an empty / corrupted UIP packet.\n");
		return 0;
	} else {
		#if NULL_NET_DEBUG_DEEP		
		int i;
		printf("UIP [%u]: ",uip_len);
		for(i=0; i<uip_len; i++) {
			printf("%02x ", uip_buf[i]);
		}
		printf(" \n");
		#endif
	}
	
	/* Copy the UIP payload to the packet buffer. */
	memcpy(pkt_data_ptr, uip_buf, uip_len);
	
	/* Update the data length of the packet buffer. */
	packetbuf_set_datalen(uip_len);
	
	/* Set the destination address as an attribute in the packet buffer. 
	 * Note that we only use a 6-byte Ethernet address, so this might be
	 * a source of instability, if accessing beyond the bounds creates a
	 * segmentation fault.
	 */
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (rimeaddr_t*)&local_dest_address);

	/* Send the packet down to the MAC. Call the interface function "send" 
	 * while the packet lies already in the packet buffer.
	 */
	NETSTACK_MAC.send(packet_sent, NULL);	
	
	return 1;
}


/* Initialize the nullnet module. */
static void nullnet_init(void) {
	
	/*
	 * Set out output function as the function to be called from uIP to
	 * send a network packet.
	 */
	tcpip_set_outputfunc(output);
	
	/*
	 * Set the Ethernet address for the ARP code. 
	 */	
	uip_lladdr_t eth_addr;
	memcpy(eth_addr.addr, unique_vif->addr, UIP_LLADDR_LEN);
	uip_setethaddr(eth_addr);
	#if NULL_NET_DEBUG
	PRINTF("NULL_NET: Copying MAC address to UIP ARP: %02x:%02x:%02x:%02x:%02x:%02x.\n",
			uip_lladdr.addr[0],
			uip_lladdr.addr[1],
			uip_lladdr.addr[2],
			uip_lladdr.addr[3],
			uip_lladdr.addr[4],
			uip_lladdr.addr[5]);
	#endif
}


/*--------------------------------------------------------------------*/
const struct network_driver nullnet_driver = {
	"nullnet",
	nullnet_init,
	input
};

#endif /* UIP_CONF_IPV6 */