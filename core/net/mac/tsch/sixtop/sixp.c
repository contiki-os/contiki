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
 *         6top Protocol (6P)
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "contiki-lib.h"
#include "lib/assert.h"

#include "sixtop.h"
#include "sixp-nbr.h"
#include "sixp-pkt.h"
#include "sixp-trans.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

static void mac_callback(void *ptr, int status, int transmissions);
static int send_back_error(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           uint8_t sfid, uint8_t seqno,
                           const linkaddr_t *dest_addr);
/*---------------------------------------------------------------------------*/
static void
mac_callback(void *ptr, int status, int transmissions)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;
  sixp_trans_state_t new_state, current_state;

  assert(trans != NULL);
  if(trans == NULL) {
    PRINTF("6P: mac_callback() fails because trans is NULL\n");
    return;
  }

  current_state = sixp_trans_get_state(trans);
  if(status == MAC_TX_OK) {
    switch(current_state) {
      case SIXP_TRANS_STATE_INIT:
        new_state = SIXP_TRANS_STATE_REQUEST_SENT;
        break;
      case SIXP_TRANS_STATE_REQUEST_RECEIVED:
        new_state = SIXP_TRANS_STATE_RESPONSE_SENT;
        break;
      case SIXP_TRANS_STATE_RESPONSE_RECEIVED:
        new_state = SIXP_TRANS_STATE_CONFIRMATION_SENT;
        break;
      default:
        PRINTF("6P: mac_callback() fails because of an unexpected state (%u)\n",
               current_state);
        return;
    }
  } else {
    /*
     * In a case of transmission failure of a request, a corresponding SF would
     * retransmit the request with a new transaction. For a response or a
     * confirmation, the same transaction will be used for retransmission as
     * long as it doesn't have timeout.
     */
    if(current_state == SIXP_TRANS_STATE_INIT) {
      /* request case */
      new_state = SIXP_TRANS_STATE_TERMINATING;
    } else {
      /* response or confirmation case: stay the same state */
      new_state = current_state;
    }
  }

  if(new_state != current_state &&
     sixp_trans_transit_state(trans, new_state) != 0) {
    PRINTF("6P: mac_callback() fails because of state transition failure\n");
    PRINTF("6P: something wrong; we're terminating the trans %p\n", trans);
    (void)sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING);
    return;
  }

  sixp_trans_invoke_callback(trans,
                             status == MAC_TX_OK ?
                             SIXP_OUTPUT_STATUS_SUCCESS :
                             SIXP_OUTPUT_STATUS_FAILURE);
  sixp_trans_set_callback(trans, NULL, NULL, 0);
}
/*---------------------------------------------------------------------------*/
static int
send_back_error(sixp_pkt_type_t type, sixp_pkt_code_t code,
                uint8_t sfid, uint8_t seqno,
                const linkaddr_t *dest_addr)
{
  /* create a 6P packet within packetbuf */
  /* XXX: set 0 as GAB and GBA for a error response */
  /* XXX: how can we make a confirmation having an error return value? */
  if(sixp_pkt_create(type, code, sfid, seqno, 0, 0, NULL, 0, NULL) < 0) {
    PRINTF("6P: failed to create a 6P packet to return an error [rc:%u]\n",
           code.value);
    return -1;
  }
  /* we don't care about how the transmission goes; no need to set callback */
  sixtop_output(dest_addr, NULL, NULL);
  return 0;
}
/*---------------------------------------------------------------------------*/
void
sixp_input(const uint8_t *buf, uint16_t len, const linkaddr_t *src_addr)
{
  sixp_pkt_t pkt;
  sixp_nbr_t *nbr;
  uint8_t invalid_schedule_generation;
  sixp_trans_t *trans;
  const sixtop_sf_t *sf;
  int16_t seqno;
  int ret;

  assert(buf != NULL && src_addr != NULL);
  if(buf == NULL || src_addr == NULL) {
    return;
  }

  if(sixp_pkt_parse(buf, len, &pkt) < 0) {
    PRINTF("6P: sixp_input() fails because off a malformed 6P packet\n");
    return;
  }

  if(pkt.type != SIXP_PKT_TYPE_REQUEST &&
     pkt.type != SIXP_PKT_TYPE_RESPONSE &&
     pkt.type != SIXP_PKT_TYPE_CONFIRMATION) {
    PRINTF("6P: sixp_input() fails because of unsupported type [type:%u]\n",
           pkt.type);
    return;
  }

  if((sf = sixtop_find_sf(pkt.sfid)) == NULL) {
    PRINTF("6P: sixp_input() fails because SF [sfid:%u] is unavailable\n",
           pkt.sfid);
    /*
     * XXX: what if the incoming packet is a response? confirmation should be
     * sent back?
     */
    if(send_back_error(SIXP_PKT_TYPE_RESPONSE,
                       (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR_SFID,
                       pkt.sfid, pkt.seqno, src_addr) < 0) {
      PRINTF("6P: sixp_input() fails to return an error response\n");
    };
    return;
  }

  nbr = sixp_nbr_find(src_addr);
  /* Generation Management */
  if(pkt.code.value == SIXP_PKT_CMD_CLEAR) {
    /* Not need to validate generation counters in a case of CMD_CLEAR */
    invalid_schedule_generation = 0;
  } else if(nbr == NULL) {
    if(pkt.gab == 0 && pkt.gba == 0) {
      invalid_schedule_generation = 0; /* valid combination */
    } else {
      PRINTF("6P: GAB/GBA should be 0 because of no corresponding nbr\n");
      invalid_schedule_generation = 1;
    }
  } else {
    PRINTF("6P: GAB: %u, GBA: %u, GTX: %u, GRX: %u\n",
           pkt.gab, pkt.gba, sixp_nbr_get_grx(nbr), sixp_nbr_get_gtx(nbr));
    if(((pkt.type == SIXP_PKT_TYPE_REQUEST ||
         pkt.type == SIXP_PKT_TYPE_CONFIRMATION) &&
        pkt.gab == sixp_nbr_get_grx(nbr) &&
        pkt.gba == sixp_nbr_get_gtx(nbr)) ||
       (pkt.type == SIXP_PKT_TYPE_RESPONSE &&
        pkt.gab == sixp_nbr_get_gtx(nbr) &&
        pkt.gba == sixp_nbr_get_grx(nbr))) {
      invalid_schedule_generation = 0; /* valid combination */
    } else {
      invalid_schedule_generation = 1;
    }
  }
  if(invalid_schedule_generation) {
    PRINTF("6P: sixp_input() fails because of schedule generation mismatch\n");
    return;
  }

  /* Transaction Management */
  trans = sixp_trans_find(src_addr);

  if(pkt.type == SIXP_PKT_TYPE_REQUEST) {
    if(trans != NULL) {
      /* Error: not supposed to have another transaction with the peer. */
      PRINTF("6P: sixp_input() fails because another request [peer_addr:");
      PRINTLLADDR((const uip_lladdr_t *)src_addr);
      PRINTF(" seqno:%u] is in process\n", sixp_trans_get_seqno(trans));
      if(send_back_error(SIXP_PKT_TYPE_RESPONSE,
                         (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR_BUSY,
                         pkt.sfid, pkt.seqno, src_addr) < 0) {
        PRINTF("6P: sixp_input() fails to return an error response");
      }
      return;
    } else if((trans = sixp_trans_alloc(&pkt, src_addr)) == NULL) {
      PRINTF("6P: sixp_input() fails because of lack of memory\n");
      if(send_back_error(SIXP_PKT_TYPE_RESPONSE,
                         (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_ERR_NORES,
                         pkt.sfid, pkt.seqno, src_addr) < 0) {
        PRINTF("6P: sixp_input() fails to return an error response\n");
      }
      return;
    }
  } else if(pkt.type == SIXP_PKT_TYPE_RESPONSE ||
            pkt.type == SIXP_PKT_TYPE_CONFIRMATION) {
    if(trans == NULL) {
      /* Error: should have a transaction for incoming packet */
      PRINTF("6P: sixp_input() fails because of no trans [peer_addr:");
      PRINTLLADDR((const uip_lladdr_t *)src_addr);
      PRINTF("]\n");
      return;
    } else if((seqno = sixp_trans_get_seqno(trans)) < 0 ||
              seqno != pkt.seqno) {
      PRINTF("6P: sixp_input() fails because of invalid seqno [seqno:%u, %u]\n",
             seqno, pkt.seqno);
      return;
    }
  }

  /* state transition */
  assert(trans != NULL);
  switch(pkt.type) {
    case SIXP_PKT_TYPE_REQUEST:
      ret = sixp_trans_transit_state(trans,
                                     SIXP_TRANS_STATE_REQUEST_RECEIVED);
      break;
    case SIXP_PKT_TYPE_RESPONSE:
      ret = sixp_trans_transit_state(trans,
                                     SIXP_TRANS_STATE_RESPONSE_RECEIVED);
      break;
    case SIXP_PKT_TYPE_CONFIRMATION:
      ret = sixp_trans_transit_state(trans,
                                     SIXP_TRANS_STATE_CONFIRMATION_RECEIVED);
      break;
    default:
      PRINTF("6P: sixp_input() fails because of unsupported type [type:%u]\n",
             pkt.type);
      return;
  }
  if(ret < 0) {
    PRINTF("6P: sixp_input() fails because of state transition failure\n");
    PRINTF("6P: something wrong; we're terminating the trans %p\n", trans);
    (void)sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING);
    return;
  }

  if(sf->input != NULL) {
    sf->input(pkt.type, pkt.code, pkt.body, pkt.body_len, src_addr);
  }

  return;
}
/*---------------------------------------------------------------------------*/
int
sixp_output(sixp_pkt_type_t type, sixp_pkt_code_t code, uint8_t sfid,
            const uint8_t *body, uint16_t body_len,
            const linkaddr_t *dest_addr,
            sixp_sent_callback_t func, void *arg, uint16_t arg_len)
{
  sixp_trans_t *trans;
  sixp_nbr_t *nbr;
  sixp_pkt_cmd_t cmd;
  int16_t seqno, gab, gba;
  sixp_pkt_t pkt;

  assert(dest_addr != NULL);

  /* validate the state of a transaction with a specified peer */
  trans = sixp_trans_find(dest_addr);
  if(type == SIXP_PKT_TYPE_REQUEST) {
    if(trans != NULL) {
      PRINTF("6P: sixp_output() fails because another trans for [peer_addr:");
      PRINTLLADDR((const uip_lladdr_t *)dest_addr);
      PRINTF("] is in process\n");
      return -1;
    } else {
      /* ready to send a request */
      /* we're going to allocate a new transaction later */
    }
  } else if(type == SIXP_PKT_TYPE_RESPONSE) {
    if(trans == NULL) {
      PRINTF("6P: sixp_output() fails because of no transaction [peer_addr:");
      PRINTLLADDR((const uip_lladdr_t *)dest_addr);
      PRINTF("]\n");
      return -1;
    } else if(sixp_trans_get_state(trans) !=
              SIXP_TRANS_STATE_REQUEST_RECEIVED) {
      PRINTF("6P: sixp_output() fails because of invalid transaction state\n");
      return -1;
    } else {
      /* ready to send a response */
    }
  } else if(type == SIXP_PKT_TYPE_CONFIRMATION) {
    if(trans == NULL) {
      PRINTF("6P: sixp_output() fails because of no transaction [peer_addr:\n");
      PRINTLLADDR((const uip_lladdr_t *)dest_addr);
      PRINTF("\n");
      return -1;
    } else if(sixp_trans_get_state(trans) !=
              SIXP_TRANS_STATE_RESPONSE_RECEIVED) {
      PRINTF("6P: sixp_output() fails because of invalid transaction state\n");
      return -1;
    } else {
      /* ready to send a confirmation */
    }
  } else {
    PRINTF("6P: sixp_output() fails because of unsupported type [type:%u]\n",
           type);
    return -1;
  }

  nbr = sixp_nbr_find(dest_addr);

  /*
   * Make sure we have a nbr for the peer if the packet is a response with
   * success so that we can manage the schedule generation.
   */
  if(nbr == NULL &&
     type == SIXP_PKT_TYPE_RESPONSE && code.value == SIXP_PKT_RC_SUCCESS &&
     ((cmd = sixp_trans_get_cmd(trans)) == SIXP_PKT_CMD_ADD ||
      cmd == SIXP_PKT_CMD_DELETE) &&
     (nbr = sixp_nbr_alloc(dest_addr)) == NULL) {
    PRINTF("6P: sixp_output() fails because of no memory for another nbr\n");
    return -1;
  }

  /* set SeqNum */
  if(type == SIXP_PKT_TYPE_REQUEST) {
    if(nbr == NULL &&
       (nbr = sixp_nbr_alloc(dest_addr)) == NULL) {
      PRINTF("6P: sixp_output() fails because it fails to allocate a nbr\n");
      return -1;
    }
    if((seqno = sixp_nbr_get_next_seqno(nbr)) < 0){
      PRINTF("6P: sixp_output() fails to get the next sequence number\n",
             seqno);
      return -1;
    }
    if(sixp_nbr_increment_next_seqno(nbr) < 0) {
      PRINTF("6P: sixp_output() fails to increment the next sequence number\n");
      return -1;
    }
  } else {
    assert(trans != NULL);
    if((seqno = sixp_trans_get_seqno(trans)) < 0) {
      PRINTF("6P: sixp_output() fails because it fails to get seqno\n");
      return -1;
    }
  }

  /* set GAB and GBA */
  if(nbr == NULL) {
    gab = gba = 0;
  } else {
    if(type == SIXP_PKT_TYPE_REQUEST ||
       type == SIXP_PKT_TYPE_CONFIRMATION) {
      gab = sixp_nbr_get_gtx(nbr);
      gba = sixp_nbr_get_grx(nbr);
    } else if(type == SIXP_PKT_TYPE_RESPONSE) {
      gba = sixp_nbr_get_gtx(nbr);
      gab = sixp_nbr_get_grx(nbr);
    } else {
      /* never come here */
      PRINTF("6P: sixp_output() fails because of an unexpected condition\n");
      return -1;
    }
  }
  if(gab < 0 || gba < 0) {
    PRINTF("6P: sixp_output() fails to get GAB or GBA\n");
    return -1;
  }

  /* create a 6P packet within packetbuf */
  if(sixp_pkt_create(type, code, sfid,
                     (uint8_t)seqno, (uint8_t)gab, (uint8_t)gba,
                     body, body_len,
                     type == SIXP_PKT_TYPE_REQUEST ? &pkt : NULL) < 0) {
    PRINTF("6P: sixp_output() fails to create a 6P packet\n");
    return -1;
  }

  /* allocate a transaction for a sending request */
  if(type == SIXP_PKT_TYPE_REQUEST) {
    assert(trans == NULL);
    if((trans = sixp_trans_alloc(&pkt, dest_addr)) == NULL) {
      PRINTF("6P: sixp_output() is aborted because of no memory\n");
      return -1;
    } else {
      /* ready for proceed */
    }
  }

  assert(trans != NULL);
  sixp_trans_set_callback(trans, func, arg, arg_len);
  sixtop_output(dest_addr, mac_callback, trans);

  return 0;
}
/*---------------------------------------------------------------------------*/
void
sixp_init(void)
{
  sixp_nbr_init();
  sixp_trans_init();
}
/*---------------------------------------------------------------------------*/
/** @} */
