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
#include "net/packetbuf.h"
#include "net/nbr-table.h"
#include "net/mac/mac.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "sys/ctimer.h"

#include "sixtop.h"
#include "sixp-packet.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

/**
 * \brief 6P Transaction States (for internal use)
 */
typedef enum {
  SIXP_TRANS_STATE_INIT = 0,
  SIXP_TRANS_STATE_REQUEST_SENT,
  SIXP_TRANS_STATE_REQUEST_RECEIVED,
  SIXP_TRANS_STATE_RESPONSE_SENT,
  SIXP_TRANS_STATE_RESPONSE_RECEIVED,
  SIXP_TRANS_STATE_CONFIRMATION_SENT,
  SIXP_TRANS_STATE_CONFIRMATION_RECEIVED,
  SIXP_TRANS_STATE_TERMINATING,
} sixp_trans_state_t;

/**
 * \brief 6P Transaction Modes (for internal use)
 */
typedef enum {
  SIXP_TRANS_MODE_2_STEP = 0,
  SIXP_TRANS_MODE_3_STEP
} sixp_trans_mode_t;

/**
 * \brief 6P Log Event (for internal use)
 */
typedef enum {
  SIXP_LOG_EVENT_SEND = 0,
  SIXP_LOG_EVENT_RECV
} sixp_log_event_t;

/**
 * \brief 6P Transaction Data Structure (for internal use)
 */
typedef struct sixp_trans {
  struct sixp_trans *next;
  const sixtop_sf_t *sf;
  linkaddr_t peer_addr;
  uint8_t seqno;
  sixp_command_id_t cmd;
  sixp_trans_state_t state;
  sixp_trans_mode_t mode;
  struct {
    sixp_sent_callback_t func;
    void *arg;
    uint16_t arg_len;
  } callback;
  struct ctimer timer;
} sixp_trans_t;

/**
 * \brief 6P Neighbor Data Structure (for internal use)
 *
 * XXX: for now, we have one nbr object per neighbor, which is shared with
 * multiple SFs. It's unclear whether we should use a different generation
 * counter for each SFs.
 */
typedef struct sixp_nbr {
  struct sixp_nbr *next;
  linkaddr_t addr;
  uint8_t next_seqno;
  uint8_t gtx;
  uint8_t grx;
} sixp_nbr_t;

static void
print_msg_event_log(sixp_type_t type, sixp_code_t code,
                    const linkaddr_t *peer_addr, sixp_log_event_t event);

static sixp_nbr_t *find_nbr(const linkaddr_t *addr);
static sixp_nbr_t *alloc_nbr(const linkaddr_t *addr);
#if 0
static void free_nbr(sixp_nbr_t *nbr);
static void clear_all_nbr(void);
#endif

static void handle_trans_timeout(void *ptr);
static void process_trans(void *ptr);
static void schedule_trans_process(sixp_trans_t *trans);
static int transit_trans_state(sixp_trans_t *trans,
                               sixp_trans_state_t new_state);
static sixp_trans_t *alloc_trans(sixp_code_t code, uint8_t sfid, uint8_t seqno,
                                 const linkaddr_t *peer_addr,
                                 sixp_trans_mode_t mode);
static void free_trans(sixp_trans_t *trans);
static sixp_trans_t *find_trans(const linkaddr_t *peer_addr);
static int trans_state_is_invalid(sixp_type_t sending_msg_type,
                                  sixp_code_t code,
                                  const linkaddr_t *peer_addr);
static sixp_trans_mode_t determine_trans_mode(sixp_command_id_t cmd,
                                              uint16_t body_len);

static int schedule_generation_is_invalid(const sixtop_ie_t *sixtop_ie,
                                          const linkaddr_t *peer_addr);

static int prepare_request(sixp_code_t code, uint8_t sfid,
                           const linkaddr_t *peer_addr, sixp_trans_mode_t mode);
static int fill_sixtop_ie(sixp_type_t type, sixp_code_t code, uint8_t sfid,
                          uint8_t seqno,
                          const sixtop_ie_body_t *body, uint16_t body_len,
                          const linkaddr_t *peer_addr,
                          sixtop_ie_t *ie);
