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
 */

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __CYGWIN__
#include <alloca.h>
#else /* __CYGWIN__ */
#include <malloc.h>
#endif /* __CYGWIN__ */

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "contiki-net.h"
#include "sys/log.h"

#include "net/wpcap.h"

/* Handle native-border-router case where the fallback has ethernet headers.
 * The command line args for native-border-router conflice with the passing
 * of the interface addresses to connect to, so both must be hard coded.
 * See comments in wpcap-drv.c
 */
#ifdef SELECT_CALLBACK
#define FALLBACK_HAS_ETHERNET_HEADERS  1
#endif

#if UIP_CONF_IPV6
#include <ws2tcpip.h>
struct in6_addr addr6;
char addr6str[64];
/*---------------------------------------------------------------------------*/
uint8_t
issame_ip6addr(struct in6_addr addr1, struct in6_addr addr2)
{
 return ((addr1.s6_addr32[0]==addr2.s6_addr32[0]) &&
		 (addr1.s6_addr32[1]==addr2.s6_addr32[1]) &&
		 (addr1.s6_addr32[2]==addr2.s6_addr32[2]) &&
		 (addr1.s6_addr32[3]==addr2.s6_addr32[3]) );
}
/*---------------------------------------------------------------------------*/
uint8_t
iszero_ip6addr(struct in6_addr addr)
{
 return ((addr.s6_addr32[0]==0) &&
		 (addr.s6_addr32[1]==0) &&
		 (addr.s6_addr32[2]==0) &&
		 (addr.s6_addr32[3]==0) );
}
/*---------------------------------------------------------------------------*/
uint8_t
sprint_ip6addr(struct in6_addr addr, char * result)
{
  unsigned char i = 0;
  unsigned char zerocnt = 0;
  unsigned char numprinted = 0;
  char * starting = result;

  *result++='[';
  while (numprinted < 8) {
    if ((addr.s6_addr16[i] == 0) && (zerocnt == 0)) {
      while(addr.s6_addr16[zerocnt + i] == 0) zerocnt++;
      if (zerocnt == 1) {
        *result++ = '0';
         numprinted++;
         break;
      }
      i += zerocnt;
      numprinted += zerocnt;
    } else {
      result += sprintf(result, "%x", (unsigned int) uip_ntohs(addr.s6_addr16[i]));
      i++;
      numprinted++;
    }
    if (numprinted != 8) *result++ = ':';
  }
  *result++=']';
  *result=0;
  return (result - starting);
}

#endif /* UIP_CONF_IPV6 */


#ifdef __CYGWIN__
__attribute__((dllimport)) extern char **__argv[];
#endif /* __CYGWIN__ */

struct pcap;

struct pcap_if {
  struct pcap_if *next;
  char *name;
  char *description;
  struct pcap_addr {
    struct pcap_addr *next;
    struct sockaddr *addr;
    struct sockaddr *netmask;
    struct sockaddr *broadaddr;
    struct sockaddr *dstaddr;
  } *addresses;
  DWORD flags;
};

struct pcap_pkthdr {
  struct timeval ts;
  DWORD caplen;
  DWORD len;
};

HMODULE wpcap;

static struct pcap *pcap;

/* uip_ethaddr is defined in uip.c. It is not used in uip6.c. 
 * If needed for some purpose it can be defined here
 */
#if UIP_CONF_IPV6
//struct uip_eth_addr uip_ethaddr;
#endif

static int (* pcap_findalldevs)(struct pcap_if **, char *);
static struct pcap *(* pcap_open_live)(char *, int, int, int, char *);
static int (* pcap_next_ex)(struct pcap *, struct pcap_pkthdr **, unsigned char **);
static int (* pcap_sendpacket)(struct pcap *, unsigned char *, int);

#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#ifdef UIP_FALLBACK_INTERFACE
static struct pcap *pfall;
struct in_addr addrfall;
#if UIP_CONF_IPV6
struct in_addr6 addrfall6;
#endif

/*---------------------------------------------------------------------------*/
static void
init(void)
{
/* Nothing to do here */
}
/*---------------------------------------------------------------------------*/
uint8_t wfall_send(uip_lladdr_t *lladdr);
#if FALLBACK_HAS_ETHERNET_HEADERS
#undef IPBUF
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[14])
static uip_ipaddr_t last_sender;
#endif

