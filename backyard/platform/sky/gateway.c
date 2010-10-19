/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: gateway.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 */

/*
 * Example gateway configuration with two IP interfaces, one SLIP over
 * USB and one over 802.11.4 radio.
 *
 * The IP address is hardcoded to 172.16.0.1 and the 172.16/16 network
 * is on the radio interface.
 *
 * The default route is over the SLIP interface.
 *
 * On the SLIP host run a standard SLIP implementation or the tunslip
 * program that can also view debug printfs, example:
 *
 * sliphost# ./tunslip 172.16.0.1 255.255.0.0
 *
 *
 * This kernel has no application code but new modules can be uploaded
 * using the codeprop program, example:
 *
 * datan$ ./codeprop 172.16.0.1 loadable_prg.ko
 * File successfully sent (1116 bytes)
 * Reply: ok
 *
 */

#include <stdio.h>
#include <string.h>

#include <io.h>

#include "contiki.h"

/* Also IP output. */
#include "net/uip-fw-drv.h"
#include "net/uaodv.h"
#include "dev/slip.h"
#include "dev/cc2420.h"

#include "dev/ds2411.h"
#include "dev/leds.h"
#include "dev/light.h"
#include "dev/xmem.h"
#include "lib/rand.h"

#include "dev/button.h"

#include "codeprop/codeprop.h"

/*
 * This is how we force inclusion of the psock library and the button
 * device driver.
 */
#include "net/psock.h"
void *force_inclusion[] = {
  &psock_init,
  &button_init,
  &uip_udp_packet_send,
};
#if 0
int
force_float_inclusion()
{
  extern int __fixsfsi;
  extern int __floatsisf;
  extern int __mulsf3;
  extern int __subsf3;

  return __fixsfsi + __floatsisf + __mulsf3 + __subsf3;
}
#endif

/* We have two IP interfaces. */
struct uip_fw_netif cc2420if =
{UIP_FW_NETIF(172,16,0,1, 255,255,0,0, cc2420_send_uaodv)};

static struct uip_fw_netif slipif =
{UIP_FW_NETIF(0,0,0,0, 255,255,255,255, slip_send)};

/* Radio stuff in network byte order. */
static u16_t panId = UIP_HTONS(0x2024);

#ifndef RF_CHANNEL
#define RF_CHANNEL              15
#endif

int
main(int argc, char **argv)
{
  /*
   * Initalize hardware.
   */
  msp430_cpu_init();
  clock_init();
  leds_init();
  leds_toggle(LEDS_ALL);
  slip_arch_init(BAUD2UBR(115200)); /* Must come before first printf */
  printf("Starting %s "
	 "($Id: gateway.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $)\n", __FILE__);
  ds2411_init();
  sensors_light_init();
  cc2420_init();
  xmem_init();
  leds_toggle(LEDS_ALL);
  /*
   * Hardware initialization done!
   */
  
  printf("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x CHANNEL %d\n",
	 ds2411_id[0], ds2411_id[1], ds2411_id[2], ds2411_id[3],
	 ds2411_id[4], ds2411_id[5], ds2411_id[6], ds2411_id[7],
	 RF_CHANNEL);

  uip_ipaddr_copy(&uip_hostaddr, &cc2420if.ipaddr);
  uip_ipaddr_copy(&uip_netmask, &cc2420if.netmask);
  printf("IP %d.%d.%d.%d netmask %d.%d.%d.%d\n",
	 uip_ipaddr_to_quad(&uip_hostaddr), uip_ipaddr_to_quad(&uip_netmask));
  cc2420_set_chan_pan_addr(RF_CHANNEL, panId, uip_hostaddr.u16[1], ds2411_id);

  srand(rand() +
	(ds2411_id[3]<<8) + (ds2411_id[4]<<6) + (ds2411_id[5]<<4) +
	(ds2411_id[6]<<2) +  ds2411_id[7]);

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);

  /* Configure IP stack. */
  uip_init();
  uip_fw_default(&slipif);	/* Point2point, no default router. */
  uip_fw_register(&cc2420if);
  tcpip_set_forwarding(1);
  
  /* Start IP stack. */
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);
  process_start(&cc2420_process, NULL);
  cc2420_on();
  process_start(&uaodv_process, NULL);

  process_start(&tcp_loader_process, NULL);

  /*
   * This is the scheduler loop.
   */
  printf("process_run()...\n");
  while (1) {
    do {
      /* Reset watchdog. */
    } while(process_run() > 0);
    /* Idle! */
  }

  return 0;
}
