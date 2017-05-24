#include "net/netstack.h"
#include "contiki-conf.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "net/ipv6/uip-ds6-route.h"

// Adding this library to edit the number of maximum routes in a node
#include "net/ipv6/uip-ds6.h"
// End of Adding this library to edit the number of maximum routes in a node

#include "net/mac/tsch/tsch.h"
#include "net/rpl/rpl.h"

#include "node-id.h"
#include "leds.h"
#include "net/ip/uiplib.h"
#include "net/ip/uip-udp-packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
///////////////////////////////////////////////

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

//#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
//#if CONFIG_VIA_BUTTON
//#include "button-sensor.h"
//#endif /* CONFIG_VIA_BUTTON */


////////////////////////////////////////////////////
// #if PLATFORM_HAS_BUTTON
// #include "dev/button-sensor.h"
// #endif

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
// extern resource_t
//   res_hello,
//   res_mirror,
//   res_chunks,
//   res_separate,
//   res_push,
//   res_event,
//   res_sub,
//   res_b1_sep_b2;
// #if PLATFORM_HAS_LEDS
// extern resource_t res_leds, res_toggle;
// #endif
// #if PLATFORM_HAS_LIGHT
// #include "dev/light-sensor.h"
// extern resource_t res_light;
// #endif

              /* Start of comment: stop add resource file info to save ROM */

              // #if PLATFORM_HAS_BATTERY
              // #include "dev/battery-sensor.h"
              // extern resource_t res_battery;
              // #endif
              // #if PLATFORM_HAS_TEMPERATURE
              // #include "dev/temperature-sensor.h"
              // extern resource_t res_temperature;
              // #endif

              /* End of comment */

/*
extern resource_t res_battery;
#endif
#if PLATFORM_HAS_RADIO
#include "dev/radio-sensor.h"
extern resource_t res_radio;
#endif
#if PLATFORM_HAS_SHT11
#include "dev/sht11/sht11-sensor.h"
extern resource_t res_sht11;
#endif
*/
///////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
// #if CONFIG_VIA_BUTTON
// AUTOSTART_PROCESSES(&node_process, &sensors_process);
// #else /* CONFIG_VIA_BUTTON */
AUTOSTART_PROCESSES(&node_process);
//#endif /* CONFIG_VIA_BUTTON */

/*---------------------------------------------------------------------------*/
static void
print_network_status(void)
{
  int i;
  uint8_t state;
  uip_ds6_defrt_t *default_route;
  uip_ds6_route_t *route;

  PRINTA("--- Network status ---\n");
  
  /* Our IPv6 addresses */
  PRINTA("- Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA("-- ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
  
  /* Our default route */
  PRINTA("- Default route:\n");
  default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  if(default_route != NULL) {
    PRINTA("-- ");
    uip_debug_ipaddr_print(&default_route->ipaddr);;
    PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
  } else {
    PRINTA("-- None\n");
  }

  /* Our routing entries */
  PRINTA("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
  route = uip_ds6_route_head();
  while(route != NULL) {
    PRINTA("-- ");
    uip_debug_ipaddr_print(&route->ipaddr);
    PRINTA(" via ");
    uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
    PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
    route = uip_ds6_route_next(route); 
  }
  
  PRINTA("----------------------\n");
}
/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  
  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;
  
 node_role = role_6ln;
  
  /* Set node with ID == 1 as coordinator, convenient in Cooja. */


  if(node_id == 1) {
    if(LLSEC802154_ENABLED) {
      node_role = role_6dr_sec;
    } else {
      node_role = role_6dr;
    }
  } else {
    node_role = role_6ln;
  }

  // printf("Init: node starting with role %s\n",
  //     node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

  tsch_set_pan_secured(LLSEC802154_ENABLED && (node_role == role_6dr_sec));
  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    net_init(&prefix);
  } else {
    net_init(NULL);
  }

///////////////////////////////////////////////////////////
 rest_init_engine();
 // rest_activate_resource(&res_hello, "test/hello");
 // rest_activate_resource(&res_push, "test/push");
  //Activate more resources below
// #if PLATFORM_HAS_LEDS
//   rest_activate_resource(&res_leds, "actuators/leds"); 
//   rest_activate_resource(&res_toggle, "actuators/toggle");
// #endif
// #if PLATFORM_HAS_LIGHT
//   rest_activate_resource(&res_light, "sensors/light"); 
//   SENSORS_ACTIVATE(light_sensor);  
// #endif

  // End of activate resources
///////////////////////////////////////////////////////////

  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 60);
  while(1) {
    print_network_status();
///////////////////////////////////////////
// if(ev == sensors_event) {
//       PRINTF("*******BUTTON*******\n");

//       // Call the event_handler for this application-specific event. 
//       res_event.trigger();

//       /* Also call the separate response example handler. */
//       res_separate.resume();
//     }
//////////////////////////////////////

    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
