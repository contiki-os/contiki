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
 * $Id: uip_arp.c,v 1.8 2010/12/14 22:45:22 dak664 Exp $
 *
 */

#include "ip64.h"
#include "ip64-eth.h"
#include "ip64-arp.h"

#include <string.h>
#include <stdio.h>

#define printf(...)

struct arp_hdr {
  struct ip64_eth_hdr ethhdr;
  uint16_t hwtype;
  uint16_t protocol;
  uint8_t hwlen;
  uint8_t protolen;
  uint16_t opcode;
  struct uip_eth_addr shwaddr;
  uip_ip4addr_t sipaddr;
  struct uip_eth_addr dhwaddr;
  uip_ip4addr_t dipaddr;
};

struct ethip_hdr {
  struct ip64_eth_hdr ethhdr;
  /* IP header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ip4addr_t srcipaddr, destipaddr;
};

struct ipv4_hdr {
  /* IP header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ip4addr_t srcipaddr, destipaddr;
};

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

struct arp_entry {
  uip_ip4addr_t ipaddr;
  struct uip_eth_addr ethaddr;
  uint8_t time;
};

static const struct ip64_eth_addr broadcast_ethaddr =
  {{0xff,0xff,0xff,0xff,0xff,0xff}};
static const uint16_t broadcast_ipaddr[2] = {0xffff,0xffff};

static struct arp_entry arp_table[UIP_ARPTAB_SIZE];

static uint8_t arptime;
static uint8_t tmpage;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

const uip_ipaddr_t uip_all_zeroes_addr;

/*---------------------------------------------------------------------------*/
/**
 * Initialize the ARP module.
 *
 */
/*---------------------------------------------------------------------------*/
void
ip64_arp_init(void)
{
  int i;
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    memset(&arp_table[i].ipaddr, 0, 4);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Periodic ARP processing function.
 *
 * This function performs periodic timer processing in the ARP module
 * and should be called at regular intervals. The recommended interval
 * is 10 seconds between the calls.
 *
 */
/*---------------------------------------------------------------------------*/
void
ip64_arp_timer(void)
{
  struct arp_entry *tabptr;
  int i;
  
  ++arptime;
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if(uip_ip4addr_cmp(&tabptr->ipaddr, &uip_all_zeroes_addr) &&
       arptime - tabptr->time >= UIP_ARP_MAXAGE) {
      memset(&tabptr->ipaddr, 0, 4);
    }
  }

}

/*---------------------------------------------------------------------------*/
static void
arp_update(uip_ip4addr_t *ipaddr, struct uip_eth_addr *ethaddr)
{
  register struct arp_entry *tabptr = arp_table;
  int i, c;
  
  /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];

