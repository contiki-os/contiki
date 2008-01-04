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
 * $Id: wpcap.c,v 1.13 2008/01/04 21:53:32 oliverschmidt Exp $
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

/* Avoid 'conflicting types' errors. */
#define htonl
#define htons

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

static int (* pcap_findalldevs)(struct pcap_if **, char *);
static struct pcap *(* pcap_open_live)(char *, int, int, int, char *);
static int (* pcap_next_ex)(struct pcap *, struct pcap_pkthdr **, unsigned char **);
static int (* pcap_sendpacket)(struct pcap *, unsigned char *, int);

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

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
  char error[256];

  if(pcap_findalldevs(&interfaces, error) == -1) {
    error_exit(error);
  }

  while(interfaces != NULL) {
    log_message("init_pcap: found interface: ", interfaces->description);

    if(interfaces->addresses != NULL &&
       interfaces->addresses->addr != NULL &&
       interfaces->addresses->addr->sa_family == AF_INET) {

      struct in_addr interface_addr;
      interface_addr = ((struct sockaddr_in *)interfaces->addresses->addr)->sin_addr;
      log_message("init_pcap:    with address: ", inet_ntoa(interface_addr));

      if(interface_addr.s_addr == addr.s_addr) {
        break;
      }
    }
    interfaces = interfaces->next;
  }

  if(interfaces == NULL) {
    error_exit("no interface found with ip addr specified on cmdline\n");
  }

  pcap = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
  if(pcap == NULL) {
    error_exit(error);
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

	uip_setethaddr((*(struct uip_eth_addr *)adapters->PhysicalAddress));
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
    error_exit("usage: <program> <ip addr of ethernet card to share>\n");
  }
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

  if(packet_header->caplen > UIP_BUFSIZE) {
    return 0;
  }

  CopyMemory(uip_buf, packet, packet_header->caplen);
  return (u16_t)packet_header->caplen;
}
/*---------------------------------------------------------------------------*/
void
wpcap_send(void)
{
  if(pcap_sendpacket(pcap, uip_buf, uip_len) == -1) {
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
