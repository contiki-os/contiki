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
 * $Id: wpcap.c,v 1.20 2010/10/19 20:30:47 oliverschmidt Exp $
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

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
static void
error_exit(char *message)
{
  printf("error_exit: %s", message);
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
            pcap = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
            if(pcap == NULL) {
              error_exit(error);
            }
//          pcap_setdirection(PCAP_D_IN);  //Not implemented in windows yet?
            return;
          }
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
/*---------------------------------------------------------------------------*/
void
wpcap_init(void)
{
  struct in_addr addr;

#ifdef __CYGWIN__
  addr.s_addr = inet_addr((*__argv)[1]);
#else /* __CYGWIN__ */
  addr.s_addr = inet_addr(__argv[1]);
#endif /* __CYGWIN__ */
  if(addr.s_addr == INADDR_NONE) {
//  error_exit("usage: <program> <ip addr of ethernet card to share>\n");
    addr.s_addr = inet_addr("10.10.10.10");
    log_message("usage: <program> <ip addr of ethernet card to share>\n-->I'll try guessing ", inet_ntoa(addr));
  } else {
    log_message("wpcap_init:cmdline address: ", inet_ntoa(addr));
  }

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

}

/*---------------------------------------------------------------------------*/
u16_t
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
    PRINTF("Discarding echoed packet\n");
    return 0;
  }
#endif /* UIP_CONF_IPV6 */

  if(packet_header->caplen > UIP_BUFSIZE) {
    return 0;
  }

  CopyMemory(uip_buf, packet, packet_header->caplen);
  return (u16_t)packet_header->caplen;

}

/*---------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
u8_t
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
    memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
  }
  memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);

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
