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
 * $Id: tfe-drv.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 *
 */

#include "contiki-net.h"
#include "cs8900a.h"

static u8_t output(void);

static const struct uip_eth_addr addr =
  {{0x00,0x00,0x00,0x64,0x64,0x64}};

SERVICE(tfe_drv_service, packet_service, { output };);

PROCESS(tfe_drv_process, "TFE driver");

/*---------------------------------------------------------------------------*/
static u8_t
output(void)
{
  uip_arp_out();
  cs8900a_send();
  
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
  
  /* Poll Ethernet device to see if there is a frame avaliable. */
  uip_len = cs8900a_poll();
  if(uip_len > 0) {
    /* A frame was avaliable (and is now read into the uip_buf), so
       we process it. */
    if(BUF->type == UIP_HTONS(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
    } else if(BUF->type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        cs8900a_send();
      }
    }
  }

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tfe_drv_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());
  
  PROCESS_BEGIN();

  
  uip_setethaddr(addr);

  cs8900a_init();
  
  SERVICE_REGISTER(tfe_drv_service);

  process_poll(&tfe_drv_process);
  
  while(1) {
    PROCESS_YIELD();
  }
  
  PROCESS_END();
}


/*---------------------------------------------------------------------------*/
