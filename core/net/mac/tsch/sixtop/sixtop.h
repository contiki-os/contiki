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
 * \file
 *         6top Protocol (6P) APIs
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#ifndef __SIXTOP_H__
#define __SIXTOP_H__

#include "net/linkaddr.h"
#include "sys/clock.h"

/*
 * The number of schedule functions which the sixtop module can have at most.
 */
#ifdef SIXTOP_CONF_NUM_OF_SCHEDULE_FUNCTIONS
#define SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS SIXTOP_CONF_NUM_OF_SCHEDULE_FUNCTIONS
#else
#define SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS 1
#endif

/*
 * The maximum number of transactions which the sixtop module can have at the
 * same time.
 */
#ifdef SIXTOP_CONF_NUM_OF_TRANSACTIONS
#define SIXTOP_NUM_OF_TRANSACTIONS SIXTOP_CONF_NUM_OF_TRANSACTIONS
#else
#define SIXTOP_NUM_OF_TRANSACTIONS 1
#endif

/* The initial sequence number used for 6P request */
#define SIXTOP_INITIAL_SEQUENCE_NUMBER 0

/*
 * The maximum number of cells which a request or response message can contain.
 */
#ifdef SIXTOP_CONF_IE_MAX_CELLS
#define SIXTOP_IE_MAX_CELLS SIXTOP_CONF_IE_MAX_CELLS
#else
#define SIXTOP_IE_MAX_CELLS 3
#endif

/* 6P Command ID */
enum sixtop_command_id {
  SIXTOP_CMD_ADD       = 0x01,
  SIXTOP_CMD_DELETE    = 0x02,
  SIXTOP_CMD_COUNT     = 0x03,
  SIXTOP_CMD_LIST      = 0x04,
  SIXTOP_CMD_CLEAR     = 0x05,
};

#define SIXTOP_CMD_MIN SIXTOP_CMD_ADD
#define SIXTOP_CMD_MAX SIXTOP_CMD_CLEAR

/* 6P Return Code */
enum sixtop_return_code {
  SIXTOP_RC_SUCCESS    = 0x06, /* Operation succeeded */
  SIXTOP_RC_ERR_VER    = 0x07, /* Unsupported 6P version */
  SIXTOP_RC_ERR_SFID   = 0x08, /* Unsupported SFID */
  SIXTOP_RC_ERR_GEN    = 0x09,
  SIXTOP_RC_ERR_BUSY   = 0x0a, /* Handling previous request */
  SIXTOP_RC_ERR_NORES  = 0x0b,
  SIXTOP_RC_ERR_RESET  = 0x0c, /* Abort 6P transaction */
  SIXTOP_RC_ERR        = 0x0d, /* Operation failed */
};
#define SIXTOP_RC_MIN SIXTOP_RC_SUCCESS
#define SIXTOP_RC_MAX SIXTOP_RC_ERR

/* type definitions */
typedef enum {
  SIXTOP_RETURN_SUCCESS,
  SIXTOP_RETURN_FAILURE
} sixtop_return_t;

typedef struct {
  uint8_t num_cells;
  uint16_t metadata;
  uint32_t cell_list[SIXTOP_IE_MAX_CELLS];
  uint16_t cell_list_len;
} sixtop_msg_body_t;

typedef void (*sixtop_request_input_t)(uint8_t cmd,
                                       const sixtop_msg_body_t *body,
                                       const linkaddr_t *peer_addr);
typedef void (*sixtop_response_input_t)(uint8_t cmd, uint8_t return_code,
                                        const sixtop_msg_body_t *body,
                                        const linkaddr_t *peer_addr);
typedef void (*sixtop_callback_t)(const sixtop_msg_body_t *body,
                                  const linkaddr_t *dest_addr,
                                  sixtop_return_t status);
typedef int (*sixtop_op_add_t)(linkaddr_t *peer_addr, uint8_t num_cells);
typedef int (*sixtop_op_delete_t)(linkaddr_t *peer_addr, uint8_t num_cells);

typedef struct {
  uint8_t sfid;
  clock_time_t timeout_interval;
  void (*init)(void);
  sixtop_request_input_t request_input;
  sixtop_response_input_t response_input;
  sixtop_op_add_t add;
  sixtop_op_delete_t delete;
} sixtop_sf_t;

typedef struct {
  uint8_t version;
  uint8_t code;
  uint8_t sfid;
  uint8_t seqno;
  uint8_t gab;
  uint8_t gba;
  uint8_t num_cells;
  uint16_t metadata;
  uint32_t cell_list[SIXTOP_IE_MAX_CELLS];
  uint16_t cell_list_len;
  /* there may be more fields to be defined */
} sixtop_ie_t;

/* available Schedule Functions */
#define SIXTOP_SFID_SF_SIMPLE 0
extern const sixtop_sf_t sf_simple_driver;

/* APIs */
/** APIs for upper layers **/
int sixtop_add_sf(const sixtop_sf_t *);
const sixtop_sf_t * sixtop_find_sf(uint8_t sfid);
int sixtop_add_cells(uint8_t sfid, linkaddr_t *peer_addr,
                     uint8_t num_cells);
int sixtop_delete_cells(uint8_t sfid, linkaddr_t *peer_addr,
                        uint8_t num_cells);

/** APIs for Schedule Functions */
void sixtop_response_output(uint8_t sfid, uint8_t cmd,
                            const sixtop_msg_body_t *body,
                            const linkaddr_t *dest_addr,
                            sixtop_callback_t callback);
void sixtop_request_output(uint8_t sfid, uint8_t cmd,
                           const sixtop_msg_body_t *body,
                           const linkaddr_t *dest_addr,
                           sixtop_callback_t callback);

/** APIs for the lower layer, i.e., TSCH MAC **/
void sixtop_input(void (*llsec_input)(void));
void sixtop_init(void);

#endif /* ! __SIXTOP_H__ */
