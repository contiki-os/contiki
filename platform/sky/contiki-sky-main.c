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
 * @(#)$Id: contiki-sky-main.c,v 1.40 2008/11/09 12:22:04 adamdunkels Exp $
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
#include "dev/battery-sensor.h"
#include "dev/serial.h"
#include "dev/sht11.h"
#include "dev/cc2420.h"
#include "dev/slip.h"
#include "dev/uart1.h"
#include "dev/watchdog.h"
#include "dev/xmem.h"

#include "net/mac/nullmac.h"
#include "net/mac/xmac.h"

#include "net/rime.h"

#include "node-id.h"
#include "cfs-coffee-arch.h"
#include "cfs/cfs-coffee.h"
#include "sys/autostart.h"
#include "sys/profile.h"


SENSORS(&button_sensor);

#ifndef WITH_UIP
#define WITH_UIP 0
#endif

#if WITH_UIP6
#include "net/sicslowpan.h"
#include "net/uip-netif.h"
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

#endif /* WITH_UIP */

#define UIP_OVER_MESH_CHANNEL 8
static uint8_t is_gateway;

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
#if UIP_CONF_IPV6
  memcpy(addr.u8, ds2411_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(rimeaddr_t); ++i) {
      addr.u8[i] = ds2411_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  rimeaddr_set_node_addr(&addr);
  printf("Rime started with address ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%d.", addr.u8[i]);
  }
  printf("%d\n", addr.u8[i]);
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
#if WITH_UIP
static void
set_gateway(void)
{
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf("%d.%d: making myself the IP network gateway.\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    uip_over_mesh_set_gateway(&rimeaddr_node_addr);
    uip_over_mesh_make_announced_gateway();
    is_gateway = 1;
  }
}
#endif /* WITH_UIP */
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
  leds_on(LEDS_RED);
  
  uart1_init(BAUD2UBR(115200)); /* Must come before first printf */
#if WITH_UIP
  slip_arch_init(BAUD2UBR(115200));
#endif /* WITH_UIP */
  
  leds_on(LEDS_GREEN);
  ds2411_init();

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
  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);
  process_start(&sensors_process, NULL);
  
  /*
   * Initialize light and humitity/temp sensors.
   */
  sensors_light_init();
  battery_sensor.activate();
  sht11_init();
  
  ctimer_init();

  cc2420_init();
  cc2420_set_pan_addr(panId, 0 /*XXX*/, ds2411_id);
  cc2420_set_channel(RF_CHANNEL);
#if WITH_NULLMAC
  rime_init(nullmac_init(&cc2420_driver));
#else
  rime_init(xmac_init(&cc2420_driver));
#endif

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
  
#if WITH_UIP6
  memcpy(&uip_lladdr.addr, ds2411_id, sizeof(uip_lladdr.addr));
  sicslowpan_init(rime_mac);
  process_start(&tcpip_process, NULL);
#endif /* WITH_UIP6 */

#if !WITH_UIP && !WITH_UIP6
  uart1_set_input(serial_input_byte);
  serial_init();
#endif
  
#if PROFILE_CONF_ON
  profile_init();
#endif /* PROFILE_CONF_ON */

  leds_off(LEDS_GREEN);

  timesynch_init();
  timesynch_set_authority_level(rimeaddr_node_addr.u8[0]);

#if WITH_UIP
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);
  
  slip_set_input_callback(set_gateway);

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
    printf("uIP started with IP address %d.%d.%d.%d\n",
	   uip_ipaddr_to_quad(&hostaddr));
  }
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
  /*  watchdog_stop();*/
  while(1) {
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
      _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF); /* LPM3 sleep. This
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
