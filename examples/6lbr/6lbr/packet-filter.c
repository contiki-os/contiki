#include "contiki-net.h"
#include "net/uip-neighbor.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"
#include "string.h"
#include "sicslow-ethernet.h"

#include "cetic-bridge.h"
#include "nvm-config.h"

#include "eth-drv.h"

extern const rimeaddr_t rimeaddr_null;

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

static int eth_output(uip_lladdr_t *  src, uip_lladdr_t *  dest);

/*---------------------------------------------------------------------------*/


static outputfunc_t wireless_outputfunc;
static inputfunc_t tcpip_inputfunc;

#define BUF ((struct uip_eth_hdr *)&ll_header[0])

#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF                      ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ND6_NS_BUF            ((uip_nd6_ns *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NA_BUF            ((uip_nd6_na *)&uip_buf[uip_l2_l3_icmp_hdr_len])

#define IS_EUI48_ADDR(a) ((a) != NULL && (a)->addr[3] == 0xFF && (a)->addr[4] == 0xFE)
#define IS_BROADCAST_ADDR(a) ((a)==NULL || rimeaddr_cmp((rimeaddr_t *)(a), &rimeaddr_null) != 0)

/*---------------------------------------------------------------------------*/

static void send_to_uip(void) {
	if(tcpip_inputfunc != NULL) {
		tcpip_inputfunc();
	} else {
		PRINTF("No input function set\n");
	}
}
/*---------------------------------------------------------------------------*/

