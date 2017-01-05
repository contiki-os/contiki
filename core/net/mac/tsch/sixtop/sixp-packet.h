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
 *         6top Protocol (6P) Packet Manipulation APIs
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#ifndef _SIXTOP_6P_PACKET_H_
#define _SIXTOP_6P_PACKET_H_

#include "sixp.h"

/* typedefs for code readability */
typedef uint8_t sixp_packet_cell_options_t;
typedef uint8_t sixp_packet_num_cells_t;
typedef uint8_t sixp_packet_reserved_t;
typedef uint16_t sixp_packet_metadata_t;
typedef uint16_t sixp_packet_max_num_cells_t;
typedef uint16_t sixp_packet_offset_t;
typedef uint32_t sixp_packet_cell_t;

/**
 * \brief Read Metadata in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param metadata Pointer to buffer to store Metadata in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_metadata(sixp_type_t type, sixp_code_t code,
                             sixp_packet_metadata_t *metadata,
                             const uint8_t *body, uint16_t body_len);

/**
 * \brief Read CellOptions in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_options Pointer to buffer to store CellOptions in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_cell_options(sixp_type_t type, sixp_code_t code,
                                 sixp_packet_cell_options_t *cell_options,
                                 const uint8_t *body, uint16_t body_len);

/**
 * \brief Read NumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param num_cells Pointer to buffer to store NumCells in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_num_cells(sixp_type_t type, sixp_code_t code,
                              sixp_packet_num_cells_t *num_cells,
                              const uint8_t *body, uint16_t body_len);

/**
 * \brief Read Reserved in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param reserved Pointer to buffer to store Reserved in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_reserved(sixp_type_t type, sixp_code_t code,
                             sixp_packet_reserved_t *reserved,
                             const uint8_t *body, uint16_t body_len);

/**
 * \brief Read Offset in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param offset Pointer to buffer to store Offset in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_offset(sixp_type_t type, sixp_code_t code,
                           sixp_packet_offset_t *offset,
                           const uint8_t *body, uint16_t body_len);

/**
 * \brief Read MaxNumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param max_num_cells Pointer to buffer to store MaxNumCells in
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_max_num_cells(sixp_type_t type, sixp_code_t code,
                                  sixp_packet_max_num_cells_t *max_num_cells,
                                  const uint8_t *body, uint16_t body_len);

/**
 * \brief Read CellList in "Other Fields" of 6P packet
 * \note If you want only the length of CellList, you can set null to cell_list.
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_list Double pointer to store the starting address of CellList
 * \param cell_list_len Pointer to store the length of CellList
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_get_cell_list(sixp_type_t type, sixp_code_t code,
                              const uint8_t **cell_list,
                              sixp_packet_offset_t *cell_list_len,
                              const uint8_t *body, uint16_t body_len);

/**
 * \brief Write Metadata in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param metadata "Metadata" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_metadata(sixp_type_t type, sixp_code_t code,
                             sixp_packet_metadata_t metadata,
                             uint8_t *body, uint16_t body_len);

/**
 * \brief Write CellOptions in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_options "CellOptions" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_cell_options(sixp_type_t type, sixp_code_t code,
                                 sixp_packet_cell_options_t cell_options,
                                 uint8_t *body, uint16_t body_len);

/**
 * \brief Write NumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param num_cells "NumCells" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_num_cells(sixp_type_t type, sixp_code_t code,
                              sixp_packet_num_cells_t num_cells,
                              uint8_t *body, uint16_t body_len);

/**
 * \brief Write MaxNumCells in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param max_num_cells "MaxNumCells" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_max_num_cells(sixp_type_t type, sixp_code_t code,
                                  sixp_packet_max_num_cells_t max_num_cells,
                                  uint8_t *body, uint16_t body_len);

/**
 * \brief Write Offset in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param offset "Offset" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_offset(sixp_type_t type, sixp_code_t code,
                           sixp_packet_offset_t offset,
                           uint8_t *body, uint16_t body_len);

/**
 * \brief Write Reserved in "Other Fields" of 6P packet
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param reserved "Reserved" to write
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_reserved(sixp_type_t type, sixp_code_t code,
                             sixp_packet_reserved_t reserved,
                             uint8_t *body, uint16_t body_len);

/**
 * \brief Write CellList in "Other Fields" of 6P packet
 * \note "offset" is specified by index in CellList
 * \param type 6P Message Type
 * \param code 6P Command Identifier or Return Code
 * \param cell_list Pointer to "CellList" to write
 * \param cell_list_len Length to write
 * \param offset Offset in the "CellList" field to start writing
 * \param body Pointer to buffer having "Other Fields"
 * \param body_len Length of body, typically "Other Fields" length
 * \return 0 on success, -1 on failure
 */
int sixp_packet_set_cell_list(sixp_type_t type, sixp_code_t code,
                              const uint8_t *cell_list,
                              uint16_t cell_list_len,
                              uint16_t offset,
                              uint8_t *body, uint16_t body_len);

/**
 * \brief Create 6P packet in packetbuf
 * \param ie 6top IE contents to be stored in the packet
 * \return 0 on success, -1 on failure
 */
int sixp_packet_create(const sixtop_ie_t *ie);

/**
 * \brief Parse 6P packet in the specified buffer
 * \param buf The pointer to a buffer having 6P packet
 * \param len Length of the 6P packet
 * \param ie The pointer to a sixtop_ie_t object to store 6P packet contents
 * \return 0 on success, -1 on failure
 */
int sixp_packet_parse(const uint8_t *buf, uint16_t len,
                      sixtop_ie_t *ie);

#endif /* !_SIXP_PACKET_H_ */
/** @} */
