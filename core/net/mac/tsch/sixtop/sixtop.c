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
 *         6top Protocol (6P) - Core
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "contiki-lib.h"

#include "lib/assert.h"
#include "net/packetbuf.h"
#include "net/mac/mac.h"
#include "net/mac/frame802154.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "sys/ctimer.h"

#include "sixtop.h"
#include "sixtop-6p.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"


typedef enum {
  SIXTOP_TRANS_STATE_INIT = 0,
  SIXTOP_TRANS_STATE_REQUEST_SENT,
  SIXTOP_TRANS_STATE_REQUEST_RECEIVED,
  SIXTOP_TRANS_STATE_RESPONSE_SENT,
  SIXTOP_TRANS_STATE_RESPONSE_RECEIVED,
  SIXTOP_TRANS_STATE_TERMINATING,
} sixtop_trans_state_t;

typedef struct sixtop_trans {
  struct sixtop_trans *next;
  const sixtop_sf_t *sf;
  linkaddr_t peer_addr;
  uint8_t seqno;
  uint8_t cmd;
  uint8_t return_code;
  sixtop_trans_state_t state;
  sixtop_msg_body_t last_msg_body;
  sixtop_callback_t callback;
  struct ctimer timer;
} sixtop_trans_t;

/* helper functions on 6top IE */
static int msg_is_request(const sixtop_ie_t *sixtop_ie);
static int msg_is_response(const sixtop_ie_t *sixtop_ie);

/* transaction-related internal functions */
static void terminate_trans(void *ptr);
static void process_trans(void *ptr);
static void schedule_trans_process(sixtop_trans_t *trans);
static int transit_trans_state(sixtop_trans_t *trans,
                               sixtop_trans_state_t new_state);
static sixtop_trans_t *alloc_trans(void);
static void free_trans(sixtop_trans_t *trans);
static sixtop_trans_t *find_trans(const linkaddr_t *peer_addr);

/* output function used for a response with an error code */
static void response_output_without_trans(uint8_t sfid,
                                          uint8_t return_code,
                                          uint8_t seqno,
                                          const linkaddr_t *dest_addr);

/* callback function to be passed to a sending function at the lower layer */
static void mac_callback(void *ptr, int status, int transmissions);

/* global variables for internal use */
MEMB(trans_memb, sixtop_trans_t, SIXTOP_NUM_OF_TRANSACTIONS);
LIST(trans_list);
const sixtop_sf_t *schedule_functions[SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS];

static int
msg_is_request(const sixtop_ie_t *sixtop_ie)
{
  if(sixtop_ie->code < SIXTOP_CMD_MIN || sixtop_ie->code > SIXTOP_CMD_MAX) {
    return 0;
  }
  return 1;
}

static int
msg_is_response(const sixtop_ie_t *sixtop_ie)
{
  if(sixtop_ie->code < SIXTOP_RC_MIN || sixtop_ie->code > SIXTOP_RC_MAX) {
    return 0;
  }
  return 1;
}

static void
terminate_trans(void *ptr)
{
  sixtop_trans_t *trans = (sixtop_trans_t *)ptr;

  assert(trans != NULL);

  ctimer_stop(&trans->timer);
  (void)transit_trans_state(trans, SIXTOP_TRANS_STATE_TERMINATING);
}

static void
process_trans(void *ptr)
{
  sixtop_trans_t *trans = (sixtop_trans_t *)ptr;

  assert(trans != NULL);

  /* make sure that the timer is stopped */
  ctimer_stop(&trans->timer);

  /* state-specific operation */
  /** handle the terminating state first **/
  if(trans->state == SIXTOP_TRANS_STATE_TERMINATING) {
    PRINTF("6top: trans [peer_addr=");
    PRINTLLADDR((const uip_lladdr_t *)&trans->peer_addr);
    PRINTF(", seqno=%u] is going to be freed\n", trans->seqno);
    free_trans(trans);
    return;
  }

  /** do for others **/
  switch(trans->state) {
  case SIXTOP_TRANS_STATE_REQUEST_RECEIVED:
    trans->sf->request_input(trans->cmd,
                             &trans->last_msg_body,
                             &trans->peer_addr);
    break;
  case SIXTOP_TRANS_STATE_RESPONSE_RECEIVED:
    trans->sf->response_input(trans->cmd, trans->return_code,
                              &trans->last_msg_body,
                              &trans->peer_addr);
    break;
  default:
    break;
  }

  /* set the timer with a timeout values defined by the SF */
  ctimer_set(&trans->timer, trans->sf->timeout_interval,
             terminate_trans, trans);
}

