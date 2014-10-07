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

#ifndef NET_SCAN_H_
#define NET_SCAN_H_

#include "contiki.h"

/** @brief Start an active scan for beacons.
 *
 * A scan_result_event event will be emitted once for each
 * beacon received. Use the packetbuf APIs to access the beacon.
 *
 * The wireless network will be unavailable until the scan finishes.
 */
extern void scan_beacon_start(clock_time_t duration_per_channel, uint16_t flags);

/** @brief Start an energy scan.
 *
 * A scan_result_event event will be emitted once for each
 * channel scanned. Use scan_get_energy() to get the maximum
 * RSSI detected on that channel.
 *
 * The wireless network will be unavailable until the scan finishes.
 */
extern void scan_energy_start(clock_time_t duration_per_channel, uint16_t flags);

/** @brief Aborts any current scan that is in progress.
 */
extern void scan_abort(void);

/** @brief Returns true (1) if currently performing a scan.
 */
extern int scan_is_scanning(void);

/** @brief Fetches the maximum energy detected on this channel
 */
extern int8_t scan_get_energy();

/** @brief Emitted when a scan result is ready.
 *
 * This is called once per received beacon during a
 * beacon scan and once per channel for an energy scan.
 */
extern process_event_t scan_result_event;

PROCESS_NAME(scan_process);

/** @brief Called by the RDC layer when a beacon frame has been received.
 *
 * Only the RDC object should call this.
 *
 * The beacon should be in the packetbuf when this is called.
 */
extern void scan_beacon_received(void);

#endif
