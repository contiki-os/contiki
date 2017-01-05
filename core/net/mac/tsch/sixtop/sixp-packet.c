/*
 * Copyright (c) 2016, Yasuyuki Tanaka.
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
 *         6top Protocol (6P) Packet Manipulation
 * \author
 *         Shalu R         <shalur@cdac.in>
 *         Lijo Thomas     <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "lib/assert.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

#include "sixp.h"
#include "sixp-packet.h"

static int create_6top_ie(const sixtop_ie_t *sixtop_ie);

/*---------------------------------------------------------------------------*/
static int
create_6top_ie(const sixtop_ie_t *sixtop_ie)
{
  uint8_t *buf = packetbuf_dataptr();
  uint8_t free_space = PACKETBUF_SIZE - packetbuf_totlen();
  uint16_t len = 0;

  assert(buf != NULL);

  /* fill first 5 octets */
  if(free_space < 5) {
    return -1;
  }

  buf[0] = SIXP_SUBIE_ID;
  buf[1] = (sixtop_ie->type << 4) | SIXP_VERSION;
  buf[2] = sixtop_ie->code.value;
  buf[3] = sixtop_ie->sfid;
  buf[4] = (sixtop_ie->gba << 6) | (sixtop_ie->gab << 4) | sixtop_ie->seqno;

  buf += 5;
  len += 5;

  /* the rest is message body called "Other Fields" */

  if(free_space < len + sixtop_ie->body_len) {
    PRINTF("6top: cannot create 6top IE; sufficient space unavailable\n");
    return -1;
  }

  if(sixtop_ie->body != NULL) {
    memcpy(buf, sixtop_ie->body, sixtop_ie->body_len);
    len += sixtop_ie->body_len;
  }

  packetbuf_set_datalen(len);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_create(const sixtop_ie_t *ie)
{
  struct ieee802154_ies ies;

  packetbuf_clear();

  /* put 6top SubIE Content into packetbuf */
  if(create_6top_ie(ie) < 0) {
    PRINTF("6top: failed to create a 6top SubIE\n");
    return -1;
  }

  memset(&ies, 0, sizeof(ies));
  ies.sixtop_ie_content_ptr = packetbuf_dataptr();
  ies.sixtop_ie_content_len = packetbuf_datalen();

  /* prepend Payload IE header */
  if(packetbuf_hdralloc(2) &&
     frame80215e_create_ie_ietf(packetbuf_hdrptr(),
                                packetbuf_totlen(),
                                &ies) < 0) {
    PRINTF("6top: failed to create a Payload IE header for IETF IE\n");
    return -1;
  }

  /* prepend Termination 1 IE */
  if(packetbuf_hdralloc(2) &&
     frame80215e_create_ie_header_list_termination_1(packetbuf_hdrptr(),
                                                     2,
                                                     &ies) < 0) {
    PRINTF("6top: failed to create a Header Termination 1\n");
    return -1;
  }

  /* specify with PACKETBUF_ATTR_METADATA that packetbuf has IEs */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_METADATA, 1);

  /* 6P packet is data frame */
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_parse(const uint8_t *buf, uint16_t len,
                  sixtop_ie_t *sixtop_ie)
{
  assert(buf != NULL && sixtop_ie != NULL);

  /* read the first 4 octets */
  if(len < 4) {
    PRINTF("6top: input SubIE is too short\n");
    return -1;
  }

  if((buf[0] & 0x0f) != SIXP_VERSION) {
    PRINTF("6top: invalid version %u\n", buf[0] & 0x0f);
    return -1;
  }

  memset(sixtop_ie, 0, sizeof(sixtop_ie_t));
  sixtop_ie->type = (buf[0] & 0x30) >> 4;
  sixtop_ie->code.value = buf[1];
  sixtop_ie->sfid = buf[2];
  sixtop_ie->seqno = buf[3] & 0x0f;
  sixtop_ie->gab = (buf[3] & 0x30) >> 4;
  sixtop_ie->gba = (buf[3] & 0xc0) >> 6;

  buf += 4;
  len -= 4;

  /* the rest is message body called "Other Fields" */
  if(sixtop_ie->type == SIXP_TYPE_REQUEST) {
    switch(sixtop_ie->code.cmd) {
      case SIXP_CMD_ADD:
      case SIXP_CMD_DELETE:
        /* Add and Delete has the same request format */
        if(len < (sizeof(sixp_packet_metadata_t) +
                  sizeof(sixp_packet_cell_options_t) +
                  sizeof(sixp_packet_num_cells_t)) ||
           (len % sizeof(uint32_t)) != 0) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_STATUS:
        if(len != (sizeof(sixp_packet_metadata_t) +
                   sizeof(sixp_packet_cell_options_t))) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_LIST:
        if(len != (sizeof(sixp_packet_metadata_t) +
                   sizeof(sixp_packet_cell_options_t) +
                   sizeof(sixp_packet_reserved_t) +
                   sizeof(sixp_packet_offset_t) +
                   sizeof(sixp_packet_max_num_cells_t))) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_CLEAR:
        if(len != sizeof(sixp_packet_metadata_t)) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      default:
        PRINTF("6top: unsupported request [code=%u]\n", sixtop_ie->code.cmd);
        return -1;
    }
  } else if(sixtop_ie->type == SIXP_TYPE_RESPONSE ||
            sixtop_ie->type == SIXP_TYPE_CONFIRMATION) {
    switch(sixtop_ie->code.rc) {
      case SIXP_RC_SUCCESS:
        /*
         * The "Other Field" contains
         * - Res to CLEAR:             Empty (length 0)
         * - Res to STATUS:            "Num. Cells"
         * - Res to ADD, DELETE, LIST: 0, 1, or multiple 6P cells
         */
        if(len != 0 &&
           len != sizeof(sixp_packet_num_cells_t) &&
           (len % sizeof(uint32_t)) != 0) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.rc, len);
          return -1;
        }
        break;
      case SIXP_RC_ERR_VER:
      case SIXP_RC_ERR_SFID:
      case SIXP_RC_ERR_GEN:
      case SIXP_RC_ERR_BUSY:
      case SIXP_RC_ERR_NORES:
      case SIXP_RC_ERR_RESET:
      case SIXP_RC_ERR:
        if(len != 0) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.rc, len);
          return -1;
        }
        break;
      default:
        PRINTF("6top: unsupported response/confirm [code=%u]\n",
               sixtop_ie->code.rc);
        return -1;
    }
  } else {
    PRINTF("6top: unsupported type [type=%u]\n", sixtop_ie->type);
    return -1;
  }

  sixtop_ie->body = buf;
  sixtop_ie->body_len = len;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_metadata(sixp_type_t type, sixp_code_t code,
                         sixp_packet_metadata_t *metadata,
                         const uint8_t *body, uint16_t body_len)
{
  if(metadata == NULL || body == NULL) {
    PRINTF("6top: cannot get metadata; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    PRINTF("6top: cannot get metadata; response won't have Metadata\n");
    return -1;
  }

  /*
   * Every request format has the Metadata field at the beginning of the "Other
   * Fields" field.
   */
  if(body_len < sizeof(*metadata)) {
    PRINTF("6top: cannot get metadata; body is too short\n");
    return -1;
  }

  /*
   * Copy the content in the Metadata field as it is since 6P has no idea about
   * the internal structure of the field.
   */
  memcpy(metadata, body, sizeof(*metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_cell_options(sixp_type_t type, sixp_code_t code,
                             sixp_packet_cell_options_t *cell_options,
                             const uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(cell_options == NULL || body == NULL) {
    PRINTF("6top: cannot get cell_options; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    PRINTF("6top: cannot get cell_options; response won't have CellOptions\n");
    return -1;
  }

  switch(code.value) {
    case SIXP_CMD_ADD:
    case SIXP_CMD_DELETE:
    case SIXP_CMD_STATUS:
      offset = sizeof(sixp_packet_metadata_t);
      break;
    default:
      PRINTF("6top: cannot get cell_options; ");
      PRINTF("packet [type=%u, code=%u] won't have CellOptions\n",
             type, code.value);
      return -1;
  }

  if(body_len < (offset + sizeof(*cell_options))) {
    PRINTF("6top: cannot get cell_options; body is too short\n");
    return -1;
  }

  /* The CellOption field is an 8-bit bitfield */
  memcpy(cell_options, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_num_cells(sixp_type_t type, sixp_code_t code,
                          sixp_packet_num_cells_t *num_cells,
                          const uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(num_cells == NULL || body == NULL) {
    PRINTF("6top: cannot get num_cells; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    offset = 0;
  } else if (code.value == SIXP_CMD_ADD || code.value == SIXP_CMD_DELETE) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t);
  } else {
    PRINTF("6top: cannot get num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have NumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*num_cells))) {
    PRINTF("6top: cannot get num_cells; body is too short\n");
    return -1;
  }

  /* NumCells is an 8-bit unsigned integer */
  memcpy(num_cells, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_reserved(sixp_type_t type, sixp_code_t code,
                         sixp_packet_reserved_t *reserved,
                         const uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(reserved == NULL || body == NULL) {
    PRINTF("6top: cannot get reserved; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t);
  } else {
    PRINTF("6top: cannot get reserved; ");
    PRINTF("packet [type=%u, code=%u] won't have Reserved\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*reserved))) {
    PRINTF("6top: cannot get reserved; body is too short\n");
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(reserved, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_offset(sixp_type_t type, sixp_code_t code,
                       sixp_packet_offset_t *cell_offset,
                       const uint8_t *body, uint16_t body_len)
{
  uint16_t offset;
  const uint8_t *p;

  if(cell_offset == NULL || body == NULL) {
    PRINTF("6top: cannot get offset; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_reserved_t);
  } else {
    PRINTF("6top: cannot get offset; ");
    PRINTF("packet [type=%u, code=%u] won't have Offset\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*cell_offset))) {
    PRINTF("6top: cannot get offset; body is too short\n");
    return -1;
  }

  /*
   * The (Cell)Offset field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  p = body + offset;
  *((uint16_t *)cell_offset) = p[0] + (p[1] << 8);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_max_num_cells(sixp_type_t type, sixp_code_t code,
                              sixp_packet_max_num_cells_t *max_num_cells,
                              const uint8_t *body, uint16_t body_len)
{
  uint16_t offset;
  const uint8_t *p;

  if(max_num_cells == NULL || body == NULL) {
    PRINTF("6top: cannot get max_num_cells; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_reserved_t) +
      sizeof(sixp_packet_offset_t);
  } else {
    PRINTF("6top: cannot get max_num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have MaxNumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*max_num_cells))) {
    PRINTF("6top: cannot get max_num_cells; body is too short\n");
    return -1;
  }

  /*
   * The MaxNumCells field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  p = body + offset;
  *((uint16_t *)max_num_cells) = p[0] + (p[1] << 8);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_get_cell_list(sixp_type_t type, sixp_code_t code,
                          const uint8_t **cell_list,
                          sixp_packet_offset_t *cell_list_len,
                          const uint8_t *body, uint16_t body_len)
{
  int offset;

  if(cell_list_len == NULL || body == NULL) {
    PRINTF("6top: cannot get cell_list; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    offset = 0;
  } else if (code.value == SIXP_CMD_ADD || code.value == SIXP_CMD_DELETE) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_num_cells_t);
  } else {
    PRINTF("6top: cannot get cell_list; ");
    PRINTF("packet [type=%u, code=%u] won't have CellList\n",
           type, code.value);
    return -1;
  }

  if(body_len < offset ||
     ((body_len - offset) % sizeof(sixp_packet_cell_t)) != 0) {
    PRINTF("6top: cannot get max_num_cells; invalid body_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_metadata(sixp_type_t type, sixp_code_t code,
                         sixp_packet_metadata_t metadata,
                         uint8_t *body, uint16_t body_len)
{
  if(body == NULL) {
    PRINTF("6top: cannot set metadata; body is null\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    PRINTF("6top: cannot set metadata; response won't have Metadata\n");
    return -1;
  }

  /*
   * Every request format has the Metadata field at the beginning of the "Other
   * Fields" field.
   */
  if(body_len < sizeof(metadata)) {
    PRINTF("6top: cannot set metadata; body is too short\n");
    return -1;
  }

  /*
   * Copy the content into the Metadata field as it is since 6P has no idea
   * about the internal structure of the field.
   */
  memcpy(body, &metadata, sizeof(metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_cell_options(sixp_type_t type, sixp_code_t code,
                             sixp_packet_cell_options_t cell_options,
                             uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(body == NULL) {
    PRINTF("6top: cannot set cell_options; body is null\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    PRINTF("6top: cannot set cell_options; response won't have CellOptions\n");
    return -1;
  }

  switch(code.value) {
    case SIXP_CMD_ADD:
    case SIXP_CMD_DELETE:
    case SIXP_CMD_STATUS:
      offset = sizeof(sixp_packet_metadata_t);
      break;
    default:
      PRINTF("6top: cannot set cell_options; ");
      PRINTF("packet [type=%u, code=%u] won't have CellOptions\n",
             type, code.value);
      return -1;
  }

  if(body_len < (offset + sizeof(cell_options))) {
    PRINTF("6top: cannot set cell_options; body is too short\n");
    return -1;
  }

  /* The CellOption field is an 8-bit bitfield */
  memcpy(body + offset, &cell_options, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_num_cells(sixp_type_t type, sixp_code_t code,
                          sixp_packet_num_cells_t num_cells,
                          uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(body == NULL) {
    PRINTF("6top: cannot set num_cells; body is null\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE) {
    offset = 0;
  } else if (code.value == SIXP_CMD_ADD || code.value == SIXP_CMD_DELETE) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t);
  } else {
    PRINTF("6top: cannot set num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have NumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(num_cells))) {
    PRINTF("6top: cannot set num_cells; body is too short\n");
    return -1;
  }

  /* NumCells is an 8-bit unsigned integer */
  memcpy(body + offset, &num_cells, sizeof(num_cells));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_reserved(sixp_type_t type, sixp_code_t code,
                         sixp_packet_reserved_t reserved,
                         uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(body == NULL) {
    PRINTF("6top: cannot set reserved; body is null\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t);
  } else {
    PRINTF("6top: cannot set reserved; ");
    PRINTF("packet [type=%u, code=%u] won't have Reserved\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(reserved))) {
    PRINTF("6top: cannot set reserved; body is too short\n");
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(body + offset, &reserved, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_offset(sixp_type_t type, sixp_code_t code,
                       sixp_packet_offset_t cell_offset,
                       uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(body == NULL) {
    PRINTF("6top: cannot set offset; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_reserved_t);
  } else {
    PRINTF("6top: cannot set offset; ");
    PRINTF("packet [type=%u, code=%u] won't have Offset\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(cell_offset))) {
    PRINTF("6top: cannot set offset; body is too short\n");
    return -1;
  }

  /*
   * The (Cell)Offset field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)cell_offset) & 0xff;
  (body + offset)[1] = (*((uint16_t *)cell_offset) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_max_num_cells(sixp_type_t type, sixp_code_t code,
                              sixp_packet_max_num_cells_t max_num_cells,
                              uint8_t *body, uint16_t body_len)
{
  uint16_t offset;

  if(body == NULL) {
    PRINTF("6top: cannot set max_num_cells; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST &&
     code.value == SIXP_CMD_LIST) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_reserved_t) +
      sizeof(sixp_packet_offset_t);
  } else {
    PRINTF("6top: cannot set max_num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have MaxNumCells\n",
           type, code.value);
  }

  if(body_len < (offset + sizeof(max_num_cells))) {
    PRINTF("6top: cannot set max_num_cells; body is too short\n");
    return -1;
  }

  /*
   * The MaxNumCells field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)max_num_cells) & 0xff;
  (body + offset)[1] = (*((uint16_t *)max_num_cells) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_packet_set_cell_list(sixp_type_t type, sixp_code_t code,
                          const uint8_t *cell_list,
                          uint16_t cell_list_len,
                          uint16_t cell_offset,
                          uint8_t *body, uint16_t body_len)
{
  int offset;

  if(cell_list == NULL || body == NULL) {
    PRINTF("6top: cannot set cell_list; invalid argument\n");
    return -1;
  }

  if(type == SIXP_TYPE_RESPONSE && code.value == SIXP_RC_SUCCESS) {
    offset = 0;
  } else if (type == (SIXP_TYPE_REQUEST &&
                      (code.value == SIXP_CMD_ADD ||
                       code.value == SIXP_CMD_DELETE))) {
    offset =
      sizeof(sixp_packet_metadata_t) +
      sizeof(sixp_packet_cell_options_t) +
      sizeof(sixp_packet_num_cells_t);
  } else {
    PRINTF("6top: cannot set cell_list; ");
    PRINTF("packet [type=%u, code=%u] won't have CellList\n",
           type, code.value);
    return -1;
  }

  offset += cell_offset * sizeof(sixp_packet_cell_t);

  if(body_len < (offset + cell_list_len) ||
     (cell_list_len % sizeof(sixp_packet_cell_t)) != 0) {
    PRINTF("6top: cannot set max_num_cells; invalid {body, cell_list}_len\n");
    return -1;
  }

  memcpy(body + offset, cell_list, cell_list_len);

  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
