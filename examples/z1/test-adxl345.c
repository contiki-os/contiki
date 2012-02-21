/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         A simple program for testing the adxl345 on-board accelerometer of the
 *         Zolertia Z1. Enables interrupts and registers callbacks for them. Then
 *         starts a constantly running readout of acceleration data.
 * \author
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 */

#include <stdio.h>
#include "contiki.h"
#include "serial-shell.h"
#include "shell-ps.h"
#include "shell-file.h"
#include "shell-text.h"
#include "dev/adxl345.h"

#define LED_INT_ONTIME        CLOCK_SECOND/2
#define ACCM_READ_INTERVAL    CLOCK_SECOND

static process_event_t ledOff_event;
/*---------------------------------------------------------------------------*/
PROCESS(accel_process, "Test Accel process");
PROCESS(led_process, "LED handling process");
AUTOSTART_PROCESSES(&accel_process, &led_process);
/*---------------------------------------------------------------------------*/
/* As several interrupts can be mapped to one interrupt pin, when interrupt 
    strikes, the adxl345 interrupt source register is read. This function prints
    out which interrupts occurred. Note that this will include all interrupts,
    even those mapped to 'the other' pin, and those that will always signal even if
    not enabled (such as watermark). */

void
print_int(uint16_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
#if ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT
  if(reg & ADXL345_INT_OVERRUN) {
    printf("Overrun ");
  }
  if(reg & ADXL345_INT_WATERMARK) {
    printf("Watermark ");
  }
  if(reg & ADXL345_INT_DATAREADY) {
    printf("DataReady ");
  }
#endif
  if(reg & ADXL345_INT_FREEFALL) {
    printf("Freefall ");
  }
  if(reg & ADXL345_INT_INACTIVITY) {
    printf("InActivity ");
  }
  if(reg & ADXL345_INT_ACTIVITY) {
    printf("Activity ");
  }
  if(reg & ADXL345_INT_DOUBLETAP) {
    printf("DoubleTap ");
  }
  if(reg & ADXL345_INT_TAP) {
    printf("Tap ");
  }
  printf("\n");
}

/*---------------------------------------------------------------------------*/
/* accelerometer free fall detection callback */

void
accm_ff_cb(uint8_t reg){
  L_ON(LEDS_B);
  process_post(&led_process, ledOff_event, NULL);
  printf("~~[%u] Freefall detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  print_int(reg);
}
/*---------------------------------------------------------------------------*/
/* accelerometer tap and double tap detection callback */

void
accm_tap_cb(uint8_t reg){
  process_post(&led_process, ledOff_event, NULL);
  if(reg & ADXL345_INT_DOUBLETAP){
    L_ON(LEDS_G);
    printf("~~[%u] DoubleTap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  } else {
    L_ON(LEDS_R);
    printf("~~[%u] Tap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  }
  print_int(reg);
}
/*---------------------------------------------------------------------------*/
/* When posted an ledOff event, the LEDs will switch off after LED_INT_ONTIME.
      static process_event_t ledOff_event;
      ledOff_event = process_alloc_event();
      process_post(&led_process, ledOff_event, NULL);
*/

static struct etimer ledETimer;
PROCESS_THREAD(led_process, ev, data) {
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == ledOff_event);
    etimer_set(&ledETimer, LED_INT_ONTIME);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    L_OFF(LEDS_R + LEDS_G + LEDS_B);
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/*  Returns a string with the argument byte written in binary.
    Example usage:
      printf("Port1: %s\n", char2bin(P1IN));
*/    
/*
static uint8_t b[9];

static uint8_t
*char2bin(uint8_t x) {
  uint8_t z;
  b[8] = '\0';
  for (z = 0; z < 8; z++) {
    b[7-z] = (x & (1 << z)) ? '1' : '0';
  }
  return b;
}
*/
/*---------------------------------------------------------------------------*/
/* Main process, setups  */

static struct etimer et;

PROCESS_THREAD(accel_process, ev, data) {
  PROCESS_BEGIN();
  {
    int16_t x, y, z;

    serial_shell_init();
    shell_ps_init();
    shell_file_init();  // for printing out files
    shell_text_init();  // for binprint

    /* Register the event used for lighting up an LED when interrupt strikes. */
    ledOff_event = process_alloc_event();

    /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

    /* Register the callback functions for each interrupt */
    ACCM_REGISTER_INT1_CB(accm_ff_cb);
    ACCM_REGISTER_INT2_CB(accm_tap_cb);

    /* Set what strikes the corresponding interrupts. Several interrupts per pin is 
      possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
    accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);

    while (1) {
	    x = accm_read_axis(X_AXIS);
	    y = accm_read_axis(Y_AXIS);
	    z = accm_read_axis(Z_AXIS);
	    printf("x: %d y: %d z: %d\n", x, y, z);

      etimer_set(&et, ACCM_READ_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

