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
#include "ieee80211_driver.h"
#include "netstack.h"
#include "packetbuf.h"
#include "ieee80211_iface_setup_process.h"
#include "ibss_setup_process.h"
#include "net_scheduler_process.h"
#include "rimeaddr.h"
#include <stdint-gcc.h>
#include "string.h"
#include "ieee80211.h"
#include "uip.h"
#include "ibss_main.h"

#define IEEE80211_DRIVER_DEBUG	1
#define IEEE80211_DRIVER_DEBUG_DEEP	0

#define DEBUG DEBUG_PRINT
#include "uip-debug.h"

/* Structure defining the sequence number. */
struct seqno {
	rimeaddr_t  sender;
	uint16_t seqno;
};

#ifdef NETSTACK_CONF_MAC_SEQNO_HISTORY
#define MAX_SEQNOS NETSTACK_CONF_MAC_SEQNO_HISTORY
#else /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
#define MAX_SEQNOS 16
#endif /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
static struct seqno received_seqnos[MAX_SEQNOS];


/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr) {
	
	/* Prepare and send the packet down for transmission. 
	 * The packet already lies in the packet buffer. We
	 * synchronously send the packet to the IBSS layer,
	 * blocking the execution until the packet eventually
	 * reaches the driver queue.
	 */
		
	uip_lladdr_t* local_dest_address = (uip_lladdr_t*)packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
	
	/* Check if the outgoing destination address is ok. */
	if (local_dest_address == NULL) {
		printf("ERROR: Could not send packet to a NULL MAC destination address.\n");
		sent(NULL, false, 0);
	}
			
	/* Send it here by calling the IBSS Kernel stack routines.
	 * This should also propagate the NET call-back function, 
	 * so the upper layers can be informed about the status of
	 * the transmission.
	 */
	ieee80211_drv_tx(sent, ptr);
}


/*---------------------------------------------------------------------------*/
static void
mac_frame_input(void)
{		
	#if IEEE80211_DRIVER_DEBUG_DEEP
	PRINTF("IEEE80211driver: mac_frame_input.\n");	
	#endif
	/* Theoretically, now is the time where the framer goes on
	 * with the frame parsing. This has, however, been done in
	 * earlier implementation steps. So now the "parsing" will
	 * just remove the MAC header, leaving the network layer 
	 * payload for the NETSTACK_NETWORK driver.
	 */
	uint8_t hdr_length = sizeof(struct ieee80211_hdr_3addr) + 2 + 6;
	
	int hdr = packetbuf_hdrreduce(hdr_length);
	if (!hdr) {
		printf("ERROR: Could not remove MAC header from the packet buffer.\n");
		return;
	}
	
	int j;
	uint8_t* pkt_data = packetbuf_dataptr();
	#if IEEE80211_DRIVER_DEBUG_DEEP
	PRINTF("IEEE80211_drv: [%u].\n", packetbuf_datalen());
	#endif
	/* We do not need to check whether the packet is destined 
	 * for us, since this has been done by the ieee80211_rx.c
	 * implementation. So we just do sanity checks here.
	 */
	if (packetbuf_totlen() > 0 && packetbuf_datalen() > 0) {
		
	   /* Check for duplicate packet by comparing the sequence number
        * of the incoming packet with the last few ones we saw. This
		* is the same as in Contiki MAC.
		*/
	   int i;
	   for (i=0; i< MAX_SEQNOS; i++) {
		   if(packetbuf_attr(PACKETBUF_ATTR_PACKET_ID) == received_seqnos[i].seqno && 
				rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),&received_seqnos[i].sender)) {
			    /* Drop the packet. */
			    PRINTF("Dropping MAC Duplicate.\n");
				return;
		    }		   
	   }
	   /* It seems that this is not a duplicate. So we update the history for
	    * the received packet sequence numbers, dropping the oldest one and
		* adding the current packet sequence number and sender address.
		*/
	   for(i=MAX_SEQNOS-1; i>0; --i) {
		   memcpy(&received_seqnos[i], &received_seqnos[i - 1],sizeof(struct seqno));
	   }
	   
	   received_seqnos[MAX_SEQNOS-1].seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);
	   rimeaddr_copy(&received_seqnos[MAX_SEQNOS-1].sender, packetbuf_addr(PACKETBUF_ADDR_SENDER));
	}
	
	/* Finally, the input function of the Network Driver shall be called. Note
	 * that the packet we send up to the network layer contains only the layer
	 * 3 payload; all MAC layer content has been removed.
	 */
	NETSTACK_NETWORK.input();
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
	/* Start the process that will handle the MAC initialization. */
	process_start(&ieee80211_iface_setup_process, NULL);
}

/*---------------------------------------------------------------------------*/
const struct mac_driver ieee80211_driver = {
	"ieee80211_mac_driver",
	init,
	send_packet,
	mac_frame_input,
	NULL,
	NULL,
	NULL,
};
/*---------------------------------------------------------------------------*/