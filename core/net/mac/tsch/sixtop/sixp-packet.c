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
        if(len < offsetof(sixp_req_add_t, cell_list) ||
           (len % sizeof(uint32_t)) != 0) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_STATUS:
        if(len != sizeof(sixp_req_status_t)) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_LIST:
        if(len != sizeof(sixp_req_list_t)) {
          PRINTF("6top: invalid message length [code=%u, len=%u]\n",
                 sixtop_ie->code.cmd, len);
          return -1;
        }
        break;
      case SIXP_CMD_CLEAR:
        if(len != sizeof(sixp_req_clear_t)) {
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
        if(len != sizeof(sixp_res_status_t) &&
           len != sizeof(sixp_res_clear_t) &&
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

  sixtop_ie->body = (const sixtop_ie_body_t *)buf;
  sixtop_ie->body_len = len;

  return 0;
}
/** @} */
