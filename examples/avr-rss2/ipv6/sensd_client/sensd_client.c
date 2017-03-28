/*
 * Copyright (c) 2015, Copyright  Robert Olsson / Radio Sensors AB  
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
 * This file is part of the Contiki operating system.
 *
 *
 * Author  : Robert Olsson robert@radio-sensors.com
 * Created : 2015-12-28
 */

/**
 * \file
 *         A simple application reporting sensor data. Can be sensd
 */

#include <string.h>
#include "contiki-net.h"
#include "leds.h"
#include "dev/leds.h"
#include "dev/battery-sensor.h"
#include "dev/temp-sensor.h"
#include "dev/temp_mcu-sensor.h"
#include "dev/light-sensor.h"
#include "dev/pulse-sensor.h"
#ifdef CO2
#include "dev/co2_sa_kxx-sensor.h"
#endif

#ifndef PERIOD
#define PERIOD 10
#endif

#define REPORT_INTERVAL	 (PERIOD * CLOCK_SECOND)

#define DEBUG DEBUG_PRINT
#include "uip-debug.h"

#define PORT 1234
#define MAX_PAYLOAD_LEN 80

static struct psock ps;
static uint8_t out_buf[MAX_PAYLOAD_LEN];
static char  in_buffer[MAX_PAYLOAD_LEN];
static int i;
static int send_now;
static clock_time_t report_interval = REPORT_INTERVAL;
static uint8_t state;
static uip_ipaddr_t addr;
static struct timer t, t1;
static struct etimer et, et1;

PROCESS(sensd_client_process, "sensd TCP client process");
PROCESS(report_timer_process, "report pacemaker");
AUTOSTART_PROCESSES
(&sensd_client_process,&report_timer_process);

static int
do_report(void)
{
  static int seq_id;
  int len = 0;
  
  seq_id++;
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "&: ");
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "TXT=ERICSON-6LOWPAN ");
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "TARGET=avr-rss2 ");
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "V_MCU=%-d ", battery_sensor.value(0));
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "T_MCU=%-d ", temp_mcu_sensor.value(0));
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "LIGHT=%-d ", light_sensor.value(0));
#ifdef CO2
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "CO2=%-d ", co2_sa_kxx_sensor.value(CO2_SA_KXX_CO2));
#endif
  len += snprintf((char *) &out_buf[len], sizeof(out_buf), "\n\r");

  return len;
}

static void
print_local_addresses(void)
{
  PRINTF("Server IPv6 addresses:\n\r");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n\r");
    }
  }
}

static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xfd00, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}

static int handle_connection(struct psock *p, long ts) {
  PSOCK_BEGIN(p);
  if(send_now) {
    do_report();
    leds_on(LEDS_YELLOW);
    PSOCK_SEND(p, (const uint8_t *)  out_buf, strlen((const char *) out_buf));
    send_now = 0;
  }
  PSOCK_END(p);
}

PROCESS_THREAD(report_timer_process, ev, data) {
  PROCESS_BEGIN();
  while(1) {
    timer_set(&t1, report_interval);
    etimer_set(&et1, timer_remaining(&t1));	
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));	
    send_now = 1;
    leds_on(LEDS_RED);
  }
  PROCESS_END();
}

PROCESS_THREAD(sensd_client_process, ev, data) {
  PROCESS_BEGIN();

  set_global_address();
  print_local_addresses();

  SENSORS_ACTIVATE(temp_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(temp_mcu_sensor);
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(pulse_sensor);
#ifdef CO2
  SENSORS_ACTIVATE(co2_sa_kxx_sensor);
#endif
  leds_init(); 
  leds_on(LEDS_RED);
  leds_on(LEDS_YELLOW);

  printf("Starting TCP client on port=%d\n", PORT);

  /* Set server address. typically sensd */
  //uip_ip6addr(&addr, 0xfe80, 0, 0, 0, 0xfec2, 0x3d00, 1, 0x63ae);
  //uip_ip6addr(&addr, 0x0000, 0, 0, 0, 0, 0xffff, 0xc0a8, 0x0201);
  //uip_ip6addr(&addr, 0x0000, 0, 0, 0, 0, 0xffff, 0xc0a8, 0x0255);
  uip_ip6addr(&addr, 0x0000, 0, 0, 0, 0, 0xffff, 0xc010, 0x7dea); //192.16.125.234 

  random_init(50);

  while(1) {

    /* Delay connection attempts */
    timer_set(&t, 2*CLOCK_SECOND);
    etimer_set(&et, timer_remaining(&t));		
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Time out teh actual connection attempt */
    timer_set(&t, 10*CLOCK_SECOND);
    printf("Connecting");
    tcp_connect(&addr, UIP_HTONS(PORT), NULL);
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    if(uip_aborted() || uip_timedout() || uip_closed()) {
      printf(" Failed\n");
    } else if(uip_connected()) {
      printf(" Success\n");
      PSOCK_INIT(&ps, (uint8_t *)in_buffer, sizeof(in_buffer));
      do {
	handle_connection(&ps, clock_seconds());
	PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
      } while(!(uip_closed() || uip_aborted() || uip_timedout()));
      printf("Disconnected\n");
    }
  }
  PROCESS_END();
}
