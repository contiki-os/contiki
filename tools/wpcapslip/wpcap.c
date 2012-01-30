/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: wpcap.c,v 1.6 2010/10/19 20:30:47 oliverschmidt Exp $
 */



#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __CYGWIN__
#include <alloca.h>
#else /* __CYGWIN__ */
#include <malloc.h>
#endif /* __CYGWIN__ */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <err.h>


#define PROGRESS(x)


static void raw_send(void *buf, int len);

struct pcap;

struct pcap_addr {
  struct pcap_addr *next;
  struct sockaddr *addr;
  struct sockaddr *netmask;
  struct sockaddr *broadaddr;
  struct sockaddr *dstaddr;
};

struct pcap_if {
  struct pcap_if *next;
  char *name;
  char *description;
  struct pcap_addr *addresses;
  DWORD flags;
};

struct pcap_pkthdr {
  struct timeval ts;
  DWORD caplen;
  DWORD len;
};

void wpcap_send(void *buf, int len);

HMODULE wpcap;

static struct pcap *pcap;

static int (* pcap_findalldevs)(struct pcap_if **, char *);
static struct pcap *(* pcap_open_live)(char *, int, int, int, char *);
static int (* pcap_next_ex)(struct pcap *, struct pcap_pkthdr **, unsigned char **);
static int (* pcap_sendpacket)(struct pcap *, unsigned char *, int);


#define BUFSIZE 1514

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

#include "net/uip.h"
#include "net/uip_arp.h"

struct ethip_hdr {
  struct uip_eth_hdr ethhdr;
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
};


struct arp_hdr {
  struct uip_eth_hdr ethhdr;
  uint16_t hwtype;
  uint16_t protocol;
  uint8_t hwlen;
  uint8_t protolen;
  uint16_t opcode;
  struct uip_eth_addr shwaddr;
  uip_ipaddr_t sipaddr;
  struct uip_eth_addr dhwaddr;
  uip_ipaddr_t dipaddr;
}  __attribute__ ((packed));

struct arp_entry {
  uip_ipaddr_t ipaddr;
  struct uip_eth_addr ethaddr;
  uint8_t time;
};
static struct uip_eth_addr uip_ethaddr = {{0,0,0,0,0,0}};
static const uip_ipaddr_t all_zeroes_addr = { { 0x0, /* rest is 0 */ } };
static const struct uip_eth_addr broadcast_ethaddr =
  {{0xff,0xff,0xff,0xff,0xff,0xff}};
static const uip_ipaddr_t broadcast_ipaddr = {{255,255,255,255}};

static struct arp_entry arp_table[UIP_ARPTAB_SIZE];

static uip_ipaddr_t ifaddr, netaddr, netmask;

static int arptime;

static int logging;