static void
output(void)
{
#if FALLBACK_HAS_ETHERNET_HEADERS&&0
  if(uip_ipaddr_cmp(&last_sender, &UIP_IP_BUF->srcipaddr)) {
    /* Do not bounce packets back to fallback if the packet was received from it */
    PRINTF("FUT: trapping pingpong");
	return;
  }
  uip_ipaddr_copy(&last_sender, &UIP_IP_BUF->srcipaddr);
#endif
    PRINTF("FUT: %u\n", uip_len);
	wfall_send(0);
}

const struct uip_fallback_interface rpl_interface = {
  init, output
};

#endif

/*---------------------------------------------------------------------------*/
static void
error_exit(char *message)
{
  printf("error_exit: %s", message);
  exit(EXIT_FAILURE);
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
        if(adapters->PhysicalAddressLength != 6) {
          error_exit("ip addr specified on cmdline does not belong to an ethernet card\n");
        }
        wsprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X",
          adapters->PhysicalAddress[0], adapters->PhysicalAddress[1],
          adapters->PhysicalAddress[2], adapters->PhysicalAddress[3],
          adapters->PhysicalAddress[4], adapters->PhysicalAddress[5]);
        log_message("set_ethaddr:  ethernetaddr: ", buffer);
#if UIP_CONF_IPV6
//      int i;for (i=0;i<6;i++) uip_ethaddr.addr[i] = adapters->PhysicalAddress[i];
#else
        uip_setethaddr((*(struct uip_eth_addr *)adapters->PhysicalAddress));
#endif
        break;
      }
    }
    adapters = adapters->Next;
  }

  if(adapters == NULL) {
    error_exit("no adapter found with ip addr specified on cmdline\n");
  }
}

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
static void
set_ethaddr6(struct in_addr6 addr)
{
  PIP_ADAPTER_ADDRESSES adapters;
  ULONG size = 0;

  if(GetAdaptersAddresses(AF_INET6, GAA_FLAG_SKIP_ANYCAST |
                                   GAA_FLAG_SKIP_MULTICAST |
                                   GAA_FLAG_SKIP_DNS_SERVER,
                          NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) {
    error_exit("error on access to adapter list size\n");
  }
  adapters = alloca(size);
  if(GetAdaptersAddresses(AF_INET6, GAA_FLAG_SKIP_ANYCAST |
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
       adapters->FirstUnicastAddress->Address.lpSockaddr->sa_family == AF_INET6) {

      struct in_addr6 adapter_addr;
      adapter_addr = ((struct sockaddr_in6 *)adapters->FirstUnicastAddress->Address.lpSockaddr)->sin6_addr;
	  sprint_ip6addr(adapter_addr, addr6str);
      log_message("set_ethaddr:  with ipv6 address: : ", addr6str);
	  if(issame_ip6addr(adapter_addr,addr6)) {
        if(adapters->PhysicalAddressLength != 6) {
          error_exit("ip addr specified on cmdline does not belong to an ethernet card\n");
        }
        wsprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X",
          adapters->PhysicalAddress[0], adapters->PhysicalAddress[1],
          adapters->PhysicalAddress[2], adapters->PhysicalAddress[3],
          adapters->PhysicalAddress[4], adapters->PhysicalAddress[5]);
        log_message("set_ethaddr:  ethernetaddr: ", buffer);
#if UIP_CONF_IPV6
//      int i;for (i=0;i<6;i++) uip_ethaddr.addr[i] = adapters->PhysicalAddress[i]; //does this need doing?
#else
        uip_setethaddr((*(struct uip_eth_addr *)adapters->PhysicalAddress));
#endif
        break;
      }
    }
    adapters = adapters->Next;
  }

  if(adapters == NULL) {
    error_exit("no adapter found with ip addr specified on cmdline\n");
  }
}
#endif
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
            pcap = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
            if(pcap == NULL) {
              error_exit(error);
            }
#ifdef UIP_FALLBACK_INTERFACE
			log_message("init_pcap:      Opened as primary interface","");
#else
			log_message("init_pcap:      Opened as interface","");
#endif
//          pcap_setdirection(PCAP_D_IN);  //Not implemented in windows yet?
			set_ethaddr(addr);

