/*
 * Copyright (c) 2014, SICS Swedish ICT.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * This file is part of the Contiki OS
 *
 */

/**
 * \file
 *         Contiki main for NXP JN516X platform
 *
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dev/watchdog.h"
#include <AppHardwareApi.h>
#include <BbcAndPhyRegs.h>
#include <recal.h>
#include "dev/uart0.h"

#include "contiki.h"
#include "net/netstack.h"

#include "dev/serial-line.h"

#include "net/ip/uip.h"
#include "dev/leds.h"

#include "lib/random.h"
#include "sys/node-id.h"
#include "rtimer-arch.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "net/rime/rime.h"

#include "dev/micromac-radio.h"
#include "MMAC.h"
/* Includes depending on connected sensor boards */
#if SENSOR_BOARD_DR1175
#include "light-sensor.h"
#include "ht-sensor.h"
SENSORS(&light_sensor, &ht_sensor);
#elif SENSOR_BOARD_DR1199
#include "button-sensor.h"
#include "pot-sensor.h"
SENSORS(&pot_sensor, &button_sensor);
#else
#include "dev/button-sensor.h"
/* #include "dev/pir-sensor.h" */
/* #include "dev/vib-sensor.h" */
/* &pir_sensor, &vib_sensor */
SENSORS(&button_sensor);
#endif
unsigned char node_mac[8];

/* Symbol defined by the linker script
 * marks the end of the stack taking into account the used heap  */
extern uint32_t heap_location;

#ifndef NETSTACK_CONF_WITH_IPV4
#define NETSTACK_CONF_WITH_IPV4 0
#endif

#if NETSTACK_CONF_WITH_IPV4
#include "net/ip/uip.h"
#include "net/ipv4/uip-fw.h"
#include "net/ipv4/uip-fw-drv.h"
#include "net/ipv4/uip-over-mesh.h"
static struct uip_fw_netif slipif =
{ UIP_FW_NETIF(192, 168, 1, 2, 255, 255, 255, 255, slip_send) };
static struct uip_fw_netif meshif =
{ UIP_FW_NETIF(172, 16, 0, 0, 255, 255, 0, 0, uip_over_mesh_send) };

#define UIP_OVER_MESH_CHANNEL 8
static uint8_t is_gateway;
#endif /* NETSTACK_CONF_WITH_IPV4 */

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) do { printf(__VA_ARGS__); } while(0)
#else
#define PRINTF(...) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
/* Reads MAC from SoC
 * Must be called before node_id_restore()
 * and network addresses initialization */
