/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: sicslowmac.h,v 1.3 2008/10/14 18:36:07 c_oflynn Exp $
 */

/**
 * \file
 *         Example glue code between the existing MAC code and the
 *         Contiki mac interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Eric Gnoske <egnoske@gmail.com>
 *         Blake Leverett <bleverett@gmail.com>
 */

/**
   \addtogroup rf230mac
*/

#ifndef __SICSLOWMAC_H__
#define __SICSLOWMAC_H__

#include <stdbool.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/radio.h"
#include "frame.h"

/* Macros & Defines */

typedef enum {
    MAC_EVENT_RX=0x10,
    MAC_EVENT_ACK,
    MAC_EVENT_NACK,
    MAC_EVENT_SCAN,
    MAC_EVENT_BEACON_REQ,
	MAC_EVENT_DROPPED,
	MAC_EVENT_TX
    /*    MAC_EVENT_TIMER */
} event_t;

typedef struct {
    event_t event;  /**< Event type, see event_t for details. */
    uint8_t *data;  /**< Associated data that goes with the event.  Depends on event type. */
} event_object_t;

extern const struct mac_driver sicslowmac_driver;

/* Prototypes */
PROCESS_NAME(mac_process);

uint8_t mac_event_pending(void);
void mac_put_event(event_object_t *object);
event_object_t *mac_get_event(void);
void mac_task(process_event_t ev, process_data_t data);
uint8_t nwk_dataIndication(void);
const  struct mac_driver *sicslowmac_init(const struct radio_driver *r);
/* void sicslowmac_input_packet(const struct radio_driver *d); */
int sicslowmac_dataRequest(void);
void sicslowmac_dataIndication(void);
void sicslowmac_resetRequest (bool setDefaultPIB);
int sicsloread(void);
void byte_reverse(uint8_t * bytes, uint8_t num);
struct mac_driver * sicslowmac_get_driver(void);
parsed_frame_t * sicslowmac_get_frame(void);

#endif /* __SICSLOWMAC_H__ */
