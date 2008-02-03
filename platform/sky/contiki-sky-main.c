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
 * @(#)$Id: contiki-sky-main.c,v 1.26 2008/02/03 21:03:19 adamdunkels Exp $
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <io.h>

#include "contiki.h"

#include "dev/button-sensor.h"
#include "dev/ds2411.h"
#include "dev/leds.h"
#include "dev/light.h"
#include "dev/serial.h"
#include "dev/sht11.h"
#include "dev/simple-cc2420.h"
#include "dev/slip.h"
#include "dev/uart1.h"
#include "dev/watchdog.h"
#include "dev/xmem.h"

#include "net/mac/nullmac.h"
#include "net/mac/xmac.h"

#include "net/rime.h"

#include "node-id.h"
#include "sys/autostart.h"
#include "sys/profile.h"


SENSORS(&button_sensor);

#ifndef WITH_UIP
#define WITH_UIP 0
#endif

#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-over-mesh.h"
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(192,168,1,2, 255,255,255,255, slip_send)};
static struct uip_fw_netif meshif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_over_mesh_send)};

#define UIP_OVER_MESH_CHANNEL 1

#endif /* WITH_UIP */

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

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
#define RF_CHANNEL              26
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
  int i;
  memset(&addr, 0, sizeof(rimeaddr_t));
  if(node_id == 0) {
    for(i = 0; i < sizeof(rimeaddr_t); ++i) {
      addr.u8[i] = ds2411_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
  rimeaddr_set_node_addr(&addr);
}
/*---------------------------------------------------------------------------*/
static void
print_processes(struct process * const processes[])
{
  /*  const struct process * const * p = processes;*/
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  putchar('\n');
}
/*--------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  /*
   * Initalize hardware.
   */
  msp430_cpu_init();
  clock_init();
  leds_init();
  leds_on(LEDS_RED);

#if WITH_UIP
  uart1_init(BAUD2UBR(57600)); /* Must come before first printf */
  slip_arch_init(BAUD2UBR(57600));
#else
  uart1_init(BAUD2UBR(115200)); /* Must come before first printf */
#endif /* WITH_UIP */
  
  leds_on(LEDS_GREEN);
  ds2411_init();
  sensors_light_init();
  sht11_init();
  leds_on(LEDS_BLUE);
  xmem_init();

  leds_off(LEDS_RED);
  rtimer_init();
  /*
   * Hardware initialization done!
   */

  /* Restore node id if such has been stored in external mem */
  node_id_restore();

  leds_off(LEDS_BLUE);
  printf(CONTIKI_VERSION_STRING " started. ");
  if(node_id > 0) {
    printf("Node id is set to %u.\n", node_id);
  } else {
    printf("Node id is not set.\n");
  }
  set_rime_addr();
  printf("MAC %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	 ds2411_id[0], ds2411_id[1], ds2411_id[2], ds2411_id[3],
	 ds2411_id[4], ds2411_id[5], ds2411_id[6], ds2411_id[7]);

#if WITH_UIP
  {
    uip_ipaddr_t hostaddr, netmask;
    
    uip_init();

    uip_ipaddr(&hostaddr, 172,16,
	       rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    uip_ipaddr(&netmask, 255,255,0,0);
    uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);
    
    uip_sethostaddr(&hostaddr);
    uip_setnetmask(&netmask);
    uip_over_mesh_set_net(&hostaddr, &netmask);
    /*    uip_fw_register(&slipif);*/
    uip_over_mesh_set_gateway_netif(&slipif);
    uip_fw_default(&meshif);
    uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
  }
#endif /* WITH_UIP */

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&sensors_process, NULL);

#if !WITH_UIP
  uart1_set_input(serial_input_byte);
  serial_init();
#endif
  
#if PROFILE_CONF_ON
  profile_init();
#endif /* PROFILE_CONF_ON */
  ctimer_init();

  leds_off(LEDS_GREEN);
  

  simple_cc2420_init();
  simple_cc2420_set_pan_addr(panId, 0 /*XXX*/, ds2411_id);
  simple_cc2420_set_channel(RF_CHANNEL);
#if WITH_NULLMAC
  rime_init(nullmac_init(&simple_cc2420_driver));
#else
  rime_init(xmac_init(&simple_cc2420_driver));
#endif

  timesynch_init();


  timesynch_set_authority_level(rimeaddr_node_addr.u8[0]);

#if WITH_UIP
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);
#endif /* WITH_UIP */

  button_sensor.activate();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);
  
  print_processes(autostart_processes);
  autostart_start(autostart_processes);
  
  /*
   * This is the scheduler loop.
   */
  watchdog_start();
  while (1) {
    int r;
#if PROFILE_CONF_ON
    profile_episode_start();
#endif /* PROFILE_CONF_ON */
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);
#if PROFILE_CONF_ON
    profile_episode_end();
#endif /* PROFILE_CONF_ON */
    
    /*
     * Idle processing.
     */
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
      watchdog_stop();
      _BIS_SR(GIE | SCG0 | /*SCG1 | */CPUOFF); /* LPM3 sleep. This
					      statement will block
					      until the CPU is
					      woken up by an
					      interrupt that sets
					      the wake up flag. */

      
      /* We get the current processing time for interrupts that was
	 done during the LPM and store it for next time around.  */
      dint();
      irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
      eint();
      watchdog_start();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
