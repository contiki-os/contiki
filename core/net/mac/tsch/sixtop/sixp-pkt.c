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

#include "sixp.h"
#include "sixp-pkt.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

static int32_t get_metadata_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_cell_options_offset(sixp_pkt_type_t type,
                                       sixp_pkt_code_t code);
static int32_t get_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_reserved_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_offset_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_max_num_cells_offset(sixp_pkt_type_t type,
                                        sixp_pkt_code_t code);
static int32_t get_cell_list_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
/*---------------------------------------------------------------------------*/
static int32_t
get_metadata_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST) {
    return 0; /* offset */
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_cell_options_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     (code.cmd == SIXP_PKT_CMD_ADD ||
      code.cmd == SIXP_PKT_CMD_DELETE ||
      code.cmd == SIXP_PKT_CMD_STATUS ||
      code.cmd == SIXP_PKT_CMD_LIST)) {
    return sizeof(sixp_pkt_metadata_t);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     (code.value == SIXP_PKT_CMD_ADD || code.value == SIXP_PKT_CMD_DELETE)) {
    return sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t);
  } else if(type == SIXP_PKT_TYPE_RESPONSE &&
            code.value == SIXP_PKT_RC_SUCCESS) {
    return 0;
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_reserved_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_offset_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_reserved_t));
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_max_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_reserved_t) +
            sizeof(sixp_pkt_offset_t));
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_cell_list_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ADD ||
                                       code.value == SIXP_PKT_CMD_DELETE)) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_num_cells_t));
  } else if((type == SIXP_PKT_TYPE_RESPONSE ||
             type == SIXP_PKT_TYPE_CONFIRMATION) &&
            code.value == SIXP_PKT_RC_SUCCESS) {
    return 0;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_metadata_t metadata,
                      uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set metadata; body is null\n");
    return -1;
  }

  if((offset = get_metadata_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set metadata [type=%u, code=%u], invalid type\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(metadata))) {
    PRINTF("6P-pkt: cannot set metadata, body is too short [body_len=%u]\n",
           body_len);
    return -1;
  }

  /*
   * Copy the content into the Metadata field as it is since 6P has no idea
   * about the internal structure of the field.
   */
  memcpy(body + offset, &metadata, sizeof(metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_metadata_t *metadata,
                      const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(metadata == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get metadata; invalid argument\n");
    return -1;
  }

  if((offset = get_metadata_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get metadata [type=%u, code=%u], invalid type\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*metadata))) {
    PRINTF("6P-pkt: cannot get metadata [type=%u, code=%u], body is too short\n",
           type, code.value);
    return -1;
  }

  /*
   * Copy the content in the Metadata field as it is since 6P has no idea about
   * the internal structure of the field.
   */
  memcpy(metadata + offset, body, sizeof(*metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_cell_options_t cell_options,
                          uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set cell_options; body is null\n");
    return -1;
  }

  if((offset = get_cell_options_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set cell_options [type=%u, code=%u], invalid type\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(cell_options))) {
    PRINTF("6P-pkt: cannot set cell_options, body is too short [body_len=%u]\n",
           body_len);
    return -1;
  }

  /* The CellOption field is an 8-bit bitfield */
  memcpy(body + offset, &cell_options, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_cell_options_t *cell_options,
                          const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(cell_options == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get cell_options; invalid argument\n");
    return -1;
  }

  if((offset = get_cell_options_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get cell_options [type=%u, code=%u], invalid type\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*cell_options))) {
    PRINTF("6P-pkt: cannot get cell_options, body is too short [body_len=%u]\n",
           body_len);
    return -1;
  }

  /* The CellOption field is an 8-bit bitfield */
  memcpy(cell_options, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       sixp_pkt_num_cells_t num_cells,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set num_cells; body is null\n");
    return -1;
  }

  if((offset = get_num_cells_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have NumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(num_cells))) {
    PRINTF("6P-pkt: cannot set num_cells; body is too short\n");
    return -1;
  }

  /* NumCells is an 8-bit unsigned integer */
  memcpy(body + offset, &num_cells, sizeof(num_cells));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       sixp_pkt_num_cells_t *num_cells,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(num_cells == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_num_cells_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have NumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*num_cells))) {
    PRINTF("6P-pkt: cannot get num_cells; body is too short\n");
    return -1;
  }

  /* NumCells is an 8-bit unsigned integer */
  memcpy(num_cells, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_reserved_t reserved,
                      uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set reserved; body is null\n");
    return -1;
  }

  if((offset = get_reserved_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set reserved; ");
    PRINTF("packet [type=%u, code=%u] won't have Reserved\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(reserved))) {
    PRINTF("6P-pkt: cannot set reserved; body is too short\n");
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(body + offset, &reserved, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_reserved_t *reserved,
                      const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(reserved == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get reserved; invalid argument\n");
    return -1;
  }

  if((offset = get_reserved_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get reserved; ");
    PRINTF("packet [type=%u, code=%u] won't have Reserved\n",
           type, code.value);
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(reserved, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                    sixp_pkt_offset_t cell_offset,
                    uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set offset; invalid argument\n");
    return -1;
  }

  if((offset = get_offset_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set offset; ");
    PRINTF("packet [type=%u, code=%u] won't have Offset\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(cell_offset))) {
    PRINTF("6P-pkt: cannot set offset; body is too short\n");
    return -1;
  }

  /*
   * The (Cell)Offset field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)&cell_offset) & 0xff;
  (body + offset)[1] = (*((uint16_t *)&cell_offset) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                    sixp_pkt_offset_t *cell_offset,
                    const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  const uint8_t *p;

  if(cell_offset == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get offset; invalid argument\n");
    return -1;
  }

  if((offset = get_offset_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get offset; ");
    PRINTF("packet [type=%u, code=%u] won't have Offset\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*cell_offset))) {
    PRINTF("6P-pkt: cannot get offset; body is too short\n");
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
sixp_pkt_set_max_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_max_num_cells_t max_num_cells,
                           uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    PRINTF("6P-pkt: cannot set max_num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_max_num_cells_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set max_num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have MaxNumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(max_num_cells))) {
    PRINTF("6P-pkt: cannot set max_num_cells; body is too short\n");
    return -1;
  }

  /*
   * The MaxNumCells field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)&max_num_cells) & 0xff;
  (body + offset)[1] = (*((uint16_t *)&max_num_cells) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_max_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_max_num_cells_t *max_num_cells,
                           const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  const uint8_t *p;

  if(max_num_cells == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get max_num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_max_num_cells_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get max_num_cells; ");
    PRINTF("packet [type=%u, code=%u] won't have MaxNumCells\n",
           type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*max_num_cells))) {
    PRINTF("6P-pkt: cannot get max_num_cells; body is too short\n");
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
sixp_pkt_set_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *cell_list,
                       uint16_t cell_list_len,
                       uint16_t cell_offset,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(cell_list == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot set cell_list; invalid argument\n");
    return -1;
  }

  if((offset = get_cell_list_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot set cell_list; ");
    PRINTF("packet [type=%u, code=%u] won't have CellList\n",
           type, code.value);
    return -1;
  }

  offset += cell_offset * sizeof(sixp_pkt_cell_t);

  if(body_len < (offset + cell_list_len)) {
    PRINTF("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
    PRINTF("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  memcpy(body + offset, cell_list, cell_list_len);
  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(cell_list_len == NULL || body == NULL) {
    PRINTF("6P-pkt: cannot get cell_list; invalid argument\n");
    return -1;
  }

  if((offset = get_cell_list_offset(type, code)) < 0) {
    PRINTF("6P-pkt: cannot get cell_list; ");
    PRINTF("packet [type=%u, code=%u] won't have CellList\n",
           type, code.value);
    return -1;
  }

  if(body_len < offset) {
    PRINTF("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    PRINTF("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
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
sixp_pkt_parse(const uint8_t *buf, uint16_t len,
               sixp_pkt_t *pkt)
{
  assert(buf != NULL && pkt != NULL);
  if(buf == NULL || pkt == NULL) {
    PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid argument\n");
    return -1;
  }

  /* read the first 4 octets */
  if(len < 4) {
    PRINTF("6P-pkt: sixp_pkt_parse() fails because it's a too short packet\n");
    return -1;
  }

  if((buf[0] & 0x0f) != SIXP_PKT_VERSION) {
    PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid version [%u]\n",
           buf[0] & 0x0f);
    return -1;
  }

  memset(pkt, 0, sizeof(sixp_pkt_t));
  pkt->type = (buf[0] & 0x30) >> 4;
  pkt->code.value = buf[1];
  pkt->sfid = buf[2];
  pkt->seqno = buf[3] & 0x0f;
  pkt->gab = (buf[3] & 0x30) >> 4;
  pkt->gba = (buf[3] & 0xc0) >> 6;

  buf += 4;
  len -= 4;

  PRINTF("6P-pkt: sixp_pkt_parse() is processing [type:%u, code:%u, len:%u]\n",
         pkt->type, pkt->code.value, len);

  /* the rest is message body called "Other Fields" */
  if(pkt->type == SIXP_PKT_TYPE_REQUEST) {
    switch(pkt->code.cmd) {
      case SIXP_PKT_CMD_ADD:
      case SIXP_PKT_CMD_DELETE:
        /* Add and Delete has the same request format */
        if(len < (sizeof(sixp_pkt_metadata_t) +
                  sizeof(sixp_pkt_cell_options_t) +
                  sizeof(sixp_pkt_num_cells_t)) ||
           (len % sizeof(uint32_t)) != 0) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_STATUS:
        if(len != (sizeof(sixp_pkt_metadata_t) +
                   sizeof(sixp_pkt_cell_options_t))) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_LIST:
        if(len != (sizeof(sixp_pkt_metadata_t) +
                   sizeof(sixp_pkt_cell_options_t) +
                   sizeof(sixp_pkt_reserved_t) +
                   sizeof(sixp_pkt_offset_t) +
                   sizeof(sixp_pkt_max_num_cells_t))) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_CLEAR:
        if(len != sizeof(sixp_pkt_metadata_t)) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      default:
        PRINTF("6P-pkt: sixp_pkt_parse() fails because of unsupported cmd\n");
        return -1;
    }
  } else if(pkt->type == SIXP_PKT_TYPE_RESPONSE ||
            pkt->type == SIXP_PKT_TYPE_CONFIRMATION) {
    switch(pkt->code.rc) {
      case SIXP_PKT_RC_SUCCESS:
        /*
         * The "Other Field" contains
         * - Res to CLEAR:             Empty (length 0)
         * - Res to STATUS:            "Num. Cells"
         * - Res to ADD, DELETE, LIST: 0, 1, or multiple 6P cells
         */
        if(len != 0 &&
           len != sizeof(sixp_pkt_num_cells_t) &&
           (len % sizeof(uint32_t)) != 0) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_RC_ERR_VER:
      case SIXP_PKT_RC_ERR_SFID:
      case SIXP_PKT_RC_ERR_GEN:
      case SIXP_PKT_RC_ERR_BUSY:
      case SIXP_PKT_RC_ERR_NORES:
      case SIXP_PKT_RC_ERR_RESET:
      case SIXP_PKT_RC_ERR:
        if(len != 0) {
          PRINTF("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      default:
        PRINTF("6P-pkt: sixp_pkt_parse() fails because of unsupported code\n");
        return -1;
    }
  } else {
    PRINTF("6P-pkt: sixp_pkt_parse() fails because of unsupported type\n");
    return -1;
  }

  pkt->body = buf;
  pkt->body_len = len;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_create(sixp_pkt_type_t type, sixp_pkt_code_t code,
                uint8_t sfid, uint8_t seqno, uint8_t gab, uint8_t gba,
                const uint8_t *body, uint16_t body_len, sixp_pkt_t *pkt)
{
  uint8_t *hdr;

  assert((body == NULL && body_len == 0) || (body != NULL && body_len > 0));
  if((body == NULL && body_len > 0) || (body != NULL && body_len == 0)) {
    PRINTF("6P-pkt: sixp_pkt_create() fails because of invalid argument\n");
    return -1;
  }

  packetbuf_clear();

  /*
   * We're going to create a packet having 6top IE header (4 octets) and body
   * (body_len octets).
   */
  if(PACKETBUF_SIZE < (packetbuf_totlen() + body_len)) {
    PRINTF("6P-pkt: sixp_pkt_create() fails because body is too long\n");
    return -1;
  }

  if(packetbuf_hdralloc(4) != 1) {
    PRINTF("6P-pkt: sixp_pkt_create fails to allocate header space\n");
    return -1;
  }
  hdr = packetbuf_hdrptr();
  /* header: write the 6top IE header, 4 octets */
  hdr[0] = (type << 4) | SIXP_PKT_VERSION;
  hdr[1] = code.value;
  hdr[2] = sfid;
  hdr[3] = (gab << 4) | (gba << 6) | seqno;

  /* data: write body */
  if(body_len > 0 && body != NULL) {
    memcpy(packetbuf_dataptr(), body, body_len);
    packetbuf_set_datalen(body_len);
  }

  /* copy information of a sending packet into pkt if necessary */
  if(pkt != NULL) {
    pkt->type = type;
    pkt->code = code;
    pkt->sfid = sfid;
    pkt->seqno = seqno;
    pkt->gab = gab;
    pkt->gba = gba;
    pkt->body = body;
    pkt->body_len = body_len;
  }

  /* packetbuf is ready to be sent */
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
