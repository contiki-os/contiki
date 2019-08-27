 
 /*
 * Copyright (c) 2019, Sancti Spiritus, Cuba.
 * All rights reserved.
 * *
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


/* Includes */

#ifndef __MLBOT_PACKET_H__
#define __MLBOT_PACKET_H__

#include "contiki.h"
#include "dev/radio.h"

#include "net/mac/frame802154.h"

/* Configuration */



/* Define tipes */

/**
 * \brief The nextEbOffset need just 3 bytes, and most of cases, just 2.
 */
typedef struct {
  uint8_t ms1b;
  uint16_t ls2b;
} eb_offset;

/**
 * \brief Defines the LB informations fields.
 */
typedef struct {
  uint8_t next_eb_channel;          /**< 5 bit. Next EB channel */
  eb_offset time_until_next_eb;     /**< 3 byte. Time in miliseconds until the next EB */
  uint8_t mac_join_metric;          /**< 1 byte. Join metric */ //Puede ser menor, TSCH_MAX_JOIN_PRIORITY tiene hasta 32, 5 bits
} payload_lb_t;

/**
 * \brief Defines the LB link quality params.
 */
typedef struct {
  radio_value_t rssi;         /**< Received Signal Strenght Indicator */
  radio_value_t lqi;          /**< Link Quality Indicator */
} link_lb_t;

/**
 * \brief Defines all LB data. //Creo se pueda identificar el LB por su fuente (addr y pan ID)
 *                             //El resto de los parametros se actualizan cada nuevo lb de dicha fuente
 */
typedef struct {
  /* The fields src_addr must come first to ensure they are aligned to the
   * CPU word size. Needed as they are accessed directly as linkaddr_t*. Note we cannot use
   * the type linkaddr_t directly here, as we always need 8 bytes, not LINKADDR_SIZE bytes. */
  uint8_t lb_handle;
  uint8_t src_addr[8];                /**< Source address */
  uint16_t src_pid;                   /**< Source PAN ID */
  payload_lb_t payload;               /**< Payload field  */
  clock_time_t next_eb_clock_time;    /**< Time in clock_time when the next EB will be send */
  link_lb_t link;                     /**< Link quality indicators */
} light_beacon_t;


/* Prototypes */

/*---------------------------------------------------------------------------*/
/* Create LB packet */

int mlbot_packet_create_lb(light_beacon_t *lb, uint8_t *buf);

/*---------------------------------------------------------------------------*/
/* Update Time until next EB and next channel in LB packet */
int mlbot_packet_update_lb(light_beacon_t *lb, uint8_t *buf);

/*---------------------------------------------------------------------------*/
/*Parse an LB payload
* \brief  Parse an LB packet payload. This funtion may used when 
          tsch_packet_parse_eb read a beacon and is not a EB.
  \param lb Light beacon struct to store the data parsed
  \param frame A beacon frame.
 */
int mlbot_packet_parse_lb(frame802154_t *frame, light_beacon_t *lb);


/*---------------------------------------------------------------------------*/
/* Init the lb buffer to tx */
uint8_t mlbot_lb_init (light_beacon_t *lb);

/** @} */
#endif /* MLBOT_PACKET_H */
/** @} */
/** @} */