static void
schedule_trans_process(sixtop_trans_t *trans)
{
  assert(trans != NULL);

  ctimer_stop(&trans->timer);
  ctimer_set(&trans->timer, 0, process_trans, trans); /* expired immediately */
}

static int
transit_trans_state(sixtop_trans_t *trans, sixtop_trans_state_t new_state)
{
  assert(trans != NULL);

  /* enforce state transition rules  */
  if((new_state == SIXTOP_TRANS_STATE_REQUEST_SENT &&
      trans->state != SIXTOP_TRANS_STATE_INIT) ||
     (new_state == SIXTOP_TRANS_STATE_REQUEST_RECEIVED &&
      trans->state != SIXTOP_TRANS_STATE_INIT) ||
     (new_state == SIXTOP_TRANS_STATE_RESPONSE_SENT &&
      trans->state != SIXTOP_TRANS_STATE_REQUEST_RECEIVED &&
      trans->state != SIXTOP_TRANS_STATE_RESPONSE_RECEIVED) ||
     (new_state == SIXTOP_TRANS_STATE_RESPONSE_RECEIVED &&
      trans->state != SIXTOP_TRANS_STATE_REQUEST_SENT &&
      trans->state != SIXTOP_TRANS_STATE_RESPONSE_SENT)) {
    /* invalid state transition */
    return -1;
  }

  PRINTF("6top: trans %p state changes from %u to %u\n",
         trans, trans->state, new_state);
  trans->state = new_state;
  schedule_trans_process(trans);
  return 0;
}

static void
mac_callback(void *ptr, int status, int transmissions)
{
  sixtop_trans_t *trans = (sixtop_trans_t *)ptr;
  int ret = 0;

  assert(trans != NULL);

  switch(trans->state) {
  case SIXTOP_TRANS_STATE_INIT:
    if(status == MAC_TX_OK) {
      ret = transit_trans_state(trans, SIXTOP_TRANS_STATE_REQUEST_SENT);
    } else {
      /*
       * We'll drop the transaction immediately when the attempt to send a
       * request failed. For the case where SF retransmits the request, a new
       * transaction will be allocated for it.
       */
      ret = transit_trans_state(trans, SIXTOP_TRANS_STATE_TERMINATING);
    }
    break;
  case SIXTOP_TRANS_STATE_REQUEST_RECEIVED:
  case SIXTOP_TRANS_STATE_RESPONSE_RECEIVED:
    if(status == MAC_TX_OK) {
      ret = transit_trans_state(trans, SIXTOP_TRANS_STATE_RESPONSE_SENT);
    } else {
      /* stay at the current state when sending a response fails */
    }
    break;
  default:
    PRINTF("6top: unexpected trans state in mac_callback()\n");
    return;
  }

  if(ret < 0) {
    PRINTF("6top: state transition failure\n");
    return;
  }

  if(trans->callback != NULL) {
    trans->callback(&trans->last_msg_body, &trans->peer_addr,
                    status == MAC_TX_OK ?
                    SIXTOP_RETURN_SUCCESS :
                    SIXTOP_RETURN_FAILURE);
    trans->callback = NULL;
  }
}

static sixtop_trans_t *
alloc_trans(void)
{
  sixtop_trans_t *trans = memb_alloc(&trans_memb);

  if(trans != NULL) {
    list_add(trans_list, trans);
  }

  memset(trans, 0, sizeof(sixtop_trans_t));
  trans->state = SIXTOP_TRANS_STATE_INIT;
  return trans;
}

static void
free_trans(sixtop_trans_t *trans)
{
  if(trans == NULL) {
    return;
  }

  list_remove(trans_list, trans);
  memb_free(&trans_memb, trans);
}

static sixtop_trans_t *
find_trans(const linkaddr_t *peer_addr)
{
  sixtop_trans_t *trans;

  assert(peer_addr != NULL);

  /*
   * The assumption here is that there is no more than two transactions for a
   * single peer.
   */
  for(trans = list_head(trans_list);
      trans != NULL; trans = trans->next) {
    if(memcmp(peer_addr, &trans->peer_addr, sizeof(linkaddr_t)) == 0) {
      return trans;
    }
  }

  return NULL;
}

