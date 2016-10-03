/*
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
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
/* #include "dev/leds.h" */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
   #define DEBUG DEBUG_PRINT
   #include "net/ip/uip-debug.h"
 */
/**************************************************************************/
/* add mock-up code for DC-DC converter */
/* #include "control.h" */
/* #include "vdc.h" */
#ifdef CO2
#include "dev/co2_sa_kxx-sensor.h"
#endif
#include "dev/dc-status-sensor.h"
#include "dev/dc-vdc-sensor.h"
#include "dev/dc-hw-sensor.h"

/* from er-rest-example/er-rexample-server.c */
#include "rest-engine.h"
/* #include "er-coap.h" */

#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif

/*
   #if PLATFORM_HAS_BUTTON
   #include "dev/button-sensor.h"
   #endif
 */
#define DEBUG 1
#if DEBUG
/* #include <stdio.h> */
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/* #define TOGGLE_INTERVAL 10 */
/* #define OBSERVE_INTERVAL 10 */

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
/*  res_hello, */
/*  res_push, */
/*  res_event, */
/*  res_sub; */
/*  res_dc_status_obs; */
/*  res_dc_status, */
#ifdef CO2
  res_dc_co2,
#endif
  res_dc_status_obs,
  res_dc_vdc,
  res_dc_hwcfg;
/*
   #if PLATFORM_HAS_LEDS
   extern resource_t
   //  res_leds,
   res_toggle;
   #endif
 */
/* #if PLATFORM_HAS_LIGHT */
/* #include "dev/light-sensor.h" */
/* extern resource_t res_light; */
/* #endif */

/**************************************************************************/

PROCESS(coap_server_process, "CoAP server process");
AUTOSTART_PROCESSES(&coap_server_process);
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

/*  SENSORS_ACTIVATE(button_sensor); */
#ifdef CO2
  SENSORS_ACTIVATE(co2_sa_kxx_sensor);
#endif
  SENSORS_ACTIVATE(dc_status_sensor);
  SENSORS_ACTIVATE(dc_vdc_sensor);
  SENSORS_ACTIVATE(dc_hw_sensor);

/**************************************************************************/

#if PLATFORM_HAS_LEDS
/* initialize leds */
  leds_init();
  leds_on(LEDS_RED);
  leds_on(LEDS_YELLOW);
#endif

/* Initilize DC-DC converter parameters */
/*  ValueInit(); */
/*  VDCInit(); */
/*
   #ifdef PARAMS_CHANNEL
   PRINTF("PARAMs channel: %u\n", PARAMS_CHANNEL);
   #endif
 */
#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();

  /*
   * Bind the resources to their Uri-Path.
   * WARNING: Activating twice only means alternate path, not two instances!
   * All static variables are the same for each URI path.
   */
/*  rest_activate_resource(&res_hello, "test/hello"); */
/*  rest_activate_resource(&res_mirror, "debug/mirror"); */
/*  rest_activate_resource(&res_chunks, "test/chunks"); */
/*  rest_activate_resource(&res_separate, "test/separate"); */
/*  rest_activate_resource(&res_push, "test/push"); */
/*  rest_activate_resource(&res_event, "sensors/button"); */
/*  rest_activate_resource(&res_sub, "test/sub"); */
/*  rest_activate_resource(&res_b1_sep_b2, "test/b1sepb2"); */
#ifdef CO2
  rest_activate_resource(&res_dc_co2, "dcdc/co2");
#endif
  rest_activate_resource(&res_dc_status_obs, "dcdc/status");
/*  rest_activate_resource(&res_dc_status, "dcdc/status"); */
  rest_activate_resource(&res_dc_vdc, "dcdc/vdc");
  rest_activate_resource(&res_dc_hwcfg, "dcdc/hwcfg");
/* #if PLATFORM_HAS_LEDS */
/* / *  rest_activate_resource(&res_leds, "actuators/leds"); * / */
/*  rest_activate_resource(&res_toggle, "actuators/toggle"); */
/* #endif */
/* #if PLATFORM_HAS_LIGHT */
/*  rest_activate_resource(&res_light, "sensors/light"); */
/*  SENSORS_ACTIVATE(light_sensor); */
/* #endif */

/**************************************************************************/

  PRINTF("CoAP server started\n");

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoWPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the
 * 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::. At present Wireshark copies Context/128 and
 * then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit
 * compressed address of aaaa::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct
 * uncompressed addresses.
 */

#if 0
/* Mode 1 - 64 bits inline */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE, (uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */

  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_MAC.off(1);

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
