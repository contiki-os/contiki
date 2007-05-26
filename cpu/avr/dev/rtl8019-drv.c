/*-----------------------------------------------------------------------------------*/
/*
 * Copyright (c) 2001-2004, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: rtl8019-drv.c,v 1.2 2007/05/26 23:05:36 oliverschmidt Exp $
 *
 */

#include "contiki-net.h"
#include "dev/rtl8019dev.h"
#include "net/uip-neighbor.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

PROCESS(rtl8019_process, "RTL8019 driver");

/*---------------------------------------------------------------------------*/
u8_t
rtl8019_output(void)
{
  uip_arp_out();
  RTL8019dev_send();

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  process_poll(&rtl8019_process);
  uip_len = RTL8019dev_poll();

  if(uip_len > 0) {
#if UIP_CONF_IPV6
    if(BUF->type == htons(UIP_ETHTYPE_IPV6)) {
      uip_neighbor_add(&IPBUF->srcipaddr, &BUF->src);
      tcpip_input();
    } else
#endif /* UIP_CONF_IPV6 */
    if(BUF->type == htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
    } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
	 should be sent out on the network, the global variable
	 uip_len is set to a value > 0. */
      if(uip_len > 0) {
	RTL8019dev_send();
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rtl8019_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  RTL8019dev_init();

  tcpip_set_outputfunc(rtl8019_output);

  process_poll(&rtl8019_process);
  
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  RTL8019dev_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
