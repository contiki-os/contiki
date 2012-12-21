/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup SeedEye Contiki SEEDEYE Platform
 *
 * @{
 */

/**
 * \file   init-net.c
 * \brief  Network initialization for the SEEDEYE port.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-25
 */

#include <contiki.h>
#include <contiki-net.h>
#include <net/netstack.h>
#include <net/queuebuf.h>
#include <net/mac/frame802154.h>

#include <lib/random.h>
#include <time.h>

#include <mrf24j40.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
init_net(uint8_t node_id)
{
  uint16_t shortaddr;
  uint64_t longaddr;
  rimeaddr_t addr;
#if WITH_UIP6
  uip_ds6_addr_t *lladdr;
  uip_ipaddr_t ipaddr;
#endif
  
  uint8_t i;

  memset(&shortaddr, 0, sizeof(shortaddr));
  memset(&longaddr, 0, sizeof(longaddr));
  *((uint8_t *)&shortaddr) = node_id >> 8;
  *((uint8_t *)&shortaddr + 1) = node_id;
  *((uint8_t *)&longaddr) = node_id >> 8;
  *((uint8_t *)&longaddr + 1) = node_id;
  for(i = 2; i < sizeof(longaddr); ++i) {
    ((uint8_t *)&longaddr)[i] = random_rand();
  }
  
  PRINTF("SHORT MAC ADDRESS %02x:%02x\n",
         *((uint8_t *) & shortaddr), *((uint8_t *) & shortaddr + 1));

  PRINTF("EXTENDED MAC ADDRESS %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
         *((uint8_t *)&longaddr),
         *((uint8_t *)&longaddr + 1),
         *((uint8_t *)&longaddr + 2),
         *((uint8_t *)&longaddr + 3),
         *((uint8_t *)&longaddr + 4),
         *((uint8_t *)&longaddr + 5),
         *((uint8_t *)&longaddr + 6),
         *((uint8_t *)&longaddr + 7));

  memset(&addr, 0, sizeof(rimeaddr_t));

  for(i = 0; i < sizeof(addr.u8); ++i) {
    addr.u8[i] = ((uint8_t *)&longaddr)[i];
  }

  rimeaddr_set_node_addr(&addr);
  
  PRINTF("Rime started with address: ");
  for(i = 0; i < sizeof(addr.u8) - 1; ++i) {
    PRINTF("%d.", addr.u8[i]);
  }
  PRINTF("%d\n", addr.u8[i]);

  queuebuf_init();

  NETSTACK_RADIO.init();
  
  mrf24j40_set_channel(RF_CHANNEL);
  mrf24j40_set_panid(IEEE802154_PANID);
  mrf24j40_set_short_mac_addr(shortaddr);
  mrf24j40_set_extended_mac_addr(longaddr);

  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  PRINTF("%s %s, channel check rate %d Hz, radio channel %u\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1 :
                         NETSTACK_RDC.channel_check_interval()), RF_CHANNEL);

#if WITH_UIP6

#if RIMEADDR_CONF_SIZE == 2
  memset(&uip_lladdr.addr, 0, sizeof(uip_lladdr.addr));
  uip_lladdr.addr[3] = 0xff;
  uip_lladdr.addr[4]= 0xfe;
  memcpy(&uip_lladdr.addr[6], &shortaddr, sizeof(shortaddr));
#else
  memcpy(&uip_lladdr.addr, &longaddr, sizeof(uip_lladdr.addr));
#endif

  process_start(&tcpip_process, NULL);

  lladdr = uip_ds6_get_link_local(-1);

  PRINTF("Tentative link-local IPv6 address ");

  for(i = 0; i < 7; ++i) {
    PRINTF("%02x%02x:", lladdr->ipaddr.u8[i * 2], lladdr->ipaddr.u8[i * 2 + 1]);
  }

  PRINTF("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);

  if(!UIP_CONF_IPV6_RPL) {
    uip_ip6addr(&ipaddr, 0x2001, 0x1418, 0x100, 0x823c, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);

    PRINTF("Tentative global IPv6 address ");

    for(i = 0; i < 7; ++i) {
      PRINTF("%02x%02x:", ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }

    PRINTF("%02x%02x\n", ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
  }
#endif
}
/*---------------------------------------------------------------------------*/

/** @} */
