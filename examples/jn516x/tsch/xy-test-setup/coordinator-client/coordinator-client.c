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
#define XY_MEASUREMENT_CFG
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/resolv.h"
#include "net/linkaddr.h"
#include "rich.h"
#include "net/mac/tsch/tsch-schedule.h"
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "leds.h"

#include <string.h>
#include <stdbool.h>

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

static struct etimer et;
static uip_ip6addr_t ip6addr_xy;
static uip_ip6addr_t ip6addr_sink;
static linkaddr_t sink_address;
static linkaddr_t xy_address;

static char udp_buf[MAX_PAYLOAD_LEN];

#define CONNECTION_STATE_SINK  0x01
#define CONNECTION_STATE_XY    0x02
#define CONNECTION_STATE_ALL   (CONNECTION_STATE_SINK | CONNECTION_STATE_XY)

#define XY_DEVICE_TIME_OUT (20 * CLOCK_SECOND)
#define XY_DEVICE_RETRIES 3

static struct uip_udp_conn *client_conn_xy;
static struct uip_udp_conn *client_conn_sink;

static int measurement_count;
static int xpos;  /* [mm] */
static int ypos;  /* [mm] */
static int scan_cnt;
static int packet_repeat;

/* Metrics per position */
typedef struct {
  int retry_histo[TSCH_CONF_MAC_MAX_FRAME_RETRIES+1];
  int failure_cnt;
} result_metrics_t;

#define NR_MEASUREMENTS (sizeof(measurement_definition)/sizeof(measurement_definition_t))

static int configured_tx_power;

/* Metrics for whole scan */
static result_metrics_t result_metrics_per_position;
static result_metrics_t result_metrics_per_scan;
static result_metrics_t result_metrics_per_measurement;

extern void rpl_link_neighbor_callback(const linkaddr_t *addr, int status, int numtx);
static int xy_and_sink_connected(void);
static int xy_ok(void);
static int cmp_linkaddr_ipaddr(const linkaddr_t *addr, uip_ip6addr_t *ip_addr);
static void report_metrics_header(measurement_definition_t *measurement_definition, int scan_cnt);

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process"); 
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static int 
xy_and_sink_connected(void)
{
  static int connection_state = 0;
  const uip_lladdr_t *tmp;

  if (!(connection_state & CONNECTION_STATE_XY)) {
    tmp = uip_ds6_nbr_lladdr_from_ipaddr(&ip6addr_xy);
    if (tmp != NULL) {
      printf(">>>>>>>>XY connected\n");
      connection_state |= CONNECTION_STATE_XY;
    }
  }
  if (!(connection_state & CONNECTION_STATE_SINK)) {
    tmp = uip_ds6_nbr_lladdr_from_ipaddr(&ip6addr_sink);
    if (tmp != 0) {
      printf(">>>>>>>>Sink connected\n");
      connection_state |= CONNECTION_STATE_SINK;
    }
  }
  if (connection_state == CONNECTION_STATE_ALL) {
    etimer_stop(&et);
  } else {
    etimer_restart(&et);
  }
  return (connection_state == CONNECTION_STATE_ALL);
}

/* Wait for OK from XY device. Called from udp_client_process on tcpip_event.
   XY-device is only device that sends data to Coordinator. */
static int
xy_ok(void)
{
  int return_value = 0;
  if (uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = '\0';
    if (!strcmp(uip_appdata, XY_OK)) {
      return_value=1;
    } else {
      PRINTF("XY Data Error: %s\n", (char *)uip_appdata);
    }
  } 
  return return_value;  
}

/* Compare link address (Rime) with IPaddress. Compare the MAC part of both 
   addresses. Return 0 if not equal. 
   Example: 
   ip address:   fe80:0:0:0:215:8d00:35:cee0
   link address: 0.15.8d.0.0.35.ce.e0  
    */
static int 
cmp_linkaddr_ipaddr(const linkaddr_t *addr, uip_ip6addr_t *ip_addr)
{
  return (memcmp(&(addr->u8[1]), &(ip_addr->u8[9]), 7)==0);  
}

