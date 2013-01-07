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
#include <avr/pgmspace.h>

#include "contiki.h"
#include "rf230bb.h"
#include "dev/rs232.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include "dev/ds2401.h"
#include "sys/node-id.h"

#if WITH_UIP6
#include "net/uip-ds6.h"
#endif /* WITH_UIP6 */

#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-over-mesh.h"
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(192,168,1,2, 255,255,255,255, slip_send)};
static struct uip_fw_netif meshif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_over_mesh_send)};

static uint8_t is_gateway;

#endif /* WITH_UIP */

#define UIP_OVER_MESH_CHANNEL 8

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  rimeaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(rimeaddr_t));
#if UIP_CONF_IPV6
  memcpy(addr.u8, ds2401_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(rimeaddr_t); ++i) {
      addr.u8[i] = ds2401_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  rimeaddr_set_node_addr(&addr);
  printf_P(PSTR("Rime started with address "));
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf_P(PSTR("%d."), addr.u8[i]);
  }
  printf_P(PSTR("%d\n"), addr.u8[i]);
}

/*--------------------------------------------------------------------------*/
#if WITH_UIP
static void
set_gateway(void)
{
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf_P(PSTR("%d.%d: making myself the IP network gateway.\n\n"),
	              rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    printf_P(PSTR("IPv4 address of the gateway: %d.%d.%d.%d\n\n"),
	              uip_ipaddr_to_quad(&uip_hostaddr));
    uip_over_mesh_set_gateway(&rimeaddr_node_addr);
    uip_over_mesh_make_announced_gateway();
    is_gateway = 1;
  }
}
#endif /* WITH_UIP */
/*---------------------------------------------------------------------------*/
void
init_net(void)
{

  set_rime_addr();
  NETSTACK_RADIO.init();
  {
    uint8_t longaddr[8];
    uint16_t shortaddr;
    
    shortaddr = (rimeaddr_node_addr.u8[0] << 8) +
                 rimeaddr_node_addr.u8[1];
    memset(longaddr, 0, sizeof(longaddr));
    rimeaddr_copy((rimeaddr_t *)&longaddr, &rimeaddr_node_addr);
    printf_P(PSTR("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n"),
             longaddr[0], longaddr[1], longaddr[2], longaddr[3],
             longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
    
    rf230_set_pan_addr(IEEE802154_PANID, shortaddr, longaddr);
  }
  rf230_set_channel(RF_CHANNEL);


#if WITH_UIP6
  memcpy(&uip_lladdr.addr, ds2401_id, sizeof(uip_lladdr.addr));
  /* Setup nullmac-like MAC for 802.15.4 */
  /* sicslowpan_init(sicslowmac_init(&cc2420_driver)); */
  /* printf(" %s channel %u\n", sicslowmac_driver.name, RF_CHANNEL); */

  /* Setup X-MAC for 802.15.4 */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  printf_P(PSTR("%s %s, channel check rate %d Hz, radio channel %d\n"),
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);

  process_start(&tcpip_process, NULL);

  printf_P(PSTR("Tentative link-local IPv6 address "));
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      printf_P(PSTR("%02x%02x:"), lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    printf_P(PSTR("%02x%02x\n"), lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }

  if(!UIP_CONF_IPV6_RPL) {
    uip_ipaddr_t ipaddr;
    int i;
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
    printf_P(PSTR("Tentative global IPv6 address "));
    for(i = 0; i < 7; ++i) {
      printf_P(PSTR("%02x%02x:"),
             ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }
    printf_P(PSTR("%02x%02x\n"),
           ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
  }

#else /* WITH_UIP6 */

  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  printf_P(PSTR("%s %s, channel check rate %d Hz, radio channel %d\n"),
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);
#endif /* WITH_UIP6 */


#if WITH_UIP
  uip_ipaddr_t hostaddr, netmask;
 
  uip_init();
  uip_fw_init();

  process_start(&tcpip_process, NULL);
  process_start(&slip_process, NULL);
  process_start(&uip_fw_process, NULL);
  
  slip_set_input_callback(set_gateway);

  /* Construct ip address from four bytes. */
  uip_ipaddr(&hostaddr, 172, 16, rimeaddr_node_addr.u8[0],
                                  rimeaddr_node_addr.u8[1]);
  /* Construct netmask from four bytes. */
  uip_ipaddr(&netmask, 255,255,0,0);

  uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);
  /* Set the IP address for this host. */
  uip_sethostaddr(&hostaddr);
  /* Set the netmask for this host. */
  uip_setnetmask(&netmask);
  
  uip_over_mesh_set_net(&hostaddr, &netmask);

  /* Register slip interface with forwarding module. */
  //uip_fw_register(&slipif);
  uip_over_mesh_set_gateway_netif(&slipif);
  /* Set slip interface to be a default forwarding interface . */
  uip_fw_default(&meshif);
  uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
  printf_P(PSTR("uIP started with IP address %d.%d.%d.%d\n"),
	       uip_ipaddr_to_quad(&hostaddr));
#endif /* WITH_UIP */

  
  
}
/*---------------------------------------------------------------------------*/
