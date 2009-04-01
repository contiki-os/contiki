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
 * $Id: init-net-uipv6.c,v 1.1 2009/04/01 13:50:12 fros4943 Exp $
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>

#include "net/rime.h"
#include "net/uip-netif.h"

#include "node-id.h"
#include "dev/cooja-radio.h"

#define PRINT6ADDR(addr) printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
static u8_t
sender(uip_lladdr_t * dst_mac)
{
  return cooja_radio.send((char*)UIP_IP_BUF, uip_len);
}
/*---------------------------------------------------------------------------*/
static void
receiver(const struct radio_driver *d)
{
  uip_len = d->read((char*)UIP_IP_BUF, UIP_BUFSIZE - UIP_LLH_LEN);
  tcpip_input();
}
/*---------------------------------------------------------------------------*/
void
init_net(void)
{
  int i;
  uint8_t addr[sizeof(uip_lladdr.addr)];
  rimeaddr_t rimeaddr;

  /* Init Rime */
  ctimer_init();
  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);
  printf("Rime started with address: ");
  for(i = 0; i < sizeof(rimeaddr_node_addr.u8) - 1; i++) {
    printf("%d.", rimeaddr_node_addr.u8[i]);
  }
  printf("%d\n", rimeaddr_node_addr.u8[i]);

  /* Init uIPv6 */
  for (i=0; i < sizeof(uip_lladdr.addr); i++) {
    addr[i] = node_id & 0xff;
  }
  memcpy(&uip_lladdr.addr, addr, sizeof(uip_lladdr.addr));
  process_start(&tcpip_process, NULL);
  printf("IPv6 started with address: ");
  PRINT6ADDR(&uip_netif_physical_if.addresses[0].ipaddr);
  printf("\n");

  /* uIPv6 <-> COOJA's packet radio */
  tcpip_set_outputfunc(sender);
  cooja_radio.set_receive_function(receiver);
}