static void wireless_input(void) {    
	int processFrame = 0;
	int forwardFrame = 0;
	PRINTF("wireless_input\n");
	//Destination filtering
	//---------------------
	if (IS_BROADCAST_ADDR(packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) { //Broadcast
    PRINTF("wireless_input : broadcast\n");
		forwardFrame = 1;
		processFrame = 1;
	} else { //unicast
    PRINTF("wireless_input: dest: ");
    PRINTLLADDR((uip_lladdr_t *) packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    PRINTF("\n");
    if (rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), (rimeaddr_t *)&wsn_mac_addr) != 0 ) {
      processFrame = 1; //For us
    } else { //For another host
#if CETIC_6LBR_TRANSPARENTBRIDGE
      //Not for us, forward it directly
      forwardFrame = 1;
      PRINTF("wireless_input : to forward\n");
#endif
    } 
  }

	//Packet forwarding
	//-----------------
#if CETIC_6LBR_TRANSPARENTBRIDGE
	if (forwardFrame) {
		PRINTF("wireless_input: forwarding frame\n");
		if (eth_output((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER), 
            (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
			//Restore packet as eth_output might have converted its content
			PRINTF("wireless_input: conversion of MAC\n");
			mac_translateIPLinkLayer(ll_802154_type);
		}
	}
#endif
  

	//Handle packet
	//-------------
	if (processFrame ) {
		PRINTF("wireless_input: processing frame\n");
		send_to_uip();
	}
}

uint8_t wireless_output(uip_lladdr_t *  src, uip_lladdr_t *  dest) {
	int ret;

	//Packet filtering
	//----------------
	//Filter out Ethernet segment traffic
	if ( IS_EUI48_ADDR(dest)) {
		return 0;
	}
	//Filter out RA/RS towards WSN
	if (UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && (UIP_ICMP_BUF->type == ICMP6_RS || UIP_ICMP_BUF->type == ICMP6_RA) ) {
		return 0;
	}

	//Packet sending
	//--------------
	if(wireless_outputfunc != NULL) {
#if CETIC_6LBR_TRANSPARENTBRIDGE
		//Set source address (must be done by hacking node address)
		rimeaddr_set_node_addr((rimeaddr_t *)src);
#endif
		PRINTF("wireless_output: sending packet\n");
		ret = wireless_outputfunc(dest);
#if CETIC_6LBR_TRANSPARENTBRIDGE
		//Restore node address
		rimeaddr_set_node_addr((rimeaddr_t *)&wsn_mac_addr);
#endif
	} else {
		ret = 0;
	}
	return ret;
}

/*---------------------------------------------------------------------------*/

void eth_input(void)
{
	uip_lladdr_t srcAddr;
	uip_lladdr_t destAddr;
	int processFrame = 0;
	int forwardFrame = 0;

	//Packet type filtering
	//---------------------
	//Keep only IPv6 traffic
	if (BUF->type != UIP_HTONS(UIP_ETHTYPE_IPV6)) {
		PRINTF("eth_input: Dropping packet type=0x%04x\n", uip_ntohs(BUF->type));
		uip_len = 0;
		return;
	}

	//Packet source Filtering
	//-----------------------
	/* IPv6 uses 33-33-xx-xx-xx-xx prefix for multicast ND stuff */
	if ((BUF->dest.addr[0] == 0x33) && (BUF->dest.addr[1] == 0x33)) {
		forwardFrame = 1;
		processFrame = 1;
		rimeaddr_copy( (rimeaddr_t *)&destAddr, &rimeaddr_null);
	} else if ((BUF->dest.addr[0] == 0xFF)
			&& (BUF->dest.addr[1] == 0xFF)
			&& (BUF->dest.addr[2] == 0xFF)
			&& (BUF->dest.addr[3] == 0xFF)
			&& (BUF->dest.addr[4] == 0xFF)
			&& (BUF->dest.addr[5] == 0xFF)) {
		/* IPv6 does not use broadcast addresses, hence this should not happen */
		PRINTF("eth_input: Dropping broadcast packet\n\r");
		uip_len = 0;
		return;
	} else {
		/* Complex Address Translation */
		if (mac_createSicslowpanLongAddr(&(BUF->dest.addr[0]), &destAddr) == 0) {
			PRINTF("eth_input: Address translation failed\n\r");
			uip_len = 0;
			return;
		}
	}

	//Packet content rewriting
	//------------------------
	//Some IP packets have link layer in them, need to change them around!
	uint8_t transReturn = mac_translateIPLinkLayer(ll_802154_type);
	if ( transReturn != 0 ) {
		PRINTF("eth_input: IPTranslation returns %d\n\r", transReturn);
	}

	//Destination filtering
	//---------------------
	if (memcmp((uint8_t *) &eth_mac_addr, BUF->dest.addr, 6) == 0) {
		processFrame = 1;
	} else {
#if CETIC_6LBR_TRANSPARENTBRIDGE
		//Not for us, forward it directly
		forwardFrame = 1;
#endif
	}

	//Handle packet
	//-------------
#if CETIC_6LBR_TRANSPARENTBRIDGE
	if ( forwardFrame ) {
		PRINTF("eth_input: Forwarding frame to ");
		PRINTLLADDR(&destAddr);
		PRINTF("\n");
		mac_createSicslowpanLongAddr(&(BUF->src.addr[0]), &srcAddr);
		wireless_output(&srcAddr, &destAddr);
	}
#endif
	if ( processFrame) {
		PRINTF("eth_input: Processing frame\n");
		send_to_uip();
	}

	uip_len = 0;
}

static int eth_output(uip_lladdr_t *  src, uip_lladdr_t *  dest)
{
	uip_ds6_nbr_t *nbr = NULL;
    PRINTF("eth_output: ");
    PRINTLLADDR(dest);
    PRINTF("\n");
	//Packet filtering
	//----------------
	if (uip_len == 0) {
		PRINTF("eth_output: uip_len = 0\n");
		return 0;
	}

	//Filter out traffic not targeted to Ethernet segment
	if ( ! IS_EUI48_ADDR(dest) && ! IS_BROADCAST_ADDR(dest) ) {
		PRINTF("eth_output: Not ethernet destination : ");
		PRINTLLADDR(dest);
		PRINTF("\n");
		return 0;
	}
	//Filter out RPL (broadcast) traffic
	if ( UIP_IP_BUF->proto == UIP_PROTO_ICMP6 &&
			UIP_ICMP_BUF->type == ICMP6_RPL &&
			(nvm_data.mode & CETIC_MODE_FILTER_RPL_MASK) != 0) {
		//PRINTF("eth_output: Filtering RPL traffic\n");
		return 0;
	}

	//IP packet alteration
	//--------------------
#if CETIC_6LBR_ROUTER
	//Modify source address
	if( (nvm_data.mode & CETIC_MODE_REWRITE_ADDR_MASK) != 0 && uip_is_addr_link_local(&UIP_IP_BUF->srcipaddr) && uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr)){
		PRINTF("eth_output: Update src address\n");
		uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &eth_ip_local_addr);
	}
#endif
#if CETIC_6LBR_SMARTBRIDGE
	//Reset Hop Limit when in smart-bridge for NDP packets
	//TODO: Is this still needed after #4467 ?
	if ( UIP_IP_BUF->proto == UIP_PROTO_ICMP6 &&
			(UIP_ICMP_BUF->type == ICMP6_NS || UIP_ICMP_BUF->type == ICMP6_NA || UIP_ICMP_BUF->type == ICMP6_RS || UIP_ICMP_BUF->type == ICMP6_RA )) {
		UIP_IP_BUF->ttl = 255;
	}
#endif
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	//Remove ROUTER flag when in bridge mode
	if ( UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type == ICMP6_NA) {
		//PRINTF("eth_output: Updating NA\n");
		UIP_ND6_NA_BUF->flagsreserved &= ~UIP_ND6_NA_FLAG_ROUTER;
	}
#endif
	//Some IP packets have link layer in them, need to change them around!
	mac_translateIPLinkLayer(ll_8023_type);

	//IP header alteration
	//--------------------
	//Remove Hop-by-hop extension header
	if(uip_ext_len > 0) {
		extern void remove_ext_hdr(void);
		uint8_t proto = *((uint8_t *)UIP_IP_BUF + 40);
		remove_ext_hdr();
		UIP_IP_BUF->proto = proto;
	}

	//Create packet header
	//--------------------
	//Packet type
	BUF->type = uip_htons(UIP_ETHTYPE_IPV6);

	//Destination address
#if CETIC_6LBR_SMARTBRIDGE
	//Check for broadcast message or unknown destination
	//TODO: Is this still needed after #4467 ?
	if( IS_BROADCAST_ADDR(dest) || (nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->destipaddr)) == NULL) {
		PRINTF("eth_output: Creating dest broadcast addr\n");
		if ( ! IS_BROADCAST_ADDR(dest) ) {
			PRINTF("eth_output: Dest addr not in nbr\n");
		}
		BUF->dest.addr[0] = 0x33;
		BUF->dest.addr[1] = 0x33;
		BUF->dest.addr[2] = UIP_IP_BUF->destipaddr.u8[12];
		BUF->dest.addr[3] = UIP_IP_BUF->destipaddr.u8[13];
		BUF->dest.addr[4] = UIP_IP_BUF->destipaddr.u8[14];
		BUF->dest.addr[5] = UIP_IP_BUF->destipaddr.u8[15];
	} else {
		PRINTF("eth_output: Creating dest addr from nbr\n");
		mac_createEthernetAddr(BUF->dest.addr, &nbr->lladdr);
	}
