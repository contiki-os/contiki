/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: hello-world.c,v 1.1 2006/10/02 21:46:46 adamdunkels Exp $
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-nd6.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "net/rpl/rpl.h"

#include <stdio.h> /* For printf() */
#include <string.h>
#include <ctype.h>

#include "cetic_bridge.h"
#include "packet-filter.h"
#include "eth-drv.h"
#include "nvm_config.h"
#include "rio.h"

#if CONTIKI_TARGET_REDBEE_ECONOTAG
#include "mc1322x.h"
#endif

#include "node-info.h"

#if CONTIKI_TARGET_NATIVE
#include "slip-cmds.h"

extern int contiki_argc;
extern char **contiki_argv;
extern int slip_config_handle_arguments(int argc, char **argv);
#endif

int ethernet_ready = 0;

//WSN

uip_lladdr_t wsn_mac_addr;
uip_ip6addr_t wsn_net_prefix;
uip_ipaddr_t wsn_ip_addr;
uip_ipaddr_t wsn_ip_local_addr;
rpl_dag_t *cetic_dag;

// Eth
ethaddr_t eth_mac_addr;
uip_lladdr_t eth_mac64_addr;
uip_ipaddr_t eth_ip_addr;
uip_ipaddr_t eth_net_prefix;
uip_ipaddr_t eth_ip_local_addr;
uip_ipaddr_t eth_dft_router;

#define DEBUG 1//DEBUG_NONE
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr[0], lladdr[1], lladdr[2], lladdr[3],lladdr[4], lladdr[5], lladdr[6], lladdr[7])
#define PRINTETHADDR(addr) printf(" %02x:%02x:%02x:%02x:%02x:%02x ",(*addr)[0], (*addr)[1], (*addr)[2], (*addr)[3], (*addr)[4], (*addr)[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#define PRINTETHADDR(addr)
#endif

/*---------------------------------------------------------------------------*/
PROCESS_NAME(webserver_nogui_process);
PROCESS_NAME(udp_server_process);
PROCESS(cetic_bridge_process, "CETIC Bridge process");

#if CONTIKI_TARGET_NATIVE
AUTOSTART_PROCESSES(&cetic_bridge_process, &border_router_cmd_process);
#else
AUTOSTART_PROCESSES(&cetic_bridge_process);
#endif
/*---------------------------------------------------------------------------*/

void cetic_bridge_set_prefix( uip_ipaddr_t *prefix, unsigned len, uip_ipaddr_t *ipaddr)
{
  PRINTF("CETIC_BRIDGE : set_prefix\n");
  if ((nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) == 0 ) {
   PRINTF("Ignoring RA\n");
   return;
  }
  
  if(cetic_dag != NULL) {
      PRINTF("created a new RPL dag\n");
      PRINTF("Setting DAG prefix : ");
      PRINT6ADDR(&prefix->u8);
      PRINTF("\n");
      rpl_set_prefix(cetic_dag, prefix, len);
#if CETIC_6LBR_TRANSPARENTBRIDGE
      cetic_dag->prefix_info.flags |= UIP_ND6_RA_FLAG_ONLINK;
#endif
	  uip_ipaddr_copy(&wsn_net_prefix, prefix);
  }
}

void cetic_bridge_init(void)
{
  uip_ipaddr_t loc_fipaddr;

#if CETIC_NODE_INFO
  node_info_init();
#endif

  //DODAGID = link-local address used !
  uip_create_linklocal_prefix(&loc_fipaddr);
  uip_ds6_set_addr_iid(&loc_fipaddr, &uip_lladdr);
  cetic_dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &loc_fipaddr);

  uip_ds6_addr_t *  local = uip_ds6_get_link_local(-1);
  uip_ipaddr_copy(&wsn_ip_local_addr, &local->ipaddr);

  PRINTF("Tentative local IPv6 address ");
  PRINT6ADDR(&wsn_ip_local_addr);
  PRINTF("\n");

#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
  
  if ((nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) == 0 ) //Manual prefix configuration
  {
	memcpy(wsn_net_prefix.u8, &nvm_data.wsn_net_prefix, sizeof(nvm_data.wsn_net_prefix));
#if CETIC_6LBR_TRANSPARENTBRIDGE
	//Set wsn prefix as on-link
    uip_ds6_prefix_add(&wsn_net_prefix, 64, 0);
#endif
    if ((nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) != 0 ) //Address auto configuration
    {
    	uip_ipaddr_copy(&wsn_ip_addr, &wsn_net_prefix);
    	uip_ds6_set_addr_iid(&wsn_ip_addr, &uip_lladdr);
    	uip_ds6_addr_add(&wsn_ip_addr, 0, ADDR_AUTOCONF);
    } else {
    	memcpy(wsn_ip_addr.u8, &nvm_data.wsn_ip_addr, sizeof(nvm_data.wsn_ip_addr));
    	uip_ds6_addr_add(&wsn_ip_addr, 0, ADDR_MANUAL);
    }
    PRINTF("Tentative global IPv6 address ");
    PRINT6ADDR(&wsn_ip_addr.u8);
    PRINTF("\n");
	memcpy(eth_dft_router.u8, &nvm_data.eth_dft_router, sizeof(nvm_data.eth_dft_router));
	uip_ds6_defrt_add(&eth_dft_router, 0);
	rpl_set_prefix(cetic_dag, &wsn_net_prefix, 64);
#if CETIC_6LBR_TRANSPARENTBRIDGE
    cetic_dag->prefix_info.flags |= UIP_ND6_RA_FLAG_ONLINK;
#endif
  } //End manual configuration
  
