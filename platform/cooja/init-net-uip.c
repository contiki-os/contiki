/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: init-net-uip.c,v 1.4 2009/04/23 09:15:51 fros4943 Exp $
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>

#include "net/rime.h"
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"

#include "node-id.h"
#include "dev/cooja-radio.h"

#include "dev/slip.h"

#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
static u8_t
sender(void)
{
  return cooja_radio.send((char*)UIP_IP_BUF, uip_len);
}
/*---------------------------------------------------------------------------*/
static void
receiver(const struct radio_driver *d)
{
  uip_len = d->read((char*)UIP_IP_BUF, UIP_BUFSIZE - UIP_LLH_LEN);
  tcpip_input();
}
/*---------------------------------------------------------------------------*/
/* Only using a single network interface */
static struct uip_fw_netif wsn_if =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, sender)};
static struct uip_fw_netif slip_if =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, slip_send)};
/*---------------------------------------------------------------------------*/
void
init_net(void)
{
  int i;
  uip_ipaddr_t hostaddr, netmask;
  rimeaddr_t rimeaddr;

  /* Init Rime */
  ctimer_init();
  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);
  printf("Rime started with address: ");
  for(i = 0; i < sizeof(rimeaddr_node_addr.u8) - 1; i++) {
    printf("%d.", rimeaddr_node_addr.u8[i]);
  }
  printf("%d\n", rimeaddr_node_addr.u8[i]);

  /* Init uIPv4 */
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);
  process_start(&slip_process, NULL);
  uip_init();
  uip_fw_init();
  uip_ipaddr(&hostaddr, 172, 16, rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);
  uip_ipaddr(&netmask, 255,255,0,0);
  uip_sethostaddr(&hostaddr);
  uip_setnetmask(&netmask);
  uip_fw_register(&wsn_if);
  uip_fw_default(&slip_if);
  rs232_set_input(slip_input_byte);
  printf("uIP started with IP address: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&hostaddr));

  /* uIPv4 <-> COOJA's packet radio */
  /*tcpip_set_outputfunc(sender);*/
  cooja_radio.set_receive_function(receiver);
}
