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
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "dev/slip.h"

#include "ip64.h"
#include "ip64-arp.h"
#include "ip64-eth-interface.h"

#include <string.h>

#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"
#define printf(...)
/*---------------------------------------------------------------------------*/
void
ip64_eth_interface_input(uint8_t *packet, uint16_t len)
{
  struct ip64_eth_hdr *ethhdr;
  ethhdr = (struct ip64_eth_hdr *)packet;

  if(ethhdr->type == UIP_HTONS(IP64_ETH_TYPE_ARP)) {
    len = ip64_arp_arp_input(packet, len);

    if(len > 0) {
      IP64_ETH_DRIVER.output(packet, len);
    }
  } else if(ethhdr->type == UIP_HTONS(IP64_ETH_TYPE_IP) &&
	    len > sizeof(struct ip64_eth_hdr)) {
    printf("-------------->\n");
    uip_len = ip64_4to6(&packet[sizeof(struct ip64_eth_hdr)],
			len - sizeof(struct ip64_eth_hdr),
			&uip_buf[UIP_LLH_LEN]);
    if(uip_len > 0) {
      printf("ip64_interface_process: converted %d bytes\n", uip_len);

      printf("ip64-interface: input source ");
      PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
      PRINTF(" destination ");
      PRINT6ADDR(&UIP_IP_BUF->destipaddr);
      PRINTF("\n");

      tcpip_input();
      printf("Done\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  printf("ip64-eth-interface: init\n");
}
/*---------------------------------------------------------------------------*/
static int
output(void)
{
  int len, ret;

  printf("ip64-interface: output source ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" destination ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  printf("<--------------\n");
  len = ip64_6to4(&uip_buf[UIP_LLH_LEN], uip_len,
		  &ip64_packet_buffer[sizeof(struct ip64_eth_hdr)]);

  printf("ip64-interface: output len %d\n", len);
  if(len > 0) {
    if(ip64_arp_check_cache(&ip64_packet_buffer[sizeof(struct ip64_eth_hdr)])) {
      printf("Create header\n");
      ret = ip64_arp_create_ethhdr(ip64_packet_buffer,
				   &ip64_packet_buffer[sizeof(struct ip64_eth_hdr)]);
      if(ret > 0) {
	len += ret;
	IP64_ETH_DRIVER.output(ip64_packet_buffer, len);
      }
    } else {
      printf("Create request\n");
      len = ip64_arp_create_arp_request(ip64_packet_buffer,
					&ip64_packet_buffer[sizeof(struct ip64_eth_hdr)]);
      return IP64_ETH_DRIVER.output(ip64_packet_buffer, len);
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
const struct uip_fallback_interface ip64_eth_interface = {
  init,
  output
};
/*---------------------------------------------------------------------------*/
