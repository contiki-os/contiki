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
 * \addtogroup sixtop
 * @{
 */
/**
 * \file
 *         6top Protocol (6P) APIs
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#ifndef _SIXP_H_
#define _SIXP_H_

#include "net/linkaddr.h"
#include "sys/clock.h"

#include "sixp-pkt.h"

#define SIXP_SUBIE_ID 0x00

/**
 * \brief The initial sequence number used for 6P request
 */
#define SIXP_INITIAL_SEQUENCE_NUMBER 0

/**
 * \brief 6P Send Status, which represents sixp_output() result.
 */
typedef enum {
  SIXP_OUTPUT_STATUS_SUCCESS, /**< SUCCESS */
  SIXP_OUTPUT_STATUS_FAILURE  /**< FAILURE */
} sixp_output_status_t;

/**
 * \brief 6P Packet Sent Handler
 */
typedef void (*sixp_sent_callback_t)(void *arg, uint16_t arg_len,
                                     const linkaddr_t *dest_addr,
                                     sixp_output_status_t status);

/**
 * \brief Input a 6P packet
 * \param buf The pointer to a buffer pointing the head of 6top IE Content
 * \param len The lengh of 6top IE Content
 * \param src_addr The Source address of an incoming packet
 * \return 0 if , -1 on failure
 */
void sixp_input(const uint8_t *buf, uint16_t len,
                const linkaddr_t *src_addr);

/**
 * \brief Output a 6P packet
 * \param type Message Type
 * \param code Message Code; Command ID or Return Code
 * \param sfid Scheduling Function Identifier
 * \param body 6top IE Content
 * \param body_len The length of 6top IE Content
 * \param dest_addr The destination Address
 * \param func callback function invoked after the transmission process
 * \param arg The pointer to an argument to be passed with the callback
 * \param arg_len The length of the argument
 * \return 0 on success, -1 on failure
 */
int sixp_output(sixp_pkt_type_t type, sixp_pkt_code_t code, uint8_t sfid,
                const uint8_t *body, uint16_t body_len,
                const linkaddr_t *dest_addr,
                sixp_sent_callback_t func, void *arg, uint16_t arg_len);

/**
 * \brief Initialize 6P Module
 * It invokes sixp_nbr_init() and sixp_trans_init().
 */
void sixp_init(void);
#endif /* ! _SIXP_H_ */
/** @} */
