/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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

/* The following libraries add IP/IPv6 support */
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"

/* This is quite handy, allows to print IPv6 related stuff in a readable way */
#include "net/ip/uip-debug.h"

/* The simple UDP library API */
#include "simple-udp.h"

/* Library used to read the metadata in the packets */
#include "net/packetbuf.h"

/* And we are including the example.h with the example configuration */
#include "../example.h"

/* Plus sensors to send data */
#if CONTIKI_TARGET_ZOUL
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#else /* Default is Z1 mote */
#include "dev/adxl345.h"
#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"
#endif

/* And this you should be familiar with from the basic lessons... */
#include "sys/etimer.h"
#include "dev/leds.h"

#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define SEND_INTERVAL	(15 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* The structure used in the Simple UDP library to create an UDP connection */
static struct simple_udp_connection mcast_connection;
/*---------------------------------------------------------------------------*/
/* Create a structure and pointer to store the data to be sent as payload */
static struct my_msg_t msg;
static struct my_msg_t *msgPtr = &msg;
/*---------------------------------------------------------------------------*/
/* Keeps account of the number of messages sent */
static uint16_t counter = 0;
/*---------------------------------------------------------------------------*/
PROCESS(mcast_example_process, "UDP multicast example process");
AUTOSTART_PROCESSES(&mcast_example_process);
/*---------------------------------------------------------------------------*/
/* This is the receiver callback, we tell the Simple UDP library to call this
 * function each time we receive a packet
 */
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  /* Variable used to store the retrieved radio parameters */
  radio_value_t aux;

  /* Create a pointer to the received data, adjust to the expected structure */
  struct my_msg_t *msgPtr = (struct my_msg_t *) data;

  leds_toggle(LEDS_GREEN);
  printf("\n***\nMessage from: ");

  /* Converts to readable IPv6 address */
  uip_debug_ipaddr_print(sender_addr);

  printf("\nData received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);

  /* The following functions are the functions provided by the RF library to
   * retrieve information about the channel number we are on, the RSSI (received
   * strenght signal indication), and LQI (link quality information)
   */

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
  printf("CH: %u ", (unsigned int) aux);

  aux = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  printf("RSSI: %ddBm ", (int8_t)aux);

  aux = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  printf("LQI: %u\n", aux);

  /* Print the received data */
#if CONTIKI_TARGET_ZOUL
  printf("ID: %u, core temp: %d.%u, ADC1: %d, ADC2: %d, ADC3: %d, batt: %u, counter: %u\n",
          msgPtr->id, msgPtr->value1 / 1000, msgPtr->value1 % 1000,
          msgPtr->value2, msgPtr->value3, msgPtr->value4, msgPtr->battery,
          msgPtr->counter);
#else
  printf("ID: %u, temp: %u, x: %d, y: %d, z: %d, batt: %u, counter: %u\n",
          msgPtr->id, msgPtr->value1, msgPtr->value2, msgPtr->value3,
          msgPtr->value4, msgPtr->battery, msgPtr->counter);
#endif
}
/*---------------------------------------------------------------------------*/
static void
take_readings(void)
{
  uint32_t aux;
  counter++;

  msg.id      = 0xAB;
  msg.counter = counter;

#if CONTIKI_TARGET_ZOUL
  msg.value1  = cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
  msg.value2  = adc_zoul.value(ZOUL_SENSORS_ADC1);
  msg.value3  = adc_zoul.value(ZOUL_SENSORS_ADC2);
  msg.value4  = adc_zoul.value(ZOUL_SENSORS_ADC3);

  aux = vdd3_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
  msg.battery = (uint16_t) aux;

  /* Print the sensor data */
  printf("ID: %u, core temp: %u.%u, ADC1: %d, ADC2: %d, ADC3: %d, batt: %u, counter: %u\n",
          msg.id, msg.value1 / 1000, msg.value1 % 1000, msg.value2, msg.value3,
          msg.value4, msg.battery, msg.counter);

#else /* Default is Z1 */
  msg.value1  = tmp102.value(TMP102_READ);
  msg.value2  = adxl345.value(X_AXIS);
  msg.value3  = adxl345.value(Y_AXIS);
  msg.value4  = adxl345.value(Z_AXIS);

  /* Convert the battery reading from ADC units to mV (powered over USB) */
  aux = battery_sensor.value(0);
  aux *= 5000;
  aux /= 4095;
  msg.battery = aux;

  /* Print the sensor data */
  printf("ID: %u, temp: %u, x: %d, y: %d, z: %d, batt: %u, counter: %u\n",
          msg.id, msg.value1, msg.value2, msg.value3, msg.value4,
          msg.battery, msg.counter);
#endif
}
/*---------------------------------------------------------------------------*/
static void
print_radio_values(void)
{
  radio_value_t aux;

  printf("\n* Radio parameters:\n");

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
  printf("   Channel %u", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &aux);
  printf(" (Min: %u, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &aux);
  printf("Max: %u)\n", aux);

  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &aux);
  printf("   Tx Power %3d dBm", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MIN, &aux);
  printf(" (Min: %3d dBm, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MAX, &aux);
  printf("Max: %3d dBm)\n", aux);

  /* This value is set in contiki-conf.h and can be changed */
  printf("   PAN ID: 0x%02X\n", IEEE802154_CONF_PANID);
}
/*---------------------------------------------------------------------------*/
static void
set_radio_default_parameters(void)
{
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, EXAMPLE_TX_POWER);
  // NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, EXAMPLE_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, EXAMPLE_CHANNEL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mcast_example_process, ev, data)
{
  static struct etimer periodic_timer;

  /* Data container used to store the IPv6 addresses */
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  /* Alternatively if you want to change the channel or transmission power, this
   * are the functions to use.  You can also change these values in runtime.
   * To check what are the regular platform values, comment out the function
   * below, so the print_radio_values() function shows the default.
   */
  set_radio_default_parameters();

  /* This blocks prints out the radio constants (minimum and maximum channel,
   * transmission power and current PAN ID (more or less like a subnet)
   */
  print_radio_values();

  /* Create the UDP connection.  This function registers a UDP connection and
   * attaches a callback function to it. The callback function will be
   * called for incoming packets. The local UDP port can be set to 0 to indicate
   * that an ephemeral UDP port should be allocated. The remote IP address can
   * be NULL, to indicate that packets from any IP address should be accepted.
   */
  simple_udp_register(&mcast_connection, UDP_CLIENT_PORT, NULL,
                      UDP_CLIENT_PORT, receiver);

  /* Activate the sensors */
#if CONTIKI_TARGET_ZOUL
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
#else /* Default is Z1 */
  SENSORS_ACTIVATE(adxl345);
  SENSORS_ACTIVATE(tmp102);
  SENSORS_ACTIVATE(battery_sensor);
#endif

  etimer_set(&periodic_timer, SEND_INTERVAL);

  while(1) {
    /* Wait for a fixed time */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    printf("\n***\nSending packet to multicast adddress ");

    /* Create a link-local multicast address to all nodes */
    uip_create_linklocal_allnodes_mcast(&addr);

    uip_debug_ipaddr_print(&addr);
    printf("\n");

    /* Take sensor readings and store into the message structure */
    take_readings();

    /* Send the multicast packet to all devices */
    simple_udp_sendto(&mcast_connection, msgPtr, sizeof(msg), &addr);

    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
