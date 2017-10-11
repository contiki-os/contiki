/*
 * Copyright (c) 2017, Arthur Courtel
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
 *
 * Author: Arthur Courtel <arthurcourtel@gmail.com>
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef BLE_ATT_H_
#define BLE_ATT_H_

#include "att-database.h"
/* opcode from spec v4.2 p 2201 and v5 p2211 */
#define ATT_ERROR_RESPONSE                0x01 /* IMPLEMENTED*/
#define ATT_MTU_REQUEST                   0x02 /* IMPLEMENTED*/
#define ATT_MTU_RESPONSE                  0x03 /* IMPLEMENTED*/
#define ATT_INFORMATION_REQUEST           0x04 /* IMPLEMENTED*/
#define ATT_INFORMATION_RESPONSE          0x05 /* IMPLEMENTED*/
#define ATT_FIND_INFO_BY_TYPE_REQUEST     0x06 /* NOT IMPLEMENTED*/
#define ATT_FIND_INFO_BY_TYPE_RESPONSE    0x07 /* NOT IMPLEMENTED*/
#define ATT_READ_BY_TYPE_REQUEST          0x08 /* IMPLEMENTED*/
#define ATT_READ_BY_TYPE_RESPONSE         0x09 /* IMPLEMENTED*/
#define ATT_READ_REQUEST                  0x0A /* IMPLEMENTED*/
#define ATT_READ_RESPONSE                 0x0B /* IMPLEMENTED*/
#define ATT_READ_BLOB_REQUEST             0x0C /* NOT IMPLEMENTED*/
#define ATT_READ_BLOB_RESPONSE            0x0D /* NOT IMPLEMENTED*/
#define ATT_READ_MULTIPLE_REQUEST         0x0E /* NOT IMPLEMENTED*/
#define ATT_READ_MULTIPLE_RESPONSE        0x0F /* NOT IMPLEMENTED*/
#define ATT_READ_BY_GROUP_TYPE_REQUEST    0x10 /* IMPLEMENTED*/
#define ATT_READ_BY_GROUP_TYPE_RESPONSE   0x11 /* IMPLEMENTED*/
#define ATT_WRITE_REQUEST                 0x12 /* IMPLEMENTED*/
#define ATT_WRITE_RESPONSE                0x13 /* IMPLEMENTED*/
#define ATT_WRITE_COMMAND_REQUEST         0x52 /* NOT IMPLEMENTED*/
#define ATT_PREPARE_WRITE_REQUEST         0x16 /* NOT IMPLEMENTED*/
#define ATT_PREPARE_WRITE_RESPONSE        0x17 /* NOT IMPLEMENTED*/
#define ATT_EXECUTE_WRITE_REQUEST         0x18 /* NOT IMPLEMENTED*/
#define ATT_EXECUTE_WRITE_RESPONSE        0x19 /* NOT IMPLEMENTED*/
#define ATT_HANDLE_VALUE_NOTIFICATION     0x1B /* NOT IMPLEMENTED*/
#define ATT_HANDLE_VALUE_INDICATION       0x1D /* IMPLEMENTED*/
#define ATT_HANDLE_VALUE_CONFIRMATION     0x1E /* NOT IMPLEMENTED*/
#define ATT_SIGNED_WRITE_COMMAND          0xD2 /* NOT IMPLEMENTED*/
/*---------------------------------------------------------------------------*/
/* Error codes for Error response PDU see spec V5 p 2182*/
#define ATT_ECODE_INVALID_HANDLE          0x01
#define ATT_ECODE_READ_NOT_PERM           0x02
#define ATT_ECODE_WRITE_NOT_PERM          0x03
#define ATT_ECODE_INVALID_PDU             0x04
#define ATT_ECODE_AUTHENTICATION          0x05
#define ATT_ECODE_REQ_NOT_SUPP            0x06
#define ATT_ECODE_INVALID_OFFSET          0x07
#define ATT_ECODE_AUTHORIZATION           0x08
#define ATT_ECODE_PREP_QUEUE_FULL         0x09
#define ATT_ECODE_ATTR_NOT_FOUND          0x0A
#define ATT_ECODE_ATTR_NOT_LONG           0x0B
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE    0x0C
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN    0x0D
#define ATT_ECODE_UNLIKELY                0x0E
#define ATT_ECODE_INSUFF_ENC              0x0F
#define ATT_ECODE_UNSUPP_GRP_TYPE         0x10
#define ATT_ECODE_INSUFF_RESOURCES        0x11
/*---------------------------------------------------------------------------*/
/* Application error, custom errors */
#define ATT_ECODE_IO                      0x80
#define ATT_ECODE_TIMEOUT                 0x81
#define ATT_ECODE_ABORTED                 0x82
#define ATT_ECODE_SENSOR_READINGS         0x83
#define ATT_ECODE_ACTION_NOT_SET          0x84
#define ATT_ECODE_BAD_NUMBER              0x85
/*---------------------------------------------------------------------------*/
/* ATT default server MTU */
#define ATT_DEFAULT_SERVER_MTU            0x10
/* This SUCCESS isused for control in some functions*/
#define SUCCESS                           0xFF
/* My GATT implementation only works if notify attribute is placed 2 attributes after data sensor attribute*/
#define HANDLE_SPACE_TO_DATA_ATTRIBUTE    2
/*---------------------------------------------------------------------------*/
uint16_t serveur_mtu;

typedef struct {
  /* ATT Service Data Unit (SDU) */
  uint8_t sdu[ATT_DEFAULT_SERVER_MTU];
  /* length of the ATT SDU */
  uint16_t sdu_length;
} att_buffer_t;

#endif /* BLE_ATT_H_ */