int
sixtop_add_sf(const sixtop_sf_t *sf)
{
  int i;

  assert(sf != NULL);

  if(sixtop_find_sf(sf->sfid) != NULL) {
    PRINTF("6top: there is another SF installed with the same sfid [%u]\n",
           sf->sfid);
    return -1;
  }

  for(i = 0; i < SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS; i++) {
    if(schedule_functions[i] == NULL) {
      schedule_functions[i] = sf;
      if(sf->init != NULL) {
        sf->init();
      }
      break;
    }
  }

  if(i == SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS) {
    PRINTF("6top: no room to install the specified SF [sfid=%u]\n",
           sf->sfid);
    return -1;
  }

  return 0;
}

const sixtop_sf_t *
sixtop_find_sf(uint8_t sfid)
{
  int i;

  for(i = 0; i < SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS; i++) {
    if(schedule_functions[i]->sfid == sfid) {
      return (const sixtop_sf_t *)schedule_functions[i];
    }
  }

  return NULL;
}

int
sixtop_add_cells(uint8_t sfid, linkaddr_t *peer_addr, uint8_t num_cells)
{
  const sixtop_sf_t *sf;
  if((sf = sixtop_find_sf(sfid)) == NULL) {
    PRINTF("6top: cannot find SF [sfid=%u]\n", sfid);
    return -1;
  }
  if(sf->add == NULL) {
    PRINTF("6top: add operation is not supported by SF [sfid=%u]\n", sfid);
    return -1;
  }
  return sf->add(peer_addr, num_cells);
}

int
sixtop_delete_cells(uint8_t sfid, linkaddr_t *peer_addr, uint8_t num_cells)
{
  const sixtop_sf_t *sf;
  if((sf = sixtop_find_sf(sfid)) == NULL) {
    PRINTF("6top: cannot find SF [sfid=%u]\n", sfid);
    return -1;
  }
  if(sf->delete == NULL) {
    PRINTF("6top: delete operation is not supported by SF [sfid=%u]\n", sfid);
    return -1;
  }
  return sf->delete(peer_addr, num_cells);
}

static void
response_output_without_trans(uint8_t sfid, uint8_t return_code,
                              uint8_t seqno, const linkaddr_t *dest_addr)
{
  /* build a 6top message within a packetbuf */
  if(sixtop_6p_build_6top_response(return_code, sfid, seqno, NULL) < 0) {
    PRINTF("6top: failed to build a 6top response message\n");
    return;
  }

  /* send the message with TSCH MAC */
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest_addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  tschmac_driver.send(NULL, NULL);

  PRINTF("6top: sending a 6top response to ");
  PRINTLLADDR((const uip_lladdr_t *)dest_addr);
  PRINTF("\n");
}

void
sixtop_response_output(uint8_t sfid, uint8_t return_code,
                       const sixtop_msg_body_t *body,
                       const linkaddr_t *dest_addr,
                       sixtop_callback_t callback)
{
  sixtop_trans_t *trans;

  assert(dest_addr != NULL);

  if((trans = find_trans(dest_addr)) == NULL) {
    PRINTF("6top: no corresponding trans for [dest_Addr=");
    PRINTLLADDR((const uip_lladdr_t *)dest_addr);
    PRINTF("]\n");
    if(callback != NULL) {
      callback(body, dest_addr, SIXTOP_RETURN_FAILURE);
    }
    return;
  }

  /* update the transaction */
  trans->return_code = return_code;
  if(body == NULL) {
    memset(&trans->last_msg_body, 0, sizeof(sixtop_msg_body_t));
  } else {
    trans->last_msg_body = *body;
  }
  trans->callback = callback;

  /* build a 6top message within a packetbuf */
  if(sixtop_6p_build_6top_response(trans->return_code, trans->sf->sfid,
                                   trans->seqno, &trans->last_msg_body) < 0) {
    PRINTF("6top: failed to build a 6top response message\n");
    callback(&trans->last_msg_body, &trans->peer_addr, SIXTOP_RETURN_FAILURE);
  }

  /* send the message with TSCH MAC */
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest_addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  tschmac_driver.send(mac_callback, trans);

  PRINTF("6top: sending a 6top response to ");
  PRINTLLADDR((const uip_lladdr_t *)&trans->peer_addr);
  PRINTF("\n");
}

