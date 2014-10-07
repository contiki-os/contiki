/*
 * Copyright (c) 2014 Robert Quattlebaum
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
 *
 * Authors: Robert Quattlebaum <darco@deepdarc.com>
 *
 */

#include "contiki.h"
#include "scan.h"
#include "netstack.h"
#include "packetbuf.h"

static uint8_t is_energy_scan;
static int8_t max_energy_on_channel;
static int32_t energy_count;
static clock_time_t channel_scan_period;
static struct process *target_process;

process_event_t scan_result_event;
PROCESS(scan_process, "scan");
/*----------------------------------------------------------------------------*/
void
scan_beacon_start(clock_time_t duration_per_channel, uint16_t flags)
{
  is_energy_scan = 0;
  channel_scan_period = duration_per_channel;
  process_exit(&scan_process);
  target_process = PROCESS_CURRENT();
  process_start(&scan_process, NULL);
}
/*----------------------------------------------------------------------------*/
void
scan_energy_start(clock_time_t duration_per_channel, uint16_t flags)
{
  is_energy_scan = 1;
  channel_scan_period = duration_per_channel;
  process_exit(&scan_process);
  target_process = PROCESS_CURRENT();
  process_start(&scan_process, NULL);
}
/*----------------------------------------------------------------------------*/
void
scan_abort(void)
{
  process_exit(&scan_process);
}
/*----------------------------------------------------------------------------*/
int
scan_is_scanning(void)
{
  return process_is_running(&scan_process);
}
/*----------------------------------------------------------------------------*/
int8_t
scan_get_energy()
{
  return max_energy_on_channel;
}
/*----------------------------------------------------------------------------*/
void
scan_beacon_received(void)
{
  process_post_synch(&scan_process, scan_result_event, NULL);
}
/*----------------------------------------------------------------------------*/
PROCESS_THREAD(scan_process, ev, data)
{
  static radio_value_t prev_channel;
  static radio_value_t curr_channel;
  static radio_value_t last_channel;
  static struct etimer timer;
  static clock_time_t next_channel_time;

  if (ev == PROCESS_EVENT_EXIT) {
    /* Restore our previous channel. */
    if (prev_channel != curr_channel) {
      NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, prev_channel);
      curr_channel = prev_channel;
    }
    return PT_EXITED;
  }

  PROCESS_BEGIN();

  if (scan_result_event == 0) {
    scan_result_event = process_alloc_event();
  }

  etimer_stop(&timer);

  PROCESS_PAUSE();

  NETSTACK_RADIO.on();

  /* Save our current channel. */
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &prev_channel);

  curr_channel = 0;
  last_channel = 1;

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &curr_channel);
  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &last_channel);

  if(is_energy_scan) {
    /* Energy Scan */

    for(;curr_channel<last_channel;curr_channel++) {
      /* Set our channel. */
      NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, curr_channel);

      next_channel_time = clock_time() + channel_scan_period;
      max_energy_on_channel = -128;

      /* Monitor the RSSI for a period of time. */
      do {
        radio_value_t rssi = -128;

        NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi);

        if (rssi > max_energy_on_channel) {
          max_energy_on_channel = rssi;
        }

        PROCESS_PAUSE();
      } while ((next_channel_time - clock_time()) > 0);

      /* Report our energy results for this channel. */
      process_post_synch(target_process, scan_result_event, NULL);
    }
  } else {
    /* Active Beacon Scan */

    for(;curr_channel<last_channel;curr_channel++) {

      NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, curr_channel);

      next_channel_time = clock_time() + channel_scan_period;
      etimer_set(&timer, 0);

      do {
        PROCESS_WAIT_EVENT();

        if (PROCESS_EVENT_TIMER == ev) {

          /* Create a beacon request. */
          packetbuf_clear();
          packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE, PACKETBUF_ATTR_PACKET_TYPE_BEACON_REQ);
          if (FRAMER_FAILED == NETSTACK_FRAMER.create()) {
            goto bail;
          }

          /* Send the beacon request. */
          NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());

          etimer_set(&timer, CLOCK_SECOND);
        } else if (scan_result_event == ev) {
          /* We have received a beacon! Let our calling process know. */
          process_post_synch(target_process, scan_result_event, NULL);
        }

      } while ((next_channel_time - clock_time()) > 0);
    }
  }

bail:

  /* Restore our previous channel. */
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, prev_channel);
  curr_channel = prev_channel;

  PROCESS_END();
}