#ifdef UIP_FALLBACK_INTERFACE
          }
          if (pfall && pcap) return;
          if(interface_addr.s_addr == addrfall.s_addr) {
            pfall = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
            if(pfall == NULL) {
              error_exit(error);
            }
			log_message("init_pcap:      Opened as fallback interface","");
			if (pcap) return;
	      }
#else
            return;
		  }
#endif

#if UIP_CONF_IPV6

        } else if(paddr->addr != NULL && paddr->addr->sa_family == AF_INET6) {
		  struct in6_addr interface_addr;
          interface_addr = ((struct sockaddr_in6 *)paddr->addr)->sin6_addr;

		  sprint_ip6addr(interface_addr, addr6str);
          log_message("init_pcap:    with ipv6 address: ", addr6str);

		  if (issame_ip6addr(interface_addr, addr6)) {
            pcap = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
            if(pcap == NULL) {
              error_exit(error);
            }
#ifdef UIP_FALLBACK_INTERFACE
			log_message("init_pcap:      Opened as primary interface","");
#endif
            set_ethaddr6(addr6);
//          pcap_setdirection(PCAP_D_IN);  //Not implemented in windows yet?
#ifdef UIP_FALLBACK_INTERFACE
          }
          if (pfall && pcap) return;  //exit when we have both interfaces

		  if (issame_ip6addr(interface_addr, addrfall6)) {
            pfall = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
            if(pfall == NULL) {
              error_exit(error);
            }
			log_message("init_pcap:      Opened as fallback interface","");
			if (pcap) return;
	      }
#else
            return;
		  }