#if CETIC_6LBR_TRANSPARENTBRIDGE
  printf("Starting as Transparent-BRIDGE\n");
#else
  printf("Starting as Smart-BRIDGE\n");
#endif
#else /* ROUTER */

	//WSN network configuration
	memcpy(wsn_net_prefix.u8, &nvm_data.wsn_net_prefix, sizeof(nvm_data.wsn_net_prefix));
    if ((nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) != 0 ) //Address auto configuration
    {
    	uip_ipaddr_copy(&wsn_ip_addr, &wsn_net_prefix);
    	uip_ds6_set_addr_iid(&wsn_ip_addr, &uip_lladdr);
    	uip_ds6_addr_add(&wsn_ip_addr, 0, ADDR_AUTOCONF);
    } else {
    	memcpy(wsn_ip_addr.u8, &nvm_data.wsn_ip_addr, sizeof(nvm_data.wsn_ip_addr));
    	uip_ds6_addr_add(&wsn_ip_addr, 0, ADDR_MANUAL);
    }
	PRINTF("Tentative global IPv6 address (WSN) ");
	PRINT6ADDR(&wsn_ip_addr.u8);
	PRINTF("\n");

	//Ethernet network configuration
	memcpy(eth_net_prefix.u8, &nvm_data.eth_net_prefix, sizeof(nvm_data.eth_net_prefix));
	if ((nvm_data.mode & CETIC_MODE_ROUTER_SEND_CONFIG) != 0 )
	{
		PRINTF("RA with autoconfig\n");
		uip_ds6_prefix_add(&eth_net_prefix, 64, 1, UIP_ND6_RA_FLAG_ONLINK|UIP_ND6_RA_FLAG_AUTONOMOUS, 30000, 30000);
	} else {
		PRINTF("RA without autoconfig\n");
		uip_ds6_prefix_add(&eth_net_prefix, 64, 0, 0, 0, 0);
	}

	memcpy(eth_dft_router.u8, &nvm_data.eth_dft_router, sizeof(nvm_data.eth_dft_router));
	uip_ds6_defrt_add(&eth_dft_router, 0);

	eth_mac64_addr.addr[0] = eth_mac_addr[0];
	eth_mac64_addr.addr[1] = eth_mac_addr[1];
	eth_mac64_addr.addr[2] = eth_mac_addr[2];
	eth_mac64_addr.addr[3] = 0xFF;
	eth_mac64_addr.addr[4] = 0xFE;
	eth_mac64_addr.addr[5] = eth_mac_addr[3];
	eth_mac64_addr.addr[6] = eth_mac_addr[4];
	eth_mac64_addr.addr[7] = eth_mac_addr[5];

    if ((nvm_data.mode & CETIC_MODE_ETH_MANUAL) == 0 ) //Address auto configuration
    {
    	uip_ipaddr_copy(&eth_ip_addr, &eth_net_prefix);
    	uip_ds6_set_addr_iid(&eth_ip_addr, &eth_mac64_addr);
    	uip_ds6_addr_add(&eth_ip_addr, 0, ADDR_AUTOCONF);
    } else {
    	memcpy(eth_ip_addr.u8, &nvm_data.eth_ip_addr, sizeof(nvm_data.eth_ip_addr));
    	uip_ds6_addr_add(&eth_ip_addr, 0, ADDR_MANUAL);
    }
	PRINTF("Tentative global IPv6 address (ETH) ");
	PRINT6ADDR(&eth_ip_addr.u8);
	PRINTF("\n");

	rpl_set_prefix(cetic_dag, &wsn_net_prefix, 64);

	//Ugly hack : in order to set WSN local address as the default address
	//We must add it afterwards as uip_ds6_addr_add allocates addr from the end of the list
	uip_ds6_addr_rm(local);

	uip_create_linklocal_prefix(&eth_ip_local_addr);
	uip_ds6_set_addr_iid(&eth_ip_local_addr, &eth_mac64_addr);
	uip_ds6_addr_add(&eth_ip_local_addr, 0, ADDR_AUTOCONF);

	uip_ds6_addr_add(&wsn_ip_local_addr, 0, ADDR_AUTOCONF);

	uip_ds6_route_info_add(&wsn_net_prefix, 64, 0, 600);

  printf("Starting as ROUTER\n");
#endif
}

/*---------------------------------------------------------------------------*/

static struct etimer reboot_timer;

PROCESS_THREAD(cetic_bridge_process, ev, data)
{
  PROCESS_BEGIN();

#if CONTIKI_TARGET_NATIVE
	slip_config_handle_arguments(contiki_argc, contiki_argv);
#endif

  load_nvm_config();

#if CONTIKI_TARGET_REDBEE_ECONOTAG
  //TODO: Should be moved to platform dep file
  set_channel(nvm_data.channel - 11);
#endif

  process_start(&eth_drv_process, NULL);

  while (!ethernet_ready) {
	  PROCESS_PAUSE();
  }

  process_start(&tcpip_process, NULL);

  PROCESS_PAUSE();

  packet_filter_init();
  cetic_bridge_init();

#if WEBSERVER
  process_start(&webserver_nogui_process, NULL);
#endif
#if UDPSERVER
  process_start(&udp_server_process, NULL);
#endif
  printf ("RPL Border Router Started\n");

#if CONTIKI_TARGET_NATIVE
  PROCESS_WAIT_EVENT();
  etimer_set(&reboot_timer, CLOCK_SECOND);
  PROCESS_WAIT_EVENT();
  printf("Exiting...\n");
  exit(0);
#endif

  PROCESS_END();
}
