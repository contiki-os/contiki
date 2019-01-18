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
 * @(#)$Id: contiki-sky-main.c,v 1.2 2007/11/15 13:08:38 nifi Exp $
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <io.h>

#include "contiki.h"

#include "dev/button-sensor.h"
#include "dev/ds2411.h"
#include "dev/sht11.h"
#include "dev/leds.h"
#include "dev/light.h"
#include "dev/xmem.h"
#include "dev/simple-cc2420.h"

#include "dev/slip.h"
#include "dev/uart1.h"

#include "net/mac/xmac.h"
#include "net/mac/nullmac.h"

#include "node-id.h"

#include "net/rime.h"

#include "sys/autostart.h"

/*#include "codeprop/codeprop.h"*/

SENSORS(&button_sensor);

extern int lpm_en;

#define WITH_UIP 0

#if WITH_UIP
static struct uip_fw_netif slipif =
{UIP_FW_NETIF(192,168,1,2, 255,255,255,255, slip_send)};
#endif /* WITH_UIP */

/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
void uip_log(char *msg) { puts(msg); }
/*---------------------------------------------------------------------------*/
/* Radio stuff in network byte order. */
static u16_t panId = 0x2024;

#ifndef RF_CHANNEL
#error define RF_CHANNEL!
#endif

/*---------------------------------------------------------------------------*/
void
force_inclusion(int d1, int d2)
{
  snprintf(NULL, 0, "%d", d1 % d2);
}
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  rimeaddr_t addr;
  addr.u16[0] = node_id;
  rimeaddr_set_node_addr(&addr);
}
/*---------------------------------------------------------------------------*/
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
  
#if WITH_UIP
  slip_arch_init(BAUD2UBR(115200)); /* Must come before first printf */
#else /* WITH_UIP */
  uart1_init(BAUD2UBR(115200)); /* Must come before first printf */
#endif /* WITH_UIP */
  
  printf("Starting %s "
	 "($Id: contiki-sky-main.c,v 1.2 2007/11/15 13:08:38 nifi Exp $)\n", __FILE__);
  ds2411_init();
  sensors_light_init();
  sht11_init();
  xmem_init();
  leds_toggle(LEDS_RED | LEDS_GREEN | LEDS_BLUE);

  rtimer_init();
  /*
   * Hardware initialization done!
   */

  /* Restore node id if such has been stored in external mem */
//  node_id_burn(3);
  node_id_restore();
  printf("node_id : %hu\n", node_id);

  printf("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	 ds2411_id[0], ds2411_id[1], ds2411_id[2], ds2411_id[3],
	 ds2411_id[4], ds2411_id[5], ds2411_id[6], ds2411_id[7]);

#if WITH_UIP
  uip_init();
  uip_sethostaddr(&slipif.ipaddr);
  uip_setnetmask(&slipif.netmask);
  uip_fw_default(&slipif);	/* Point2point, no default router. */
  tcpip_set_forwarding(0);
#endif /* WITH_UIP */

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&sensors_process, NULL);
  ctimer_init();

  set_rime_addr();

  simple_cc2420_init();
  simple_cc2420_set_chan_pan_addr(RF_CHANNEL, panId, 0 /*XXX*/, ds2411_id);
  simple_cc2420_set_txpower(31);
  nullmac_init(&simple_cc2420_driver);
  rime_init(&nullmac_driver);
//  xmac_init(&simple_cc2420_driver);
//  rime_init(&xmac_driver);

  /*  rimeaddr_set_node_addr*/
#if WITH_UIP
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);
#endif /* WITH_UIP */

  button_sensor.activate();
  
  printf("Autostarting processes\n");
  autostart_start((struct process **) autostart_processes);

  energest_init();
  
  /*
   * This is the scheduler loop.
   */
  printf("process_run()...\n");
  ENERGEST_ON(ENERGEST_TYPE_CPU);
  while (1) {
    do {
      /* Reset watchdog. */
    } while(process_run() > 0);

    /*
     * Idle processing.
     */
    if(lpm_en) {
    int s = splhigh();		/* Disable interrupts. */
    if(process_nevents() != 0) {
      splx(s);			/* Re-enable interrupts. */
    } else {
    	
      static unsigned long irq_energest = 0;
      /* Re-enable interrupts and go to sleep atomically. */
      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);
      /* We only want to measure the processing done in IRQs when we
	 are asleep, so we discard the processing time done when we
	 were awake. */
      energest_type_set(ENERGEST_TYPE_IRQ, irq_energest);
      _BIS_SR(GIE | SCG0 | /*SCG1 |*/ CPUOFF); /* LPM3 sleep. */
      /* We get the current processing time for interrupts that was
	 done during the LPM and store it for next time around.  */
      dint();
      irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
      eint();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
    } 
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
