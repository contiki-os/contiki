/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
/**
 * \addtogroup openmote-cc2538
 * @{
 *
 * \defgroup openmote-examples OpenMote-CC2538 Example Projects
 * @{
 *
 * Example project demonstrating the OpenMote-CC2538 functionality
 *
 * @{
 *
 * \file
 * Example demonstrating the OpenMote-CC2538 platform
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/rime/broadcast.h"

#include "dev/adxl346.h"
#include "dev/max44009.h"
#include "dev/sht21.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define BROADCAST_CHANNEL   129
/*---------------------------------------------------------------------------*/
PROCESS(openmote_demo_process, "OpenMote-CC2538 demo process");
AUTOSTART_PROCESSES(&openmote_demo_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  leds_toggle(LEDS_GREEN);
  printf("Received %u bytes: '0x%04x'\n", packetbuf_datalen(),
         *(uint16_t *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(openmote_demo_process, ev, data)
{
  static struct etimer et;
  static int16_t counter;
  static uint16_t adxl346_present, sht21_present, max44009_present;
  static int16_t accel, light, temperature, humidity;

  PROCESS_EXITHANDLER(broadcast_close(&bc))

  PROCESS_BEGIN();

  /* Initialize and calibrate the ADXL346 sensor */
  adxl346_present = SENSORS_ACTIVATE(adxl346);
  if(adxl346_present == ADXL346_ERROR) {
    printf("ADXL346 sensor is NOT present!\n");
    leds_on(LEDS_YELLOW);
  } else {
    adxl346.configure(ADXL346_CALIB_OFFSET, 0);
  }

  /* Initialize the MAX44009 sensor */
  max44009_present = SENSORS_ACTIVATE(max44009);
  if(max44009_present == MAX44009_ERROR) {
    printf("MAX44009 sensor is NOT present!\n");
    leds_on(LEDS_ORANGE);
  }

  /* Initialize the SHT21 sensor */
  sht21_present = SENSORS_ACTIVATE(sht21);
  if(sht21_present == SHT21_ERROR) {
    printf("SHT21 sensor is NOT present!\n");
    leds_on(LEDS_RED);
  }

  counter = 0;
  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  printf("****************************************\n");

  while(1) {
    etimer_set(&et, CLOCK_SECOND);

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      if(adxl346_present != ADXL346_ERROR) {
        leds_on(LEDS_YELLOW);
        accel = adxl346.value(ADXL346_READ_X_mG);
        printf("X Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
        accel = adxl346.value(ADXL346_READ_Y_mG);
        printf("Y Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
        accel = adxl346.value(ADXL346_READ_Z_mG);
        printf("Z Acceleration: %d.%u G\n", accel / 1000, accel % 1000);
        leds_off(LEDS_YELLOW);
      }

      if(max44009_present != MAX44009_ERROR) {
        leds_on(LEDS_ORANGE);
        light = max44009.value(MAX44009_READ_LIGHT);
        printf("Light: %u.%ulux\n", light / 100, light % 100);
        leds_off(LEDS_ORANGE);
      }

      if(sht21_present != SHT21_ERROR) {
        leds_on(LEDS_RED);
        temperature = sht21.value(SHT21_READ_TEMP);
        printf("Temperature: %u.%uC\n", temperature / 100, temperature % 100);
        humidity = sht21.value(SHT21_READ_RHUM);
        printf("Rel. humidity: %u.%u%%\n", humidity / 100, humidity % 100);
        leds_off(LEDS_RED);
      }

      printf("****************************************\n");
    }

    if(ev == sensors_event) {
      if(data == &button_sensor) {
        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
           BUTTON_SENSOR_PRESSED_LEVEL) {
          leds_toggle(LEDS_GREEN);
          packetbuf_copyfrom(&counter, sizeof(counter));
          broadcast_send(&bc);
        }
      }
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