static void
log_message(char *msg1, char *msg2)
{
  if(logging) {
    printf("Log: %s %s\n", msg1, msg2);
  }
}
/*---------------------------------------------------------------------------*/
static void
error_exit(char *msg1)
{
  printf("error_exit: %s", msg1);
  exit(EXIT_FAILURE);
}
/*---------------------------------------------------------------------------*/
static void
init_pcap(struct in_addr addr)
{
  struct pcap_if *interfaces;
  struct pcap_addr *paddr;
  char error[256];

  if(pcap_findalldevs(&interfaces, error) == -1) {
    error_exit(error);
  }

  while(interfaces != NULL) {
    log_message("init_pcap: found interface: ", interfaces->description);

    if(interfaces->addresses != NULL) {
      for(paddr = interfaces->addresses;
	  paddr != NULL;
	  paddr = paddr->next) {
	if(paddr->addr != NULL && paddr->addr->sa_family == AF_INET) {
	  
	  struct in_addr interface_addr;
	  interface_addr = ((struct sockaddr_in *)paddr->addr)->sin_addr;
	  log_message("init_pcap:    with address: ", inet_ntoa(interface_addr));
	  
	  if(interface_addr.s_addr == addr.s_addr) {
	    pcap = pcap_open_live(interfaces->name, BUFSIZE, 0, -1, error);
	    if(pcap == NULL) {
	      error_exit(error);
	    }
	    return;
	  }
	}
      }
    }
    interfaces = interfaces->next;
  }

  if(interfaces == NULL) {
    error_exit("no interface found with ip addr specified on cmdline\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
setethaddr(struct uip_eth_addr *a)
{
  memcpy(&uip_ethaddr, a, sizeof(struct uip_eth_addr));
}
/*---------------------------------------------------------------------------*/
static void
set_ethaddr(struct in_addr addr)
{
  PIP_ADAPTER_ADDRESSES adapters;
  ULONG size = 0;

  if(GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST |
				   GAA_FLAG_SKIP_MULTICAST |
				   GAA_FLAG_SKIP_DNS_SERVER,
				   NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) {
    error_exit("error on access to adapter list size\n");
  }
  adapters = alloca(size);
  if(GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST |
				   GAA_FLAG_SKIP_MULTICAST |
				   GAA_FLAG_SKIP_DNS_SERVER,
				   NULL, adapters, &size) != ERROR_SUCCESS) {
    error_exit("error on access to adapter list\n");
  }

  while(adapters != NULL) {

    char buffer[256];
    WideCharToMultiByte(CP_ACP, 0, adapters->Description, -1,
			buffer, sizeof(buffer), NULL, NULL);
    log_message("set_ethaddr: found adapter: ", buffer);

    if(adapters->FirstUnicastAddress != NULL &&
       adapters->FirstUnicastAddress->Address.lpSockaddr != NULL &&
       adapters->FirstUnicastAddress->Address.lpSockaddr->sa_family == AF_INET) {

      struct in_addr adapter_addr;
      adapter_addr = ((struct sockaddr_in *)adapters->FirstUnicastAddress->Address.lpSockaddr)->sin_addr;
      log_message("set_ethaddr:  with address: ", inet_ntoa(adapter_addr));

      if(adapter_addr.s_addr == addr.s_addr) {
	printf("Using local network interface with address %s\n",
	       inet_ntoa(adapter_addr));
	if(adapters->PhysicalAddressLength != 6) {
	  error_exit("ip addr specified on cmdline does not belong to an ethernet card\n");
	}
	wsprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X",
		 adapters->PhysicalAddress[0], adapters->PhysicalAddress[1],
		 adapters->PhysicalAddress[2], adapters->PhysicalAddress[3],
		 adapters->PhysicalAddress[4], adapters->PhysicalAddress[5]);
	log_message("set_ethaddr:  ethernetaddr: ", buffer);

	setethaddr((struct uip_eth_addr *)adapters->PhysicalAddress);
	break;
      }
    }
    adapters = adapters->Next;
  }

  if(adapters == NULL) {
    error_exit("no adapter found with ip addr specified on cmdline\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
print_packet(unsigned char *buf, int len)
{
  int i;

  for(i = 0; i < len; ++i) {
    printf("0x%02x, ", buf[i]);
    if(i % 8 == 7) {
      printf("\n");
    }
  }
  printf("\n\n");
}
/*---------------------------------------------------------------------------*/
static void
uip_arp_update(uip_ipaddr_t *ipaddr, struct uip_eth_addr *ethaddr)
{
  struct arp_entry *tabptr;
  int i, tmpage, c;
  
  /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {

    tabptr = &arp_table[i];
    /* Only check those entries that are actually in use. */
    if(!uip_ipaddr_cmp(&tabptr->ipaddr, &all_zeroes_addr)) {

      /* Check if the source IP address of the incoming packet matches
         the IP address in this ARP table entry. */
      if(uip_ipaddr_cmp(ipaddr, &tabptr->ipaddr)) {
	 
	/* An old entry found, update this and return. */
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;

	return;
      }
    }
  }

  /* If we get here, no existing ARP table entry was found, so we
     create one. */

  /* First, we try to find an unused entry in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if(uip_ipaddr_cmp(&tabptr->ipaddr, &all_zeroes_addr)) {
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
  uip_ipaddr_copy(&tabptr->ipaddr, ipaddr);
  memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
  tabptr->time = arptime;
}
/*---------------------------------------------------------------------------*/
/**
 * Prepend Ethernet header to an outbound IP packet and see if we need
 * to send out an ARP request.
 *
 * This function should be called before sending out an IP packet. The
 * function checks the destination IP address of the IP packet to see
 * what Ethernet MAC address that should be used as a destination MAC
 * address on the Ethernet.
 *
 * If the destination IP address is in the local network (determined
 * by logical ANDing of netmask and our IP address), the function
 * checks the ARP cache to see if an entry for the destination IP
 * address is found. If so, an Ethernet header is prepended and the
 * function returns. If no ARP cache entry is found for the
 * destination IP address, the packet in the uip_buf[] is replaced by
 * an ARP request packet for the IP address. The IP packet is dropped
 * and it is assumed that they higher level protocols (e.g., TCP)
 * eventually will retransmit the dropped packet.
 *
 * If the destination IP address is not on the local network, the IP
 * address of the default router is used instead.
 *
 * When the function returns, a packet is present in the uip_buf[]
 * buffer, and the length of the packet is in the global variable
 * uip_len.
 */
/*---------------------------------------------------------------------------*/
static int
arp_out(struct ethip_hdr *iphdr, int len)
{
#if 1
  struct arp_entry *tabptr;
  struct arp_hdr *arphdr = (struct arp_hdr *)iphdr;
  uip_ipaddr_t ipaddr;
  int i;
#endif

#if 1
  /* Find the destination IP address in the ARP table and construct
     the Ethernet header. If the destination IP addres isn't on the
     local network, we use the default router's IP address instead.

     If not ARP table entry is found, we overwrite the original IP
     packet with an ARP request for the IP address. */

  /* First check if destination is a local broadcast. */
  if(uip_ipaddr_cmp(&iphdr->destipaddr, &broadcast_ipaddr)) {
    memcpy(iphdr->ethhdr.dest.addr, broadcast_ethaddr.addr, 6);
  } else {
    /* Check if the destination address is on the local network. */
#if 1
    if(!uip_ipaddr_maskcmp(&iphdr->destipaddr, &netaddr, &netmask)) {
      /* Destination address was not on the local network, so we need to
	 use the default router's IP address instead of the destination
	 address when determining the MAC address. */
      uip_ipaddr_copy(&ipaddr, &ifaddr);
    } else {
#else
      {
#endif
      /* Else, we use the destination IP address. */
      uip_ipaddr_copy(&ipaddr, &iphdr->destipaddr);
    }
      
    for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
      tabptr = &arp_table[i];
      if(uip_ipaddr_cmp(&ipaddr, &tabptr->ipaddr)) {
	break;
      }
    }

    if(i == UIP_ARPTAB_SIZE) {
      /* The destination address was not in our ARP table, so we
	 overwrite the IP packet with an ARP request. */

      memset(arphdr->ethhdr.dest.addr, 0xff, 6);
      memset(arphdr->dhwaddr.addr, 0x00, 6);
      memcpy(arphdr->ethhdr.src.addr, uip_ethaddr.addr, 6);
      memcpy(arphdr->shwaddr.addr, uip_ethaddr.addr, 6);
    
      uip_ipaddr_copy(&arphdr->dipaddr, &ipaddr);
      uip_ipaddr_copy(&arphdr->sipaddr, &netaddr);
      arphdr->opcode = UIP_HTONS(ARP_REQUEST); /* ARP request. */
      arphdr->hwtype = UIP_HTONS(ARP_HWTYPE_ETH);
      arphdr->protocol = UIP_HTONS(UIP_ETHTYPE_IP);
      arphdr->hwlen = 6;
      arphdr->protolen = 4;
      arphdr->ethhdr.type = UIP_HTONS(UIP_ETHTYPE_ARP);

      /*      uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];*/

      return sizeof(struct arp_hdr);
    }

    /* Build an ethernet header. */
    memcpy(iphdr->ethhdr.dest.addr, tabptr->ethaddr.addr, 6);
    }
#endif /* 0 */
  memcpy(iphdr->ethhdr.src.addr, uip_ethaddr.addr, 6);

  iphdr->ethhdr.type = UIP_HTONS(UIP_ETHTYPE_IP);

  return len + sizeof(struct uip_eth_hdr);
}
/*---------------------------------------------------------------------------*/
void *
do_arp(void *buf, int len)
{
  struct arp_hdr *hdr;

  hdr = (struct arp_hdr *)buf;
  if(hdr->ethhdr.type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
    if(hdr->opcode == UIP_HTONS(ARP_REQUEST)) {
      /* Check if the ARP is for our network */
      /*      printf("ARP for %d.%d.%d.%d we are %d.%d.%d.%d/%d.%d.%d.%d\n",
	     uip_ipaddr_to_quad(&hdr->dipaddr),
	     uip_ipaddr_to_quad(&netaddr),
	     uip_ipaddr_to_quad(&netmask));*/
      if(uip_ipaddr_maskcmp(&hdr->dipaddr, &netaddr, &netmask)) {
	uip_ipaddr_t tmpaddr;
	
	/*	printf("ARP for us.\n");*/
	uip_arp_update(&hdr->sipaddr, &hdr->shwaddr);
	
	hdr->opcode = UIP_HTONS(ARP_REPLY);
	
	memcpy(&hdr->dhwaddr.addr, &hdr->shwaddr.addr, 6);
	memcpy(&hdr->shwaddr.addr, &uip_ethaddr.addr, 6);
	memcpy(&hdr->ethhdr.src.addr, &uip_ethaddr.addr, 6);
	memcpy(&hdr->ethhdr.dest.addr, &hdr->dhwaddr.addr, 6);
	
	uip_ipaddr_copy(&tmpaddr, &hdr->dipaddr);
	uip_ipaddr_copy(&hdr->dipaddr, &hdr->sipaddr);
	uip_ipaddr_copy(&hdr->sipaddr, &tmpaddr);
	
	hdr->ethhdr.type = UIP_HTONS(UIP_ETHTYPE_ARP);
	raw_send(hdr, sizeof(struct arp_hdr));
	return NULL;
      }
    } else if(hdr->opcode == UIP_HTONS(ARP_REPLY)) {
      /* ARP reply. We insert or update the ARP table if it was meant
	 for us. */
      if(uip_ipaddr_maskcmp(&hdr->dipaddr, &netaddr, &netmask)) {
	uip_arp_update(&hdr->sipaddr, &hdr->shwaddr);
      }
    }
  }
  if(hdr->ethhdr.type == UIP_HTONS(UIP_ETHTYPE_IP)) {
    return &((struct ethip_hdr *)hdr)->vhl;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
cleanup(void)
{
  char buf[1024];

  snprintf(buf, sizeof(buf), "route delete %d.%d.%d.%d",
	                     uip_ipaddr_to_quad(&ifaddr));
  printf("%s\n", buf);
  system(buf);
}

/*---------------------------------------------------------------------------*/
static void
remove_route(int s)
{
  cleanup();
  _exit(0);
}
/*---------------------------------------------------------------------------*/
void
wpcap_start(char *ethcardaddr, char *slipnetaddr, char *slipnetmask, int log)
{
  struct in_addr addr;
  char buf[4000];
  uint32_t tmpaddr;
  
  logging = log;
  
  addr.s_addr = inet_addr(ethcardaddr);
  tmpaddr = inet_addr(ethcardaddr);
  memcpy(&ifaddr.u16[0], &tmpaddr, sizeof(tmpaddr));
  tmpaddr = inet_addr(slipnetaddr);
  memcpy(&netaddr.u16[0], &tmpaddr, sizeof(tmpaddr));
  tmpaddr = inet_addr(slipnetmask);
  memcpy(&netmask.u16[0], &tmpaddr, sizeof(tmpaddr));

  printf("Network address %d.%d.%d.%d/%d.%d.%d.%d\n",
	 uip_ipaddr_to_quad(&netaddr),
	 uip_ipaddr_to_quad(&netmask));
  
  snprintf(buf, sizeof(buf), "route add %d.%d.%d.%d mask %d.%d.%d.%d %d.%d.%d.%d",
	   uip_ipaddr_to_quad(&netaddr),
	   uip_ipaddr_to_quad(&netmask),
	   uip_ipaddr_to_quad(&ifaddr));
  printf("%s\n", buf);
  system(buf);
  signal(SIGTERM, remove_route);

  log_message("wpcap_init: cmdline address: ", inet_ntoa(addr));

  
  wpcap = LoadLibrary("wpcap.dll");
  pcap_findalldevs = (int (*)(struct pcap_if **, char *))
		     GetProcAddress(wpcap, "pcap_findalldevs");
  pcap_open_live   = (struct pcap *(*)(char *, int, int, int, char *))
		     GetProcAddress(wpcap, "pcap_open_live");
  pcap_next_ex     = (int (*)(struct pcap *, struct pcap_pkthdr **, unsigned char **))
		     GetProcAddress(wpcap, "pcap_next_ex");
  pcap_sendpacket  = (int (*)(struct pcap *, unsigned char *, int))
		     GetProcAddress(wpcap, "pcap_sendpacket");

  if(pcap_findalldevs == NULL || pcap_open_live  == NULL ||
     pcap_next_ex     == NULL || pcap_sendpacket == NULL) {
    error_exit("error on access to winpcap library\n");
  }

  init_pcap(addr);
  set_ethaddr(addr);

  return;
#if 0
  while(1) {
    int ret;
    
    ret = wpcap_poll(buf);
    if(ret > 0) {
      /*      print_packet(buf, ret);*/
      if(do_arp(buf, ret)) {
	printf("IP packet\n");
      }
    }
    sleep(1);
  }
#endif
}
/*---------------------------------------------------------------------------*/
uint16_t
wpcap_poll(char **buf)
{
  struct pcap_pkthdr *packet_header;
  unsigned char *packet;
  int len;
  char *buf2;

  switch(pcap_next_ex(pcap, &packet_header, &packet)) {
  case -1:
    error_exit("error on poll\n");
  case 0:
    return 0;
  }

  if(packet_header->caplen > BUFSIZE) {
    return 0;
  }

  CopyMemory(*buf, packet, packet_header->caplen);
  len = packet_header->caplen;
  /*  printf("len %d\n", len);*/
  buf2 = do_arp(*buf, len);
  if(buf2 == NULL) {
    return 0;
  } else {
    len = len - (buf2 - *buf);
    *buf = buf2;
    return len;
  }
}
/*---------------------------------------------------------------------------*/
void
wpcap_send(void *buf, int len)
{
  char buf2[4000];

  memcpy(&buf2[sizeof(struct uip_eth_hdr)], buf, len);
  len = arp_out((struct ethip_hdr *)buf2, len);

  raw_send(buf2, len);
}
/*---------------------------------------------------------------------------*/
static void
raw_send(void *buf, int len)
{
  /*  printf("sending len %d\n", len);*/
  if(pcap_sendpacket(pcap, buf, len) == -1) {
    print_packet(buf, len);
    error_exit("error on send\n");
  }
}
/*---------------------------------------------------------------------------*/
void
wpcap_exit(void)
{
  FreeLibrary(wpcap);
}
/*---------------------------------------------------------------------------*/