#endif
#endif /* UIP_CONF_IPV6 */
		}
      }
    }
    interfaces = interfaces->next;

  }

  if(interfaces == NULL) {
    error_exit("no interface found with specified ip address\n");
  }
}
/*---------------------------------------------------------------------------*/
void
wpcap_init(void)
{
  struct in_addr addr;
  addr.s_addr = INADDR_NONE;  //255.255.255.255
#ifdef UIP_FALLBACK_INTERFACE
  addrfall.s_addr = INADDR_NONE;
#endif

  /* Pick up possible ip addresses from command line */
#ifdef __CYGWIN__
  if ((*__argv)[1]) {
    addr.s_addr = inet_addr((*__argv)[1]);
#if UIP_CONF_IPV6
    uiplib_ipaddrconv((*__argv)[1],(uip_ipaddr_t*) &addr6.s6_addr);
#endif
#ifdef UIP_FALLBACK_INTERFACE
    if ((*__argv)[2]) {
      addrfall.s_addr = inet_addr((*__argv)[2]);
#if UIP_CONF_IPV6
      uiplib_ipaddrconv((*__argv)[2],(uip_ipaddr_t*) &addrfall6.s6_addr);
#endif
    }
#endif
  }

#else /* __CYGWIN__ */
/* VC++ build on win32 platform. Currently the platform has no ipv6 support */
  addr.s_addr = inet_addr(__argv[1]);
#if UIP_CONF_IPV6
  if((__argv)[1])
  uiplib_ipaddrconv((__argv)[1],(uip_ipaddr_t*) &addr6.s6_addr);
#endif
#ifdef UIP_FALLBACK_INTERFACE
  addrfall.s_addr = inet_addr(__argv[2]);
#if UIP_CONF_IPV6
  if((__argv)[2])
  uiplib_ipaddrconv((__argv)[2],(uip_ipaddr_t*) &addrfall6.s6_addr);
#endif
#endif
#endif /* __CYGWIN__ */

#if DEBUG
  log_message("wpcap_init:Passed ipv4 ", inet_ntoa(addr));
  sprint_ip6addr(addr6, addr6str);
  log_message("wpcap_init:Passed ipv6 ", addr6str);
#ifdef UIP_FALLBACK_INTERFACE
  log_message("wpcap_init:Passed fallback ipv4 ", inet_ntoa(addrfall));
  sprint_ip6addr(addrfall6, addr6str);
  log_message("wpcap_init:Passed fallback ipv6 ", addr6str);
#endif
#endif

  /* Use build defaults if not enough addresses passed */
#if UIP_CONF_IPV6

#ifdef UIP_FALLBACK_INTERFACE
  if(addrfall.s_addr == INADDR_NONE) {
	if(iszero_ip6addr(addrfall6)) {
#ifdef WPCAP_FALLBACK_ADDRESS
      addrfall.s_addr = inet_addr(WPCAP_FALLBACK_ADDRESS);
//	  if(addrfall.s_addr == INADDR_NONE) {  //use ipv6 if contiki-conf.h override
        uiplib_ipaddrconv(WPCAP_FALLBACK_ADDRESS,(uip_ipaddr_t*) &addrfall6.s6_addr);
//	  }
#else
//    addrfall.s_addr = inet_addr("10.2.10.10");
      uiplib_ipaddrconv("bbbb::1",(uip_ipaddr_t*) &addrfall6.s6_addr);
#endif
    }
  }
#endif

  if(addr.s_addr == INADDR_NONE) {
	if(iszero_ip6addr(addr6)) {
#ifdef WPCAP_INTERFACE_ADDRESS
      addr.s_addr = inet_addr(WPCAP_INTERFACE_ADDRESS);
//	  if(addr.s_addr == INADDR_NONE) {
        uiplib_ipaddrconv(WPCAP_INTERFACE_ADDRESS,(uip_ipaddr_t*) &addr6.s6_addr);
//	  }
#else
      addr.s_addr = inet_addr("10.10.10.10");   //prefer ipv4 default for legacy compatibility
//    uiplib_ipaddrconv("aaaa::1",(uip_ipaddr_t*) &addr6.s6_addr);
#endif

#ifdef UIP_FALLBACK_INTERFACE
      log_message("usage: <program> <ip addr of interface> <ip addr of fallback>\n","");
	  if(addr.s_addr != INADDR_NONE) {
		  log_message("-->I'll try interface address ", inet_ntoa(addr));
	  } else {
	    sprint_ip6addr(addr6, addr6str);
	    log_message("-->I'll try interface address ", addr6str);
	  }
	  if(addrfall.s_addr != INADDR_NONE) {
		  log_message("-->      and fallback address ", inet_ntoa(addrfall));
	  } else {
	    sprint_ip6addr(addrfall6, addr6str);
	    log_message("-->      and fallback address ", addr6str);
	  }
#else
      if(addr.s_addr != INADDR_NONE) {
        log_message("usage: <program> <ip addr of ethernet card to share>\n-->I'll try guessing ", inet_ntoa(addr));
	  } else {
	    sprint_ip6addr(addr6, addr6str);
	    log_message("usage: <program> <ip addr of ethernet card to share>\n-->I'll try guessing ", addr6str);
	  }
#endif
    }
  }
#else /* ip4 build */
  if(addr.s_addr == INADDR_NONE) {
#ifdef WPCAP_INTERFACE_ADDRESS
    addr.s_addr = inet_addr(WPCAP_INTERFACE_ADDRESS);
#else
    addr.s_addr = inet_addr("10.10.10.10");
#endif
    log_message("usage: <program> <ip addr of ethernet card to share>\n-->I'll try guessing ", inet_ntoa(addr));
  }
#endif /* UIP_CONF_IPV6 */

#if DEBUG
  log_message("wpcap_init:Using ipv4 ", inet_ntoa(addr));
  sprint_ip6addr(addr6, addr6str);
  log_message("wpcap_init:Using ipv6 ", addr6str);
#ifdef UIP_FALLBACK_INTERFACE
  log_message("wpcap_init:Using fallback ipv4 ", inet_ntoa(addrfall));
  sprint_ip6addr(addrfall6, addr6str);
  log_message("wpcap_init:Using fallback ipv6 ", addr6str);
#endif
#endif

 //   log_message("wpcap_init:cmdline address: ", inet_ntoa(addr));


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

}

