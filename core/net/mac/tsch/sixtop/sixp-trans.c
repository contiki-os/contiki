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
 *         Transaction Management for 6top Protocol (6P)
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "contiki-lib.h"
#include "lib/assert.h"

#include "sixtop.h"
#include "sixtop-conf.h"
#include "sixp-trans.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

/**
 * \brief 6P Transaction Data Structure (for internal use)
 */
typedef struct sixp_trans {
  struct sixp_trans *next;
  const sixtop_sf_t *sf;
  linkaddr_t peer_addr;
  uint8_t seqno;
  sixp_pkt_cmd_t cmd;
  sixp_trans_state_t state;
  sixp_trans_mode_t mode;
  struct {
    sixp_sent_callback_t func;
    void *arg;
    uint16_t arg_len;
  } callback;
  struct ctimer timer;
} sixp_trans_t;

static void handle_trans_timeout(void *ptr);
static void process_trans(void *ptr);
static void schedule_trans_process(sixp_trans_t *trans);
static void free_trans(sixp_trans_t *trans);
static sixp_trans_mode_t determine_trans_mode(const sixp_pkt_t *req);

MEMB(trans_memb, sixp_trans_t, SIXTOP_6P_MAX_TRANSACTIONS);
LIST(trans_list);

/*---------------------------------------------------------------------------*/
static void
handle_trans_timeout(void *ptr)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;

  assert(trans != NULL);
  if(trans == NULL) {
    return;
  }

  if(trans->sf->timeout != NULL) {
    trans->sf->timeout(trans->cmd,
                       (const linkaddr_t *)&trans->peer_addr);
  }

  (void)sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING);
}
/*---------------------------------------------------------------------------*/
static void
process_trans(void *ptr)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;

  assert(trans != NULL);
  if(trans == NULL) {
    return;
  }

  /* make sure that the timer is stopped */
  ctimer_stop(&trans->timer);

  /* state-specific operation */
  if(trans->state == SIXP_TRANS_STATE_TERMINATING) {
    /* handle the terminating state first */
    PRINTF("6P-trans: trans [peer_addr:");
    PRINTLLADDR((const uip_lladdr_t *)&trans->peer_addr);
    PRINTF(", seqno:%u] is going to be freed\n", trans->seqno);
    free_trans(trans);
    return;
  }

  switch(trans->state) {
    /* do for others */
    case SIXP_TRANS_STATE_RESPONSE_SENT:
    case SIXP_TRANS_STATE_RESPONSE_RECEIVED:
      if(trans->mode == SIXP_TRANS_MODE_2_STEP) {
        (void)sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING);
      }
      break;
    case SIXP_TRANS_STATE_CONFIRMATION_SENT:
    case SIXP_TRANS_STATE_CONFIRMATION_RECEIVED:
      (void)sixp_trans_transit_state(trans, SIXP_TRANS_STATE_TERMINATING);
      break;
    case SIXP_TRANS_STATE_TERMINATING:
    default:
      break;
  }

  if(trans->state != SIXP_TRANS_STATE_TERMINATING) {
    /* set the timer with a timeout values defined by the SF */
    ctimer_set(&trans->timer, trans->sf->timeout_interval,
               handle_trans_timeout, trans);
  }
}
/*---------------------------------------------------------------------------*/
static void
schedule_trans_process(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    return;
  }

  ctimer_stop(&trans->timer);
  ctimer_set(&trans->timer, 0, process_trans, trans); /* expires immediately */
}
/*---------------------------------------------------------------------------*/
static void
free_trans(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    return;
  }

  list_remove(trans_list, trans);
  memb_free(&trans_memb, trans);
}
/*---------------------------------------------------------------------------*/
static sixp_trans_mode_t
determine_trans_mode(const sixp_pkt_t *req)
{
  uint16_t cell_list_len;

  assert(req != NULL);
  if(req == NULL) {
    return SIXP_TRANS_MODE_UNAVAILABLE;
  }

  /*
   * We consider a transaction as 3-step if and only if its request command is
   * either Add or Delete AND its cell_list is empty. Otherwise, 2-step.
   */
  if(req->type == SIXP_PKT_TYPE_REQUEST &&
     (req->code.cmd == SIXP_PKT_CMD_ADD ||
      req->code.cmd == SIXP_PKT_CMD_DELETE) &&
     sixp_pkt_get_cell_list(req->type, (sixp_pkt_code_t)req->code.value,
                            NULL, &cell_list_len,
                            req->body, req->body_len) == 0 &&
     cell_list_len == 0) {
    return SIXP_TRANS_MODE_3_STEP;
  }

  return SIXP_TRANS_MODE_2_STEP;
}
/*---------------------------------------------------------------------------*/
int
sixp_trans_transit_state(sixp_trans_t *trans, sixp_trans_state_t new_state)
{
  assert(trans != NULL);
  if(trans == NULL) {
    PRINTF("6top: invalid argument, trans is NULL\n");
    return -1;
  }

  /* enforce state transition rules  */
  if(new_state == SIXP_TRANS_STATE_TERMINATING ||
     (new_state == SIXP_TRANS_STATE_REQUEST_SENT &&
      trans->state == SIXP_TRANS_STATE_INIT) ||
     (new_state == SIXP_TRANS_STATE_REQUEST_RECEIVED &&
      trans->state == SIXP_TRANS_STATE_INIT) ||
     (new_state == SIXP_TRANS_STATE_RESPONSE_SENT &&
      trans->state == SIXP_TRANS_STATE_REQUEST_RECEIVED) ||
     (new_state == SIXP_TRANS_STATE_RESPONSE_RECEIVED &&
      trans->state == SIXP_TRANS_STATE_REQUEST_SENT) ||
     (new_state == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED &&
      trans->state == SIXP_TRANS_STATE_RESPONSE_SENT &&
      trans->mode == SIXP_TRANS_MODE_3_STEP) ||
     (new_state == SIXP_TRANS_STATE_CONFIRMATION_SENT &&
      trans->state == SIXP_TRANS_STATE_RESPONSE_RECEIVED &&
      trans->mode == SIXP_TRANS_MODE_3_STEP)) {
    PRINTF("6P-trans: trans %p state changes from %u to %u\n",
           trans, trans->state, new_state);
    trans->state = new_state;
    schedule_trans_process(trans);
    return 0;
  }

  /* invalid transition */
  PRINTF("6P-trans: invalid transaction, from %u to %u, detected on trans %p\n",
         trans->state, new_state, trans);
  return -1;
}
/*---------------------------------------------------------------------------*/
sixp_pkt_cmd_t
sixp_trans_get_cmd(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    return SIXP_PKT_CMD_UNAVAILABLE;
  }
  return trans->cmd;
}
/*---------------------------------------------------------------------------*/
sixp_trans_state_t
sixp_trans_get_state(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    return SIXP_TRANS_STATE_UNAVAILABLE;
  }
  return trans->state;
}
/*---------------------------------------------------------------------------*/
int16_t
sixp_trans_get_seqno(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    PRINTF("6P-trans: sixp_trans_get_seqno() fails because trans is NULL\n");
    return -1;
  }
  return trans->seqno;
}
/*---------------------------------------------------------------------------*/
sixp_trans_mode_t
sixp_trans_get_mode(sixp_trans_t *trans)
{
  assert(trans != NULL);
  if(trans == NULL) {
    PRINTF("6P-trans: sixp_trans_get_mode() fails because trans is NULL\n");
    return SIXP_TRANS_STATE_UNAVAILABLE;
  }
  return trans->mode;
}
/*---------------------------------------------------------------------------*/
void
sixp_trans_invoke_callback(sixp_trans_t *trans, sixp_output_status_t status)
{
  assert(trans != NULL);

  if(trans == NULL || trans->callback.func == NULL) {
    return;
  }
  trans->callback.func(trans->callback.arg, trans->callback.arg_len,
                       &trans->peer_addr, status);
}
/*---------------------------------------------------------------------------*/
void
sixp_trans_set_callback(sixp_trans_t *trans,
                        sixp_sent_callback_t func, void *arg, uint16_t arg_len)
{
  assert(trans != NULL);
  if(trans == NULL) {
    return;
  }
  trans->callback.func = func;
  trans->callback.arg = arg;
  trans->callback.arg_len = arg_len;
}
/*---------------------------------------------------------------------------*/
sixp_trans_t *
sixp_trans_alloc(const sixp_pkt_t *pkt, const linkaddr_t *peer_addr)
{
  const sixtop_sf_t *sf;
  sixp_trans_t *trans;

  assert(pkt != NULL && peer_addr != NULL);
  if(pkt == NULL || peer_addr == NULL) {
    PRINTF("6P-trans: sixp_trans_alloc() fails because of invalid argument\n");
    return NULL;
  }

  if((sf = sixtop_find_sf(pkt->sfid)) == NULL) {
    PRINTF("6P-trans: sixp_trans_alloc() fails; no suitable SF [sfid:%u]\n",
           pkt->sfid);
    return NULL;
  }

  if(sixp_trans_find(peer_addr) != NULL) {
    PRINTF("6P-trans: sixp_trans_alloc() fails because another trans with ");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF("is in process\n");
    return NULL;
  }

  if((trans = memb_alloc(&trans_memb)) == NULL) {
    PRINTF("6P-trans: sixp_trans_alloc() fails because of lack of memory\n");
    return NULL;
  }

  memset(trans, 0, sizeof(sixp_trans_t));
  trans->sf = sf;
  trans->peer_addr = *peer_addr;
  trans->seqno = pkt->seqno;
  trans->cmd = pkt->code.value;
  trans->state = SIXP_TRANS_STATE_INIT;
  trans->mode = determine_trans_mode(pkt);
  list_add(trans_list, trans);

  return trans;
}
/*---------------------------------------------------------------------------*/
sixp_trans_t *
sixp_trans_find(const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  assert(peer_addr != NULL);
  if(peer_addr == NULL) {
    return NULL;
  }

  /*
   * XXX: we don't support concurrent 6P transactions which is mentioned in
   * Section 4.3.3, draft-ietf-6tisch-6top-protocol-03.
   *
   * The assumption here is that there is one transactions for a single peer at
   * most.
   */
  for(trans = list_head(trans_list);
      trans != NULL; trans = trans->next) {
    if(memcmp(peer_addr, &trans->peer_addr, sizeof(linkaddr_t)) == 0) {
      return trans;
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
int
sixp_trans_init(void)
{
  list_init(trans_list);
  memb_init(&trans_memb);
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
