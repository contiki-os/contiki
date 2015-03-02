/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 * \addtogroup cc32xx
 * @{
 *
 * \defgroup cc32xx-wifi cc32xx Wireless Network driver
 *
 * IP64 Driver for the cc32xx Wireless Network controller
 * @{
 *
 * \file
 * 		Implementation of the cc32xx Wireless Network driver
 * \author
 *      Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "contiki-net.h"
#include "net/wifi.h"

#include "net/wifi-drv.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

PROCESS(wifi_drv_process, "CC32xx WLAN driver");

/*---------------------------------------------------------------------------*/
uint8_t
wifi_drv_output(void)
{
	uip_arp_out();
	wifi_drv_send();

	return 0;
}
/*---------------------------------------------------------------------------*/
void
wifi_drv_send(void)
{
	wifi_send(&uip_buf[0], uip_len);
}
/*---------------------------------------------------------------------------*/
static void
wifi_drv_pollhandler(void)
{
	process_poll(&wifi_drv_process);
	uip_len = wifi_read(&uip_buf[0], UIP_BUFSIZE);

	if (uip_len > 0)
	{
		if (BUF->type == uip_htons(UIP_ETHTYPE_IP))
		{
			uip_len -= sizeof(struct uip_eth_hdr);
			tcpip_input();
		}
		else
		{
			if (BUF->type == uip_htons(UIP_ETHTYPE_ARP))
			{
				uip_arp_arpin();
				/* If the above function invocation resulted in data that
				 should be sent out on the network, the global variable
				 uip_len is set to a value > 0. */
				if (uip_len > 0)
				{
					wifi_drv_send();
				}
			}
		}
	}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wifi_drv_process, ev, data)
{
	PROCESS_POLLHANDLER(wifi_drv_pollhandler());
	PROCESS_BEGIN();

	wifi_init();
	tcpip_set_outputfunc(wifi_drv_output);

	process_poll(&wifi_drv_process);

	PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

	wifi_exit();

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