/*---------------------------------------------------------------------------*/
uint16_t
wpcap_poll(void)
{
  struct pcap_pkthdr *packet_header;
  unsigned char *packet;

  switch(pcap_next_ex(pcap, &packet_header, &packet)) {
  case -1:
    error_exit("error on poll\n");
  case 0:
    return 0;
  }

#if UIP_CONF_IPV6
/* Since pcap_setdirection(PCAP_D_IN) is not implemented in winpcap all outgoing packets
 * will be echoed back. The stack will ignore any packets not addressed to it, but initial
 * ipv6 neighbor solicitations are addressed to everyone and the echoed NS sent on startup
 * would be processed as a conflicting NS race which would cause a stack shutdown.
 * So discard all packets with our source address (packet starts destaddr, srcaddr, ...)
 *
 */
  int i;
  for (i=0;i<UIP_LLADDR_LEN;i++) if (*(packet+UIP_LLADDR_LEN+i)!=uip_lladdr.addr[i]) break;
  if (i==UIP_LLADDR_LEN) {
    PRINTF("SIN: Discarding echoed packet\n");
    return 0;
  }
  
/* To implement multihop, ignore packets to us from specified source macs
 */
//  printf("ethtype=%x %x",*(packet+2*UIP_LLADDR_LEN),*(packet+2*UIP_LLADDR_LEN+1));
// printf("hopcount=%u",*(packet+21));
#if 0
  if (0
//   || (*(packet+11) ==0x1)   //20 ignores router
//  || (*(packet+11) ==0x10)
    || (*(packet+11) ==0x20)  //router ignores 20
  ) {
   printf("i%x",*(packet+11));
    return 0;
  }
/* If we are not the recipient, ignore packets from other RPL nodes */
  if (0
    || (*(packet+5) !=0x1)   //router
//  || (*(packet+11) ==0x10)
 //   || (*(packet+11) ==0x20)  //router ignores 20
  ) {
   printf("r%x",*(packet+11));
    return 0;
  }
#endif

#endif /* UIP_CONF_IPV6 */

  if(packet_header->caplen > UIP_BUFSIZE) {
    return 0;
  }
//  PRINTF("SIN: %lu\n", packet_header->caplen);
  CopyMemory(uip_buf, packet, packet_header->caplen);
  return (uint16_t)packet_header->caplen;

}

#ifdef UIP_FALLBACK_INTERFACE
uint16_t
wfall_poll(void)
{
  struct pcap_pkthdr *packet_header;
  unsigned char *packet;

  switch(pcap_next_ex(pfall, &packet_header, &packet)) {
  case -1:
    error_exit("error on fallback poll\n");
  case 0:
    return 0;
  }
#if UIP_CONF_IPV6
#if FALLBACK_HAS_ETHERNET_HEADERS
#define ETHERNET_LLADDR_LEN 6
#else
#define ETHERNET_LLADDR_LEN UIP_LLADDR_LEN
#endif
/* Since pcap_setdirection(PCAP_D_IN) is not implemented in winpcap all outgoing packets
 * will be echoed back. The stack will ignore any packets not addressed to it, but initial
 * ipv6 neighbor solicitations are addressed to everyone and the echoed NS sent on startup
 * would be processed as a conflicting NS race which would cause a stack shutdown.
 * So discard all packets with our source address (packet starts destaddr, srcaddr, ...)
 *
 */
  int i;
  for (i=0;i<ETHERNET_LLADDR_LEN;i++) if (*(packet+ETHERNET_LLADDR_LEN+i)!=uip_lladdr.addr[i]) break;
  if (i==ETHERNET_LLADDR_LEN) {
    PRINTF("Discarding echoed packet\n");
    return 0;
  }
#endif /* UIP_CONF_IPV6 */

  if(packet_header->caplen > UIP_BUFSIZE) {
    return 0;
  }
  PRINTF("FIN: %lu\n", packet_header->caplen);
  CopyMemory(uip_buf, packet, packet_header->caplen);
  return (uint16_t)packet_header->caplen;

}

#endif

