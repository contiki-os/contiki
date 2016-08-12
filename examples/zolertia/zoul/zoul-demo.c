/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 *
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
 */
/**
 * \addtogroup zoul
 * @{
 *
 * \defgroup zoul-examples Zoul examples
 * @{
 *
 * \defgroup zoul-demo Zoul demo application
 *
 *   Example project demonstrating the Zoul module on the RE-Mote and Firefly
 *   platforms.
 *
 * - Boot sequence: LEDs flashing (Red, then yellow, finally green)
 *
 * - etimer/clock : Every LOOP_INTERVAL clock ticks (LOOP_PERIOD secs) the LED
 *                  defined as LEDS_PERIODIC will turn on
 * - rtimer       : Exactly LEDS_OFF_HYSTERISIS rtimer ticks later,
 *                  LEDS_PERIODIC will turn back off
 * - ADC sensors  : On-chip VDD / 3, temperature, and Phidget sensor
 *                  values are printed over UART periodically.
 * - UART         : Every LOOP_INTERVAL the Remote will print something over
 *                  the UART. Receiving an entire line of text over UART (ending
 *                  in \\r) will cause LEDS_SERIAL_IN to toggle
 * - Radio comms  : BTN_USER sends a rime broadcast. Reception of a rime
 *                  packet will toggle LEDs defined as LEDS_RF_RX
 * - Button       : Keeping the button pressed will print a counter that
 *                  increments every BUTTON_PRESS_EVENT_INTERVAL ticks
 *
 * @{
 *
 * \file
 *     Example demonstrating the Zoul module on the RE-Mote & Firefly platforms
 */
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/netstack.h"
#include "net/rime/broadcast.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD         8
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define LEDS_OFF_HYSTERISIS ((RTIMER_SECOND * LOOP_PERIOD) >> 1)
#define LEDS_PERIODIC       LEDS_BLUE
#define LEDS_BUTTON         LEDS_RED
#define LEDS_SERIAL_IN      LEDS_GREEN
#define LEDS_REBOOT         LEDS_ALL
#define LEDS_RF_RX          (LEDS_YELLOW | LEDS_RED)
#define BUTTON_PRESS_EVENT_INTERVAL (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
static struct etimer et;
static struct rtimer rt;
static uint16_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(zoul_demo_process, "Zoul demo process");
AUTOSTART_PROCESSES(&zoul_demo_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  leds_toggle(LEDS_RF_RX);
  printf("*** Received %u bytes from %u:%u: '0x%04x'\n", packetbuf_datalen(),
         from->u8[0], from->u8[1], *(uint16_t *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
static void
rt_callback(struct rtimer *t, void *ptr)
{
  leds_off(LEDS_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(zoul_demo_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&bc))

  PROCESS_BEGIN();

  counter = 0;

  /* Disable the radio duty cycle and keep the radio on */
  NETSTACK_MAC.off(1);

  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  /* Configure the user button */
  button_sensor.configure(BUTTON_SENSOR_CONFIG_TYPE_INTERVAL,
                          BUTTON_PRESS_EVENT_INTERVAL);

  /* Configure the ADC ports */
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

  printf("Zoul test application\n");

  etimer_set(&et, LOOP_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      leds_on(LEDS_PERIODIC);

      printf("-----------------------------------------\n"
             "Counter = 0x%08x\n", counter);

      printf("VDD = %d mV\n",
             vdd3_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED));

      printf("Temperature = %d mC\n",
             cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED));

      printf("ADC1 = %d raw\n",
             adc_zoul.value(ZOUL_SENSORS_ADC1));

      printf("ADC3 = %d raw\n",
             adc_zoul.value(ZOUL_SENSORS_ADC3));

      etimer_set(&et, LOOP_INTERVAL);
      rtimer_set(&rt, RTIMER_NOW() + LEDS_OFF_HYSTERISIS, 1,
                 rt_callback, NULL);
      counter++;

    } else if(ev == sensors_event) {
      if(data == &button_sensor) {
        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
           BUTTON_SENSOR_PRESSED_LEVEL) {
          printf("Button pressed\n");
          packetbuf_copyfrom(&counter, sizeof(counter));
          broadcast_send(&bc);
        } else {
          printf("...and released!\n");
        }
      }

    } else if(ev == serial_line_event_message) {
      leds_toggle(LEDS_SERIAL_IN);
    } else if(ev == button_press_duration_exceeded) {
      printf("Button pressed for %d ticks [%u events]\n",
             (*((uint8_t *)data) * BUTTON_PRESS_EVENT_INTERVAL),
             button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION));
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
