/*
 * Copyright (c) 2016, Michael Spoerk
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Michael Spoerk <mi.spoerk@gmail.com>
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-icmp6.h"
#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define CLIENT_PORT     61617
#define SERVER_ADDR     "aaaa::1"
//#define SERVER_ADDR     "fe80::21a:7dff:feda:7114"
#define SERVER_PORT     61616

#define ECHO_INTERVAL   (1 * CLOCK_SECOND)
#define SEND_INTERVAL   (1 * CLOCK_SECOND)

#define TEST_PAYLOAD    "IPv6 over BLE message from TI SensorTag"
/*---------------------------------------------------------------------------*/
static struct etimer timer;

static struct uip_icmp6_echo_reply_notification echo_notification;
static unsigned short echo_received = 0;

static uip_ipaddr_t server_addr;
static struct uip_udp_conn *conn;

static char send_buf[80];
/*---------------------------------------------------------------------------*/
PROCESS(ble_udp_demo_process, "BLE UDP demo process");
AUTOSTART_PROCESSES(&ble_udp_demo_process);
/*---------------------------------------------------------------------------*/
void
echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  printf("echo response received\n");
  echo_received = 1;
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;
  unsigned int data_len;
  if(uip_newdata()) {
    str = uip_appdata;
    data_len = uip_datalen();
    str[data_len] = '\0';

    printf("udp data (len: %d) received: %s\n", data_len, str);
  }
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
    unsigned short msg_len = strlen(TEST_PAYLOAD);
    sprintf(send_buf, TEST_PAYLOAD);
    printf("sending %d bytes of UDP payload\n", msg_len);
    uip_udp_packet_send(conn, send_buf, msg_len);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ble_udp_demo_process, ev, data)
{
  PROCESS_BEGIN();
  printf("BLE UDP demo process started\n");
  leds_on(LEDS_GREEN);

  uiplib_ipaddrconv(SERVER_ADDR, &server_addr);
  uip_icmp6_echo_reply_callback_add(&echo_notification, echo_reply_handler);

  /* wait for an echo response from the server */
  do {
      leds_on(LEDS_RED);
      uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
      etimer_set(&timer, ECHO_INTERVAL);
      leds_off(LEDS_RED);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  } while(!echo_received);

  /* creating the UDP connection */
  conn = udp_new(&server_addr, UIP_HTONS(SERVER_PORT), NULL);
  udp_bind(conn, UIP_HTONS(CLIENT_PORT));
  printf("UDP connection created\n");

  etimer_set(&timer, SEND_INTERVAL);
  while(1) {
      PROCESS_YIELD();
      if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) {
          timeout_handler();
                etimer_set(&timer, SEND_INTERVAL);
      }
      else if(ev == tcpip_event) {
          tcpip_handler();
      }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

