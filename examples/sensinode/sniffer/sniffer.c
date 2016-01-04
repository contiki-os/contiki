/*
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

#include "contiki.h"

#if CC2430_RF_CONF_HEXDUMP
#include "cc2430_sfr.h"
#endif

#if CC2420_CONF_RF_SNIFFER
#include "cc2420.h"
#endif

#if CC2538_RF_CONF_SNIFFER
#include "dev/cc2538-rf.h"
#endif

//#define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/
PROCESS(sniffer_process, "Sniffer process");
AUTOSTART_PROCESSES(&sniffer_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sniffer_process, ev, data)
{

  PROCESS_BEGIN();

  PRINTF("Sniffer started\n");

#if CC2430_RF_CONF_HEXDUMP
  /* Turn off cc2430 Address Recognition - We need to accept all frames */
  MDMCTRL0H &= ~0x08;
#endif

#if CC2420_CONF_RF_SNIFFER || CC2538_RF_CONF_SNIFFER
  static uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  /* Turn off RF Address Recognition - We need to accept all frames */
  //FRMFILT0 &= ~0x01;
  radio_value_t radio_rx_mode;
  NETSTACK_RADIO.get_value(RADIO_PARAM_RX_MODE, &radio_rx_mode);
  NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, radio_rx_mode & (~RADIO_RX_MODE_ADDRESS_FILTER));
  NETSTACK_RADIO.on();
#endif

  PROCESS_EXIT();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
