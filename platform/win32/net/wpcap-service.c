/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: wpcap-service.c,v 1.1 2006/09/23 20:25:03 oliverschmidt Exp $
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>

#pragma comment(lib, "wsock32")

#define htons		  /* Avoid 'redefinition' error. */
#include "contiki.h"
#include "contiki-net.h"

void send_packet(void);

SERVICE(wpcap_service, packet_service, { (u8_t (*)(void))send_packet });

PROCESS(wpcap_service_process, "WinPcap driver");

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
  unsigned __int32 flags;
};

struct pcap_pkthdr {
  struct timeval ts;
  unsigned __int32 caplen;
  unsigned __int32 len;
};

static struct pcap *pcap;

static int (* pcap_findalldevs)(struct pcap_if **, char *);
static struct pcap *(* pcap_open_live)(char *, int, int, int, char *);
static int (* pcap_next_ex)(struct pcap *, struct pcap_pkthdr **, unsigned char **);
static int (* pcap_sendpacket)(struct pcap *, unsigned char *, int);

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  struct pcap_pkthdr *packet_header;
  unsigned char *packet;

  process_poll(&wpcap_service_process);

  switch(pcap_next_ex(pcap, &packet_header, &packet)) {
  case -1: 
    exit(1);
  case 0:
    return;
  }

  uip_len = packet_header->caplen;
  CopyMemory(uip_buf, packet, uip_len);

  if(BUF->type == HTONS(UIP_ETHTYPE_IP)) {
    tcpip_input();
  } else if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) {
    uip_arp_arpin();
    if(uip_len > 0) {
      if(pcap_sendpacket(pcap, uip_buf, uip_len) == -1) {
	exit(1);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void)
{
  uip_arp_out();

  if(pcap_sendpacket(pcap, uip_buf, uip_len) == -1) {
    exit(1);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wpcap_service_process, ev, data)
{
  static HMODULE wpcap;

  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  wpcap = LoadLibrary("wpcap.dll");
  (FARPROC)pcap_findalldevs = GetProcAddress(wpcap, "pcap_findalldevs");
  (FARPROC)pcap_open_live   = GetProcAddress(wpcap, "pcap_open_live");
  (FARPROC)pcap_next_ex     = GetProcAddress(wpcap, "pcap_next_ex");
  (FARPROC)pcap_sendpacket  = GetProcAddress(wpcap, "pcap_sendpacket");

  if(pcap_findalldevs == NULL || pcap_open_live  == NULL ||
     pcap_next_ex     == NULL || pcap_sendpacket == NULL) {
    exit(1);
  }

  {
    struct in_addr cmdline_addr;
    struct pcap_if *interfaces;
    char error[256];

    cmdline_addr.s_addr = inet_addr(__argv[1]);
    if(cmdline_addr.s_addr == INADDR_NONE) {
      exit(1);
    }
    debug_printf("Cmdline address: %s\n", inet_ntoa(cmdline_addr));

    if(pcap_findalldevs(&interfaces, error) == -1) {
      exit(1);
    }

    while(interfaces != NULL) {
      debug_printf("Found interface: %s\n", interfaces->description);
      if(interfaces->addresses != NULL && 
	 interfaces->addresses->addr != NULL &&
	 interfaces->addresses->addr->sa_family == AF_INET) {
	struct in_addr interface_addr;
	interface_addr = ((struct sockaddr_in *)interfaces->addresses->addr)->sin_addr;
	debug_printf("   With address: %s\n", inet_ntoa(interface_addr));
	if(interface_addr.s_addr == cmdline_addr.s_addr) {
	  break;
	}     
      }
      interfaces = interfaces->next;
    }

    if(interfaces == NULL) {
      exit(1);
    }

    pcap = pcap_open_live(interfaces->name, UIP_BUFSIZE, 0, -1, error);
    if(pcap == NULL) {
      exit(1);
    }
  }

  uip_arp_init();

  SERVICE_REGISTER(wpcap_service);

  process_poll(&wpcap_service_process);

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			   ev == PROCESS_EVENT_SERVICE_REMOVED);

  SERVICE_REMOVE(wpcap_service);

  FreeLibrary(wpcap);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
