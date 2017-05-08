/*
 * Copyright (c) 2016, Yasuyuki Tanaka
 * Copyright (c) 2016, Centre for Development of Advanced Computing (C-DAC).
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
 *         6top Protocol (6P) Packet Manipulation APIs
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#ifndef _SIXTOP_6P_PACKET_H_
#define _SIXTOP_6P_PACKET_H_

#define SIXP_PKT_VERSION  0x01

/* typedefs for code readability */
typedef uint8_t sixp_pkt_cell_options_t;
typedef uint8_t sixp_pkt_num_cells_t;
typedef uint8_t sixp_pkt_reserved_t;
typedef uint16_t sixp_pkt_metadata_t;
typedef uint16_t sixp_pkt_max_num_cells_t;
typedef uint16_t sixp_pkt_offset_t;
typedef uint32_t sixp_pkt_cell_t;

/**
 * \brief 6P Message Types
 */
typedef enum  {
  SIXP_PKT_TYPE_REQUEST      = 0x00, /**< 6P Request */
  SIXP_PKT_TYPE_RESPONSE     = 0x01, /**< 6P Response */
  SIXP_PKT_TYPE_CONFIRMATION = 0x02, /**< 6P Confirmation */
  SIXP_PKT_TYPE_RESERVED     = 0x03, /**< Reserved */
} sixp_pkt_type_t;

/**
 * \brief 6P Command Identifiers
 */
typedef enum {
  SIXP_PKT_CMD_ADD          = 0x01, /**< CMD_ADD */
  SIXP_PKT_CMD_DELETE       = 0x02, /**< CMD_DELETE */
  SIXP_PKT_CMD_STATUS       = 0x03, /**< CMD_STATUS */
  SIXP_PKT_CMD_LIST         = 0x04, /**< CMD_LIST */
  SIXP_PKT_CMD_CLEAR        = 0x05, /**< CMD_CLEAR */
  SIXP_PKT_CMD_UNAVAILABLE          /**< for internal use */
} sixp_pkt_cmd_t;

/**
 * \brief 6P Return Codes
 */
typedef enum {
  SIXP_PKT_RC_SUCCESS    = 0x06, /**< RC_SUCCESS */
  SIXP_PKT_RC_ERR_VER    = 0x07, /**< RC_ERR_VER */
  SIXP_PKT_RC_ERR_SFID   = 0x08, /**< RC_ERR_SFID */
  SIXP_PKT_RC_ERR_GEN    = 0x09, /**< RC_ERR_GEN */
  SIXP_PKT_RC_ERR_BUSY   = 0x0a, /**< RC_ERR_BUSY */
  SIXP_PKT_RC_ERR_NORES  = 0x0b, /**< RC_ERR_NORES */
  SIXP_PKT_RC_ERR_RESET  = 0x0c, /**< RC_ERR_RESET */
  SIXP_PKT_RC_ERR        = 0x0d, /**< RC_ERR */
} sixp_pkt_rc_t;

/**
 * \brief 6P Codes integrating Command IDs and Return Codes
 */
typedef union {
  sixp_pkt_cmd_t cmd; /**< 6P Command Identifier */
  sixp_pkt_rc_t rc;   /**< 6P Return Code */
  uint8_t value;      /**< 8-bit unsigned integer value */
} sixp_pkt_code_t;

/**
 * \brief 6P Cell Options
 */
typedef enum {
  SIXP_PKT_CELL_OPTION_TX     = 0x01, /**< TX Cell */
  SIXP_PKT_CELL_OPTION_RX     = 0x02, /**< RX Cell */
  SIXP_PKT_CELL_OPTION_SHARED = 0x04  /**< SHARED Cell */
} sixp_pkt_cell_option_t;

/**
 * \brief 6top IE Structure
 */
typedef struct {
  sixp_pkt_type_t type; /**< Type */
  sixp_pkt_code_t code; /**< Code */
  uint8_t sfid;         /**< SFID */
  uint8_t seqno;        /**< SeqNum */
  uint8_t gab;          /**< GAB */
  uint8_t gba;          /**< GBA */
  const uint8_t *body;  /**< Other Fields... */
  uint16_t body_len;    /**< The length of Other Fields */
} sixp_pkt_t;

/**
 * \brief Write Metadata into "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param metadata Metadata to write
 * \param body The pointer to "Other Fields" in a buffer
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_metadata_t metadata,
                          uint8_t *body, uint16_t body_len);

/**
 * \brief Read Metadata stored in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param metadata The pointer to a buffer to store Metadata in
 * \param body The pointer to the buffer having "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_metadata_t *metadata,
                          const uint8_t *body, uint16_t body_len);

/**
 * \brief Write CellOptions in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_options "CellOptions" to write
 * \param body The pointer to buffer having "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                              sixp_pkt_cell_options_t cell_options,
                              uint8_t *body, uint16_t body_len);

/**
 * \brief Read CellOptions in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_options The pointer to buffer to store CellOptions in
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                              sixp_pkt_cell_options_t *cell_options,
                              const uint8_t *body, uint16_t body_len);

/**
 * \brief Write NumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param num_cells "NumCells" to write
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_num_cells_t num_cells,
                           uint8_t *body, uint16_t body_len);

/**
 * \brief Read NumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param num_cells The pointer to buffer to store NumCells in
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_num_cells_t *num_cells,
                           const uint8_t *body, uint16_t body_len);
/**
 * \brief Write Reserved in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param reserved "Reserved" to write
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_reserved_t reserved,
                          uint8_t *body, uint16_t body_len);

/**
 * \brief Read Reserved in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param reserved The pointer to buffer to store Reserved in
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_reserved_t *reserved,
                          const uint8_t *body, uint16_t body_len);

/**
 * \brief Write Offset in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param offset "Offset" to write
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                        sixp_pkt_offset_t offset,
                        uint8_t *body, uint16_t body_len);
/**
 * \brief Read Offset in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param offset The pointer to buffer to store Offset in
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                        sixp_pkt_offset_t *offset,
                        const uint8_t *body, uint16_t body_len);

/**
 * \brief Write MaxNumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param max_num_cells "MaxNumCells" to write
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_max_num_cells(sixp_pkt_type_t type,
                               sixp_pkt_code_t code,
                               sixp_pkt_max_num_cells_t max_num_cells,
                               uint8_t *body, uint16_t body_len);

/**
 * \brief Read MaxNumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param max_num_cells The pointer to buffer to store MaxNumCells in
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_max_num_cells(sixp_pkt_type_t type,
                               sixp_pkt_code_t code,
                               sixp_pkt_max_num_cells_t *max_num_cells,
                               const uint8_t *body, uint16_t body_len);

/**
 * \brief Write CellList in "Other Fields" of 6P packet
 * \note "offset" is specified by index in CellList
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_list The pointer to "CellList" to write
 * \param cell_list_len Length to write
 * \param offset Offset in the "CellList" field to start writing
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_set_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t offset,
                           uint8_t *body, uint16_t body_len);
/**
 * \brief Read CellList in "Other Fields" of 6P packet
 * \note If you want only the length of CellList, you can set null to cell_list.
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_list The double pointer to store the starting address of CellList
 * \param cell_list_len Pointer to store the length of CellList
 * \param body The pointer to buffer pointing to "Other Fields"
 * \param body_len The length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_get_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t **cell_list,
                           sixp_pkt_offset_t *cell_list_len,
                           const uint8_t *body, uint16_t body_len);

/**
 * \brief Parse a 6P packet
 * \param buf The pointer to a buffer pointing 6top IE Content
 * \param len The length of the buffer
 * \param pkt The pointer to a sixp_pkt_t structure to store packet info
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_parse(const uint8_t *buf, uint16_t len,
                   sixp_pkt_t *pkt);

/**
 * \brief Create a 6P packet
 * \param type 6P Message Type
 * \param code 6P Message Code, Command Identifier or Return Code
 * \param sfid Scheduling Function Identifier
 * \param seqno Sequence Number
 * \param gab GAB
 * \param gba GBA
 * \param body The pointer to "Other Fields" in a buffer
 * \param body_len The length of body, typically "Other Fields" length
 * \param pkt The pointer to a sixp_pkt_t structure to store packet info
 * (option)
 * \return 0 on success, -1 on failure
 */
int sixp_pkt_create(sixp_pkt_type_t type, sixp_pkt_code_t code,
                    uint8_t sfid, uint8_t seqno, uint8_t gab, uint8_t gba,
                    const uint8_t *body, uint16_t body_len,
                    sixp_pkt_t *pkt);

#endif /* !_SIXP_PKT_H_ */
/** @} */
