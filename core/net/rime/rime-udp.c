/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         A MAC protocol using UDP over IPv6.
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include <string.h>

#include "net/uip.h"
#include "net/uip-udp-packet.h"
#include "net/uip-netif.h"
#include "net/rime/rime-udp.h"
#include "net/packetbuf.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

#ifndef RIME_CONF_UDP_PORT
#define RIME_UDP_PORT		9508
#else
#define RIME_UDP_PORT		RIME_CONF_UDP_PORT
#endif /* RIME_CONF_UDP_PORT */

static struct uip_udp_conn *broadcast_conn;
static struct uip_udp_conn *unicast_conn;

static void (* receiver_callback)(const struct mac_driver *);

PROCESS(rime_udp_process, "Rime over UDP process");

PROCESS_THREAD(rime_udp_process, ev, data)
{
  static uip_ipaddr_t ipaddr;

  PROCESS_BEGIN();

  broadcast_conn = udp_broadcast_new(UIP_HTONS(RIME_UDP_PORT), NULL);
  if(broadcast_conn == NULL) {
    PRINTF("rime-udp: Failed to allocate a broadcast connection!\n");
  }

  uip_create_unspecified(&ipaddr);
  unicast_conn = udp_new(&ipaddr, UIP_HTONS(RIME_UDP_PORT), NULL);
  if(unicast_conn == NULL) {
    PRINTF("rime-udp: Failed to allocate a unicast connection!\n");
  }

  udp_bind(unicast_conn, UIP_HTONS(RIME_UDP_PORT));

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    if(uip_newdata()) {
      packetbuf_clear();
      memmove(packetbuf_hdrptr(), uip_appdata, uip_datalen());
      PRINTF("rime-udp: received %d bytes\n", uip_datalen());
      receiver_callback(&rime_udp_driver);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent_callback, void *ptr)
{
  const rimeaddr_t *addr;

  addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  PRINTF("rime-udp: Sending %d bytes to %d.%d\n", packetbuf_totlen(),
         addr->u8[0], addr->u8[1]);

  if(rimeaddr_cmp(&rimeaddr_null, addr)) {
    uip_udp_packet_send(broadcast_conn,
                        packetbuf_hdrptr(), packetbuf_totlen());
    mac_call_sent_callback(sent_callback, ptr, MAC_TX_OK, 1);
  } else {
    uip_ip6addr(&unicast_conn->ripaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
    uip_netif_addr_autoconf_set(&unicast_conn->ripaddr, (uip_lladdr_t *)addr);
    uip_udp_packet_send(unicast_conn,
                        packetbuf_hdrptr(), packetbuf_totlen());
    uip_create_unspecified(&unicast_conn->ripaddr);
  }
  return;
}
/*---------------------------------------------------------------------------*/
static int
input_packet(void)
{
  packetbuf_set_datalen(uip_datalen());
  return uip_datalen();
}
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short
check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  process_start(&rime_udp_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver rime_udp_driver = {
  "rime-udp",
  init,
  send_packet,
  input_packet,
  on,
  off,
  check_interval,
};
/*---------------------------------------------------------------------------*/
