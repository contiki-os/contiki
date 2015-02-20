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
 * \defgroup cc32xx-wifi cc32xx IP64 Wireless Network driver
 *
 * IP64 Driver for the cc32xx Wireless Network controller
 * @{
 *
 * \file
 * Implementation of the cc32xx IP64 Wireless Network driver
 */

#include "contiki.h"
#include "net/wifi.h"
#include "net/wifi-ip64-drv.h"

#include "net/ip64/ip64.h"
#include "net/ip64/ip64-eth.h"

#include <string.h>

PROCESS(wifi_ip64_driver_process, "CC32xx WLAN IP64 driver");

/*---------------------------------------------------------------------------*/
static void
wifi_ip64_init(void)
{
	uint32_t hostaddr = uip_htonl(wifi_own_ip);
	uint32_t netmask = uip_htonl(wifi_netmask);
	uint32_t gateway = uip_htonl(wifi_gateway);

	// Setup Ethernet address
	memcpy(ip64_eth_addr.addr, wifi_mac_addr, sizeof(wifi_mac_addr));

	// Setup IP, Gateway and Netmask
	ip64_set_hostaddr((uip_ip4addr_t *)&hostaddr);
	ip64_set_netmask((uip_ip4addr_t *)&netmask);
	ip64_set_draddr((uip_ip4addr_t *)&gateway);

	// Startup driver process
	process_start(&wifi_ip64_driver_process, NULL);
}
/*---------------------------------------------------------------------------*/
static int
wifi_ip64_output(uint8_t *packet, uint16_t len)
{
	wifi_send(packet, len);
	return len;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wifi_ip64_driver_process, ev, data)
{
	static int len;
	static struct etimer e;
	PROCESS_BEGIN();

	while(1)
	{
		etimer_set(&e, 1);
		PROCESS_WAIT_EVENT();
		len = wifi_read(ip64_packet_buffer, ip64_packet_buffer_maxlen);
		if(len > 0)
		{
			IP64_INPUT(ip64_packet_buffer, len);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
const struct ip64_driver wifi_ip64_driver = {
	wifi_ip64_init,
	wifi_ip64_output
};
/*---------------------------------------------------------------------------*/
