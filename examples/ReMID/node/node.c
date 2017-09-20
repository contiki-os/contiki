/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */


#include "contiki.h"
#include "net/rpl/rpl.h"
#include "tools/rpl-tools.h"
#include "rest-engine.h"
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/temperature-sensor.h"
#include "dev/light-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "dev/battery-sensor.h"

#ifndef RES_RPL_UPDATE_INTERVAL
#define RES_RPL_UPDATE_INTERVAL (30 * CLOCK_SECOND)
#endif

extern resource_t resource_rpl_dag;
extern resource_t res_leds, res_toggle;
extern resource_t res_light;
extern resource_t res_temperature;
extern resource_t res_humidity;
extern resource_t res_battery;

static struct ctimer rpl_changed_timer;

static void
res_rpl_changed_handler(void *ptr)
{
  REST.notify_subscribers(&resource_rpl_dag);
}


void
rpl_changed_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr, int num_routes)
{
  /* We have added or removed a routing entry, notify subscribers */
  if(event == UIP_DS6_NOTIFICATION_ROUTE_ADD || event == UIP_DS6_NOTIFICATION_ROUTE_RM) {
    ctimer_set(&rpl_changed_timer, RES_RPL_UPDATE_INTERVAL, res_rpl_changed_handler, NULL);
  }
}


/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");

AUTOSTART_PROCESSES(&node_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  printf("node started\n");
  PROCESS_BEGIN();

  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr } node_role;
  node_role = role_6ln;

  printf("Init: node starting with role %s\n",
         node_role == role_6ln ? "6ln" : "6dr");

  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  }

  rest_init_engine();
  rest_activate_resource(&resource_rpl_dag, "rpl/RPLInfo");
  static struct uip_ds6_notification n;
  uip_ds6_notification_add(&n, rpl_changed_callback);  
  rest_activate_resource(&res_toggle, "actuators/Toggle");
  rest_activate_resource(&res_light, "sensors/Light"); 
  SENSORS_ACTIVATE(light_sensor);
  rest_activate_resource(&res_temperature, "sensors/Temperature");  
  SENSORS_ACTIVATE(temperature_sensor);
  rest_activate_resource(&res_humidity, "sensors/Humidity");  
  SENSORS_ACTIVATE(sht11_sensor);   
  rest_activate_resource(&res_battery, "sensors/Battery");  
  SENSORS_ACTIVATE(battery_sensor); 
  

  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 60);
  while(1) {
    print_network_status();
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
