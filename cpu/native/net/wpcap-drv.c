/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: wpcap-drv.c,v 1.7 2010/10/19 18:29:04 adamdunkels Exp $
 */

#include "contiki-net.h"
#include "net/uip-neighbor.h"
#include "net/wpcap.h"

#include "net/wpcap-drv.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(wpcap_process, "WinPcap driver");

/*---------------------------------------------------------------------------*/
#if !UIP_CONF_IPV6
u8_t
wpcap_output(void)
{
   uip_arp_out();
   wpcap_send();  

   return 0;
}
#endif /* !UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  process_poll(&wpcap_process);
  uip_len = wpcap_poll();

  if(uip_len > 0) {
#if UIP_CONF_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    } else
#endif /* UIP_CONF_IPV6 */
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
#if !UIP_CONF_IPV6
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
       uip_arp_arpin();      //math
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
       if(uip_len > 0) {
         wpcap_send();
       }
#endif /* !UIP_CONF_IPV6 */
    } else {
      uip_len = 0;
    }
  }

#ifdef UIP_FALLBACK_INTERFACE

  process_poll(&wpcap_process);
  uip_len = wfall_poll();

  if(uip_len > 0) {
#if UIP_CONF_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    } else
	 goto bail;
#endif /* UIP_CONF_IPV6 */
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
#if !UIP_CONF_IPV6
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
       uip_arp_arpin();      //math
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
       if(uip_len > 0) {
         wfall_send();
       }
#endif /* !UIP_CONF_IPV6 */
    } else {
bail:
      uip_len = 0;
    }
  }
#endif

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wpcap_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  wpcap_init();

#if !UIP_CONF_IPV6
  tcpip_set_outputfunc(wpcap_output);
#else
  tcpip_set_outputfunc(wpcap_send);
#endif /* !UIP_CONF_IPV6 */

  process_poll(&wpcap_process);

  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  wpcap_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