/* Report the results of a completed measurement */
static void 
report_metrics_header(measurement_definition_t *measurement_definition, int scan_cnt)
{
  int k;

  printf("\n\nREPORTED METRICS OF SCAN\n");
  printf("Scan %d of %d\n", scan_cnt+1, measurement_definition->scan_cnt);
  printf("X step size [mm], %d\n", measurement_definition->xstep);
  printf("Y step size [mm], %d\n", measurement_definition->ystep);
  printf("Number of XY locations, %d\n", ((X_LIMIT/measurement_definition->xstep)+1) * ((Y_LIMIT/measurement_definition->ystep)+1) );
  printf("Number of measurements per XY location, %d\n", measurement_definition->number_packets);
  k = ((X_LIMIT/measurement_definition->xstep)+1) * ((Y_LIMIT/measurement_definition->ystep)+1);
  k *= measurement_definition->number_packets;
  printf("Total number packets in scan,%d\n", k);
  k *= measurement_definition->scan_cnt;
  printf("Total number packets in measurement,%d\n", k);
  printf("Max retries, %d\n", TSCH_CONF_MAC_MAX_FRAME_RETRIES);
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, TX_SINK_POWER);
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &k);
  printf("TX power to sink [dBm], %d\n", k); 
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, configured_tx_power);
  printf("Hop sequence, ");
  for (k=0; k<sizeof(TSCH_CONF_DEFAULT_HOPPING_SEQUENCE); k++) {
    printf("%d ",TSCH_CONF_DEFAULT_HOPPING_SEQUENCE[k]); 
  }
  printf("\n,,Retries\n");
  printf("X,Y,");
  for (k=0; k<=TSCH_CONF_MAC_MAX_FRAME_RETRIES; k++) {
    printf("%d,", k);
  } 
  printf("Failures\n");
}

/* Callback function called when a packet has been sent (uip-ds6-nbr.c). 
   It passes destination address, status (see enum in mac.h) and number of 
   transmissions. */ 
