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
 * @(#)$Id: ssd.c,v 1.1 2006/10/10 08:30:48 bg- Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>

#include "contiki.h"

/* Also IP output. */
#include "net/uip-fw-service.h"
#include "net/uaodv.h"
#include "net/dhcpc.h"
#include "dev/slip.h"
#include "dev/cc2420.h"

#include "dev/ds2411.h"
#include "dev/leds.h"
#include "dev/light.h"
#include "dev/xmem.h"

#include "dev/button.h"

#include "codeprop/codeprop.h"

#include "lib/crtk.h"

/* This is how we force inclusion of the psock library. */
#include "net/psock.h"
void *force_psock_inclusion = &psock_init;

void uip_log(char *msg) { puts(msg); }

#define USB_PLUGGED() (P1IN & BV(2))

struct uip_fw_netif cc2420if =
{UIP_FW_NETIF(0,0,0,0, 255,255,255,255, cc2420_send_uaodv)};

static struct uip_fw_netif slipif =
{UIP_FW_NETIF(0,0,0,0, 255,255,255,255, slip_send)};

PROCESS(usb_process, "Button process");
PROCESS(dhclient_process, "Dhclient process");

/* Radio stuff in network byte order. */
static u16_t panId = HTONS(0x2024);

#define RF_CHANNEL              26

int
main(int argc, char **argv)
{
  /*
   * Initalize hardware.
   */
  msp430_cpu_init();
  clock_init();
  leds_init();
  leds_toggle(LEDS_RED | LEDS_GREEN | LEDS_BLUE);
  slip_arch_init();		/* Must come before first printf */
  printf("Starting %s "
	 "($Id: ssd.c,v 1.1 2006/10/10 08:30:48 bg- Exp $)\n", __FILE__);
  ds2411_init();
  sensors_light_init();
  cc2420_init();
  xmem_init();
  button_init(NULL);
  leds_toggle(LEDS_RED | LEDS_GREEN | LEDS_BLUE);
  /*
   * Hardware initialization done!
   */
  
  printf("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	 ds2411_id[0], ds2411_id[1], ds2411_id[2], ds2411_id[3],
	 ds2411_id[4], ds2411_id[5], ds2411_id[6], ds2411_id[7]);

  srand((ds2411_id[3]<<8) + (ds2411_id[4]<<6) + (ds2411_id[5]<<4) +
	(ds2411_id[6]<<2) +  ds2411_id[7]);

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);

  /* Configure IP stack. */
  uip_init();
  
  /* Start IP stack. */
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);
  process_start(&cc2420_process, NULL);
  cc2420_on();
  process_start(&dhclient_process, NULL);

  process_start(&usb_process, NULL);
  process_start(&tcp_loader_process, NULL);

  process_start(&crtk_process, NULL);

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

PROCESS_THREAD(dhclient_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("dhclient_process starting\n");

  leds_on(LEDS_GREEN);

  uip_setipid(rand());

  /* For now use 0.0.0.0 as our IP address. */
  uip_ipaddr(&uip_hostaddr, 0,0,0,0);
  cc2420_set_chan_pan_addr(RF_CHANNEL, panId, uip_hostaddr.u16[1], ds2411_id);

  uip_fw_init();
  tcpip_set_forwarding(1);
  if(USB_PLUGGED())
    uip_fw_default(&slipif);
  else
    uip_fw_default(&cc2420if);

  dhcpc_init(ds2411_id, sizeof(ds2411_id));
  dhcpc_request();

  while(1) {
    PROCESS_WAIT_EVENT();
    dhcpc_appcall(ev, data);
  }

 exit:
  printf("dhclient_process exiting\n");
  PROCESS_END();
}

static char dhcp_is_conf;

#define ip2quad(p) uip_ipaddr1(p),uip_ipaddr2(p),uip_ipaddr3(p),uip_ipaddr4(p)

void
dhcpc_configured(const struct dhcpc_state *s)
{
  if(dhcp_is_conf)
    printf("dhcp reconfigure %d.%d.%d.%d\n", ip2quad(&s->ipaddr));
  else {
    printf("dhcpc_configured %d.%d.%d.%d\n", ip2quad(&s->ipaddr));
    leds_off(LEDS_GREEN);
    dhcp_is_conf = 1;
    process_start(&uaodv_process, NULL);
  }

  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);

  uip_ipaddr_copy(  &slipif.ipaddr,  &s->ipaddr);
  /* Constant slipif netmask 255.255.255.255 */
  uip_ipaddr_copy(&cc2420if.ipaddr,  &s->ipaddr);
  uip_ipaddr_copy(&cc2420if.netmask, &s->netmask);
  /* resolv_conf(s->dnsaddr); */

  /*
   * Now we also have a new short MAC address!
   */
  cc2420_set_chan_pan_addr(RF_CHANNEL, panId, uip_hostaddr.u16[1], ds2411_id);

  uip_fw_init();
  tcpip_set_forwarding(1);
  if(USB_PLUGGED())
    uip_fw_default(&slipif);
  else
    uip_fw_default(&cc2420if);
}

void
dhcpc_unconfigured(const struct dhcpc_state *s)
{
  printf("dhcpc_unconfigured\n");
  leds_on(LEDS_GREEN);
  dhcp_is_conf = 0;
  process_exit(&uaodv_process);

  uip_ipaddr(&uip_hostaddr, 0,0,0,0);
  uip_ipaddr(&uip_netmask, 0,0,0,0);
  uip_ipaddr(&uip_draddr, 0,0,0,0);

  /* New short MAC address. */
  cc2420_set_chan_pan_addr(RF_CHANNEL, panId, uip_hostaddr.u16[1], ds2411_id);

  uip_fw_init();
  tcpip_set_forwarding(1);
  if(USB_PLUGGED())
    uip_fw_default(&slipif);
  else
    uip_fw_default(&cc2420if);
}

PROCESS_THREAD(usb_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("usb_process starting\n");

  while(1) {
    /* Assume SLIP works. */
    uip_fw_init();
    tcpip_set_forwarding(1);
    if(dhcp_is_conf)
      uip_fw_default(&slipif);
    else {
      uip_fw_register(&slipif);	/* Broadcast on both. */
      uip_fw_register(&cc2420if);
    }
    while(slip_active && USB_PLUGGED()) {
      slip_active = 0;
      
      leds_toggle(LEDS_BLUE);
      etimer_set(&etimer, CLOCK_SECOND*1);
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));

      leds_toggle(LEDS_BLUE);
      etimer_set(&etimer, CLOCK_SECOND*2);
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    }

    /* Assume SLIP doesn't work. */
    uip_fw_init();
    tcpip_set_forwarding(1);
    if(dhcp_is_conf)
      uip_fw_default(&cc2420if);
    else {
      uip_fw_register(&slipif);	/* Broadcast on both. */
      uip_fw_register(&cc2420if);
    }
    while(!(slip_active && USB_PLUGGED())) {
      etimer_set(&etimer, CLOCK_SECOND);
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    }
  }

 exit:
  printf("usb_process exiting\n");
  PROCESS_END();
}
