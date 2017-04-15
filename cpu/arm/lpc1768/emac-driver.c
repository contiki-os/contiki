/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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

#include "emac-driver.h"
#include "emac.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(emac_lpc1768, "LPC1768 EMAC Service Process");

static struct etimer timer;  /* for periodic ARP processing */

static void
pollhandler(void)
{

  uip_len = tapdev_read(uip_buf);

  if(uip_len > 0) {
#if UIP_CONF_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    }
#else
    if(BUF->type == UIP_HTONS(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_input();

      if(uip_len > 0) {
        uip_arp_out();
        tapdev_send(uip_buf, uip_len);
      }
    } else if(BUF->type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      if(uip_len > 0) {
        tapdev_send(uip_buf, uip_len);
      }
    }
#endif
    /* If we don't know how to process it, just discard the packet */
    else {
      uip_len = 0;
    }
  }
}
#if UIP_CONF_IPV6
uint8_t
send_packet(uip_lladdr_t *lladdr)
#else
uint8_t
send_packet(void)
#endif
{
#if UIP_CONF_IPV6
  /*
   * If L3 dest is multicast, build L2 multicast address
   * as per RFC 2464 section 7
   * else fill with th eaddrsess in argument
   */
  if(lladdr == NULL) {
    /* the dest must be multicast */
    (&BUF->dest)->addr[0] = 0x33;
    (&BUF->dest)->addr[1] = 0x33;
    (&BUF->dest)->addr[2] = IPBUF->destipaddr.u8[12];
    (&BUF->dest)->addr[3] = IPBUF->destipaddr.u8[13];
    (&BUF->dest)->addr[4] = IPBUF->destipaddr.u8[14];
    (&BUF->dest)->addr[5] = IPBUF->destipaddr.u8[15];
  } else {
    memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
  } memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
  BUF->type = UIP_HTONS(UIP_ETHTYPE_IPV6);  /* math tmp */

  uip_len += sizeof(struct uip_eth_hdr);

#else
  uip_arp_out();
#endif

  tapdev_send(uip_buf, uip_len);
}
/* This is just a wrapper for the Ethernet module interrupt */
/* to call a contiki process_poll function */
void
poll_eth_driver(void)
{
  process_poll(&emac_lpc1768);
}
PROCESS_THREAD(emac_lpc1768, ev, data)
{

  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN()
  ;

  tapdev_init();

  tcpip_set_outputfunc(send_packet);

  process_poll(&emac_lpc1768);

#if UIP_CONF_IPV6
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

#else
  /* 10 second ARP timer */
  etimer_set(&timer, 10 * CLOCK_SECOND);

  while(ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT()
    ;

    if(ev == PROCESS_EVENT_TIMER) {
      etimer_set(&timer, 10 * CLOCK_SECOND);
      uip_arp_timer();
    }
  }
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