extern void 
packet_sent_callback(const linkaddr_t *addr, int status, int numtx)
{
#if UIP_CONF_IPV6_RPL
  /* This is the original callback in the stack (uip_ds6.h). Insure it is called
     before our application callback */
  rpl_link_neighbor_callback(addr, status, numtx);    
#endif    
  if (cmp_linkaddr_ipaddr(addr, &ip6addr_sink)) {
      /* Update metrics for sink during active test */
    if (packet_repeat != 0) {
      if ((numtx <= (TSCH_CONF_MAC_MAX_FRAME_RETRIES+1)) && (status == MAC_TX_OK)) {
        result_metrics_per_position.retry_histo[numtx-1]++;
      }
      if ((numtx > (TSCH_CONF_MAC_MAX_FRAME_RETRIES+1)) || (status != MAC_TX_OK)) {
        result_metrics_per_position.failure_cnt++;
        printf("Failure: ");
        switch(status) {
          case(MAC_TX_OK):
            printf("Max retries exceeded: %d\n", numtx-1);
            break;
          case(MAC_TX_COLLISION):
            printf("No TX because of collision\n");
            break;  
          case(MAC_TX_NOACK):
            printf("No ACK\n");
            break;  
          case(MAC_TX_DEFERRED):
            printf("Deferred by MAC for later\n");
            break;  
          case(MAC_TX_ERR):
            printf("Error. Retry may be done\n");
            break;  
          case(MAC_TX_ERR_FATAL):
            printf("Fatal error. Retry useless\n");
            break;  
          default:
            printf("Unknown MAC status returned");
            break;
        }
      }
      process_post(&udp_client_process, PROCESS_EVENT_CONTINUE, NULL);
    }        
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  uip_ip6addr_t ip6addr_prefix;
  struct tsch_slotframe *sf;
  static int xy_retry_cnt;
  static int ok;
  static int k;

  PROCESS_BEGIN();
  leds_init();
  printf("Coordinator client process started\n\n");
  printf("Initial settings:\n");  
  report_metrics_header(&measurement_definition[0],0);

  /* Delay before start */
  k = PRE_MEASUREMENT_DELAY;
  printf("Wait pre-measurement delay ...\n");
  while (k != 0) {
    printf("%d min to go\n",k);
    etimer_set(&et, 60 *CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    k--;
  }
  /* Derive the IPv6 addresses of the servers; i.e. sink and xy device */
  uiplib_ipaddrconv(QUOTEME(UDP_ADDR_XY), &ip6addr_xy);
  uiplib_ipaddrconv(QUOTEME(UDP_ADDR_SINK), &ip6addr_sink);
  /* Derive link address from IPv6 addres */
  sink_address.u8[0] = 0;
  memcpy(&sink_address.u8[1],&ip6addr_sink.u8[9],7);
  xy_address.u8[0] = 0;
  memcpy(&xy_address.u8[1],&ip6addr_xy.u8[9],7);

  /* Start RICH. Client acts as coordinator/"router" */
  uip_ip6addr(&ip6addr_prefix, UDP_ADDR_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  rich_init(&ip6addr_prefix);
  /* TS0: EB slot. Already configured with tsch_schedule_create_minimal() 
     at tsch_start_coordinator()*/  
  sf = tsch_schedule_get_slotframe_by_handle(0);
  /* TS1: C->XY */
	tsch_schedule_add_link(sf, LINK_OPTION_TX, LINK_TYPE_NORMAL, &xy_address, 1, 0);
  /* TS2 .. TS16: C->Sink */
  for (k=2; k<=16; k++) {
  	tsch_schedule_add_link(sf, LINK_OPTION_TX, LINK_TYPE_NORMAL, &sink_address, k, 0);
  }
  /* TS17: Sink->C */
	tsch_schedule_add_link(sf, LINK_OPTION_RX , LINK_TYPE_NORMAL, &sink_address, 17, 0);
  /* TS18: XY->C */
	tsch_schedule_add_link(sf, LINK_OPTION_RX , LINK_TYPE_NORMAL, &xy_address, 18, 0);
  tsch_schedule_print();

  /* Set-up socket with XY device  */
  client_conn_xy = udp_new(&ip6addr_xy, UIP_HTONS(3000), NULL);
  udp_bind(client_conn_xy, UIP_HTONS(3001));
  /* Get-up socket with Sink device */
  client_conn_sink = udp_new(&ip6addr_sink, UIP_HTONS(3000), NULL);
  udp_bind(client_conn_sink, UIP_HTONS(3001));

  /* New connection with remote hosts. 
     First there should be a check if Sink and XY plotter are responding */
  printf("Wait for XY and Sink to connect .....\n");
  etimer_set(&et, CHECK_CONNECT_INTERVAL);
  /* Wait till servers have TSCH connection. Done by regularly sending message */
  while(!xy_and_sink_connected()) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 
  }
  
  /* Measurement execution */
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &configured_tx_power);

  printf("Measurements execution ....\n");
  measurement_count = 0;
  /* For all measurements 
     Precondition: there must be association */
  while(measurement_count < NR_MEASUREMENTS) {
    scan_cnt = 0;
    /* Reset results */
    memset(&result_metrics_per_measurement, 0, sizeof(result_metrics_per_measurement));
    /* For all scans of the measurement */
    while (scan_cnt != measurement_definition[measurement_count].scan_cnt) {
      report_metrics_header(&measurement_definition[measurement_count],scan_cnt);
      memset(&result_metrics_per_scan, 0, sizeof(result_metrics_per_scan));
      xpos = 0;
      ypos = 0;
      /* For all Y positions */
      while (ypos <= Y_LIMIT) {
        /* For all X positions */
        while (xpos <= X_LIMIT) {
          /* Set new XY position. 
             Wait till XY device returns ok or after xy_retry_count retries expires.
             To ease parsing at XY side, transfer coordinates after XY_LOCATION as GCODE */
          sprintf(udp_buf, "%s G01 X%d Y%d Z%d", XY_LOCATION, xpos, ypos, 0);       
          xy_retry_cnt = 5;
          do {
            etimer_set(&et, 20*CLOCK_SECOND);
            uip_udp_packet_send(client_conn_xy, udp_buf, strlen(udp_buf));
            /* Wait for ok message over UDP from XY device that new position is stable */
            ok = 0;
            while (!ok) {
              PROCESS_WAIT_EVENT_UNTIL((ev == tcpip_event) || (ev == PROCESS_EVENT_TIMER)); 
              if (ev == tcpip_event) {
                ok = xy_ok();
                if (ok) {
                  xy_retry_cnt = 0;
                }
              } else {
                if (etimer_expired(&et)) {
                  /* No ok received, timer time out, try to do a retry of XY transmission */
                  xy_retry_cnt--;
                  break;
                }
              }
            }
          } while(xy_retry_cnt != 0);
          etimer_stop(&et);
          /* XY position reached if ok==TRUE */        
          if (ok) {          
            packet_repeat=measurement_definition[measurement_count].number_packets;
            memset(&result_metrics_per_position, 0, sizeof(result_metrics_per_position));
            sprintf(udp_buf, "TEST\n");
            NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, TX_SINK_POWER);
            /* For all packet repeats */
            while (packet_repeat != 0) {
              uip_udp_packet_send(client_conn_sink, udp_buf, strlen(udp_buf));
              PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
              /* Attn: packet_send_callback uses packet_repeat as well ! */
              packet_repeat--;
            }
            /* Restore power to configured value */
            NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, configured_tx_power);
            /* print intermediate results */
            printf("%d,%d,", xpos,ypos);
            for (k=0; k<=TSCH_CONF_MAC_MAX_FRAME_RETRIES; k++) {
              printf("%d,",result_metrics_per_position.retry_histo[k]);   
              result_metrics_per_scan.retry_histo[k] += result_metrics_per_position.retry_histo[k];
            }
            printf("%d\n", result_metrics_per_position.failure_cnt);
            result_metrics_per_scan.failure_cnt += result_metrics_per_position.failure_cnt;
            xpos += measurement_definition[measurement_count].xstep;
          } else {
            printf("New position could not be reached. Measurement aborted\n");
            xpos = X_LIMIT + 1;
          }
        } 
        if (ok) {
          ypos += measurement_definition[measurement_count].ystep;
          xpos = 0;
        } else {
          break;
        }
      }
      if (ok) {
        /* Devices present. XY device back to reference position */
        printf("Totals of scan,,");
        for (k=0; k<=TSCH_CONF_MAC_MAX_FRAME_RETRIES; k++) {
          printf("%d,",result_metrics_per_scan.retry_histo[k]);   
          result_metrics_per_measurement.retry_histo[k] += result_metrics_per_scan.retry_histo[k];
        }
        printf("%d\n\n", result_metrics_per_scan.failure_cnt);
        result_metrics_per_measurement.failure_cnt += result_metrics_per_scan.failure_cnt;
        sprintf(udp_buf, "%s", XY_RESET);
        uip_udp_packet_send(client_conn_xy, udp_buf, strlen(udp_buf));
        while(!xy_ok()) {
          /* Wait for ok message over UDP from XY device */
          PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event); 
        }
        xpos = 0;
        ypos = 0;
        scan_cnt++;
      } else {
        break;
      }
    } 
    if (ok) {
      measurement_count++;
    } else {
      break;
    }
  }
  printf("Totals of measurement,,");
  for (k=0; k<=TSCH_CONF_MAC_MAX_FRAME_RETRIES; k++) {
    printf("%d,",result_metrics_per_measurement.retry_histo[k]);   
  }
  printf("%d\n\n", result_metrics_per_measurement.failure_cnt);
  PRINTF("\nALL FINISHED\n");
  /* Switch on LEDs D3 when finished */
  leds_on(LEDS_GP0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
