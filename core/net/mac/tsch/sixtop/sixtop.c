/*
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
 *         IEEE 802.15.4 TSCH MAC Sixtop.
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 */

#include <stdio.h>
#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "lib/random.h"
#include "sixtop.h"
#include "net/mac/frame802154e-ie.h"

#define DEBUG DEBUG_PRINT
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#endif

#define WRITE16(buf, val) \
  do { ((uint8_t *)(buf))[0] = (val) & 0xff; \
       ((uint8_t *)(buf))[1] = ((val) >> 8) & 0xff; \
  } while(0);

#define READ16(buf, var) \
  (var) = ((uint8_t *)(buf))[0] | ((uint8_t *)(buf))[1] << 8

/* Required number of links specified in Link Request IE */
uint8_t sixtop_request_num_links = 0;
/* Sequence Number of Link Request IE */
static int sixtop_request_seqno = 0;
/* Sequence Number of Link Response IE */
static int sixtop_response_seqno = 0;
/* Destination address of the last packet sent */
static linkaddr_t last_pkt_address;
/* IE information of the last packet sent */
struct ieee802154_ies last_pkt_ies;
/* SlotFrame ID specified in Link Request IE */
struct tsch_slotframe *req_sf;

/*---------------------------------------------------------------------------*/
/* Update the packet to be send with Sixtop Information Elements. Returns length of IE */
int
sixtop_update_ie(uint8_t *buf, struct ieee802154_ies *ies)
{
  if(ies != NULL) {
    uint8_t i = 0, code = 0, ie_len = 0, max = 0;

    buf[2] = ies->ie_sixtop.subIE_id;
    buf[3] = ies->ie_sixtop.version_code;
    buf[4] = ies->ie_sixtop.schedule_fn_id;

    /* 6P Code */
    code = ies->ie_sixtop.version_code >> 4;

    switch(code) {
    case CMD_ADD:
    case CMD_DELETE:
      buf[5] = ies->ie_sixtop.num_links;
      buf[6] = ies->ie_sixtop.frame_id;

      if(code == CMD_ADD) {
        max = SIXTOP_IE_MAX_LINKS;
      } else {     /* CMD_DELETE */
        max = sixtop_request_num_links;
      }
      for(i = 0; i < max; i++) {
        WRITE16(buf + 7 + i * 4, ies->ie_sixtop.linkList[i].timeslot);
        WRITE16(buf + 7 + i * 4 + 2, ies->ie_sixtop.linkList[i].channel_offset);
      }

      ie_len = 5 + 4 * max;
      break;

    case RC_SUCCESS:     /* Response Code Success */
      for(i = 0; i < sixtop_request_num_links; i++) {
        WRITE16(buf + 5 + i * 4, ies->ie_sixtop.linkList[i].timeslot);
        WRITE16(buf + 5 + i * 4 + 2, ies->ie_sixtop.linkList[i].channel_offset);
      }

      ie_len = 3 + 4 * sixtop_request_num_links;
      break;

    default:
      return -1;
    }
    return ie_len + 2; /* IE_len + IE_descriptor_len */
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
/* Create a Sixtop IE for specified 6P code. Returns length of IEEE802.15.4e packet */
int
sixtop_create_ie(uint8_t *buf, linkaddr_t dest_addr, uint8_t code,
                 uint8_t sf_id, struct sixtop_link *sl)
{
  uint8_t i = 0, len = 0, curr_len = 0, max = 0, sixtop_ie_offset = 0;
  /* Schedule Function ID */
  uint8_t schedule_fn_id = 0; /* SF0 */
  struct ieee802154_ies ies;

  /* IEEE 802.15.4 MAC Frame Header */
  frame802154_t p;
  memset(&p, 0, sizeof(p));

  /* MAC Header : Frame Control Field */
  p.fcf.frame_type = FRAME802154_DATAFRAME;
  p.fcf.security_enabled = 0;
  p.fcf.frame_pending = 0;
  p.fcf.ack_required = 1;
  p.fcf.panid_compression = 0;
  p.fcf.sequence_number_suppression = FRAME802154_SUPPR_SEQNO;
  p.fcf.ie_list_present = 1;
  p.fcf.dest_addr_mode = FRAME802154_LONGADDRMODE;
  p.fcf.frame_version = FRAME802154_IEEE802154E_2012;
  p.fcf.src_addr_mode = FRAME802154_LONGADDRMODE;

  /* MAC Header : Sequence Number Field */
  switch(code) {
  case CMD_ADD:
  case CMD_DELETE:
    p.seq = sixtop_request_seqno;
    sixtop_request_seqno++;
    break;

  case RC_SUCCESS:     /* Response Code Success */
    p.seq = sixtop_response_seqno;
    break;

  default:
    break;
  }

  /* MAC Header : PAN ID and Address Fields */
  p.dest_pid = frame802154_get_pan_id();
  linkaddr_copy((linkaddr_t *)&p.dest_addr, &dest_addr);
  p.src_pid = frame802154_get_pan_id();
  linkaddr_copy((linkaddr_t *)&p.src_addr, &linkaddr_node_addr);

  /* MAC Header : Auxillary Security Header */
#if TSCH_SECURITY_ENABLED
  if(tsch_is_pan_secured) {
    p.fcf.security_enabled = 1;
    p.aux_hdr.security_control.security_level = TSCH_SECURITY_KEY_SEC_LEVEL_ACK;
    p.aux_hdr.security_control.key_id_mode = FRAME802154_1_BYTE_KEY_ID_MODE;
    p.aux_hdr.security_control.frame_counter_suppression = 1;
    p.aux_hdr.security_control.frame_counter_size = 1;
    p.aux_hdr.key_index = TSCH_SECURITY_KEY_INDEX_ACK;
  }
#endif /* TSCH_SECURITY_ENABLED */

  /* Copy MAC Header to Buffer */
  if((curr_len = frame802154_create(&p, buf)) == 0) {
    return 0;
  }

  memset(&ies, 0, sizeof(ies));

  /* Header IE Termination 1 */
  /* First add header-IE termination IE to stipulate that next come payload IEs */
  if((len = frame80215e_create_ie_header_list_termination_1(buf + curr_len, PACKETBUF_SIZE - curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += len;

  /* Save offset of the Sixtop IE descriptor, we need to know the total length
   * before writing it */
  sixtop_ie_offset = curr_len;

  /* Update Sixtop Information Elements  */
  ies.ie_sixtop.subIE_id = SIXTOP_SUBIE_ID;
  ies.ie_sixtop.version_code = (code << 4) | SIXTOP_VERSION;
  ies.ie_sixtop.schedule_fn_id = schedule_fn_id;

  switch(code) {
  case CMD_ADD:
  case CMD_DELETE:
    if(code == CMD_ADD) {
      max = SIXTOP_IE_MAX_LINKS;
    } else {
      max = 1;
    }

    ies.ie_sixtop.frame_id = sf_id;
    ies.ie_sixtop.num_links = sixtop_request_num_links;

    for(i = 0; i < max; i++) {
      if(sl[i].link_option != LINK_OPTION_OFF) {
        ies.ie_sixtop.linkList[i].timeslot = sl[i].timeslot;
        ies.ie_sixtop.linkList[i].channel_offset = sl[i].channel_offset;
      }
    }
    break;

  case RC_SUCCESS:           /* Response Code Success */
    for(i = 0; i < sixtop_request_num_links; i++) {
      ies.ie_sixtop.linkList[i].timeslot = sl[i].timeslot;
      ies.ie_sixtop.linkList[i].channel_offset = sl[i].channel_offset;
    }
    break;

  default:
    break;
  }

  /* Save a copy of Sixtop IE */
  last_pkt_ies = ies;

  /* Copy Sixtop IE content to buffer */
  if((len = sixtop_update_ie(buf + curr_len, &ies)) == -1) {
    return -1;
  }
  curr_len += len;

  /* Outer IE descriptor - Sixtop */
  ies.ie_mlme_len = curr_len - sixtop_ie_offset - 2;
  if((len = frame80215e_create_ie_sixtop(buf + sixtop_ie_offset, PACKETBUF_SIZE - sixtop_ie_offset, &ies)) == -1) {
    return -1;
  }

  return curr_len;
}
/*---------------------------------------------------------------------------*/
/* Fetch the CandidateLinkList for Addition. Returns 0 if success */
int
sixtop_get_link_list_for_addition(uint8_t sf_id, struct sixtop_link *sl)
{
  uint8_t i = 0, index = 0;
  /* Flag to prevent repeated slots */
  uint8_t slot_check = 1;
  uint16_t random_slot = 0;
  struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(sf_id);

  do {
    /* Randomly select a slot offset within TSCH_SCHEDULE_DEFAULT_LENGTH */
    random_slot = ((random_rand() & 0xFF)) % TSCH_SCHEDULE_DEFAULT_LENGTH;

    if(!tsch_schedule_get_link_by_timeslot(sf, random_slot)) { /* NULL value indicates a free link */

      /* To prevent repeated slots */
      for(i = 0; i < index; i++) {
        if(!(sl[i].timeslot == random_slot)) {
          /* Random selection resulted in a free slot */
          if(i == index - 1) { /* Checked till last index of link list */
            slot_check = 1;
            break;
          }
        } else {
          /* Slot already present in CandidateLinkList */
          slot_check++;
          break;
        }
      }

      /* Random selection resulted in a free slot, add it to linklist */
      if(slot_check == 1) {
        sl[index].timeslot = random_slot;
        sl[index].channel_offset = 0;
        sl[index].link_option = LINK_OPTION_TX;
        index++;
        slot_check++;
      } else if(slot_check > TSCH_SCHEDULE_DEFAULT_LENGTH) {
        PRINTF("TSCH-sixtop:! Number of trials for free slot exceeded...\n");
        return -1;
        break; /* exit while loop */
      }
    }
  } while(index < SIXTOP_IE_MAX_LINKS);

  if(index == 0) {
    return -1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
/* Fetches the CandidateLinkList (atmost one link) for Deletion. Returns 0 if success */
int
sixtop_get_link_list_for_deletion(uint8_t sf_id, struct sixtop_link *sl, linkaddr_t *dest_addr)
{
  uint8_t i = 0, index = 0;
  struct tsch_link *l;
  struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(sf_id);

  for(i = 0; i < TSCH_SCHEDULE_DEFAULT_LENGTH; i++) {
    l = tsch_schedule_get_link_by_timeslot(sf, i);

    if(l) {
      /* Non-zero value indicates a scheduled link */
      if((linkaddr_cmp(&l->addr, dest_addr)) && (l->link_options == LINK_OPTION_TX)) {
        /* This link is scheduled as a TX link to the specified neighbor */
        sl[index].timeslot = i;
        sl[index].channel_offset = l->channel_offset;
        sl[index].link_option = LINK_OPTION_TX;
        index++;
        break;   /* delete atmost one */
      }
    }
  }

  if(index == 0) {
    return -1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
/* Adds links to the schedule as Tx/Rx depending on the sixtop_state value */
void
sixtop_add_links_to_schedule(uint8_t sf_id, struct sixtop_link *sl,
                             linkaddr_t *previous_hop, uint8_t state)
{
  uint8_t i = 0, num_added_links = 0;

  for(i = 0; i < SIXTOP_IE_MAX_LINKS; i++) {
    if(sl[i].timeslot != 0xFFFF) {
      switch(state) {
      case SIXTOP_ADD_RESPONSE_WAIT_SENDDONE:
        PRINTF("TSCH-sixtop: Schedule link %d as RX with node %u\n", sl[i].timeslot, previous_hop->u8[7]);
        /* Add a TX link to neighbor */
        tsch_schedule_add_link(tsch_schedule_get_slotframe_by_handle(sf_id),
                               LINK_OPTION_RX, LINK_TYPE_NORMAL, previous_hop,
                               sl[i].timeslot, sl[i].channel_offset);
        break;

      case SIXTOP_ADD_RESPONSE_RECEIVED:
        PRINTF("TSCH-sixtop: Schedule link %d as TX with node %u\n", sl[i].timeslot, previous_hop->u8[7]);
        /* Add a RX link to neighbor */
        tsch_schedule_add_link(tsch_schedule_get_slotframe_by_handle(sf_id),
                               LINK_OPTION_TX, LINK_TYPE_NORMAL, previous_hop,
                               sl[i].timeslot, sl[i].channel_offset);
        break;

      default:
        PRINTF("TSCH-sixtop:! Sixtop_state error\n");
        break;
      }

      num_added_links++;

      if(num_added_links == sixtop_request_num_links) {
        break;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Remove links from schedule */
void
sixtop_remove_links_from_schedule(uint8_t sf_id, struct sixtop_link *sl,
                                  linkaddr_t *previous_hop)
{
  uint8_t i = 0;
  struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(sf_id);

  for(i = 0; i < sixtop_request_num_links; i++) {
    PRINTF("TSCH-sixtop: Remove link %d from the schedule\n", sl[i].timeslot);
    tsch_schedule_remove_link_by_timeslot(sf, sl[i].timeslot);
  }
}
/*---------------------------------------------------------------------------*/
/* Process the callback of a Sixtop IE by sixtop_state */
void
sixtop_ie_process_callback(struct ieee802154_ies *ies, linkaddr_t *dest_addr)
{
  uint8_t i = 0, frame_id = 0;
  struct sixtop_link sl[SIXTOP_IE_MAX_LINKS];

  switch(sixtop_state) {
  case SIXTOP_ADD_REQUEST_WAIT_SENDDONE:
    sixtop_state = SIXTOP_ADD_RESPONSE_WAIT;
    break;

  case SIXTOP_DELETE_REQUEST_WAIT_SENDDONE:
    sixtop_state = SIXTOP_DELETE_RESPONSE_WAIT;
    break;

  case SIXTOP_ADD_RESPONSE_WAIT_SENDDONE:
  case SIXTOP_DELETE_RESPONSE_WAIT_SENDDONE:
    frame_id = ies->ie_sixtop.frame_id;

    if(sixtop_request_num_links > 0) {
      for(i = 0; i < sixtop_request_num_links; i++) {
        sl[i].timeslot = ies->ie_sixtop.linkList[i].timeslot;
        sl[i].channel_offset = ies->ie_sixtop.linkList[i].channel_offset;
      }
      if(sixtop_state == SIXTOP_ADD_RESPONSE_WAIT_SENDDONE) {
        /* Add links to TSCH schedule */
        sixtop_add_links_to_schedule(frame_id, sl, dest_addr, sixtop_state);
      } else {   /* SIXTOP_DELETE_RESPONSE_WAIT_SENDDONE */
        /* Remove links from TSCH schedule */
        sixtop_remove_links_from_schedule(frame_id, sl, dest_addr);
      }
    }

    break;

  default:
    /* log error */
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* Tx callback for Sixtop IE */
static void
sixtop_packet_sent(void *ptr, int status, int transmissions)
{
  /*PRINTF("TSCH-sixtop: Sixtop IE sent to %u, st %d-%d\n",
         last_pkt_address.u8[7], status, transmissions);*/

  /* Process the callback by sixtop_state */
  sixtop_ie_process_callback(&last_pkt_ies, &last_pkt_address);
}
/*---------------------------------------------------------------------------*/
/* Initiates transmission of a Link Request */
int
sixtop_create_link_request(uint8_t code, linkaddr_t *dest_addr, uint16_t num_Links)
{
  uint8_t len = 0;
  struct sixtop_link sl[SIXTOP_IE_MAX_LINKS];

  /* Get time-source neighbor */
  struct tsch_neighbor *n = tsch_queue_get_time_source();

  req_sf = tsch_schedule_get_slotframe_by_handle(0); /* Slotframe 0 */
  memset(sl, 0, sizeof(sl));

  if(dest_addr == NULL) {
    return -1;
  }

  sixtop_request_num_links = num_Links;

  if(code == CMD_ADD) {
    /* Fetch CandidateLinkList for addition */
    if(sixtop_get_link_list_for_addition(req_sf->handle, sl) == -1) {
      return -1;
    }
  } else if(code == CMD_DELETE) {
    /* Fetch CandidateLinkList for deletion */
    if(sixtop_get_link_list_for_deletion(req_sf->handle, sl, &n->addr) == -1) {
      return -1;
    }
  }

  if(tsch_queue_packet_count(&n->addr) == 0) {
    /* Get a free buffer */
    packetbuf_clear();
    packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, SIXTOP_CONF_MAX_MAC_TRANSMISSIONS);

    /* Save destination address of last packet sent */
    last_pkt_address = n->addr;

    /* Create IE by 6P code */
    len += sixtop_create_ie(packetbuf_dataptr(), n->addr, code, req_sf->handle, sl);

    if(len != 0) {
      struct tsch_packet *p;
      packetbuf_set_datalen(len);

      /* Add packet to Tx queue. TSCH layer schedules transmission */
      if(!(p = tsch_queue_add_packet(&n->addr, sixtop_packet_sent, NULL))) {
        PRINTF("TSCH-sixtop:! could not enqueue 6top packet\n");
      }
    } else {
      return -1;
    }
  }

  if(code == CMD_ADD) {
    sixtop_state = SIXTOP_ADD_REQUEST_WAIT_SENDDONE;
  } else if(code == CMD_DELETE) {
    sixtop_state = SIXTOP_DELETE_REQUEST_WAIT_SENDDONE;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/* Initiates transmission of a Link Response (On receiving a Link Request)  */
void
sixtop_create_link_response(uint8_t code, linkaddr_t *dest_addr, struct ieee802154_ies *ies)
{
  uint8_t i = 0, len = 0, frame_id = 0;
  struct sixtop_link sl[SIXTOP_IE_MAX_LINKS];

  /* Save destination address of last packet sent */
  linkaddr_copy((linkaddr_t *)&last_pkt_address, dest_addr);

  frame_id = ies->ie_sixtop.frame_id;

  for(i = 0; i < SIXTOP_IE_MAX_LINKS; i++) {
    sl[i].timeslot = ies->ie_sixtop.linkList[i].timeslot;
    sl[i].channel_offset = ies->ie_sixtop.linkList[i].channel_offset;
  }

  if(tsch_queue_packet_count(&last_pkt_address) == 0) {
    /* Get a free buffer */
    packetbuf_clear();
    packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, SIXTOP_CONF_MAX_MAC_TRANSMISSIONS);

    /* Create a Sixtop Link Response packet */
    len += sixtop_create_ie(packetbuf_dataptr(), last_pkt_address, RC_SUCCESS, frame_id, sl);

    if(len != 0) {
      struct tsch_packet *p;

      packetbuf_set_datalen(len);
      last_pkt_ies = *ies;

      /* Add packet to Tx queue. TSCH layer schedules transmission */
      if(!(p = tsch_queue_add_packet(&last_pkt_address, sixtop_packet_sent, NULL))) {
        PRINTF("TSCH-sixtop:! could not enqueue 6top packet\n");
      }
    }

    if(code == CMD_ADD) {
      sixtop_state = SIXTOP_ADD_RESPONSE_WAIT_SENDDONE;
    } else if(code == CMD_DELETE) {
      sixtop_state = SIXTOP_DELETE_RESPONSE_WAIT_SENDDONE;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link addition
 * Neighbor can be specified from application or upper layers
 * Currently Time source neighbor is taken as default
 * Returns 0 if Success
 */
int
sixtop_add_links(linkaddr_t *dest_addr, uint8_t num_Links)
{
  /* Create a Sixtop Add Request. Return 0 if Success */
  if(sixtop_create_link_request(CMD_ADD, dest_addr, num_Links) == -1) {
    PRINTF("TSCH-sixtop:! Add link failed");
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link deletion
 * Neighbor can be specified from application or upper layers
 * Currently Time source neighbor is taken as default
 * Returns 0 if Success
 */
int
sixtop_remove_link(linkaddr_t *dest_addr)
{
  /* Create a Sixtop Delete Request. Return 0 if Success */
  if(sixtop_create_link_request(CMD_DELETE, dest_addr, 1) == -1) {  /* delete atmost one */
    PRINTF("TSCH-sixtop:! Delete link failed");
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Check links for addition/deletion. Returns 0 if feasible */
int
sixtop_are_links_feasible(uint8_t code, uint8_t frame_id, struct sixtop_link *sl)
{
  uint8_t i = 0;

  /* Counter for infeasible number of links
   * A non-zero value indicates infeasibility */
  uint8_t infeasible_num_links = sixtop_request_num_links;

  if(sixtop_request_num_links > SIXTOP_IE_MAX_LINKS) {
    return -1;
  } else {
    if(code == CMD_ADD) {
      do {
        /* Checking availability of requested number of slots */
        if(tsch_schedule_get_link_by_timeslot(tsch_schedule_get_slotframe_by_handle(0), sl[i].timeslot) == NULL) {
          /* A free link present */
          infeasible_num_links--;
        } else {
          sl[i].timeslot = 0xFFFF;     /* Error */
        }
        i++;
      } while(i < sixtop_request_num_links && infeasible_num_links > 0);
    } else if(code == CMD_DELETE) {
      do {
        /* Ensure before delete */
        if(tsch_schedule_get_link_by_timeslot(tsch_schedule_get_slotframe_by_handle(0), sl[i].timeslot)) {
          /* A scheduled link present */
          infeasible_num_links--;
        } else {
          sl[i].timeslot = 0xFFFF;     /* Error */
        }
        i++;
      } while(i < sixtop_request_num_links && infeasible_num_links > 0);
    }

    if(infeasible_num_links == 0) { /* Links are feasible */

      while(i < sixtop_request_num_links) {
        sl[i].timeslot = 0xFFFF;     /* Error */
        i++;
      }

      return 0;  /* Return success(0) */
    }
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
/* Process Sixtop IE by code */
void
sixtop_process_by_code(uint8_t code, struct ieee802154_ies *ies, linkaddr_t *dest_addr)
{
  uint8_t frame_id = 0, i = 0;
  struct sixtop_link sl[SIXTOP_IE_MAX_LINKS];

  switch(code) {
  case CMD_ADD:
  case CMD_DELETE:
    frame_id = ies->ie_sixtop.frame_id;

    /* Are the links feasible for addition/deletion. Returns 0 if feasible */
    if(!(sixtop_are_links_feasible(code, frame_id, ies->ie_sixtop.linkList))) {

      PRINTF("TSCH-sixtop: Send Link Response to node %d\n", dest_addr->u8[7]);
      /* Links are feasible. Create Link Response packet */
      sixtop_create_link_response(code, dest_addr, ies);
    }
    break;

  case RC_SUCCESS:     /* Response Code Success */
    frame_id = req_sf->handle;

    for(i = 0; i < sixtop_request_num_links; i++) {
      sl[i].timeslot = ies->ie_sixtop.linkList[i].timeslot;
      sl[i].channel_offset = ies->ie_sixtop.linkList[i].channel_offset;
    }

    /* Add/delete links to/from Schedule */
    if(sixtop_state == SIXTOP_ADD_RESPONSE_RECEIVED) {
      sixtop_add_links_to_schedule(frame_id, sl, dest_addr, sixtop_state);
    } else if(sixtop_state == SIXTOP_DELETE_RESPONSE_RECEIVED) {
      sixtop_remove_links_from_schedule(frame_id, sl, dest_addr);
    }
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* Parse a Sixtop IE. Returns length of IE */
int
sixtop_parse_ie(const uint8_t *buf, linkaddr_t *dest_addr)
{
  struct ieee802154_ies ies;
  uint8_t i = 0, max = 0, ie_len = 0;

  /* Parse the 6top message elements */
  int subIE_id = buf[4];
  int version_code = buf[5];

  /* Parse the 6P Code */
  int code = (version_code & 0xF0) >> 4;

  /* Check for 6P SubIE and 6P Version */
  if(subIE_id == SIXTOP_SUBIE_ID && (version_code & SIXTOP_VERSION)) {

    ies.ie_sixtop.subIE_id = buf[4];
    ies.ie_sixtop.version_code = buf[5];
    ies.ie_sixtop.schedule_fn_id = buf[6];

    switch(code) {
    case CMD_ADD:
    case CMD_DELETE:
      ies.ie_sixtop.num_links = buf[7];
      ies.ie_sixtop.frame_id = buf[8];

      /* Save the number of links specified in Link Request IE */
      sixtop_request_num_links = ies.ie_sixtop.num_links;

      if(sixtop_request_num_links == 0) {
        PRINTF("TSCH-sixtop:! Requested number of links is zero\n");
        return -1;
      }

      if(code == CMD_ADD) {
        max = SIXTOP_IE_MAX_LINKS;
        PRINTF("TSCH-sixtop: Received a 6P Add Request for %d links from node %d with LinkList : ",
               sixtop_request_num_links, dest_addr->u8[7]);
      } else {         /* CMD_DELETE */
        max = ies.ie_sixtop.num_links;         /* delete atmost one */
        PRINTF("TSCH-sixtop: Received a 6P Delete Request for %d links from node %d with LinkList : ",
               sixtop_request_num_links, dest_addr->u8[7]);
      }

      /* Parse the Candidate Link List */
      for(i = 0; i < max; i++) {
        READ16(buf + 9 + i * 4, ies.ie_sixtop.linkList[i].timeslot);
        READ16(buf + 9 + i * 4 + 2, ies.ie_sixtop.linkList[i].channel_offset);
        PRINTF("%d ", ies.ie_sixtop.linkList[i].timeslot);
      }
      PRINTF("\n");

      /* Update the IE length */
      ie_len = 9 + (max * 4);

      /* Process Sixtop IE by code */
      sixtop_process_by_code(code, &ies, dest_addr);

      break;

    case RC_SUCCESS:         /* Response Code Success */
      if(sixtop_state == SIXTOP_ADD_RESPONSE_WAIT) {
        PRINTF("TSCH-sixtop: Received a 6P Add Response from node %d with LinkList : ", dest_addr->u8[7]);
        sixtop_state = SIXTOP_ADD_RESPONSE_RECEIVED;
      } else if(sixtop_state == SIXTOP_DELETE_RESPONSE_WAIT) {
        PRINTF("TSCH-sixtop: Received a 6P Delete Response from node %d with LinkList : ", dest_addr->u8[7]);
        sixtop_state = SIXTOP_DELETE_RESPONSE_RECEIVED;
      }

      /* Parse the Candidate Link List */
      for(i = 0; i < sixtop_request_num_links; i++) {
        READ16(buf + 7 + i * 4, ies.ie_sixtop.linkList[i].timeslot);
        READ16(buf + 7 + i * 4 + 2, ies.ie_sixtop.linkList[i].channel_offset);
        PRINTF("%d ", ies.ie_sixtop.linkList[i].timeslot);
      }
      PRINTF("\n");

      /* Update the IE length */
      ie_len = 7 + (sixtop_request_num_links * 4);

      /* Process Sixtop IE by code */
      sixtop_process_by_code(code, &ies, dest_addr);

      break;

    default:
      break;
    }
  }

  return ie_len;
}
/*---------------------------------------------------------------------------*/
/* Is it a Sixtop IE? Returns 0 if success */
int
sixtop_is_sixtop_ie(const uint8_t *buf, int buf_size,
                    frame802154_t *frame, struct ieee802154_ies *ies)
{
  uint8_t curr_len = 0;
  int ret;

  if(frame == NULL || buf_size < 0) {
    return -1;
  }

  /* Parse 802.15.4-2006 frame, i.e. all fields before Information Elements */
  if((ret = frame802154_parse((uint8_t *)buf, buf_size, frame)) == 0) {
    PRINTF("TSCH-sixtop:! parse_sixtop_ie: failed to parse frame\n");
    return -1;
  }

  if(frame->fcf.frame_version < FRAME802154_IEEE802154E_2012
     || frame->fcf.frame_type != FRAME802154_DATAFRAME) {
    PRINTF("TSCH-sixtop:! parse_sixtop_ie: frame is not a valid Sixtop IE. Frame version %u, type %u, FCF %02x %02x\n",
           frame->fcf.frame_version, frame->fcf.frame_type, buf[0], buf[1]);
    PRINTF("TSCH-sixtop:! parse_sixtop_ie: frame was from 0x%x/", frame->src_pid);
    PRINTLLADDR((const uip_lladdr_t *)&frame->src_addr);
    PRINTF(" to 0x%x/", frame->dest_pid);
    PRINTLLADDR((const uip_lladdr_t *)&frame->dest_addr);
    PRINTF("\n");
    return -1;
  }
  curr_len += ret;

  if(ies != NULL) {
    memset(ies, 0, sizeof(struct ieee802154_ies));
  }
  if(frame->fcf.ie_list_present) {
    int mic_len = 0;

    /* Parse information elements. We need to substract the MIC length, as the exact payload len is needed while parsing */
    if((ret = frame802154e_parse_information_elements(buf + curr_len, buf_size - curr_len - mic_len, ies)) == -1) {
      PRINTF("TSCH-sixtop:! parse_sixtop_ie: failed to parse IEs\n");
      return -1;
    } else if(!ret) {
      PRINTF("TSCH-sixtop: Sixtop IE received\n");
      return 0; /* Sixtop IE found. Return success(0) */
    }
    curr_len += ret;
  }

  return curr_len;
}
/*---------------------------------------------------------------------------*/
/* Set the Sequence Number of Link Response as in Link Request */
/* Function is called from TSCH layer on receiving a Sixtop IE */
void
sixtop_set_seqno(uint8_t seq_num)
{
  sixtop_response_seqno = seq_num;
}
