/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/random.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "mqtt-res.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_CLIENT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define SENSORS_NAME_EXPAND(x, y) x##y
#define SENSORS_NAME(x, y) SENSORS_NAME_EXPAND(x, y)
/*---------------------------------------------------------------------------*/
/* Payload length of ICMPv6 echo requests used to measure RSSI with def rt */
#define ECHO_REQ_PAYLOAD_LEN   20
/*---------------------------------------------------------------------------*/
#ifdef DEFAULT_ORG_CONF_ID
#define DEFAULT_ORG_ID                DEFAULT_ORG_CONF_ID
#else
#define DEFAULT_ORG_ID                "zolertia"
#endif
/*---------------------------------------------------------------------------*/
/* Include there the sensors processes to include */
PROCESS_NAME(SENSORS_NAME(MQTT_SENSORS, _sensors_process));
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
process_event_t sensors_stop_event;
process_event_t sensors_restart_event;
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_res_process, "MQTT resources process");
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
static struct uip_icmp6_echo_reply_notification echo_reply_notification;
static int def_rt_rssi = 0;
/*---------------------------------------------------------------------------*/
/* Converts the IPv6 address to string */
static int
ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr)
{
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}
/*---------------------------------------------------------------------------*/
/* Handles the ping response and updates the RSSI value */
static void
echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data,
                   uint16_t datalen)
{
  if(uip_ip6addr_cmp(source, uip_ds6_defrt_choose())) {
    def_rt_rssi = sicslowpan_get_last_rssi();
  }
}
/*---------------------------------------------------------------------------*/
static void
ping_parent(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
    PRINTF("MQTT Res: Parent not available\n");
    return;
  }

  uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
                 ECHO_REQ_PAYLOAD_LEN);
}
/*---------------------------------------------------------------------------*/
void
mqtt_res_parent_rssi(char *buf, uint8_t len)
{
  memset(buf, 0, len);
  snprintf(buf, len, "%d", def_rt_rssi);
}
/*---------------------------------------------------------------------------*/
int
mqtt_check_int_chunk_len(uint16_t chunk, uint8_t chunk_len)
{
  uint8_t int_len;

  if(chunk < 10) {
    int_len = 1;
  } else if(chunk < 100) {
    int_len = 2;
  } else if(chunk < 1000) {
    int_len = 3;
  } else if(chunk < 10000) {
    int_len = 4;
  } else {
    /* 65535 should be the maximum number */
    int_len = 5;
  }

  if(int_len == chunk_len) {
    return 0;
  }

  PRINTF("MQTT Res: payload mismatch: %u vs %u\n", chunk_len, int_len);
  return 1;
}
/*---------------------------------------------------------------------------*/
void
mqtt_res_uptime(char *buf, uint8_t len)
{
  memset(buf, 0, len);
  snprintf(buf, len, "%lu", clock_seconds());
}
/*---------------------------------------------------------------------------*/
void
mqtt_res_parent_addr(char *buf, uint8_t len)
{
  memset(buf, 0, len);
  ipaddr_sprintf(buf, len, uip_ds6_defrt_choose());
}
/*---------------------------------------------------------------------------*/
void
mqtt_res_client_id(char *buf, uint8_t len)
{
  /* Create Client ID using IBM bluemix format */
  memset(buf, 0, len);
  snprintf(buf, len, "d:%s:%s:%02x%02x%02x%02x%02x%02x",
           DEFAULT_ORG_ID, DEVICE_ID,
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
           linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);
}
/*---------------------------------------------------------------------------*/
void
mqtt_res_u16_addr(char *buf, uint8_t len)
{
  radio_value_t aux;
  NETSTACK_RADIO.get_value(RADIO_PARAM_16BIT_ADDR, &aux);

  /* Return the 16-bit address of the device */
  memset(buf, 0, len);
  snprintf(buf, len, "%04u", aux);
}
/*---------------------------------------------------------------------------*/
void
activate_sensors(uint8_t state)
{
  if(state) {
    process_post(&SENSORS_NAME(MQTT_SENSORS, _sensors_process),
                 sensors_restart_event, NULL);
  } else {
    process_post(&SENSORS_NAME(MQTT_SENSORS, _sensors_process),
                 sensors_stop_event, NULL);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_res_process, ev, data)
{
  PROCESS_BEGIN();

  def_rt_rssi = 0x8000000;
  uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
                                    echo_reply_handler);

  PRINTF("\nMQTT default resources process started\n");

  ping_parent();

  sensors_stop_event = process_alloc_event();
  sensors_restart_event = process_alloc_event();

  if(MQTT_RES_PING_INTERVAL) {
    PRINTF("MQTT Res: Starting periodic ping %u\n", MQTT_RES_PING_INTERVAL);
    etimer_set(&et, CLOCK_SECOND * MQTT_RES_PING_INTERVAL);
  }

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      PRINTF("MQTT Res: Pinging the parent\n");
      ping_parent();
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */

