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

#define SIXP_SUBIE_ID 0x00
#define SIXP_VERSION  0x01

/**
 * \brief 6P Message Types
 */
typedef enum  {
  SIXP_TYPE_REQUEST      = 0b00, /**< 6P Request */
  SIXP_TYPE_RESPONSE     = 0b01, /**< 6P Response */
  SIXP_TYPE_CONFIRMATION = 0b10, /**< 6P Confirmation */
  SIXP_TYPE_RESERVED     = 0b11, /**< Reserved */
} sixp_type_t;

/**
 * \brief 6P Command Identifiers
 */
typedef enum {
  SIXP_CMD_ADD       = 0x01, /**< CMD_ADD */
  SIXP_CMD_DELETE    = 0x02, /**< CMD_DELETE */
  SIXP_CMD_STATUS    = 0x03, /**< CMD_STATUS */
  SIXP_CMD_LIST      = 0x04, /**< CMD_LIST */
  SIXP_CMD_CLEAR     = 0x05, /**< CMD_CLEAR */
} sixp_command_id_t;

/**
 * \brief 6P Return Codes
 */
typedef enum {
  SIXP_RC_SUCCESS    = 0x06, /**< RC_SUCCESS */
  SIXP_RC_ERR_VER    = 0x07, /**< RC_ERR_VER */
  SIXP_RC_ERR_SFID   = 0x08, /**< RC_ERR_SFID */
  SIXP_RC_ERR_GEN    = 0x09, /**< RC_ERR_GEN */
  SIXP_RC_ERR_BUSY   = 0x0a, /**< RC_ERR_BUSY */
  SIXP_RC_ERR_NORES  = 0x0b, /**< RC_ERR_NORES */
  SIXP_RC_ERR_RESET  = 0x0c, /**< RC_ERR_RESET */
  SIXP_RC_ERR        = 0x0d, /**< RC_ERR */
} sixp_return_code_t;

/**
 * \brief 6P Codes integrating Command IDs and Return Codes
 */
typedef union {
  sixp_command_id_t cmd; /**< 6P Command Identifier */
  sixp_return_code_t rc; /**< 6P Return Code */
  uint8_t value;         /**< 8-bit unsigned integer value */
} sixp_code_t;

/**
 * \brief 6P Cell Options
 */
typedef enum {
  SIXP_CELL_OPTION_TX     = 0x01, /**< TX Cell */
  SIXP_CELL_OPTION_RX     = 0x02, /**< RX Cell */
  SIXP_CELL_OPTION_SHARED = 0x04  /**< SHARED Cell */
} sixp_cell_option_t;

/**
 * \brief 6P Schedule Generation Type
 */
typedef enum {
  SIXP_GEN_TYPE_TX, /**< TX Cell */
  SIXP_GEN_TYPE_RX  /**< RX Cell */
} sixp_gen_type_t;


/**
 * \brief 6P Send Status, which represents sixp_send() result.
 */
typedef enum {
  SIXP_SEND_STATUS_SUCCESS, /**< SUCCESS */
  SIXP_SEND_STATUS_FAILURE  /**< FAILURE */
} sixp_send_status_t;

/**
 * \brief 6P Add Request Format
 */
typedef struct {
  uint16_t metadata;     /**< Metadata */
  uint8_t cell_options;  /**< CellOptions */
  uint8_t num_cells;     /**< NumCells */
  uint32_t cell_list[0]; /**< CellList */
} __attribute__((packed)) sixp_req_add_t;

/**
 * \brief 6P Delete Request Format
 */
typedef sixp_req_add_t sixp_req_delete_t;

/**
 * \brief 6P Status Request Format
 */
typedef struct {
  uint16_t metadata;    /**< Metadata */
  uint8_t cell_options; /**< CellOptions */
} __attribute__((packed)) sixp_req_status_t;

/**
 * \brief 6P List Request Format
 */
typedef struct {
  uint16_t metadata;      /**< Metadata */
  uint8_t cell_options;   /**< CellOptions */
  uint8_t reserved;       /**< Reserved */
  uint16_t offset;        /**< Offset */
  uint16_t max_num_cells; /**< MaxNumCells */
} __attribute__((packed)) sixp_req_list_t;

/**
 * \brief 6P Clear Request Format
 */
typedef struct {
  uint16_t metadata; /**< Metadata */
} __attribute__((packed)) sixp_req_clear_t;

/**
 * \brief 6P Response Format to Add Request
 */
typedef struct {
  uint32_t cell_list[0]; /**< CellList */
} __attribute__((packed)) sixp_res_add_t;

/**
 * \brief 6P Response Format to Delete Request
 */
typedef sixp_res_add_t sixp_res_delete_t;

/**
 * \brief 6P Response Format to Status Request
 */
typedef struct {
  uint8_t num_cells; /**< NumCells */
} __attribute__((packed)) sixp_res_status_t;

/**
 * \brief 6P Response Format to List Request
 */
typedef sixp_res_add_t sixp_res_list_t;

/**
 * \brief 6P Response Format to Clear Request
 */
typedef struct {
} __attribute__((packed)) sixp_res_clear_t;


/**
 * \brief 6top IE Message Body
 */
typedef union {
  sixp_req_add_t add_req;       /**< Add Request */
  sixp_req_delete_t delete_req; /**< Delete Request */
  sixp_req_status_t status_req; /**< Status Request */
  sixp_req_list_t list_req;     /**< List Request */
  sixp_req_clear_t clear_req;   /**< Clear Request */

  sixp_res_add_t add_res;       /**< Add Response */
  sixp_res_delete_t delete_res; /**< Delete Response */
  sixp_res_status_t status_res; /**< Status Response */
  sixp_res_list_t list_res;     /**< List Response */
  sixp_res_clear_t clear_res;   /**< Clear Response */
} sixtop_ie_body_t;

/**
 * \brief 6top IE Structure (not packed)
 */
typedef struct {
  sixp_type_t type;             /**< Type */
  sixp_code_t code;             /**< Code */
  uint8_t sfid;                 /**< SFID */
  uint8_t seqno;                /**< SeqNum */
  uint8_t gab;                  /**< GAB */
  uint8_t gba;                  /**< GBA */
  const sixtop_ie_body_t *body; /**< Other Fields... */
  uint16_t body_len;            /**< Length of Other Fields */
} sixtop_ie_t;

/**
 * \brief 6P Request Input Handler
 */
typedef void (*sixp_request_input_t)(sixp_command_id_t cmd,
                                     const sixtop_ie_body_t *body,
                                     uint16_t body_len,
                                     const linkaddr_t *peer_addr);
/**
 * \brief 6P Response (and Confirmation) Input Handler
 */
typedef void (*sixp_response_input_t)(sixp_command_id_t cmd,
                                      sixp_return_code_t return_code,
                                      const sixtop_ie_body_t *body,
                                      uint16_t body_len,
                                      const linkaddr_t *peer_addr);

/**
 * \brief 6P Timeout Handler
 */
typedef void (*sixp_timeout_handler_t)(uint8_t cmd,
                                       const linkaddr_t *dest_addr);

/**
 * \brief 6P Packet Sent Handler
 */
typedef void (*sixp_sent_callback_t)(void *arg, uint16_t arg_len,
                                     const linkaddr_t *dest_addr,
                                     sixp_send_status_t status);

/**
 * \brief Send a 6top IE
 * \param type Message Type
 * \param code Message Code; Command ID or Return Code
 * \param sfid Schedule Function Identifier
 * \param body 6top IE Message Body
 * \param body_len The length of Message Body
 * \param dest_addr Destination Address
 * \param func Callback Function invoked after the transmission process
 * \param arg The pointer to an argument to be passed to the Callback Function
 * \param arg_len The length of the argument
 * \return 0 on success, -1 on failure
 */
int sixp_send(sixp_type_t type, sixp_code_t code, uint8_t sfid,
              const sixtop_ie_body_t *body, uint16_t body_len,
              const linkaddr_t *dest_addr,
              sixp_sent_callback_t func, void *arg, uint16_t arg_len);


/**
 * \brief Advance Schedule Generation Counter
 * \param peer_addr Peer Address with whom it shares a concerned counter
 * \param type Type to specify which counter is advanced: TX or RX
 * \return 0 on success, -1 on failure
 */
int sixp_advance_generation(const linkaddr_t *peer_addr,
                            sixp_gen_type_t type);

/**
 * \brief Initialize 6P Module
 */
void sixp_init(void);


/**
 * \brief Input 6top IE
 * \param buf The pointer to a buffer storing a 6top IE
 * \param len The lengh of the 6top IE
 * \param src_addr Source address of the 6top IE
 */
void sixp_input(const uint8_t *buf, uint16_t len,
                const linkaddr_t *src_addr);

#endif /* ! _SIXP_H_ */
/** @} */
