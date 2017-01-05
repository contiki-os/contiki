/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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
 * \addtogroup net
 * @{
 */
/**
 * \defgroup sixtop 6TiSCH Operation Sublayer (6top)
 * @{
 */
/**
 * \file
 *         6TiSCH Operation Sublayer (6top) APIs
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#ifndef _SIXTOP_H_
#define _SIXTOP_H_

/**
 * \brief The maximum number of Scheduling Functions in the system.
 */
#ifdef SIXTOP_CONF_MAX_SCHEDULE_FUNCTIONS
#define SIXTOP_MAX_SCHEDULE_FUNCTIONS SIXTOP_CONF_MAX_SCHEDULE_FUNCTIONS
#else
#define SIXTOP_MAX_SCHEDULE_FUNCTIONS 1
#endif

/**
 * \brief The maximum number of transactions which the sixtop module can handle
 * at the same time.
 */
#ifdef SIXTOP_CONF_6P_MAX_TRANSACTIONS
#define SIXTOP_6P_MAX_TRANSACTIONS SIXTOP_CONF_6P_MAX_TRANSACTIONS
#else
#define SIXTOP_6P_MAX_TRANSACTIONS 1
#endif

/**
 * \brief The initial sequence number used for 6P request
 */
#define SIXTOP_6P_INITIAL_SEQUENCE_NUMBER 0

/**
 * \brief The maximum number of neighbor objects which 6P can have at the same
 * time.
 */
#ifdef SIXTOP_CONF_6P_MAX_NEIGHBORS
#define SIXTOP_6P_MAX_NEIGHBORS SIXTOP_CONF_6P_MAX_NEIGHBORS
#else
#include "net/nbr-table.h"
#define SIXTOP_6P_MAX_NEIGHBORS NBR_TABLE_MAX_NEIGHBORS
#endif

#include "sixp.h"

/**
 * /brief Scheduling Function Driver
 */
typedef struct {
  uint8_t sfid;                             /**< SFID */
  clock_time_t timeout_interval;            /**< Timeout Value */
  void (*init)(void);                       /**< Init Function */
  sixp_request_input_t request_input;       /**< Request Handler */
  sixp_response_input_t response_input;     /**< Response Handler */
  sixp_response_input_t confirmation_input; /**< Confirm. Handler */
  sixp_timeout_handler_t timeout_handler;   /**< Timeout Handler */
} sixtop_sf_t;
/**
 * \var sixtop_sf_t::sfid
 * managed:   0x00-0xfe
 * unmanaged: 0xf0-0xfe
 * reserved:  0xff
 */


/**
 * \brief Add a Scheduling Function (SF) to 6top Sublayer
 * \param sf The pointer to a Scheduling Function driver
 * \return 0 on success, -1 on failure
 *
 * If a SF which has the same SFID as the specified one has already been added,
 * -1 will be returned.
 *
 * If there is no room to another SF, -1 will be returned as well. You can
 * specify how many SFs can be added with SIXTOP_CONF_MAX_SCHEDULE_FUNCTIONS.
 */
int sixtop_add_sf(const sixtop_sf_t *sf);

/**
 * \brief Find a SF which has been added by SFID
 * \param sfid Scheduling Function Identifier for the concerned SF
 * \return The pointer to a SF having the specified SFID on success, NULL on
 *         failure (not found)
 */
const sixtop_sf_t *sixtop_find_sf(uint8_t sfid);

/**
 * \brief Output a frame stored in packetbuf
 * \param dest_addr Destination address for the outgoing frame
 * \param callback MAC callback function to get the TX result
 * \param arg The pointer to an argument which is returned by the callback
 */
void sixtop_output(const linkaddr_t *dest_addr,
                   mac_callback_t callback, void *arg);

/**
 * \brief Input a frame stored in packetbuf
 * \param llsec_input An upper layer input function which is called at the end
 *                    of sixtop_input()
 */
void sixtop_input(void (*llsec_input)(void));

/**
 * \brief Initialize 6top module
 */
void sixtop_init(void);

/**
 * \brief Initialize SFs which has been added in the system
 */
void sixtop_init_sf(void);

#endif /* !_SIXTOP_H_ */
/** @} */
/** @} */