#else
	if(IS_BROADCAST_ADDR(dest)) {
		BUF->dest.addr[0] = 0x33;
		BUF->dest.addr[1] = 0x33;
		BUF->dest.addr[2] = UIP_IP_BUF->destipaddr.u8[12];
		BUF->dest.addr[3] = UIP_IP_BUF->destipaddr.u8[13];
		BUF->dest.addr[4] = UIP_IP_BUF->destipaddr.u8[14];
		BUF->dest.addr[5] = UIP_IP_BUF->destipaddr.u8[15];
	} else {
		mac_createEthernetAddr(BUF->dest.addr, dest);
	}
#endif

	//Source address
#if CETIC_6LBR_TRANSPARENTBRIDGE
	memcpy(BUF->src.addr, src, 6);
#endif
#if CETIC_6LBR_SMARTBRIDGE
#if CETIC_ND_PROXY
	memcpy(BUF->src.addr, eth_mac_addr, 6);
#else
	if((nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr)) == NULL) {
		PRINTF("eth_output: Creating default src addr\n");
		memcpy(BUF->src.addr, eth_mac_addr, 6);
	} else {
		PRINTF("eth_output: Creating src addr from nbr\n");
		mac_createEthernetAddr(BUF->src.addr, &nbr->lladdr);
	}
#endif
#endif
#if CETIC_6LBR_ROUTER
	memcpy(BUF->src.addr, eth_mac_addr, 6);
#endif

	//Sending packet
	//--------------
	PRINTF("eth_output: Sending packet to ethernet");
    PRINTLLADDR(dest);
    PRINTF("\n");
	eth_drv_send();

	return 1;
}

/*---------------------------------------------------------------------------*/

#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE

static uint8_t bridge_output(uip_lladdr_t * a) {
	uip_lladdr_t src;
	PRINTF("bridge_output: Sending packet to ");
	if ( ! IS_BROADCAST_ADDR(a) ) {
		PRINTLLADDR(a);
	} else {
		PRINTF("Broadcast");
	}
	PRINTF("\n");

	wireless_output(&wsn_mac_addr, a);
	eth_output(&wsn_mac_addr, a);

	return 0;
}
#else
static uint8_t bridge_output(uip_lladdr_t * a) {
	if (uip_len == 0) {
		printf("ERROR: Trying to send empty packet\n");
		return 0;
	}
	PRINTF("bridge_output: Sending packet to ");
	if ( ! IS_BROADCAST_ADDR(a) ) {
		PRINTLLADDR(a);
	} else {
		PRINTF("Broadcast");
	}
	PRINTF("\n");
	if ( IS_BROADCAST_ADDR(a)) {
		//Obviously we can not guess the target segment for a multicast packet
		//So we have to check the packet source prefix (and match it on the Ethernet segment prefix)
		//or, in case of link-local packet, check packet type and/or packet data
		if ((UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type == ICMP6_RA) ||
			(UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type == ICMP6_NS &&
			    uip_ipaddr_prefixcmp(&eth_net_prefix, &UIP_ND6_NS_BUF->tgtipaddr, 64)) ||
			uip_ipaddr_prefixcmp(&eth_net_prefix, &UIP_IP_BUF->srcipaddr, 64)) {
			eth_output(NULL, a);
		} else {
			//ret = wireless_output(NULL, a);
			wireless_output(NULL, a);
		}
	} else {
		if (IS_EUI48_ADDR(a) ) {
			eth_output(NULL, a);
		} else {
			wireless_output(NULL, a);
		}
	}
	return 0;
}
#endif

/*---------------------------------------------------------------------------*/

void packet_filter_init(void) {
	wireless_outputfunc = tcpip_get_outputfunc();
	tcpip_set_outputfunc(bridge_output);

	tcpip_inputfunc = tcpip_get_inputfunc();

	tcpip_set_inputfunc(wireless_input);
}
/*---------------------------------------------------------------------------*/
