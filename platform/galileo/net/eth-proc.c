/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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
 */

#include <stdio.h>

#include "contiki-net.h"
#include "net/ipv4/uip-neighbor.h"
#include "net/eth-proc.h"
#include "eth.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(eth_process, "Ethernet");

/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV6
static uint8_t
output(const uip_lladdr_t *dest_mac)
{
  if (dest_mac == NULL) {
    /* broadcast packet */
    memset(&BUF->dest, 0xFF, UIP_LLH_LEN);
  } else {
    memcpy(&BUF->dest, dest_mac, UIP_LLH_LEN);
  }
  memcpy(&BUF->src, uip_lladdr.addr, UIP_LLH_LEN);
  quarkX1000_eth_send();

  return 0;
}
#else
static uint8_t
output(void)
{
  uip_arp_out();
  quarkX1000_eth_send();

  return 0;
}
#endif /* NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  process_poll(&eth_process);
  quarkX1000_eth_poll(&uip_len);

  if(uip_len > 0) {
#if NETSTACK_CONF_WITH_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    }
#else
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        quarkX1000_eth_send();
      }
    }
#endif /* NETSTACK_CONF_WITH_IPV6 */
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(eth_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  tcpip_set_outputfunc(output);

  process_poll(&eth_process);

  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
