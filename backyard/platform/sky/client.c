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
 * @(#)$Id: client.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 */

/*
 * Example configuration with one statically configured IP interface.
 *
 * The IP address is hardcoded to 172.16.0.9 and the 172.16/16 network
 * is on the radio interface.
 *
 * The default route and gateway is at 172.16.0.1.
 *
 * On the SLIP gateway run a standard SLIP implementation or the
 * tunslip program that can also view debug printfs, example:
 *
 * sliphost# ./tunslip 172.16.0.1 255.255.0.0
 *
 *
 * This kernel has no application code but new modules can be uploaded
 * using the codeprop program, example:
 *
 * datan$ ./codeprop 172.16.0.9 loadable_prg.ko
 * File successfully sent (1116 bytes)
 * Reply: ok
 *
 */

#include <stdio.h>
#include <stdlib.h>
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

#include "dev/button.h"

#include "codeprop/codeprop.h"

/* This is how we force inclusion of the psock library. */
#include "net/psock.h"
void *force_psock_inclusion = &psock_init;
void *force_udp_inclusion = &uip_udp_packet_send;

struct uip_fw_netif cc2420if =
{UIP_FW_NETIF(172,16,0,9, 255,255,0,0, cc2420_send_uaodv)};

PROCESS(button_process, "Button process");

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
	 "($Id: client.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $)\n", __FILE__);
  ds2411_init();
  sensors_light_init();
  cc2420_init();
  xmem_init();
  button_init(&button_process);
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
  uip_ipaddr(&uip_draddr, 172,16,0,1);
  printf("IP %d.%d.%d.%d netmask %d.%d.%d.%d default router %d.%d.%d.%d\n",
	 uip_ipaddr_to_quad(&uip_hostaddr),
	 uip_ipaddr_to_quad(&uip_netmask),
	 uip_ipaddr_to_quad(&uip_draddr));
  cc2420_set_chan_pan_addr(RF_CHANNEL, panId, uip_hostaddr.u16[1], ds2411_id);

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);

  /* Configure IP stack. */
  uip_init();
  uip_fw_default(&cc2420if);
  tcpip_set_forwarding(1);
  
  /* Start IP stack. */
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&cc2420_process, NULL);
  cc2420_on();
  process_start(&uaodv_process, NULL);

  process_start(&button_process, NULL);
  process_start(&tcp_loader_process, NULL);

  /*
   * This is the scheduler loop.
   */
  printf("process_run()...\n");
  while (1) {
    do {
      /* Reset watchdog. */
    } while(process_run() > 0);

    /*
     * Idle processing.
     */
    int s = splhigh();		/* Disable interrupts. */
    if(process_nevents() != 0) {
      splx(s);			/* Re-enable interrupts. */
    } else {
      /* Re-enable interrupts and go to sleep atomically. */
      _BIS_SR(GIE | SCG0 | CPUOFF); /* LPM1 sleep. */
    }
  }

  return 0;
}

PROCESS_THREAD(button_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("button_process starting\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_MSG && data != NULL
       && ((struct button_msg *)data)->type == BUTTON_MSG_TYPE) {
      printf("button press\n");

      leds_toggle(LEDS_ALL);
      etimer_set(&etimer, CLOCK_SECOND);
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
      leds_toggle(LEDS_ALL);
    }
  }

 exit:
  printf("button_process exiting\n");
  PROCESS_END();
}
