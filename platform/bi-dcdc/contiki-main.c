/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <sys/autostart.h>
#include <sys/clock.h>
#include <contiki-net.h>
#include <net/dhcpc.h>
#include "debug-uart.h"
#include "emac-driver.h"
#include "contiki-conf.h"
#include <net/uip-debug.h>
#include "dev/leds.h"
#include "system_LPC17xx.h"

int main()
{
	debug_uart_setup();
	clock_init();
	process_init();

	process_start(&etimer_process, NULL );
	process_start(&emac_lpc1768, NULL );
	process_start(&tcpip_process, NULL );

#if UIP_CONF_IPV6
  uip_lladdr.addr[0] = EMAC_ADDR0;
  uip_lladdr.addr[1] = EMAC_ADDR1;
  uip_lladdr.addr[2] = EMAC_ADDR2;
  uip_lladdr.addr[3] = EMAC_ADDR3;
  uip_lladdr.addr[4] = EMAC_ADDR4;
  uip_lladdr.addr[5] = EMAC_ADDR5;

  uip_ds6_addr_t *lladdr;
  lladdr = uip_ds6_get_link_local(-1);
#else
  uip_ipaddr_t addr;
  uip_ethaddr.addr[0] = EMAC_ADDR0;
  uip_ethaddr.addr[1] = EMAC_ADDR1;
  uip_ethaddr.addr[2] = EMAC_ADDR2;
  uip_ethaddr.addr[3] = EMAC_ADDR3;
  uip_ethaddr.addr[4] = EMAC_ADDR4;
  uip_ethaddr.addr[5] = EMAC_ADDR5;
  uip_setethaddr(uip_ethaddr);

  uip_ipaddr(&addr, 192, 168, 1, 5);
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 255, 255, 255, 0);
  uip_setnetmask(&addr);

  uip_ipaddr(&addr, 192, 168, 1, 1);
  uip_setdraddr(&addr);
#endif

	autostart_start(autostart_processes);

	do
	{
	} while (process_run() > 0);

	return 0;
}

void
uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}

void
log_message(const char *part1, const char *part2)
{
  printf("log: %s: %s\n", part1, part2);
}

