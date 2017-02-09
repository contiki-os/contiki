/*
* Copyright (c) 2015 NXP B.V.
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
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/

#include "contiki-conf.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "rpl-tools.h"
#include "node-id.h"
#include "waveform.h"
#include "leds.h"
#include "net/ip/uiplib.h"
#include "net/ip/uip-udp-packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define INTERVAL    (10)
#define BLINK_TIME  (CLOCK_SECOND/4)

typedef enum {
  WAVEFORM_SIN = 0,
  WAVEFORM_TRIANGLE = 1,
  WAVEFORM_POS_SAWTOOTH = 2,
  WAVEFORM_NEG_SAWTOOTH = 3,
  NUMBER_OF_WAVEFORMS = 4
} waveform_t;

typedef struct {
  const int8 * table;
  char * str;
} wave_t;

static const wave_t waveform_table[] = {  {sin_table,           "SINE"},            /* WAVEFORM_SIN */ 
                                          {triangle_table,      "TRIANGLE"},        /* WAVEFORM_TRIANGLE */
                                          {pos_sawtooth_table,  "POS-SAWTOOTH"},    /* WAVEFORM_POS_SAWTOOTH */
                                          {neg_sawtooth_table,  "NEG_SAWTOOTH"}};   /* WAVEFORM_NEG_SAWTOOTH */
                                        
static int total_time = 0;
static int selected_waveform = 0;

static void udp_rx_handler(void);
static void my_sprintf(char * udp_buf, int8_t value);

static struct uip_udp_conn *udp_conn_rx;
static struct uip_udp_conn *udp_conn_tx;
static uip_ip6addr_t ip6addr_host;
static int host_found = 0;
static char udp_buf[120];
static char *post_mssg = "Trigger";

/*******************************************************************************/
/* Local functions */
/*******************************************************************************/
static void 
udp_rx_handler(void)
{
  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = '\0';
    printf("UDP data from host: %s\n", (char *)uip_appdata);
    if (!host_found) {
      /* Create socket to talk back to host */
      uip_ipaddr_copy(&ip6addr_host, &UIP_IP_BUF->srcipaddr);
      udp_conn_tx = udp_new(&ip6addr_host, UIP_HTONS(8186), NULL);
      host_found = 1;
    }
  }
}

static void
my_sprintf(char * udp_buf, int8_t value)
{
  /* Fill the buffer with 4 ASCII chars */
  if (value < 0) {
    *udp_buf++ = '-';
  } else {
    *udp_buf++ = '+';
  } 
  value = abs(value);
  *udp_buf++ = value/100 + '0';
  value %= 100;
  *udp_buf++ = value/10 + '0';
  value %= 10;
  *udp_buf++ = value + '0';
  *udp_buf = 0;
}

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node");
PROCESS(led_process, "LED");
AUTOSTART_PROCESSES(&node_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  static int sample_count = 0;
  static struct etimer et;
  extern unsigned char node_mac[8];
 
  leds_init();
  
  /* 3 possible roles:
   * - role_6ln: simple node, will join any network, secured or not
   * - role_6dr: DAG root, will advertise (unsecured) beacons
   * - role_6dr_sec: DAG root, will advertise secured beacons
   * */
  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;

  /* Set node with ID == 1 as coordinator, handy in Cooja. */
  if(node_id == 1) {
    if(LLSEC802154_ENABLED) {
      node_role = role_6dr_sec;
    } else {
      node_role = role_6dr;
    }
  } else {
    node_role = role_6ln;
  }

  printf("Init: node starting with role %s\n",
      node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

#if WITH_TSCH
  tsch_set_pan_secured(LLSEC802154_ENABLED && (node_role == role_6dr_sec));
#endif /* WITH_TSCH */
  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xbbbb, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  }
  
  /* Selected waveform depends on LS byte of MAC  */
  selected_waveform = node_mac[7] % NUMBER_OF_WAVEFORMS;
  printf("LS-Byte=0x%x; waveform=%d\n", node_mac[7], selected_waveform);

  process_start(&led_process, NULL);

  /* Listen to any host on 8185 */
  udp_conn_rx = udp_new(NULL, 0, NULL);
  udp_bind(udp_conn_rx, UIP_HTONS(8185));
    
  /* Wait for timer event 
     On timer event, handle next sample */
  etimer_set(&et, INTERVAL*CLOCK_SECOND);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) || (ev == tcpip_event));
    if (ev == tcpip_event) {
      udp_rx_handler();
    }
    if (etimer_expired(&et) ) { 
      /* Restart timer */
      total_time += INTERVAL;
      if (host_found) {
        /* Make sample count dependent on asn. After a disconnect, waveforms remain
           synchronous. Use node_mac to create phase offset between waveforms in different nodes */
        sample_count = ((tsch_current_asn.ls4b/((1000/(TSCH_CONF_DEFAULT_TIMESLOT_LENGTH/1000)))/INTERVAL)+node_mac[7]) % (SIZE_OF_WAVEFORM-1);
        printf("%d sec. waveform=%s. cnt=%d. value=%d\n", total_time, waveform_table[selected_waveform].str, sample_count, waveform_table[selected_waveform].table[sample_count]);
        my_sprintf(udp_buf, waveform_table[selected_waveform].table[sample_count]);
        uip_udp_packet_send(udp_conn_tx, udp_buf, strlen(udp_buf));
        /* Switch LED on and start blink timer (callback timer) */
        process_post(&led_process, PROCESS_EVENT_CONTINUE, post_mssg);
      } else {
        printf("No host\n");
      }
      etimer_restart(&et); 
      if (total_time%60 == 0) {
        /* Print network status once per minute */
        print_network_status();
      }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(led_process, ev, data)
{
  PROCESS_BEGIN();

  /* Switch on leds for 0.25msec after event posted. */
  static struct etimer led_et;

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_CONTINUE)  && (!strcmp(data, post_mssg)));
    leds_on(LEDS_RED);
    etimer_set(&led_et, CLOCK_SECOND/4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_et));
    etimer_stop(&led_et);
    leds_off(LEDS_RED);
  } 
  PROCESS_END();
}

