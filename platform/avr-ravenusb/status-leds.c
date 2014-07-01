/**
 * \file
 *         Controls the status LEDs on the RZUSBStick.
 * \author
 *         Robert Quattlebaum <darco@deepdarc.com>
 *
 */

/*
 * Copyright (c) 2013, Robert Quattlebaum.
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
 * Author: Robert Quattlebaum <darco@deepdarc.com>
 *
 */

#include "status-leds.h"

PROCESS(status_leds_process, "Status LEDs");

static uint8_t ledRX_timer, ledTX_timer, ledVCP_timer;

static enum {
  STATUS_LED_UNENUMERATED,
  STATUS_LED_INACTIVE,
  STATUS_LED_READY
} device_status_indicator;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(status_leds_process, ev, data_proc)
{
  static struct etimer et;

  static struct etimer et_indicator;

  PROCESS_BEGIN();

  Leds_init();

  etimer_set(&et, CLOCK_SECOND / 30);
  etimer_set(&et_indicator, CLOCK_SECOND / 8);

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&et_indicator)) {
      if(device_status_indicator == STATUS_LED_READY) {
        jackdaw_led_STAT_on();
      } else if(device_status_indicator == STATUS_LED_INACTIVE) {
        jackdaw_led_STAT_toggle();
        etimer_set(&et_indicator, (CLOCK_SECOND / 8));
      } else {
        jackdaw_led_STAT_toggle();
        etimer_set(&et_indicator, (CLOCK_SECOND / 3));
      }
    }

    if(etimer_expired(&et)) {
      if(0 != ledRX_timer) {
        ledRX_timer--;
        if(ledRX_timer & (1 << 1)) {
          jackdaw_led_RX_on();
        } else {
          jackdaw_led_RX_off();
        }
      } else
        jackdaw_led_RX_off();

      if(0 != ledTX_timer) {
        ledTX_timer--;
        if(ledTX_timer & (1 << 1)) {
          jackdaw_led_TX_on();
        } else {
          jackdaw_led_TX_off();
        }
      } else
        jackdaw_led_TX_off();

      if(0 != ledVCP_timer) {
        ledVCP_timer--;
        if(ledVCP_timer & (1 << 2)) {
          jackdaw_led_VCP_on();
        } else {
          jackdaw_led_VCP_off();
        }
      } else {
        jackdaw_led_VCP_off();
      }

      if(ledRX_timer || ledTX_timer || ledVCP_timer) {
        etimer_restart(&et);
      }
    }
  } /* while(1) */

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void
status_leds_radio_tx()
{
  if(!(ledRX_timer || ledTX_timer || ledVCP_timer)) {
    process_poll(&status_leds_process);
  }
  ledTX_timer |= (1 << 2);
  if(((ledTX_timer - 1) & (1 << 1))) {
    jackdaw_led_TX_on();
  }
}

/*---------------------------------------------------------------------------*/
void
status_leds_radio_rx()
{
  if(!(ledRX_timer || ledTX_timer || ledVCP_timer)) {
    process_poll(&status_leds_process);
  }
  ledRX_timer |= (1 << 2);
  if(((ledRX_timer - 1) & (1 << 1))) {
    jackdaw_led_RX_on();
  }
}

/*---------------------------------------------------------------------------*/
void
status_leds_serial_tx()
{
  if(!(ledRX_timer || ledTX_timer || ledVCP_timer)) {
    process_poll(&status_leds_process);
  }
  ledVCP_timer |= (1 << 3);
  if(((ledVCP_timer - 1) & (1 << 2))) {
    jackdaw_led_VCP_on();
  }
}

/*---------------------------------------------------------------------------*/
void
status_leds_serial_rx()
{
  status_leds_serial_tx();
}

static bool radio_is_on;

/*---------------------------------------------------------------------------*/
void
status_leds_radio_on()
{
  if(!radio_is_on) {
    status_leds_serial_tx();
  }
  radio_is_on = true;
}

/*---------------------------------------------------------------------------*/
void
status_leds_radio_off()
{
  radio_is_on = false;
}

/*---------------------------------------------------------------------------*/
void
status_leds_unenumerated()
{
  device_status_indicator = STATUS_LED_UNENUMERATED;
}

/*---------------------------------------------------------------------------*/
void
status_leds_inactive()
{
  device_status_indicator = STATUS_LED_INACTIVE;
}

/*---------------------------------------------------------------------------*/
void
status_leds_ready()
{
  device_status_indicator = STATUS_LED_READY;
}

/*---------------------------------------------------------------------------*/
void
status_leds_will_sleep()
{
  if(STATUS_LED_READY == device_status_indicator) {
    jackdaw_led_STAT_off();
  }
}

/*---------------------------------------------------------------------------*/
void
status_leds_did_wake()
{
  if(STATUS_LED_READY == device_status_indicator) {
    jackdaw_led_STAT_on();
  }
}
