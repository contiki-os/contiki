/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         6top Protocol (6P) Message Manipulation
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "lib/assert.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

#include "sixtop.h"

#define SIXTOP_6P_SUBIE_ID 0x00
#define SIXTOP_6P_VERSION  0x01

#define WRITE16(buf, val)                               \
  do { ((uint8_t *)(buf))[0] = (val) & 0xff;            \
    ((uint8_t *)(buf))[1] = ((val) >> 8) & 0xff;        \
  } while(0);

#define READ16(buf, var)                                        \
  (var) = ((uint8_t *)(buf))[0] | ((uint8_t *)(buf))[1] << 8

static int
create_6top_ie(const sixtop_ie_t *sixtop_ie)
{
  uint8_t *buf = packetbuf_dataptr();
  uint8_t free_space = PACKETBUF_SIZE - packetbuf_totlen();
  int may_have_cell_list;
  uint16_t len = 0;
  uint16_t cell_list_bytes = 0;

  assert(buf != NULL);

  if(free_space < 4) {
    return -1;
  }
  /* common part */
  buf[0] = SIXTOP_6P_SUBIE_ID;
  buf[1] = (sixtop_ie->code << 4) | SIXTOP_6P_VERSION;
  buf[2] = sixtop_ie->sfid;
  buf[3] = sixtop_ie->seqno & 0x0f;

  buf += 4;
  len += 4;

  switch(sixtop_ie->code) {
  case SIXTOP_CMD_ADD:
  case SIXTOP_CMD_DELETE:
    if((free_space - len) < 3) {
      return -1;
    }

    buf[0] = sixtop_ie->num_cells;
    WRITE16(&buf[1], 0); // metadata
    buf += 3;
    len += 3;

    may_have_cell_list = 1;
    break;
  case SIXTOP_RC_SUCCESS:
    may_have_cell_list = 1;
    break;
  default:
    break;
  }

  if(may_have_cell_list) {
    if((free_space - len) < sixtop_ie->cell_list_len * sizeof(uint32_t)) {
      return -1;
    }

    cell_list_bytes = sixtop_ie->cell_list_len * sizeof(uint32_t);
    memcpy(buf, sixtop_ie->cell_list, cell_list_bytes);
    buf += cell_list_bytes;
    len += cell_list_bytes;
  }

  packetbuf_set_datalen(len);
  return 0;
}

static int
build_6top_msg(sixtop_ie_t *sixtop_ie) {

  struct ieee802154_ies ies;

  packetbuf_clear();

  /* put 6top SubIE Content into packetbuf */
  if(create_6top_ie(sixtop_ie) < 0){
    PRINTF("6top: failed to create a 6top SubIE\n");
    return -1;
  }

  memset(&ies, 0, sizeof(ies));
  ies.sixtop_ie_content_ptr = packetbuf_dataptr();
  ies.sixtop_ie_content_len = packetbuf_datalen();

  /* prepend Payload IE header */
  if(packetbuf_hdralloc(2) &&
     frame80215e_create_ie_iana_ietf(packetbuf_hdrptr(),
                                     packetbuf_totlen(),
                                     &ies) < 0) {
    PRINTF("6top: failed to create a Payload IE header for IANA-IETF IE\n");
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

  /* other necessities to sending a IEEE 802.15.4 frame */
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);

  return 0;
}

int
sixtop_6p_build_6top_request(uint8_t code, uint8_t sfid,
                             uint8_t seqno, const sixtop_msg_body_t *body)
{
  sixtop_ie_t sixtop_ie;

  memset(&sixtop_ie, 0, sizeof(sixtop_ie));
  sixtop_ie.version = SIXTOP_6P_VERSION;
  sixtop_ie.code = code;
  sixtop_ie.sfid = sfid;
  sixtop_ie.seqno = seqno;

  if(body != NULL) {
    switch(code) {
    case SIXTOP_CMD_ADD:
    case SIXTOP_CMD_DELETE:
      sixtop_ie.num_cells = body->num_cells;
      sixtop_ie.metadata = body->metadata;
      memcpy(sixtop_ie.cell_list, body->cell_list,
             body->cell_list_len * sizeof(uint32_t));
      sixtop_ie.cell_list_len = body->cell_list_len;
      break;
    default:
      /* unsupported */
      break;
    }
  }

  return build_6top_msg(&sixtop_ie);
}

int
sixtop_6p_build_6top_response(uint8_t code, uint8_t sfid,
                              uint8_t seqno, const sixtop_msg_body_t *body)
{
  sixtop_ie_t sixtop_ie;

  memset(&sixtop_ie, 0, sizeof(sixtop_ie));
  sixtop_ie.code = SIXTOP_6P_VERSION;
  sixtop_ie.code = code;
  sixtop_ie.sfid = sfid;
  sixtop_ie.seqno = seqno;
  if(body != NULL) {
    memcpy(sixtop_ie.cell_list, body->cell_list,
           body->cell_list_len * sizeof(uint32_t));
    sixtop_ie.cell_list_len = body->cell_list_len;
  }

  return build_6top_msg(&sixtop_ie);
}

int
sixtop_6p_parse_6top_ie(const uint8_t *buf, uint16_t len,
                        sixtop_ie_t *sixtop_ie)
{
  int may_have_cell_list = 0;
  assert(buf != NULL && sixtop_ie != NULL);

  if(len < 3) {
    PRINTF("6top: input SubIE is too short\n");
    return -1;
  }

  memset(sixtop_ie, 0, sizeof(sixtop_ie));
  sixtop_ie->version = buf[0] & 0x0f;
  sixtop_ie->code = (buf[0] & 0xf0) >> 4;
  sixtop_ie->sfid = buf[1];
  sixtop_ie->seqno = buf[2] & 0x07;
  sixtop_ie->gab = (buf[2] & 0x30) >> 0x04;
  sixtop_ie->gba = (buf[2] & 0xc0) >> 0x04;

  if(sixtop_ie->version != SIXTOP_6P_VERSION) {
    PRINTF("6top: invalid version %u\n", sixtop_ie->version);
    return -1;
  }

  buf += 3;
  len -= 3;

  switch(sixtop_ie->code) {
  case SIXTOP_CMD_ADD:
  case SIXTOP_CMD_DELETE:
    if(len < 3) {
      PRINTF("6top: input SubIE is too short for SIXTOP_CMD_ADD/SIXTOP_CMD_DELETE\n");
      return -1;
    }
    sixtop_ie->num_cells = buf[0];
    memcpy(&sixtop_ie->metadata, &buf[1], sizeof(uint16_t));
    buf += 3;
    len -= 3;
    may_have_cell_list = 1;
    break;

  case SIXTOP_RC_SUCCESS:
    may_have_cell_list = 1;
    break;
  default:
    break;
  }

  if(may_have_cell_list && len > 0) {
    if(len < sizeof(uint32_t) ||
       len % sizeof(uint32_t) != 0) {
      PRINTF("6top: cannot parse the cell list part\n");
      return -1;
    }

    if((len / sizeof(uint32_t)) > SIXTOP_IE_MAX_CELLS) {
      sixtop_ie->cell_list_len = SIXTOP_IE_MAX_CELLS;
    } else {
      sixtop_ie->cell_list_len = len / sizeof(uint32_t);
    }

    memcpy(sixtop_ie->cell_list, buf,
           sixtop_ie->cell_list_len * sizeof(uint32_t));
  }

  return 0;
}
