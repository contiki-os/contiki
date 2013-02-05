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
 * This is a modified contiki driver example.
 * Author: Maciej Wasilak (wasilak@gmail.com)
 *
 * $Id: enc28j60-drv.c,v 1.2 2007/05/26 23:05:36 oliverschmidt Exp $
 *
 */

#include "enc28j60.h"
#include "enc28j60-drv.h"
#include "contiki-net.h"
#include "net/uip-neighbor.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"
#include "string.h"
#include "sicslow_ethernet.h"

#include "cetic_bridge.h"
#include "nvm_config.h"
#include "packet-filter.h"

#include "isr.h"

#define DEBUG 0
#include "net/uip-debug.h"

PROCESS(eth_drv_process, "ENC28J60 driver");

extern uint8_t prefixCounter;
extern ethernet_ready;

#if UIP_CONF_LLH_LEN == 0
uint8_t ll_header[ETHERNET_LLH_LEN];
#endif

extern void eth_input(void);

//TEMPORARY
uint16_t
uip_ipchksum(void)
{
	return 0;
}

/*---------------------------------------------------------------------------*/

void
eth_drv_send(void)
{
	PRINTF("ENC28 send: %d bytes : %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x  %x:%x %x %x %x %x %x %x\n",
			uip_len,
			ll_header[0], ll_header[1],ll_header[2],ll_header[3],ll_header[4],ll_header[5],
			ll_header[6], ll_header[7],ll_header[8],ll_header[9],ll_header[10],ll_header[11],
			ll_header[12], ll_header[13],
			uip_buf[0], uip_buf[1], uip_buf[2], uip_buf[3], uip_buf[4], uip_buf[5] );

	disable_int(enc28j60PacketSend(uip_len + sizeof(struct uip_eth_hdr), uip_buf));
}

/*
 * Placeholder - switching off enc28 chip wasn't yet considered
 */
void eth_drv_exit(void)
{}

/*
 * Wrapper for lowlevel enc28j60 init code
 * in current configuration it reads the Ethernet driver MAC address
 * from EEPROM memory
 */
void eth_drv_init()
{
	PRINTF("ENC28J60 init\n");
	enc28j60Init(eth_mac_addr);

	prefixCounter = 0;
}

/*---------------------------------------------------------------------------*/
void
enc28j60_pollhandler(void)
{
	//process_poll(&enc28j60_process);

	disable_int(uip_len = enc28j60PacketReceive(UIP_BUFSIZE,uip_buf));

	if (uip_len > 0) {
		eth_input();
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(eth_drv_process, ev, data)
{
	//PROCESS_POLLHANDLER(enc28j60_pollhandler());

	PROCESS_BEGIN();

	printf("ENC-28J60 Process started\n");
	eth_drv_init();

	ethernet_ready = 1;

	while (1) {
		enc28j60_pollhandler();
		PROCESS_PAUSE();
	}

	eth_drv_exit();

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