void
sixtop_request_output(uint8_t sfid, uint8_t cmd,
                      const sixtop_msg_body_t *body,
                      const linkaddr_t *dest_addr,
                      sixtop_callback_t callback)
{
  sixtop_trans_t *trans;
  const sixtop_sf_t *sf;
  struct tsch_neighbor *nbr;

  assert(dest_addr != NULL && body != NULL);

  if((sf = sixtop_find_sf(sfid)) == NULL) {
    PRINTF("6top: cannot find the specified SF [sfid=%u]\n", sfid);
    if(callback != NULL) {
      callback(body, dest_addr, SIXTOP_RETURN_FAILURE);
    }
    return;
  }

  if((trans = find_trans(dest_addr)) != NULL) {
    PRINTF("6top: trans for [dest_Addr=");
    PRINTLLADDR((const uip_lladdr_t *)dest_addr);
    PRINTF("] in process\n");
    if(callback != NULL) {
      callback(body, dest_addr, SIXTOP_RETURN_FAILURE);
    }
    return;
  }

  if((trans = alloc_trans()) == NULL) {
    PRINTF("6top: no memory available for a new transaction\n");
    if(callback != NULL) {
      callback(body, dest_addr, SIXTOP_RETURN_FAILURE);
    }
    return;
  }

  /* setup a new transaction */
  memset(trans, 0, sizeof(sixtop_trans_t));
  trans->sf = sf;
  trans->peer_addr = *dest_addr;
  if((nbr = tsch_queue_get_nbr(dest_addr)) == NULL) {
    trans->seqno = SIXTOP_INITIAL_SEQUENCE_NUMBER;
  } else {
    nbr->sixtop_last_seqno++;
    trans->seqno = nbr->sixtop_last_seqno;
  }
  trans->cmd = cmd;
  if(body == NULL) {
    memset(&trans->last_msg_body, 0, sizeof(sixtop_msg_body_t));
  } else {
    trans->last_msg_body = *body;
  }
  trans->callback = callback;

  /* build a 6top request within a packetbuf */
  if(sixtop_6p_build_6top_request(trans->cmd, trans->sf->sfid,
                                  trans->seqno, &trans->last_msg_body) < 0) {
    PRINTF("6top:v failed to build a 6top request message\n");
    callback(&trans->last_msg_body, &trans->peer_addr, SIXTOP_RETURN_FAILURE);
  }

  /* send the message with TSCH MAC */
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest_addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  tschmac_driver.send(mac_callback, trans);

  PRINTF("6top: sending a 6top request to ");
  PRINTLLADDR((const uip_lladdr_t *)&trans->peer_addr);
  PRINTF("\n");
}