static void mac_callback(void *ptr, int status, int transmissions);
static int send_6p_packet(sixp_type_t type, sixp_code_t code, uint8_t sfid,
                          uint8_t seqno,
                          const sixtop_ie_body_t *body, uint16_t body_len,
                          const linkaddr_t *dest_addr);

static void request_input(const sixtop_ie_t *ie, const linkaddr_t *peer_addr);
static void response_input(const sixtop_ie_t *ie, const linkaddr_t *peer_addr);
static void confirmation_input(const sixtop_ie_t *ie,
                               const linkaddr_t *peer_addr);


MEMB(trans_memb, sixp_trans_t, SIXTOP_6P_MAX_TRANSACTIONS);
LIST(trans_list);

NBR_TABLE(sixp_nbr_t, sixp_nbrs);

/*---------------------------------------------------------------------------*/
static void
print_msg_event_log(sixp_type_t type, sixp_code_t code,
                    const linkaddr_t *peer_addr, sixp_log_event_t event)
{
#if DEBUG
  PRINTF("6top: ");
  PRINTF("%s ", event == SIXP_LOG_EVENT_SEND ? "sending" : "received");
  PRINTF("a 6top ");
  switch(type) {
    case SIXP_TYPE_REQUEST:
      PRINTF("request ");
      break;
    case SIXP_TYPE_RESPONSE:
      PRINTF("response ");
      break;
    case SIXP_TYPE_CONFIRMATION:
      PRINTF("confirmation ");
      break;
    default:
      PRINTF("packet ");
      break;
  }
  PRINTF("[code=%u] %s ", code.value,
         event == SIXP_LOG_EVENT_SEND ? "to" : "from");
  PRINTLLADDR((const uip_lladdr_t *)peer_addr);
  PRINTF("\n");
#endif /* DEBUG */
}
/*---------------------------------------------------------------------------*/
static sixp_nbr_t *
find_nbr(const linkaddr_t *addr)
{
  assert(addr != NULL);

  return nbr_table_get_from_lladdr(sixp_nbrs, addr);
}
/*---------------------------------------------------------------------------*/
static sixp_nbr_t *
alloc_nbr(const linkaddr_t *addr)
{
  sixp_nbr_t *nbr;

  assert(addr != NULL);

  if(find_nbr(addr) != NULL) {
    PRINTF("6top: cannot add nbr; the nbr exists\n");
    return NULL;
  }

  if((nbr = (sixp_nbr_t *)nbr_table_add_lladdr(sixp_nbrs,
                                               addr,
                                               NBR_TABLE_REASON_SIXTOP,
                                               NULL)) == NULL) {
    PRINTF("6top: cannot add nbr; no memory\n");
    return NULL;
  }

  linkaddr_copy(&nbr->addr, addr);
  nbr->next_seqno = SIXTOP_6P_INITIAL_SEQUENCE_NUMBER;
  nbr->gtx = 0;
  nbr->grx = 0;

  return nbr;
}
/*---------------------------------------------------------------------------*/
static void
handle_trans_timeout(void *ptr)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;

  if(trans->sf->timeout_handler != NULL) {
    trans->sf->timeout_handler(trans->cmd,
                               (const linkaddr_t *)&trans->peer_addr);
  }

  transit_trans_state(trans, SIXP_TRANS_STATE_TERMINATING);
}
/*---------------------------------------------------------------------------*/
static void
process_trans(void *ptr)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;

  assert(trans != NULL);

  /* make sure that the timer is stopped */
  ctimer_stop(&trans->timer);

  /* state-specific operation */
  if(trans->state == SIXP_TRANS_STATE_TERMINATING) {
    /* handle the terminating state first */
    PRINTF("6top: trans [peer_addr=");
    PRINTLLADDR((const uip_lladdr_t *)&trans->peer_addr);
    PRINTF(", seqno=%u] is going to be freed\n", trans->seqno);
    free_trans(trans);
    return;
  }

  switch(trans->state) {
    /* do for others */
    case SIXP_TRANS_STATE_RESPONSE_SENT:
    case SIXP_TRANS_STATE_RESPONSE_RECEIVED:
      if(trans->mode == SIXP_TRANS_MODE_2_STEP) {
        transit_trans_state(trans, SIXP_TRANS_STATE_TERMINATING);
      }
      break;
    case SIXP_TRANS_STATE_CONFIRMATION_SENT:
    case SIXP_TRANS_STATE_CONFIRMATION_RECEIVED:
      transit_trans_state(trans, SIXP_TRANS_STATE_TERMINATING);
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

  ctimer_stop(&trans->timer);
  ctimer_set(&trans->timer, 0, process_trans, trans); /* expired immediately */
}
/*---------------------------------------------------------------------------*/
static int
transit_trans_state(sixp_trans_t *trans, sixp_trans_state_t new_state)
{
  assert(trans != NULL);

  /* enforce state transition rules  */
  if((new_state == SIXP_TRANS_STATE_REQUEST_SENT &&
      trans->state != SIXP_TRANS_STATE_INIT) ||
     (new_state == SIXP_TRANS_STATE_REQUEST_RECEIVED &&
      trans->state != SIXP_TRANS_STATE_INIT) ||
     (new_state == SIXP_TRANS_STATE_RESPONSE_SENT &&
      trans->state != SIXP_TRANS_STATE_REQUEST_RECEIVED) ||
     (new_state == SIXP_TRANS_STATE_RESPONSE_RECEIVED &&
      trans->state != SIXP_TRANS_STATE_REQUEST_SENT) ||
     (new_state == SIXP_TRANS_STATE_CONFIRMATION_SENT &&
      trans->state != SIXP_TRANS_STATE_RESPONSE_RECEIVED &&
      trans->mode == SIXP_TRANS_MODE_3_STEP) ||
     (new_state == SIXP_TRANS_STATE_CONFIRMATION_RECEIVED &&
      trans->state != SIXP_TRANS_STATE_RESPONSE_SENT &&
      trans->mode == SIXP_TRANS_MODE_3_STEP)) {
    /* invalid state transition */
    return -1;
  }

  PRINTF("6top: trans %p state changes from %u to %u\n",
         trans, trans->state, new_state);

  trans->state = new_state;
  schedule_trans_process(trans);

  return 0;
}
/*---------------------------------------------------------------------------*/
static sixp_trans_t *
alloc_trans(sixp_code_t code, uint8_t sfid, uint8_t seqno,
            const linkaddr_t *peer_addr, sixp_trans_mode_t mode)
{
  sixp_trans_t *trans;

  assert(peer_addr != NULL);

  if(sixtop_find_sf(sfid) == NULL) {
    PRINTF("6top: cannot find the specified SF [sfid=%u]\n", sfid);
    return NULL;
  }

  if(find_trans(peer_addr) != NULL) {
    PRINTF("6top: trans with ");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF(" already exists\n");
    return NULL;
  }

  if((trans = memb_alloc(&trans_memb)) == NULL) {
    PRINTF("6top: lack of memory\n");
    return NULL;
  }

  memset(trans, 0, sizeof(sixp_trans_t));
  trans->sf = sixtop_find_sf(sfid);
  trans->peer_addr = *peer_addr;
  trans->seqno = seqno;
  trans->cmd = code.value;
  trans->state = SIXP_TRANS_STATE_INIT;
  trans->mode = mode;

  list_add(trans_list, trans);

  return trans;
}
/*---------------------------------------------------------------------------*/
static void
free_trans(sixp_trans_t *trans)
{
  assert(trans != NULL);

  list_remove(trans_list, trans);
  memb_free(&trans_memb, trans);
}
/*---------------------------------------------------------------------------*/
static sixp_trans_t *
find_trans(const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  assert(peer_addr != NULL);

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
static int
trans_state_is_invalid(sixp_type_t sending_msg_type, sixp_code_t code,
                       const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  trans = find_trans(peer_addr);

  switch(sending_msg_type) {
    case SIXP_TYPE_REQUEST:
      /* XXX: we don't support concurrent transactions */
      if(trans != NULL) {
        PRINTF("6top: trans for [peer_addr=");
        PRINTLLADDR((const uip_lladdr_t *)peer_addr);
        PRINTF("] is in process\n");
        return 1;
      }
      break;
    case SIXP_TYPE_RESPONSE:
      if(trans == NULL) {
        if(code.value == SIXP_RC_SUCCESS) {
          PRINTF("6top: should have trans for successful transaction\n");
          return 1;
        }
      } else {
        if(trans->state != SIXP_TRANS_STATE_REQUEST_RECEIVED) {
          PRINTF("6top: invalid state [%u]; cannot send a response\n",
                 trans->state);
          return 1;
        }
      }
      break;
    case SIXP_TYPE_CONFIRMATION:
      if(trans == NULL) {
        PRINTF("6top: no corresponding trans for [peer_addr=");
        PRINTLLADDR((const uip_lladdr_t *)peer_addr);
        PRINTF("]\n");
        return 1;
      }
      if(trans->state != SIXP_TRANS_STATE_RESPONSE_RECEIVED) {
        PRINTF("6top: invalid state [%u]; cannot send a confirmation\n",
               trans->state);
        return 1;
      }
      break;
    default:
      PRINTF("6top: unsupported type %u\n", sending_msg_type);
      return 1;
  }

  return 0; /* valid state */
}
/*---------------------------------------------------------------------------*/
static sixp_trans_mode_t
determine_trans_mode(sixp_command_id_t cmd, uint16_t body_len)
{
  sixp_trans_mode_t mode;

  /*
   * We consider a transaction as 3-step if and only if its request command is
   * either Add or Delete AND its cell_list is empty
   */

  if((cmd == SIXP_CMD_ADD &&
      body_len == offsetof(sixp_req_add_t, cell_list)) ||
     (cmd == SIXP_CMD_DELETE &&
      body_len == offsetof(sixp_req_delete_t, cell_list))) {
    mode = SIXP_TRANS_MODE_3_STEP;
  } else {
    mode = SIXP_TRANS_MODE_2_STEP;
  }

  return mode;
}
/*---------------------------------------------------------------------------*/
static int
schedule_generation_is_invalid(const sixtop_ie_t *sixtop_ie,
                               const linkaddr_t *peer_addr)
{
  sixp_nbr_t *nbr;

  assert(sixtop_ie != NULL && peer_addr != NULL);

  if((sixtop_ie->type == SIXP_TYPE_REQUEST &&
      sixtop_ie->code.value == SIXP_CMD_CLEAR) ||
     ((sixtop_ie->type == SIXP_TYPE_RESPONSE ||
       sixtop_ie->type == SIXP_TYPE_RESPONSE) &&
      sixtop_ie->code.value == SIXP_RC_ERR_GEN)) {
    /*
     * The draft-03 says, "if the code of the 6P Request is CMD_CLEAR, the
     * schedule generation inconsistency MUST be ignored." In this case, return
     * 0, which means "the schedule generation is not invalid (it is valid)".
     */

    /*
     * XXX: it's unclear how to deal with generation inconsistency in response
     * or confirmation.
     */
    return 0;
  }

  nbr = find_nbr(peer_addr);

  PRINTF("6top: verify schedule counter - GTX %u, GRX %u, ",
         nbr == NULL ? 0 : nbr->gtx,
         nbr == NULL ? 0 : nbr->grx);
  PRINTF("GAB %u, GBA %u\n", sixtop_ie->gab, sixtop_ie->gba);

  if(nbr == NULL) {
    /*
     * We've not had any transaction with the peer before; both GAB and GBA MUST
     * be 0 (clear or never scheduled). In other words, if GAB is not zero, it's
     * invalid. The same applies to GBA.
     */
    return (sixtop_ie->gab != 0 || sixtop_ie->gba != 0);
  } else if(sixtop_ie->type == SIXP_TYPE_REQUEST ||
            sixtop_ie->type == SIXP_TYPE_CONFIRMATION) {
    /*
     * Follow what the draft-03 says, "when node B receives a 6P Request of
     * (it's typo, should be "or") 6P confirmation from node A, it verifies that
     * GAB==GRX and GBA==GTX."
     */
    return (sixtop_ie->gab != nbr->grx || sixtop_ie->gba != nbr->gtx);
  } else if(sixtop_ie->type == SIXP_TYPE_RESPONSE) {
    /*
     * Follow what the draft-03 says, "when node A receives a 6P Response from
     * node B, it verifies that GAB==GTX and GBA==GRX."
     */
    return (sixtop_ie->gab != nbr->gtx || sixtop_ie->gba != nbr->grx);
  }

  /* not expected to come here; treat this case as invalid returning 1 */
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare_request(sixp_code_t code, uint8_t sfid, const linkaddr_t *peer_addr,
                sixp_trans_mode_t mode)
{
  sixp_trans_t *trans;
  sixp_nbr_t *nbr;

  assert(peer_addr != NULL);

  /* alloc nbr if necessary */
  nbr = find_nbr(peer_addr);
  if(nbr == NULL && (nbr = alloc_nbr(peer_addr)) == NULL) {
    PRINTF("6top: cannot allocate nbr for ");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF("\n");
    return -1;
  }

  /* alloc trans */
  trans = alloc_trans(code, sfid, nbr->next_seqno, peer_addr, mode);
  if(trans == NULL) {
    PRINTF("6top: cannot send 6P packet; failed to allocate trans\n");
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
fill_sixtop_ie(sixp_type_t type, sixp_code_t code, uint8_t sfid, uint8_t seqno,
               const sixtop_ie_body_t *body, uint16_t body_len,
               const linkaddr_t *peer_addr,
               sixtop_ie_t *ie)
{
  sixp_nbr_t *nbr;

  assert(peer_addr != NULL && ie != NULL);

  memset(ie, 0, sizeof(sixtop_ie_t));

  ie->type = type;
  ie->code.value = code.value;
  ie->sfid = sfid;
  ie->seqno = seqno;

  if((nbr = find_nbr(peer_addr)) == NULL) {
    ie->gab = 0;
    ie->gba = 0;
  } else {
    if(type == SIXP_TYPE_REQUEST || type == SIXP_TYPE_CONFIRMATION) {
      ie->gab = nbr->gtx;
      ie->gba = nbr->grx;
    } else if(type == SIXP_TYPE_RESPONSE) {
      ie->gab = nbr->grx;
      ie->gba = nbr->gtx;
    } else {
      PRINTF("6top: unexpected type %u\n", type);
      return -1;
    }
  }

  ie->body = body_len == 0 ? NULL : body;
  ie->body_len = body == NULL ? 0 : body_len;

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
mac_callback(void *ptr, int status, int transmissions)
{
  sixp_trans_t *trans = (sixp_trans_t *)ptr;
  int ret = 0;

  assert(trans != NULL);

  switch(trans->state) {
    case SIXP_TRANS_STATE_INIT:
      if(status == MAC_TX_OK) {
        ret = transit_trans_state(trans, SIXP_TRANS_STATE_REQUEST_SENT);
      } else {
        /*
         * We'll drop the transaction immediately when the attempt to send a
         * request failed. For the case where SF retransmits the request, a new
         * transaction will be allocated.
         */
        ret = transit_trans_state(trans, SIXP_TRANS_STATE_TERMINATING);
      }
      break;
    case SIXP_TRANS_STATE_REQUEST_RECEIVED:
      if(status == MAC_TX_OK) {
        ret = transit_trans_state(trans, SIXP_TRANS_STATE_RESPONSE_SENT);
      } else {
        /* stay at the current state when sending a response fails */
      }
      break;
    case SIXP_TRANS_STATE_RESPONSE_RECEIVED:
      if(status == MAC_TX_OK) {
        ret = transit_trans_state(trans, SIXP_TRANS_STATE_CONFIRMATION_SENT);
      } else {
        /* stay at the current state when sending a response fails */
      }
      break;
    default:
      PRINTF("6top: unexpected trans state (%u) in mac_callback()\n",
             trans->state);
      break;
  }

  if(ret < 0) {
    PRINTF("6top: state transition failure\n");
  }

  if(trans->callback.func != NULL) {
    trans->callback.func(trans->callback.arg, trans->callback.arg_len,
                         &trans->peer_addr,
                         status == MAC_TX_OK ?
                         SIXP_SEND_STATUS_SUCCESS :
                         SIXP_SEND_STATUS_FAILURE);
  }
  memset(&trans->callback, 0, sizeof(trans->callback));
}
/*---------------------------------------------------------------------------*/
static int
send_6p_packet(sixp_type_t type, sixp_code_t code, uint8_t sfid, uint8_t seqno,
               const sixtop_ie_body_t *body, uint16_t body_len,
               const linkaddr_t *dest_addr)
{
  sixtop_ie_t ie;

  if(fill_sixtop_ie(type, code, sfid, seqno,
                    body, body_len, dest_addr, &ie) < 0) {
    PRINTF("6top: failed to fill sixtop_ie\n");
    return -1;
  }

  if(sixp_packet_create(&ie) < 0) {
    PRINTF("6top: failed to build a 6top request message\n");
    return -1;
  }

  sixtop_output(dest_addr, mac_callback, (void *)find_trans(dest_addr));

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
request_input(const sixtop_ie_t *ie, const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  assert(ie != NULL && peer_addr != NULL);

  if((trans = find_trans(peer_addr)) != NULL) {
    PRINTF("6top: another request [peer_addr=");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF(" seqno=%u] is in process\n", trans->seqno);
    send_6p_packet(SIXP_TYPE_RESPONSE,
                   (sixp_code_t)(sixp_return_code_t)SIXP_RC_ERR_BUSY,
                   ie->sfid, ie->seqno, NULL, 0, peer_addr);
    return;
  }

  if((trans = alloc_trans(ie->code, ie->sfid, ie->seqno,
                          peer_addr,
                          determine_trans_mode(ie->code.value,
                                               ie->body_len))) == NULL) {
    PRINTF("6top: no memory available for a new transaction\n");
    send_6p_packet(SIXP_TYPE_RESPONSE,
                   (sixp_code_t)(sixp_return_code_t)SIXP_RC_ERR_NORES,
                   ie->sfid, ie->seqno, NULL, 0, peer_addr);

    return;
  }

  if(transit_trans_state(trans, SIXP_TRANS_STATE_REQUEST_RECEIVED) < 0) {
    PRINTF("6top: invalid state [%u] to receive a 6top request\n",
           trans->state);
    return;
  }

  if(trans->sf->request_input != NULL) {
    trans->sf->request_input(ie->code.cmd, ie->body, ie->body_len, peer_addr);
  }
}
/*---------------------------------------------------------------------------*/
static void
response_input(const sixtop_ie_t *ie, const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  assert(ie != NULL && peer_addr != NULL);

  if((trans = find_trans(peer_addr)) == NULL || trans->seqno != ie->seqno) {
    PRINTF("6top: no correspondent trans. [peer=");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF(", seqno=%u]\n", ie->seqno);
    return;
  }

  if(transit_trans_state(trans, SIXP_TRANS_STATE_RESPONSE_RECEIVED) < 0) {
    PRINTF("6top: invalid state [%u] to receive a 6top response\n",
           trans->state);
    return;
  }

  if(trans->sf->response_input != NULL) {
    trans->sf->response_input(trans->cmd, ie->code.rc, ie->body, ie->body_len,
                              peer_addr);
  }
}
/*---------------------------------------------------------------------------*/
static void
confirmation_input(const sixtop_ie_t *ie, const linkaddr_t *peer_addr)
{
  sixp_trans_t *trans;

  assert(ie != NULL && peer_addr != NULL);

  if((trans = find_trans(peer_addr)) == NULL || trans->seqno != ie->seqno) {
    PRINTF("6top: no correspondent trans. [peer=");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF(", seqno=%u]\n", ie->seqno);
    return;
  }

  if(transit_trans_state(trans, SIXP_TRANS_STATE_CONFIRMATION_RECEIVED) < 0) {
    PRINTF("6top: invalid state [%u] to receive a 6top confirmation\n",
           trans->state);
    return;
  }

  if(trans->sf->confirmation_input != NULL) {
    trans->sf->confirmation_input(trans->cmd, ie->code.rc,
                                  ie->body, ie->body_len,
                                  peer_addr);
  }
}
/*---------------------------------------------------------------------------*/
int
sixp_send(sixp_type_t type, sixp_code_t code, uint8_t sfid,
          const sixtop_ie_body_t *body, uint16_t body_len,
          const linkaddr_t *dest_addr,
          sixp_sent_callback_t func, void *arg, uint16_t arg_len)
{
  sixp_trans_t *trans;

  assert(dest_addr != NULL);

  if(trans_state_is_invalid(type, code, dest_addr)) {
    PRINTF("6top: cannot send 6P packet; invalid transaction state\n");
    return -1;
  }

  if(type == SIXP_TYPE_REQUEST) {
    /* special process for the request case: allocate trans, sure to have nbr */
    if(prepare_request(code, sfid, dest_addr,
                       determine_trans_mode(code.cmd, body_len)) < 0) {
      PRINTF("6top: cannot send 6P request; prepare_request() failed\n");
      return -1;
    }
  }

  trans = find_trans(dest_addr);
  assert(trans != NULL);

  if(send_6p_packet(type, code, sfid, trans->seqno,
                    body, body_len, dest_addr) < 0) {
    if(type == SIXP_TYPE_REQUEST) {
      /* throw away trans; will allocate another trans for retransmission */
      transit_trans_state(trans, SIXP_TRANS_STATE_TERMINATING);
    }
    PRINTF("6top: failed to send a 6P packet\n");
    return -1;
  }

  print_msg_event_log(type, code, dest_addr, SIXP_LOG_EVENT_SEND);

  if(trans != NULL) {
    trans->callback.func = func == NULL ? NULL : func;
    trans->callback.arg = arg_len == 0 ? NULL : arg;
    trans->callback.arg_len = arg == NULL ? 0 : arg_len;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
sixp_input(const uint8_t *buf, uint16_t len, const linkaddr_t *src_addr)
{
  sixtop_ie_t ie;

  assert(buf != NULL && src_addr);

  if(sixp_packet_parse(buf, len, &ie) < 0) {
    PRINTF("6top: received malformed 6top SubIE\n");
    return;
  }

  if(sixtop_find_sf(ie.sfid) == NULL) {
    PRINTF("6top: SF [sfid=%u] is unavailable\n",
           ie.sfid);
    send_6p_packet(SIXP_TYPE_RESPONSE,
                   (sixp_code_t)(sixp_return_code_t)SIXP_RC_ERR_SFID,
                   ie.sfid, ie.seqno, NULL, 0, src_addr);
    return;
  }

  if(schedule_generation_is_invalid(&ie, src_addr)) {
    PRINTF("6top: invalid schedule generation\n");
    send_6p_packet(SIXP_TYPE_RESPONSE,
                   (sixp_code_t)(sixp_return_code_t)SIXP_RC_ERR_GEN,
                   ie.sfid, ie.seqno, NULL, 0, src_addr);
    return;
  }

  print_msg_event_log(ie.type, ie.code, src_addr, SIXP_LOG_EVENT_RECV);

  switch(ie.type) {
    case SIXP_TYPE_REQUEST:
      request_input(&ie, src_addr);
      break;
    case SIXP_TYPE_RESPONSE:
      response_input(&ie, src_addr);
      break;
    case SIXP_TYPE_CONFIRMATION:
      confirmation_input(&ie, src_addr);
      break;
    default:
      PRINTF("6top: unsupported type [type=%u]\n", ie.type);
      break;
  }

  return;
}
/*---------------------------------------------------------------------------*/
int
sixp_advance_generation(const linkaddr_t *peer_addr,
                        sixp_gen_type_t type)
{
  sixp_nbr_t *nbr;
  uint8_t *gen_ctr;

  if(((nbr = find_nbr(peer_addr)) == NULL) &&
     ((nbr = alloc_nbr(peer_addr)) == NULL)) {
    PRINTF("6top: cannot found/added nbr for ");
    PRINTLLADDR((const uip_lladdr_t *)peer_addr);
    PRINTF("\n");
    return -1;
  }

  switch(type) {
    case SIXP_GEN_TYPE_TX:
      gen_ctr = &nbr->gtx;
      break;
    case SIXP_GEN_TYPE_RX:
      gen_ctr = &nbr->grx;
      break;
    default:
      /* not expected to come here */
      PRINTF("6top: invalid schedule generation type %u\n", type);
      return -1;
  }

  switch(*gen_ctr) {
    case 0b00:
    case 0b10:
      *gen_ctr = 0b01;
      break;
    case 0b01:
      *gen_ctr = 0b10;
      break;
    default:
      /* not expected to come here */
      PRINTF("6top: invalid schedule generation value %u\n", *gen_ctr);
      break;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
sixp_init(void)
{
  list_init(trans_list);
  memb_init(&trans_memb);

  nbr_table_register(sixp_nbrs, NULL);
}
/*---------------------------------------------------------------------------*/
/** @} */
