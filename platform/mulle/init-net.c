/*
 * Copyright (c) 2010, University of Colombo School of Computing
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
 * @(#)$$
 */

/**
 * \file
 *         Network initialization for the MICAz port.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include "queuebuf.h"
#include "K60.h"

#include "contiki.h"
#include "rf230bb.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if WITH_UIP6
#include "net/ipv6/uip-ds6.h"
#endif /* WITH_UIP6 */

#define UIP_OVER_MESH_CHANNEL 8
#ifndef NODE_ID
#define NODE_ID 1
#warning Node id = 1
#else
#warning Using user defined node id
#endif

static unsigned char id[8] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, NODE_ID };

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;
  unsigned int i;

  /* memset(&addr, 0x65, sizeof(linkaddr_t)); */
  memcpy(addr.u8, id, sizeof(addr.u8));

  linkaddr_set_node_addr(&addr);
  PRINTF("Rime started with address ");
  PRINTF("%d", addr.u8[0]);
  for(i = 1; i < sizeof(addr.u8); i++) {
    PRINTF(".%d", addr.u8[i]);
  }
  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
void
init_net(void)
{
#ifndef WITH_SLIP
  id[0] = (((SIM->UIDL) >> (8 * 0)) & 0xFF) | 0x02;
  id[1] = ((SIM->UIDL) >> (8 * 1)) & 0xFF;
  id[2] = ((SIM->UIDL) >> (8 * 2)) & 0xFF;
  id[3] = ((SIM->UIDL) >> (8 * 3)) & 0xFF;
  id[4] = ((SIM->UIDML) >> (8 * 0)) & 0xFF;
  id[5] = ((SIM->UIDML) >> (8 * 1)) & 0xFF;
  id[6] = ((SIM->UIDML) >> (8 * 2)) & 0xFF;
  id[7] = ((SIM->UIDML) >> (8 * 3)) & 0xFF;
#else
  /* Use fixt address for border router. */
  id[0] = 0x02;
  id[7] = 0x01;
#endif
#if WITH_UIP6
  set_rime_addr();
  NETSTACK_RADIO.init();
  {
    uint8_t longaddr[8];
    uint16_t shortaddr;

    shortaddr = (linkaddr_node_addr.u8[0] << 8) +
      linkaddr_node_addr.u8[1];
    memset(longaddr, 0, sizeof(longaddr));
    linkaddr_copy((linkaddr_t *)&longaddr, &linkaddr_node_addr);
    rf230_set_pan_addr(IEEE802154_CONF_PANID, shortaddr, longaddr);
  }
  rf230_set_channel(RF_CHANNEL);

  memcpy(&uip_lladdr.addr, id, sizeof(uip_lladdr.addr));

  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  PRINTF("%s %s, channel check rate %d Hz, radio channel %d\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1 :
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);

  process_start(&tcpip_process, NULL);

  PRINTF("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      PRINTF("%04x:", lladdr->ipaddr.u8[i * 2] * 256 +
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    PRINTF("%04x\n", lladdr->ipaddr.u8[14] * 256 + lladdr->ipaddr.u8[15]);
  }

  if(!UIP_CONF_IPV6_RPL) {
    uip_ipaddr_t ipaddr;
    int i;
    uip_ip6addr(&ipaddr, 0xfdfd, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
    PRINTF("Tentative global IPv6 address ");
    for(i = 0; i < 7; ++i) {
      PRINTF("%04x:",
             ipaddr.u8[i * 2] * 256 + ipaddr.u8[i * 2 + 1]);
    }
    PRINTF("%04x\n",
           ipaddr.u8[7 * 2] * 256 + ipaddr.u8[7 * 2 + 1]);
  }

#else /* If no radio stack should be used only turn on radio and set it to sleep for minimal power consumption */
  rf230_init();
  rf230_driver.off();
#endif /* WITH_UIP6 */
}
/*---------------------------------------------------------------------------*/

unsigned char *
init_net_get_addr()
{
  return id;
}
