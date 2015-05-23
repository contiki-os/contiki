/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "contiki.h"
#include "contiki-net.h"
#include "ip64-dhcpc.h"

#include "ip64.h"
#include "ip64-eth.h"
#include "ip64-addr.h"

#include <stdio.h>

PROCESS(ip64_ipv4_dhcp_process, "IPv4 DHCP");

uip_ipaddr_t uip_hostaddr; /* Needed because it is referenced by dhcpc.c */


/*---------------------------------------------------------------------------*/
void
ip64_ipv4_dhcp_init(void)
{
  printf("Starting DHCPv4\n");
  process_start(&ip64_ipv4_dhcp_process, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ip64_ipv4_dhcp_process, ev, data)
{
  PROCESS_BEGIN();

  ip64_dhcpc_init(&ip64_eth_addr, sizeof(ip64_eth_addr));

  printf("Inited\n");

  ip64_dhcpc_request();
  printf("Requested\n");
  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == tcpip_event ||
       ev == PROCESS_EVENT_TIMER) {
      ip64_dhcpc_appcall(ev, data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
ip64_dhcpc_configured(const struct ip64_dhcpc_state *s)
{
  uip_ip6addr_t ip6dnsaddr;
  printf("DHCP Configured with %d.%d.%d.%d\n",
	 s->ipaddr.u8[0], s->ipaddr.u8[1],
	 s->ipaddr.u8[2], s->ipaddr.u8[3]);

  ip64_set_hostaddr((uip_ip4addr_t *)&s->ipaddr);
  ip64_set_netmask((uip_ip4addr_t *)&s->netmask);
  ip64_set_draddr((uip_ip4addr_t *)&s->default_router);
  ip64_addr_4to6((uip_ip4addr_t *)&s->dnsaddr, &ip6dnsaddr);
  //  mdns_conf(&ip6dnsaddr);
}
/*---------------------------------------------------------------------------*/
void
ip64_dhcpc_unconfigured(const struct ip64_dhcpc_state *s)
{
}
/*---------------------------------------------------------------------------*/