    /* Only check those entries that are actually in use. */
    if(!uip_ip4addr_cmp(&tabptr->ipaddr, &uip_all_zeroes_addr)) {

      /* Check if the source IP address of the incoming packet matches
         the IP address in this ARP table entry. */
      if(uip_ip4addr_cmp(ipaddr, &tabptr->ipaddr)) {
	 
	/* An old entry found, update this and return. */
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;

	return;
      }
    }
	tabptr++;
  }

  /* If we get here, no existing ARP table entry was found, so we
     create one. */

  /* First, we try to find an unused entry in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if(uip_ip4addr_cmp(&tabptr->ipaddr, &uip_all_zeroes_addr)) {
      break;
    }
  }

  /* If no unused entry is found, we try to find the oldest entry and
     throw it away. */
  if(i == UIP_ARPTAB_SIZE) {
    tmpage = 0;
    c = 0;
    for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
      tabptr = &arp_table[i];
      if(arptime - tabptr->time > tmpage) {
	tmpage = arptime - tabptr->time;
	c = i;
      }
    }
    i = c;
    tabptr = &arp_table[i];
  }

  /* Now, i is the ARP table entry which we will fill with the new
     information. */
  uip_ip4addr_copy(&tabptr->ipaddr, ipaddr);
  memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
  tabptr->time = arptime;
}
/*---------------------------------------------------------------------------*/
uint16_t
ip64_arp_arp_input(const uint8_t *packet, uint16_t packet_len)
{
  struct arp_hdr *arphdr = (struct arp_hdr *)packet;

  if(packet_len < sizeof(struct arp_hdr)) {
    printf("ip64_arp_arp_input: len too small %d\n", packet_len);
    return 0;
  }

  switch(arphdr->opcode) {
  case UIP_HTONS(ARP_REQUEST):
    /* ARP request. If it asked for our address, we send out a
       reply. */
    printf("ip64_arp_arp_input: request for %d.%d.%d.%d (we are %d.%d.%d.%d)\n",
	   arphdr->dipaddr.u8[0], arphdr->dipaddr.u8[1],
	   arphdr->dipaddr.u8[2], arphdr->dipaddr.u8[3],
	   ip64_get_hostaddr()->u8[0], ip64_get_hostaddr()->u8[1],
	   ip64_get_hostaddr()->u8[2], ip64_get_hostaddr()->u8[3]);
    if(uip_ip4addr_cmp(&arphdr->dipaddr, ip64_get_hostaddr())) {
      /* First, we register the one who made the request in our ARP
	 table, since it is likely that we will do more communication
	 with this host in the future. */
      arp_update(&arphdr->sipaddr, &arphdr->shwaddr);
      
      arphdr->opcode = UIP_HTONS(ARP_REPLY);

      memcpy(arphdr->dhwaddr.addr, arphdr->shwaddr.addr, 6);
      memcpy(arphdr->shwaddr.addr, ip64_eth_addr.addr, 6);
      memcpy(arphdr->ethhdr.src.addr, ip64_eth_addr.addr, 6);
      memcpy(arphdr->ethhdr.dest.addr, arphdr->dhwaddr.addr, 6);

      uip_ip4addr_copy(&arphdr->dipaddr, &arphdr->sipaddr);
      uip_ip4addr_copy(&arphdr->sipaddr, ip64_get_hostaddr());

      arphdr->ethhdr.type = UIP_HTONS(IP64_ETH_TYPE_ARP);
      return sizeof(struct arp_hdr);
    }
    break;
  case UIP_HTONS(ARP_REPLY):
    /* ARP reply. We insert or update the ARP table if it was meant
       for us. */
    if(uip_ip4addr_cmp(&arphdr->dipaddr, ip64_get_hostaddr())) {
      arp_update(&arphdr->sipaddr, &arphdr->shwaddr);
    }
    break;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
ip64_arp_check_cache(const uint8_t *nlhdr)
{
  struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr *)nlhdr;
  uip_ip4addr_t broadcast_addr;
  struct arp_entry *tabptr = arp_table;

  printf("check cache %d.%d.%d.%d\n",
	 uip_ipaddr_to_quad(&ipv4_hdr->destipaddr));
  
  /* First check if destination is a local broadcast. */
  uip_ipaddr(&broadcast_addr, 255,255,255,255);
  if(uip_ip4addr_cmp(&ipv4_hdr->destipaddr, &broadcast_addr)) {
    printf("Return 1\n");
    return 1;
  } else if(ipv4_hdr->destipaddr.u8[0] == 224) {
    /* Multicast. */
    return 1;
  } else {
    uip_ip4addr_t ipaddr;
    int i;
    /* Check if the destination address is on the local network. */
    if(!uip_ipaddr_maskcmp(&ipv4_hdr->destipaddr,
			   ip64_get_hostaddr(),
			   ip64_get_netmask())) {
      /* Destination address was not on the local network, so we need to
	 use the default router's IP address instead of the destination
	 address when determining the MAC address. */
      uip_ip4addr_copy(&ipaddr, ip64_get_draddr());
    } else {
      /* Else, we use the destination IP address. */
      uip_ip4addr_copy(&ipaddr, &ipv4_hdr->destipaddr);
    }
    for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
      if(uip_ip4addr_cmp(&ipaddr, &tabptr->ipaddr)) {
	break;
      }
      tabptr++;
    }

    if(i == UIP_ARPTAB_SIZE) {
      return 0;
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ip64_arp_create_ethhdr(uint8_t *llhdr, const uint8_t *nlhdr)
{
  struct arp_entry *tabptr = arp_table;
  struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr *)nlhdr;
  struct ip64_eth_hdr *ethhdr = (struct ip64_eth_hdr *)llhdr;
  uip_ip4addr_t broadcast_addr;
  
  /* Find the destination IP address in the ARP table and construct
     the Ethernet header. If the destination IP addres isn't on the
     local network, we use the default router's IP address instead.

     If not ARP table entry is found, we overwrite the original IP
     packet with an ARP request for the IP address. */

  /* First check if destination is a local broadcast. */
  uip_ipaddr(&broadcast_addr, 255,255,255,255);
  if(uip_ip4addr_cmp(&ipv4_hdr->destipaddr, &broadcast_addr)) {
    memcpy(&ethhdr->dest.addr, &broadcast_ethaddr.addr, 6);
  } else if(ipv4_hdr->destipaddr.u8[0] == 224) {
    /* Multicast. */
    ethhdr->dest.addr[0] = 0x01;
    ethhdr->dest.addr[1] = 0x00;
    ethhdr->dest.addr[2] = 0x5e;
    ethhdr->dest.addr[3] = ipv4_hdr->destipaddr.u8[1];
    ethhdr->dest.addr[4] = ipv4_hdr->destipaddr.u8[2];
    ethhdr->dest.addr[5] = ipv4_hdr->destipaddr.u8[3];
  } else {
    uip_ip4addr_t ipaddr;
    int i;
    /* Check if the destination address is on the local network. */
    if(!uip_ipaddr_maskcmp(&ipv4_hdr->destipaddr,
			   ip64_get_hostaddr(),
			   ip64_get_netmask())) {
      /* Destination address was not on the local network, so we need to
	 use the default router's IP address instead of the destination
	 address when determining the MAC address. */
      uip_ip4addr_copy(&ipaddr, ip64_get_draddr());
    } else {
      /* Else, we use the destination IP address. */
      uip_ip4addr_copy(&ipaddr, &ipv4_hdr->destipaddr);
    }
    for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
      if(uip_ip4addr_cmp(&ipaddr, &tabptr->ipaddr)) {
	break;
      }
      tabptr++;
    }

    if(i == UIP_ARPTAB_SIZE) {
      return 0;
    }

    memcpy(ethhdr->dest.addr, tabptr->ethaddr.addr, 6);

  }
  memcpy(ethhdr->src.addr, ip64_eth_addr.addr, 6);
  
  ethhdr->type = UIP_HTONS(IP64_ETH_TYPE_IP);
  return sizeof(struct ip64_eth_hdr);
}
/*---------------------------------------------------------------------------*/
int
ip64_arp_create_arp_request(uint8_t *llhdr, const uint8_t *nlhdr)
{
  struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr *)nlhdr;
  struct arp_hdr *arp_hdr = (struct arp_hdr *)llhdr;
  uip_ip4addr_t ipaddr;
  
  if(!uip_ipaddr_maskcmp(&ipv4_hdr->destipaddr,
			 ip64_get_hostaddr(),
			 ip64_get_netmask())) {
    /* Destination address was not on the local network, so we need to
       use the default router's IP address instead of the destination
       address when determining the MAC address. */
    uip_ip4addr_copy(&ipaddr, ip64_get_draddr());
  } else {
    /* Else, we use the destination IP address. */
    uip_ip4addr_copy(&ipaddr, &ipv4_hdr->destipaddr);
  }
  
  memset(arp_hdr->ethhdr.dest.addr, 0xff, 6);
  memset(arp_hdr->dhwaddr.addr, 0x00, 6);
  memcpy(arp_hdr->ethhdr.src.addr, ip64_eth_addr.addr, 6);
  memcpy(arp_hdr->shwaddr.addr, ip64_eth_addr.addr, 6);

  uip_ip4addr_copy(&arp_hdr->dipaddr, &ipaddr);
  uip_ip4addr_copy(&arp_hdr->sipaddr, ip64_get_hostaddr());
  arp_hdr->opcode = UIP_HTONS(ARP_REQUEST);
  arp_hdr->hwtype = UIP_HTONS(ARP_HWTYPE_ETH);
  arp_hdr->protocol = UIP_HTONS(IP64_ETH_TYPE_IP);
  arp_hdr->hwlen = 6;
  arp_hdr->protolen = 4;
  arp_hdr->ethhdr.type = UIP_HTONS(IP64_ETH_TYPE_ARP);
  
  uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];

  return sizeof(struct arp_hdr);
}
/*---------------------------------------------------------------------------*/