void
sixtop_input(void (*llsec_input)(void))
{
  uint8_t *hdr_ptr, *payload_ptr;
  uint16_t hdr_len, payload_len;

  frame802154_t frame;
  struct ieee802154_ies ies;
  sixtop_ie_t sixtop_ie;
  linkaddr_t peer_addr;

  const sixtop_sf_t *sf;
  sixtop_trans_t *trans;

  assert(llsec_input != NULL);

  /*
   * A received *DATA* frame is supposed to be stored in packetbuf by
   * framer_802154.parse(). packetbuf_dataptr() points at the starting address
   * of the IE field or Frame Payload field if it's available. FCS should not be
   * in packetbuf, which is expected to be stripped at a radio.
   */

  payload_ptr = packetbuf_dataptr();
  payload_len = packetbuf_datalen();
  hdr_len = packetbuf_hdrlen();
  hdr_ptr = payload_ptr - hdr_len;

  memcpy(&peer_addr, packetbuf_addr(PACKETBUF_ADDR_SENDER), sizeof(peer_addr));

  if(frame802154_parse(hdr_ptr, hdr_len, &frame) == 0) {
    /* parse error; should not occur, anyway */
    PRINTF("6top: frame802154_parse error\n");
    return;
  }

  /*
   * We don't need to check the frame version nor frame type. The frame version
   * is turned out to be 0b10 automatically if the frame has a IE list. The
   * frame type is supposed to be DATA as mentioned above.
   */
  assert(frame.fcf.frame_type == FRAME802154_DATAFRAME);
  if(frame.fcf.ie_list_present &&
     frame802154e_parse_information_elements(payload_ptr,
                                             payload_len, &ies) >= 0 &&
     ies.sixtop_ie_content_ptr != NULL &&
     ies.sixtop_ie_content_len > 0) {

    if(sixtop_6p_parse_6top_ie(ies.sixtop_ie_content_ptr,
                               ies.sixtop_ie_content_len,
                               &sixtop_ie) < 0) {
      /* malformed 6top IE */
      PRINTF("6top: malformed 6top SubIE\n");
      return;
    }

    if((sf = sixtop_find_sf(sixtop_ie.sfid)) == NULL) {
      PRINTF("6top: SF [sfid=%u] is unavailable\n",
             sixtop_ie.sfid);
      response_output_without_trans(sixtop_ie.sfid,
                                    SIXTOP_RC_ERR_SFID,
                                    sixtop_ie.seqno,
                                    &peer_addr);
      return;
    }

    trans = find_trans((const linkaddr_t *)&peer_addr);

    if(msg_is_request(&sixtop_ie)) {
      if(trans != NULL) {
        PRINTF("6top: another request [peer_addr=");
        PRINTLLADDR((const uip_lladdr_t *)&peer_addr);
        PRINTF("seqno=%u] in process\n", trans->seqno);
        response_output_without_trans(sixtop_ie.sfid,
                                      SIXTOP_RC_ERR_BUSY,
                                      sixtop_ie.seqno,
                                      &peer_addr);
        return;
      }

      if((trans = alloc_trans()) == NULL) {
        PRINTF("6top: no memory available for a new transaction\n");
        response_output_without_trans(sixtop_ie.sfid,
                                      SIXTOP_RC_ERR_NORES,
                                      sixtop_ie.seqno,
                                      &peer_addr);
        return;
      }

      if(transit_trans_state(trans, SIXTOP_TRANS_STATE_REQUEST_RECEIVED) < 0) {
        PRINTF("6top: invalid state [%u] to receive a request\n",
               trans->state);
        return;
      }

      /* setup a new transaction */
      trans->sf = sf;
      trans->peer_addr = peer_addr;
      trans->seqno = sixtop_ie.seqno;
      trans->cmd = sixtop_ie.code;
      trans->last_msg_body.num_cells = sixtop_ie.num_cells;
      memcpy(trans->last_msg_body.cell_list, sixtop_ie.cell_list,
             sixtop_ie.cell_list_len * sizeof(uint32_t));
      trans->last_msg_body.cell_list_len = sixtop_ie.cell_list_len;
      PRINTF("6top: received a 6top request from ");
      PRINTLLADDR((const uip_lladdr_t *)&peer_addr);
      PRINTF("\n");
    } else if(msg_is_response(&sixtop_ie)) {
      if(trans == NULL || trans->seqno != sixtop_ie.seqno) {
        PRINTF("6top: no correspondent trans. [peer=");
        PRINTLLADDR((const uip_lladdr_t *)&peer_addr);
        PRINTF(", seqno=%u]\n", sixtop_ie.seqno);
        return;
      }

      if(transit_trans_state(trans, SIXTOP_TRANS_STATE_RESPONSE_RECEIVED) < 0) {
        PRINTF("6top: invalid state [%u] to receive a response\n",
               trans->state);
        return;
      }

      /* update the ongoing transaction */
      trans->return_code = sixtop_ie.code;
      memcpy(trans->last_msg_body.cell_list, sixtop_ie.cell_list,
             sixtop_ie.cell_list_len * sizeof(uint32_t));
      trans->last_msg_body.cell_list_len = sixtop_ie.cell_list_len;
      PRINTF("6top: received a 6top response from ");
      PRINTLLADDR((const uip_lladdr_t *)&peer_addr);
      PRINTF("\n");
    } else {
      PRINTF("6top: unsupported msg [code=%u]\n", sixtop_ie.code);
      return;
    }

    /*
     * move payloadbuf_dataptr() to the beginning of the next layer for further
     * processing
     */
    packetbuf_hdrreduce(ies.sixtop_ie_content_ptr - payload_ptr +
                        ies.sixtop_ie_content_len);
  }

  /* Pass to the upper layer */
  llsec_input();
}

void
sixtop_init(void)
{
  int i;

  list_init(trans_list);
  memb_init(&trans_memb);

  for(i = 0; i < SIXTOP_NUM_OF_SCHEDULE_FUNCTIONS; i++) {
    schedule_functions[i] = NULL;
  }
}
