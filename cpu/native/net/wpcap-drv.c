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
 */

#include "contiki-net.h"
#include "net/ipv4/uip-neighbor.h"
#include "net/wpcap.h"

#include "net/wpcap-drv.h"
#include <string.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/* It is not particularly easy to install tun interfaces in Windows/cygwin, so wpcap
 * is used instead. The ip4 or ip6 address of the interface to connect to is passed
 * on the command line that invokes the minimal-net and native executables.
 *
 * The minimal-net border router uses wpcap to connect to both primary
 * and fallback interfaces. It is passed two addresses, and the uip stack is compiled
 * with space for the ethernet headers on both interfaces.
 *
 * However the native border router uses wpcap to connect to a fallback interface only.
 * The primary interface is the serial connection to the slip radio, and the
 * uip stack is compiled without space for ethernet headers.
 * The following define adds or strips ethernet headers from the fallback interface.
 * Since it is at present used only with the native border router, it is also used
 * as a hack to bypass polling of the primary interface.
 *
 * SELECT_CALLBACK is defined in /examples/ipv6/native-border-router/project-conf.h
 */
#ifdef SELECT_CALLBACK
#define FALLBACK_HAS_ETHERNET_HEADERS  1
#endif

PROCESS(wpcap_process, "WinPcap driver");

/*---------------------------------------------------------------------------*/
#if !NETSTACK_CONF_WITH_IPV6
uint8_t
wpcap_output(void)
{
   uip_arp_out();
   wpcap_send();  

   return 0;
}
#endif /* !NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
#if !FALLBACK_HAS_ETHERNET_HEADERS //native br is fallback only
  process_poll(&wpcap_process);
  uip_len = wpcap_poll();

  if(uip_len > 0) {
#if NETSTACK_CONF_WITH_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
//     printf("wpcap poll calls tcpip");
      tcpip_input();
    } else
#endif /* NETSTACK_CONF_WITH_IPV6 */
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
#if !NETSTACK_CONF_WITH_IPV6
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
       uip_arp_arpin();      //math
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
       if(uip_len > 0) {
         wpcap_send();
       }
#endif /* !NETSTACK_CONF_WITH_IPV6 */
    } else {
      uip_len = 0;
    }
  }
#endif
#ifdef UIP_FALLBACK_INTERFACE

  process_poll(&wpcap_process);
  uip_len = wfall_poll();

  if(uip_len > 0) {
#if FALLBACK_HAS_ETHERNET_HEADERS
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
	//remove ethernet header and pass ipv6 packet to stack
	uip_len-=14;
//{int i;printf("\n0000 ");for (i=0;i<uip_len;i++) printf("%02x ",*(unsigned char*)(uip_buf+i));printf("\n");}
 //   memcpy(uip_buf, uip_buf+14, uip_len);
        memcpy(&uip_buf[UIP_LLH_LEN], uip_buf+14, uip_len);  //LLH_LEN is zero for native border router to slip radio
//	CopyMemory(uip_buf, uip_buf+14, uip_len);
//{int i;printf("\n0000 ");for (i=0;i<uip_len;i++) printf("%02x ",*(char*)(uip_buf+i));printf("\n");}	
      tcpip_input();
    } else
	 goto bail;
#elif NETSTACK_CONF_WITH_IPV6
    if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
      tcpip_input();
    } else
	 goto bail;
#endif /* NETSTACK_CONF_WITH_IPV6 */
    if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
#if !NETSTACK_CONF_WITH_IPV6
    } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
       uip_arp_arpin();      //math
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
       if(uip_len > 0) {
         wfall_send();
       }
#endif /* !NETSTACK_CONF_WITH_IPV6 */
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

#if !NETSTACK_CONF_WITH_IPV6
  tcpip_set_outputfunc(wpcap_output);
#else
#if !FALLBACK_HAS_ETHERNET_HEADERS
  tcpip_set_outputfunc(wpcap_send);
#endif
#endif /* !NETSTACK_CONF_WITH_IPV6 */

  process_poll(&wpcap_process);

  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  wpcap_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
