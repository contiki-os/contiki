/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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
 * \file
 *         A 6P Simple Schedule Function
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "contiki-lib.h"

#include "lib/assert.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"

#include "sf-simple.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

typedef struct {
  uint16_t timeslot_offset;
  uint16_t channel_offset;
} sf_simple_cell_t;

static const uint16_t slotframe_handle = 0;
static uint8_t res_storage[4 + SF_SIMPLE_MAX_LINKS * 4];
static uint8_t req_storage[4 + SF_SIMPLE_MAX_LINKS * 4];

/*
 * scheduling policy:
 * add: if and only if all the requested cells are available, accept the request
 * delete: if and only if all the requested cells are in use, accept the request
 */

static void
read_cell(const uint8_t *buf, sf_simple_cell_t *cell)
{
  cell->timeslot_offset = buf[0] + (buf[1] << 8);
  cell->channel_offset = buf[2] + (buf[3] << 8);
}

static void
print_cell_list(const uint32_t *cell_list, uint16_t cell_list_len)
{
  uint16_t i;
  sf_simple_cell_t cell;

  for(i = 0; i < (cell_list_len / sizeof(cell)); i++) {
    read_cell((const uint8_t *)&cell_list[i], &cell);
    PRINTF("%u ", cell.timeslot_offset);
  }
}

