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
 
  /**
 * \file
 *         Functions needed by the Windows application for uip6-bridge.
 *         Thanks to Oliver Schmidt for the original code.
 * \author
 *         Salvatore Pitrulli <salvopitru@users.sourceforge.net>
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

/* Avoid 'conflicting types' errors. */
#define htonl
#define htons

#define PROGRESS(x)


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


#include "net/uip.h"
#include "net/uip_arp.h"


struct uip_eth_addr uip_ethaddr = {{0,0,0,0,0,0}};

static char interface_name[256] = "";

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
setethaddr(struct uip_eth_addr *a)
{
  memcpy(&uip_ethaddr, a, sizeof(struct uip_eth_addr));
}
/*---------------------------------------------------------------------------*/
static void
init_pcap(struct uip_eth_addr *addr)
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

	if (adapters->PhysicalAddressLength == 6) {
		wsprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X",
		 adapters->PhysicalAddress[0], adapters->PhysicalAddress[1],
		 adapters->PhysicalAddress[2], adapters->PhysicalAddress[3],
		 adapters->PhysicalAddress[4], adapters->PhysicalAddress[5]);

		log_message("Physical address: ", buffer);

		if(memcmp(addr,adapters->PhysicalAddress,sizeof(struct uip_eth_addr))==0){
			char error[256] = "";

			WideCharToMultiByte(CP_ACP, 0, adapters->FriendlyName, -1,
				interface_name, sizeof(interface_name), NULL, NULL);
			printf("Using local network interface: %s\n",interface_name);

			strcpy(buffer,"\\Device\\NPF_");
			strncat(buffer,adapters->AdapterName,200);
			pcap = pcap_open_live(buffer, sizeof(buffer), 0, -1, error);
			if(pcap == NULL) {
				error_exit(error);
			}
			break;
		}
	}

    adapters = adapters->Next;
  }

  if(adapters == NULL) {
    error_exit("no adapter found with Ethernet address specified on cmdline\n");
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
char * wpcap_start(struct uip_eth_addr *addr, int log)
{
  
  logging = log;
  
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
  setethaddr(addr);

  return interface_name;

}
/*---------------------------------------------------------------------------*/
uint16_t
wpcap_poll(char * buf)
{
  struct pcap_pkthdr *packet_header;
  unsigned char *packet;
  struct uip_eth_hdr * eth_hdr;

  switch(pcap_next_ex(pcap, &packet_header, &packet)) {
  case -1:
    error_exit("error on poll\n");
  case 0:
    return 0;
  }

  if(packet_header->caplen > BUFSIZE) {
    return 0;
  }

  eth_hdr = (struct uip_eth_hdr *)packet;

  if(memcmp(&uip_ethaddr,&eth_hdr->src,sizeof(struct uip_eth_addr))!=0){
	  // It's not a packet originated from the interface itself.
	  return 0;
  }

  CopyMemory(buf, packet, packet_header->caplen);
  return packet_header->caplen;

}
/*---------------------------------------------------------------------------*/
void
wpcap_send(void *buf, int len)
{

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