static void
init_node_mac(void)
{
  tuAddr psExtAddress;
  vMMAC_GetMacAddress(&psExtAddress.sExt);
  node_mac[7] = psExtAddress.sExt.u32L;
  node_mac[6] = psExtAddress.sExt.u32L >> (uint32_t)8;
  node_mac[5] = psExtAddress.sExt.u32L >> (uint32_t)16;
  node_mac[4] = psExtAddress.sExt.u32L >> (uint32_t)24;
  node_mac[3] = psExtAddress.sExt.u32H;
  node_mac[2] = psExtAddress.sExt.u32H >> (uint32_t)8;
  node_mac[1] = psExtAddress.sExt.u32H >> (uint32_t)16;
  node_mac[0] = psExtAddress.sExt.u32H >> (uint32_t)24;
}
/*---------------------------------------------------------------------------*/
#if !PROCESS_CONF_NO_PROCESS_NAMES
static void
print_processes(struct process *const processes[])
{
  /*  const struct process * const * p = processes;*/
  PRINTF("Starting");
  while(*processes != NULL) {
    PRINTF(" '%s'", (*processes)->name);
    processes++;
  }
  putchar('\n');
}
#endif /* !PROCESS_CONF_NO_PROCESS_NAMES */
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV4
static void
set_gateway(void)
{
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf("%d.%d: making myself the IP network gateway.\n\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    printf("IPv4 address of the gateway: %d.%d.%d.%d\n\n",
           uip_ipaddr_to_quad(&uip_hostaddr));
    uip_over_mesh_set_gateway(&linkaddr_node_addr);
    uip_over_mesh_make_announced_gateway();
    is_gateway = 1;
  }
}
#endif /* NETSTACK_CONF_WITH_IPV4 */
/*---------------------------------------------------------------------------*/
static void
start_autostart_processes()
{
#if !PROCESS_CONF_NO_PROCESS_NAMES
  print_processes(autostart_processes);
#endif /* !PROCESS_CONF_NO_PROCESS_NAMES */
  autostart_start(autostart_processes);
}
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV6
static void
start_uip6(void)
{
  NETSTACK_NETWORK.init();

#ifndef WITH_SLIP_RADIO
  process_start(&tcpip_process, NULL);
#else
#if WITH_SLIP_RADIO == 0
  process_start(&tcpip_process, NULL);
#endif
#endif /* WITH_SLIP_RADIO */

#if DEBUG
  PRINTF("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      PRINTF("%02x%02x:", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
      /* make it hardcoded... */
    }
    lladdr->state = ADDR_AUTOCONF;

    PRINTF("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }
#endif /* DEBUG */

  if(!UIP_CONF_IPV6_RPL) {
    uip_ipaddr_t ipaddr;
    int i;
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
    PRINTF("Tentative global IPv6 address ");
    for(i = 0; i < 7; ++i) {
      PRINTF("%02x%02x:",
             ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }
    PRINTF("%02x%02x\n",
           ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
  }
}
#endif /* NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
static void
set_linkaddr(void)
{
  int i;
  linkaddr_t addr;
  memset(&addr, 0, LINKADDR_SIZE);
#if NETSTACK_CONF_WITH_IPV6
  memcpy(addr.u8, node_mac, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < LINKADDR_SIZE; ++i) {
      addr.u8[i] = node_mac[LINKADDR_SIZE - 1 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  linkaddr_set_node_addr(&addr);
#if DEBUG
  PRINTF("Link-layer address: ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    PRINTF("%d.", addr.u8[i]);
  }
  PRINTF("%d\n", addr.u8[i]);
#endif
}
/*---------------------------------------------------------------------------*/
#if USE_EXTERNAL_OSCILLATOR
static bool_t
init_xosc(void)
{
  /* The internal 32kHz RC oscillator is used by default;
   * Initialize and enable the external 32.768kHz crystal.
   */
  vAHI_Init32KhzXtal();
  /* wait for 1.0 seconds for the crystal to stabilize */
  clock_time_t start = clock_time();
  clock_time_t now;
  do {
    now = clock_time();
    watchdog_periodic();
  } while(now - start < CLOCK_SECOND);
  /* switch to the 32.768 kHz crystal */
  return bAHI_Set32KhzClockMode(E_AHI_XTAL);
}
#endif
/*---------------------------------------------------------------------------*/
#if WITH_TINYOS_AUTO_IDS
uint16_t TOS_NODE_ID = 0x1234; /* non-zero */
uint16_t TOS_LOCAL_ADDRESS = 0x1234; /* non-zero */
#endif /* WITH_TINYOS_AUTO_IDS */
int
main(void)
{
  /* Set stack overflow address for detecting overflow in runtime */
  vAHI_SetStackOverflow(TRUE, ((uint32_t *)&heap_location)[0]);

  clock_init();
  watchdog_init();
  leds_init();
  leds_on(LEDS_ALL);
  init_node_mac();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  node_id_restore();

#if WITH_TINYOS_AUTO_IDS
  node_id = TOS_NODE_ID;
#endif /* WITH_TINYOS_AUTO_IDS */
  /* for setting "hardcoded" IEEE 802.15.4 MAC addresses */
#ifdef IEEE_802154_MAC_ADDRESS
  {
    uint8_t ieee[] = IEEE_802154_MAC_ADDRESS;
    memcpy(node_mac, ieee, sizeof(uip_lladdr.addr));
    node_mac[7] = node_id & 0xff;
  }
#endif

  /* Initialize random with a seed from the SoC random generator.
   * This must be done before selecting the high-precision external oscillator.
   */
  vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT, E_AHI_INTS_DISABLED);
  random_init(u16AHI_ReadRandomNumber());

  process_init();
  ctimer_init();
  uart0_init(UART_BAUD_RATE); /* Must come before first PRINTF */

#if USE_EXTERNAL_OSCILLATOR
  init_xosc();
#endif

#if NETSTACK_CONF_WITH_IPV4
  slip_arch_init(UART_BAUD_RATE);
#endif /* NETSTACK_CONF_WITH_IPV4 */

  /* check for reset source */
  if(bAHI_WatchdogResetEvent()) {
    PRINTF("Init: Watchdog timer has reset device!\r\n");
  }
  process_start(&etimer_process, NULL);
  set_linkaddr();
  netstack_init();

#if NETSTACK_CONF_WITH_IPV6
#if UIP_CONF_IPV6_RPL
  PRINTF(CONTIKI_VERSION_STRING " started with IPV6, RPL\n");
#else
  PRINTF(CONTIKI_VERSION_STRING " started with IPV6\n");
#endif
#elif NETSTACK_CONF_WITH_IPV4
  PRINTF(CONTIKI_VERSION_STRING " started with IPV4\n");
#else
  PRINTF(CONTIKI_VERSION_STRING " started\n");
#endif

  if(node_id > 0) {
    PRINTF("Node id is set to %u.\n", node_id);
  } else {
    PRINTF("Node id is not set.\n");
  }
#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, node_mac, sizeof(uip_lladdr.addr));
  queuebuf_init();
#endif /* NETSTACK_CONF_WITH_IPV6 */

  PRINTF("%s %s %s\n", NETSTACK_LLSEC.name, NETSTACK_MAC.name, NETSTACK_RDC.name);

#if !NETSTACK_CONF_WITH_IPV4 && !NETSTACK_CONF_WITH_IPV6
  uart0_set_input(serial_line_input_byte);
  serial_line_init();
#endif

#if TIMESYNCH_CONF_ENABLED
  timesynch_init();
  timesynch_set_authority_level((linkaddr_node_addr.u8[0] << 4) + 16);
#endif /* TIMESYNCH_CONF_ENABLED */

#if NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL); /* Start IP output */
  process_start(&slip_process, NULL);

  slip_set_input_callback(set_gateway);

  {
    uip_ipaddr_t hostaddr, netmask;

    uip_init();

    uip_ipaddr(&hostaddr, 172, 16,
               linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    uip_ipaddr(&netmask, 255, 255, 0, 0);
    uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);

    uip_sethostaddr(&hostaddr);
    uip_setnetmask(&netmask);
    uip_over_mesh_set_net(&hostaddr, &netmask);
    /*    uip_fw_register(&slipif);*/
    uip_over_mesh_set_gateway_netif(&slipif);
    uip_fw_default(&meshif);
    uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
    PRINTF("uIP started with IP address %d.%d.%d.%d\n",
           uip_ipaddr_to_quad(&hostaddr));
  }
#endif /* NETSTACK_CONF_WITH_IPV4 */

  watchdog_start();
  NETSTACK_LLSEC.init();

#if NETSTACK_CONF_WITH_IPV6
  start_uip6();
#endif /* NETSTACK_CONF_WITH_IPV6 */
  start_autostart_processes();

  leds_off(LEDS_ALL);
  int r;
  while(1) {
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);
    /*
     * Idle processing.
     */
    watchdog_stop();

#if DCOSYNCH_CONF_ENABLED
    /* Calibrate the DCO every DCOSYNCH_PERIOD
     * if we have more than 500uSec until next rtimer
     * PS: Calibration disables interrupts and blocks for 200uSec.
     *  */
    static unsigned long last_dco_calibration_time = 0;
    if(clock_seconds() - last_dco_calibration_time > DCOSYNCH_PERIOD) {
      if(rtimer_arch_get_time_until_next_wakeup() > RTIMER_SECOND / 2000) {
        /* PRINTF("ContikiMain: Calibrating the DCO\n"); */
        eAHI_AttemptCalibration();
        /* Patch to allow CpuDoze after calibration */
        vREG_PhyWrite(REG_PHY_IS, REG_PHY_INT_VCO_CAL_MASK);
        last_dco_calibration_time = clock_seconds();
      }
    }
#endif /* DCOSYNCH_CONF_ENABLED */
    ENERGEST_OFF(ENERGEST_TYPE_CPU);
    ENERGEST_ON(ENERGEST_TYPE_LPM);
    vAHI_CpuDoze();
    watchdog_start();
    ENERGEST_OFF(ENERGEST_TYPE_LPM);
    ENERGEST_ON(ENERGEST_TYPE_CPU);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
AppColdStart(void)
{
  /* After reset or sleep with memory off */
  main();
}
/*---------------------------------------------------------------------------*/
void
AppWarmStart(void)
{
  /* Wakeup after sleep with memory on.
   * TODO: Need to initialize devices but not the application state */
  main();
}
/*---------------------------------------------------------------------------*/