static void
add_links_to_schedule(const linkaddr_t *peer_addr, uint8_t link_option,
                      const uint32_t *cell_list, uint16_t cell_list_len)
{
  /* add only the first valid cell */

  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  if(slotframe == NULL) {
    return;
  }

  for(i = 0; i < (cell_list_len / sizeof(cell)); i++) {
    read_cell((const uint8_t *)&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    PRINTF("sf-simple: Schedule link %d as %s with node %u\n",
           cell.timeslot_offset,
           link_option == LINK_OPTION_RX ? "RX" : "TX",
           peer_addr->u8[7]);
    tsch_schedule_add_link(slotframe,
                           link_option, LINK_TYPE_NORMAL, peer_addr,
                           cell.timeslot_offset, cell.channel_offset);
    break;
  }
}

static void
remove_links_to_schedule(const uint32_t *cell_list, uint16_t cell_list_len)
{
  /* remove all the cells */

  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  if(slotframe == NULL) {
    return;
  }

  for(i = 0; i < (cell_list_len / sizeof(cell)); i++) {
    read_cell((const uint8_t *)&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    tsch_schedule_remove_link_by_timeslot(slotframe,
                                          cell.timeslot_offset);
  }
}

static void
add_response_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_send_status_t status)
{
  sixp_res_add_t *add_res = (sixp_res_add_t *)arg;
  uint16_t cell_list_len;

  assert(add_res != NULL && dest_addr != NULL);

  cell_list_len = arg_len - offsetof(sixp_res_add_t, cell_list);

  if(status == SIXP_SEND_STATUS_SUCCESS) {
    add_links_to_schedule(dest_addr, LINK_OPTION_RX,
                          add_res->cell_list, cell_list_len);
    sixp_advance_generation(dest_addr, SIXP_GEN_TYPE_RX);
  }
}

static void
delete_response_sent_callback(void *arg, uint16_t arg_len,
                              const linkaddr_t *dest_addr,
                              sixp_send_status_t status)
{
  sixp_res_delete_t *delete_res = (sixp_res_delete_t *)arg;
  uint16_t cell_list_len;

  assert(delete_res != NULL && dest_addr != NULL);

  cell_list_len = arg_len - offsetof(sixp_res_delete_t, cell_list);

  if(status == SIXP_SEND_STATUS_SUCCESS) {
    remove_links_to_schedule(delete_res->cell_list, cell_list_len);
    sixp_advance_generation(dest_addr, SIXP_GEN_TYPE_RX);
  }
}

static void
add_req_input(const sixtop_ie_body_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{
  uint8_t i;
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int feasible_link;

  const sixp_req_add_t *add_req = (const sixp_req_add_t *)body;
  sixp_res_add_t *add_res = (sixp_res_add_t *)res_storage;
  uint16_t cell_list_len;
  uint16_t res_len;

  assert(add_req != NULL && peer_addr != NULL);

  cell_list_len = body_len - offsetof(sixp_req_add_t, cell_list);

  PRINTF("sf-simple: Received a 6P Add Request for %d links from node %d with LinkList : ",
         add_req->num_cells, peer_addr->u8[7]);
  print_cell_list(add_req->cell_list, cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  if(add_req->num_cells > 0 && cell_list_len > 0) {
    memset(add_res, 0, sizeof(res_storage));
    res_len = sizeof(sixp_res_add_t);

    /* checking availability for requested slots */
    for(i = 0, feasible_link = 0;
        i < cell_list_len && feasible_link < add_req->num_cells;
        i += sizeof(cell)) {
      read_cell((const uint8_t *)&add_req->cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) == NULL) {
        memcpy(&add_res->cell_list[feasible_link], &cell, sizeof(cell));
        res_len += sizeof(cell);
        feasible_link++;
      }
    }

    if(feasible_link == add_req->num_cells) {
      /* Links are feasible. Create Link Response packet */
      PRINTF("sf-simple: Send a 6P Response to node %d\n", peer_addr->u8[7]);
      sixp_send(SIXP_TYPE_RESPONSE, (sixp_code_t)(sixp_return_code_t)SIXP_RC_SUCCESS,
                SF_SIMPLE_SFID,
                (const sixtop_ie_body_t *)add_res, res_len, peer_addr,
                add_response_sent_callback, add_res, res_len);
    }
  }
}

static void
delete_req_input(const sixtop_ie_body_t *body, uint16_t body_len,
                 const linkaddr_t *peer_addr)
{
  uint8_t i;
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  sixp_req_delete_t *delete_req = (sixp_req_delete_t *)body;
  sixp_res_delete_t *delete_res = (sixp_res_delete_t *)res_storage;
  uint16_t cell_list_len;
  uint16_t res_len;
  int removed_link;

  assert(delete_req != NULL && peer_addr != NULL);

  cell_list_len = body_len - offsetof(sixp_req_delete_t, cell_list);

  PRINTF("sf-simple: Received a 6P Delete Request for %d links from node %d with LinkList : ",
         delete_req->num_cells, peer_addr->u8[7]);
  print_cell_list(delete_req->cell_list, cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  memset(delete_res, 0, sizeof(res_storage));
  res_len = sizeof(sixp_res_delete_t);

  if(delete_req->num_cells > 0 && cell_list_len > 0) {
    /* ensure before delete */
    for(i = 0, removed_link = 0; i < (cell_list_len / sizeof(cell)); i++) {
      read_cell((const uint8_t *)&delete_req->cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) != NULL) {
        memcpy(&delete_res->cell_list[removed_link], &cell, sizeof(cell));
        res_len += sizeof(cell);
        removed_link++;
      }
    }
  }


  /* Links are feasible. Create Link Response packet */
  PRINTF("sf-simple: Send a 6P Response to node %d\n", peer_addr->u8[7]);
  sixp_send(SIXP_TYPE_RESPONSE, (sixp_code_t)(sixp_return_code_t)SIXP_RC_SUCCESS,
            SF_SIMPLE_SFID,
            (const sixtop_ie_body_t *)delete_res, res_len, peer_addr,
            delete_response_sent_callback, delete_res, res_len);
}

static void
request_input(sixp_command_id_t cmd,
              const sixtop_ie_body_t *body, uint16_t body_len,
              const linkaddr_t *peer_addr)
{
  assert(body != NULL && peer_addr != NULL);

  switch(cmd) {
    case SIXP_CMD_ADD:
      add_req_input(body, body_len, peer_addr);
      break;
    case SIXP_CMD_DELETE:
      delete_req_input(body, body_len, peer_addr);
      break;
    default:
      /* unsupported request */
      break;
  }
}
static void
response_input(sixp_command_id_t cmd, sixp_return_code_t return_code,
               const sixtop_ie_body_t *body, uint16_t body_len,
               const linkaddr_t *peer_addr)
{
  sixp_res_add_t *add_res = (sixp_res_add_t *)body;
  sixp_res_delete_t *delete_res = (sixp_res_delete_t *)body;
  uint16_t cell_list_len;

  assert(add_res != NULL && delete_res != NULL && peer_addr != NULL);

  if(return_code == SIXP_RC_SUCCESS) {
    switch(cmd) {
      case SIXP_CMD_ADD:
        cell_list_len = body_len - offsetof(sixp_res_add_t, cell_list);
        PRINTF("sf-simple: Received a 6P Add Response with LinkList : ");
        print_cell_list(add_res->cell_list, cell_list_len);
        PRINTF("\n");
        add_links_to_schedule(peer_addr, LINK_OPTION_TX,
                              add_res->cell_list, cell_list_len);
        sixp_advance_generation(peer_addr, SIXP_GEN_TYPE_TX);
        break;
      case SIXP_CMD_DELETE:
        cell_list_len = body_len - offsetof(sixp_res_delete_t, cell_list);
        PRINTF("sf-simple: Received a 6P Delete Response with LinkList : ");
        print_cell_list(delete_res->cell_list, cell_list_len);
        PRINTF("\n");
        remove_links_to_schedule(delete_res->cell_list, cell_list_len);
        sixp_advance_generation(peer_addr, SIXP_GEN_TYPE_TX);
        break;
      case SIXP_CMD_STATUS:
      case SIXP_CMD_LIST:
      case SIXP_CMD_CLEAR:
      default:
        PRINTF("sf-simple: unsupported response\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link addition
 */
int
sf_simple_add_links(linkaddr_t *peer_addr, uint8_t num_Links)
{
  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  sixp_req_add_t *add_req = (sixp_req_add_t *)req_storage;
  uint8_t req_len;
  sf_simple_cell_t cell_list[SF_SIMPLE_MAX_LINKS];

  /* Flag to prevent repeated slots */
  uint8_t slot_check = 1;
  uint16_t random_slot = 0;

  assert(peer_addr != NULL && sf != NULL);

  do {
    /* Randomly select a slot offset within TSCH_SCHEDULE_DEFAULT_LENGTH */
    random_slot = ((random_rand() & 0xFF)) % TSCH_SCHEDULE_DEFAULT_LENGTH;

    if(tsch_schedule_get_link_by_timeslot(sf, random_slot) == NULL) {

      /* To prevent repeated slots */
      for(i = 0; i < index; i++) {
        if(cell_list[i].timeslot_offset != random_slot) {
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
        cell_list[index].timeslot_offset = random_slot;
        cell_list[index].channel_offset = 0;

        index++;
        slot_check++;
      } else if(slot_check > TSCH_SCHEDULE_DEFAULT_LENGTH) {
        PRINTF("sf-simple:! Number of trials for free slot exceeded...\n");
        return -1;
        break; /* exit while loop */
      }
    }
  } while(index < SF_SIMPLE_MAX_LINKS);

  /* Create a Sixtop Add Request. Return 0 if Success */
  if(index == 0 ) {
    return -1;
  }

  memset(req_storage, 0, sizeof(req_storage));
  add_req->cell_options |= SIXP_CELL_OPTION_TX;
  add_req->num_cells = num_Links;
  memcpy(add_req->cell_list, cell_list, index * sizeof(sf_simple_cell_t));
  req_len = sizeof(sixp_req_add_t) + index * sizeof(sf_simple_cell_t);
  sixp_send(SIXP_TYPE_REQUEST, (sixp_code_t)(sixp_command_id_t)SIXP_CMD_ADD,
            SF_SIMPLE_SFID,
            (const sixtop_ie_body_t *)add_req, req_len, peer_addr,
            NULL, NULL, 0);

  PRINTF("sf-simple: Send a 6P Add Request for %d links to node %d with LinkList : ",
         add_req->num_cells, peer_addr->u8[7]);
  print_cell_list(add_req->cell_list, index * sizeof(sf_simple_cell_t));
  PRINTF("\n");

  return 0;
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link deletion
 */
int
sf_simple_remove_links(linkaddr_t *peer_addr)
{
  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  struct tsch_link *l;

  sixp_req_delete_t *delete_req = (sixp_req_delete_t *)req_storage;
  uint16_t req_len;
  sf_simple_cell_t cell;

  assert(peer_addr != NULL && sf != NULL);

  for(i = 0; i < TSCH_SCHEDULE_DEFAULT_LENGTH; i++) {
    l = tsch_schedule_get_link_by_timeslot(sf, i);

    if(l) {
      /* Non-zero value indicates a scheduled link */
      if((linkaddr_cmp(&l->addr, peer_addr)) && (l->link_options == LINK_OPTION_TX)) {
        /* This link is scheduled as a TX link to the specified neighbor */
        cell.timeslot_offset = i;
        cell.channel_offset = l->channel_offset;
        index++;
        break;   /* delete atmost one */
      }
    }
  }

  if(index == 0) {
    return -1;
  }

  memset(req_storage, 0, sizeof(req_storage));
  delete_req->num_cells = 1;
  memcpy(delete_req->cell_list, &cell, sizeof(cell));
  req_len = sizeof(sixp_req_delete_t) + sizeof(cell);
  sixp_send(SIXP_TYPE_REQUEST, (sixp_code_t)(sixp_command_id_t)SIXP_CMD_DELETE,
            SF_SIMPLE_SFID,
            (const sixtop_ie_body_t *)delete_req, req_len, peer_addr,
            NULL, NULL, 0);

  PRINTF("sf-simple: Send a 6P Delete Request for %d links to node %d with LinkList : ",
         delete_req->num_cells, peer_addr->u8[7]);
  print_cell_list(delete_req->cell_list, sizeof(cell));
  PRINTF("\n");

  return 0;
}

const sixtop_sf_t sf_simple_driver = {
  SF_SIMPLE_SFID,
  CLOCK_SECOND,
  NULL,
  request_input,
  response_input,
  NULL
};
