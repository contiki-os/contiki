/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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

#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ip/uipopt.h"

#if NETSTACK_CONF_WITH_IPV6
#include "tapdev6.h"
#else
#include "tapdev.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "tapdev-drv.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(tapdev_process, "TAP driver");

/*---------------------------------------------------------------------------*/
#if !NETSTACK_CONF_WITH_IPV6
uint8_t
tapdev_output(void)
{
   uip_arp_out();
   tapdev_send();  
   return 0;
}
#endif
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  uip_len = tapdev_poll();

  if(uip_len > 0) {
#if NETSTACK_CONF_WITH_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    } else
#endif /* NETSTACK_CONF_WITH_IPV6 */
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
#if !NETSTACK_CONF_WITH_IPV6 //math
       uip_arp_arpin();
       /* If the above function invocation resulted in data that
	  should be sent out on the network, the global variable
	  uip_len is set to a value > 0. */
       if(uip_len > 0) {
	  tapdev_send();
       }
#endif              
    } else {
      uip_len = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tapdev_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  tapdev_init();
#if !NETSTACK_CONF_WITH_IPV6
  tcpip_set_outputfunc(tapdev_output);
#else
  tcpip_set_outputfunc(tapdev_send);
#endif
  process_poll(&tapdev_process);

  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  tapdev_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