/*---------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
uint8_t
wpcap_send(uip_lladdr_t *lladdr)
{
  if(lladdr == NULL) {
/* the dest must be multicast*/
    (&BUF->dest)->addr[0] = 0x33;
    (&BUF->dest)->addr[1] = 0x33;
    (&BUF->dest)->addr[2] = IPBUF->destipaddr.u8[12];
    (&BUF->dest)->addr[3] = IPBUF->destipaddr.u8[13];
    (&BUF->dest)->addr[4] = IPBUF->destipaddr.u8[14];
    (&BUF->dest)->addr[5] = IPBUF->destipaddr.u8[15];
  } else {
   //when fallback used this gets ptr to lladdr of all zeroes on forwarded packets, turn them into multicast(?)
    if ((lladdr->addr[0]==0)&&(lladdr->addr[1]==0)&&(lladdr->addr[2]==0)&&(lladdr->addr[3]==0)&&(lladdr->addr[4]==0)&&(lladdr->addr[5]==0)) {
     (&BUF->dest)->addr[0] = 0x33;
     (&BUF->dest)->addr[1] = 0x33;
     (&BUF->dest)->addr[2] = IPBUF->destipaddr.u8[12];
     (&BUF->dest)->addr[3] = IPBUF->destipaddr.u8[13];
     (&BUF->dest)->addr[4] = IPBUF->destipaddr.u8[14];
     (&BUF->dest)->addr[5] = IPBUF->destipaddr.u8[15];
    } else {
      memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
    }
  }
  memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
  PRINTF("SUT: %u\n", uip_len);
  PRINTF("Src= %02x %02x %02x %02x %02x %02x",(&BUF->src)->addr[0],(&BUF->src)->addr[1],(&BUF->src)->addr[2],(&BUF->src)->addr[3],(&BUF->src)->addr[4],(&BUF->src)->addr[5]);
  PRINTF("  Dst= %02x %02x %02x %02x %02x %02x",(&BUF->dest)->addr[0],(&BUF->dest)->addr[1],(&BUF->dest)->addr[2],(&BUF->dest)->addr[3],(&BUF->dest)->addr[4],(&BUF->dest)->addr[5]);
  PRINTF("  Type=%04x\n",BUF->type);
  BUF->type = UIP_HTONS(UIP_ETHTYPE_IPV6);
  uip_len += sizeof(struct uip_eth_hdr);
  if(pcap_sendpacket(pcap, uip_buf, uip_len) == -1) {
    error_exit("error on send\n");
  }
return 0;
}
#ifdef UIP_FALLBACK_INTERFACE
uint8_t
wfall_send(uip_lladdr_t *lladdr)
{
#if FALLBACK_HAS_ETHERNET_HEADERS
	//make room for ethernet header
//{int i;printf("\n");for (i=0;i<uip_len;i++) printf("%02x ",*(char*)(uip_buf+i));printf("\n");}
{int i;for(i=uip_len;i>=0;--i) *(char *)(uip_buf+i+14) = *(char *)(uip_buf+i);}
//{int i;printf("\n");for (i=0;i<uip_len;i++) printf("%02x ",*(char*)(uip_buf+i));printf("\n");}
#endif
  if(lladdr == NULL) {
/* the dest must be multicast*/
    (&BUF->dest)->addr[0] = 0x33;
    (&BUF->dest)->addr[1] = 0x33;
    (&BUF->dest)->addr[2] = IPBUF->destipaddr.u8[12];
    (&BUF->dest)->addr[3] = IPBUF->destipaddr.u8[13];
    (&BUF->dest)->addr[4] = IPBUF->destipaddr.u8[14];
    (&BUF->dest)->addr[5] = IPBUF->destipaddr.u8[15];
  } else {
    memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
  }
  memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
  PRINTF("FUT: %u\n", uip_len);
  PRINTF("Srcf= %02x %02x %02x %02x %02x %02x",(&BUF->src)->addr[0],(&BUF->src)->addr[1],(&BUF->src)->addr[2],(&BUF->src)->addr[3],(&BUF->src)->addr[4],(&BUF->src)->addr[5]);
  PRINTF("  Dstf= %02x %02x %02x %02x %02x %02x",(&BUF->dest)->addr[0],(&BUF->dest)->addr[1],(&BUF->dest)->addr[2],(&BUF->dest)->addr[3],(&BUF->dest)->addr[4],(&BUF->dest)->addr[5]);
  PRINTF("  Typef=%04x\n",BUF->type);
  BUF->type = UIP_HTONS(UIP_ETHTYPE_IPV6);
  uip_len += sizeof(struct uip_eth_hdr);
  if(pcap_sendpacket(pfall, uip_buf, uip_len) == -1) {
    error_exit("error on fallback send\n");
  }
return 0;
}
#endif
#else /* UIP_CONF_IPV6 */
void
wpcap_send(void)
{
  /*  if(rand() % 1000 > 900) {
    printf("Drop\n");
    return;
  } else {
    printf("Send\n");
    }*/

  if(pcap_sendpacket(pcap, uip_buf, uip_len) == -1) {
    error_exit("error on send\n");
  }
}
#endif /* UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/
void
wpcap_exit(void)
{
  FreeLibrary(wpcap);
}
/*---------------------------------------------------------------------------*/
