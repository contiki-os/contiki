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

#include "net/mac/mac.h"
#include "net/linkaddr.h"

#include "sixp-pkt.h"

/**
 * \brief Input Handler of Scheduling Function
 * \param type 6P Message Type of an input packet
 * \param code Code, 6P Command Identifier or Return Code, of an input packet
 * \param body Body, "Other Fields", of an input packet
 * \param body_len The length of body
 * \param body src_addr Source address of an input packet
 */
typedef void (* sixtop_sf_input)(sixp_pkt_type_t type,
                                 sixp_pkt_code_t code,
                                 const uint8_t *body,
                                 uint16_t body_len,
                                 const linkaddr_t *src_addr);

/**
 * \brief Timeout Handler of Scheduling Function
 * \param cmd 6P Command (Identifier) in process under the transaction
 * \param peer_addr The peer address of the transaction
 */
typedef void (* sixtop_sf_timeout)(sixp_pkt_cmd_t cmd,
                                   const linkaddr_t *peer_addr);
/**
 * /brief Scheduling Function Driver
 */
typedef struct {
  uint8_t sfid;                  /**< SFID */
  clock_time_t timeout_interval; /**< Timeout Value */
  void (*init)(void);            /**< Init Function */
  sixtop_sf_input input;         /**< Input Handler */
  sixtop_sf_timeout timeout;     /**< Transaction Timeout Handler */
} sixtop_sf_t;
/**
 * \var sixtop_sf_t::sfid
 * managed:   0x00-0xfe
 * unmanaged: 0xf0-0xfe
 * reserved:  0xff
 */


/**
 * \brief Add a Scheduling Function (SF) to 6top Sublayer
 * \param sf The pointer to a Scheduling Function Driver
 * \return 0 on success, -1 on failure
 *
 * If there is a SF whose SF is identical to one of a SF specified to this API,
 * the addition will fail and -1 will be returned. If there is no room to
 * another SF, -1 will be returned as well. You can specify how many SFs can be
 * added with SIXTOP_CONF_MAX_SCHEDULING_FUNCTIONS.
 */
int sixtop_add_sf(const sixtop_sf_t *sf);

/**
 * \brief Find a SF which has been added by SFID
 * \param sfid Scheduling Function Identifier of a SF
 * \return The pointer to a SF driver having the specified SFID on success, NULL
 *         on failure (not found)
 */
const sixtop_sf_t *sixtop_find_sf(uint8_t sfid);

/**
 * \brief Output a 6P packet which is supposestored in packetbuf
 * \param dest_addr Destination address of the outgoing packet
 * \param callback MAC callback function to get a TX result
 * \param arg The pointer to an argument which is returned with the MAC callback
 */
void sixtop_output(const linkaddr_t *dest_addr,
                   mac_callback_t callback, void *arg);

/**
 * \brief Input a packet stored in packetbuf
 * \param llsec_input An upper layer input function which is called at the end
 *                    of sixtop_input()
 */
void sixtop_input(void (*llsec_input)(void));

/**
 * \brief Initialize 6top module
 * This initialization function removes all the SFs which has been installed
 * into the 6top sub-layer. In addition, it invokes sixp_init().
 */
void sixtop_init(void);

/**
 * \brief Initialize installed SFs which has been added in the system
 * This function is supposed to be invoked every time the node gets associated.
 */
void sixtop_init_sf(void);

#endif /* !_SIXTOP_H_ */
/** @} */
/** @} */
