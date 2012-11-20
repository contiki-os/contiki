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

/**
 * \file
 *	uIP initialization for the MSB-430 port.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "sys/node-id.h"
#include "dev/slip.h"
#include "dev/cc1020-uip.h"
#include "contiki-msb430.h"

static struct uip_fw_netif slipif =
{UIP_FW_NETIF(172,16,0,0, 255,255,255,0, slip_send)};

static struct uip_fw_netif wsnif =
{UIP_FW_NETIF(0,0,0,0, 0,0,0,0, cc1020_uip_send)};

void
init_net(void)
{
  uip_ipaddr_t hostaddr;
  cc1020_init(cc1020_config_19200);

  uip_init();
  uip_fw_init();
  /*tcpip_set_forwarding(1);*/

  process_start(&tcpip_process, NULL);
  process_start(&slip_process, NULL);
  process_start(&uip_fw_process, NULL);
  
  rs232_set_input(slip_input_byte);

  cc1020_uip_init();
  cc1020_on();
  
  if (node_id > 0) {
    uip_ipaddr(&hostaddr, 172, 16, 1, node_id);
    uip_sethostaddr(&hostaddr);
  }

  uip_fw_register(&slipif);
  uip_fw_default(&wsnif);
}
