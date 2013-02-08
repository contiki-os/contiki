/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Example to demonstrate-test the sensors functionality on
 *         sensinode/cc2430 devices.
 *
 *         A UDP/IPv6 process waits for requests from a monitoring station
 *         and responds with sensor values.
 *
 *         The message exchange is based on a custom protocol.
 *         Check sensors-driver.c for protocol details.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"
#include <stdio.h>

#if CONTIKI_TARGET_SENSINODE
#include "debug.h"
#include "dev/sensinode-sensors.h"
#else
#define putstring(s)
#endif

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

#define SERVER_PORT       60000

#define SENSOR_OK      0
#define SENSOR_ADC_OFF 1
#define SENSOR_UNKNOWN 2

int8_t read_sensor(char *rs);
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor adc_sensor;
/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  memset(buf, 0, MAX_PAYLOAD_LEN);

  if(uip_newdata()) {
    len = uip_datalen();
    memcpy(buf, uip_appdata, len);
    PRINTF("%u bytes from [", len);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
    len = read_sensor(buf);
    if(len) {
      server_conn->rport = UIP_UDP_BUF->srcport;
      uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
      uip_udp_packet_send(server_conn, buf, len);
      PRINTF("Sent %u bytes\n", len);
    }

    /* Restore server connection to allow data from any node */
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
  }
  return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  static struct sensors_sensor *b1;
  static struct sensors_sensor *b2;
#endif

  PROCESS_BEGIN();
  putstring("Starting UDP server\n");

#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  putstring("Button X: Toggle LED X\n");
#endif

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(SERVER_PORT));

#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  b1 = sensors_find(BUTTON_1_SENSOR);
  b2 = sensors_find(BUTTON_2_SENSOR);
#endif

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
    } else if(ev == sensors_event && data != NULL) {
      if(data == b1) {
        leds_toggle(LEDS_GREEN);
      } else if(data == b2) {
        leds_toggle(LEDS_RED);
      }
#endif /* (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON) */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
