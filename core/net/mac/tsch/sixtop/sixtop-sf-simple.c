/**
 * \file
 *         A 6P Simple Schedule Function
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 */

#include "contiki-lib.h"

#include "lib/assert.h"
#include "net/mac/tsch/tsch-schedule.h"

#include "sixtop.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

typedef struct {
  uint16_t timeslot_offset;
  uint16_t channel_offset;
} sf_simple_cell_t;

static uint16_t slotframe_handle = 0; /* should be assigned out of this file */
static const uint8_t sf_simple_sfid = 0;

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

  for(i = 0; i < cell_list_len; i++) {
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

  for(i = 0; i < cell_list_len; i++) {
    read_cell((const uint8_t *)&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    PRINTF("6top-sf-simple: Schedule link %d as %s with node %u\n",
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

  for(i = 0; i < cell_list_len; i++) {
    read_cell((const uint8_t *)&cell_list[i], &cell);
    if(cell.timeslot_offset == 0xffff) {
      continue;
    }

    tsch_schedule_remove_link_by_timeslot(slotframe,
                                          cell.timeslot_offset);
  }
}

static void
add_response_sent_callback(const sixtop_msg_body_t *body,
                           const linkaddr_t *dest_addr,
                           sixtop_return_t status)
{
  assert(dest_addr != NULL && body != NULL);

  if(status == SIXTOP_RETURN_SUCCESS) {
    add_links_to_schedule(dest_addr, LINK_OPTION_RX,
                          body->cell_list, body->cell_list_len);
  }
}

static void
delete_response_sent_callback(const sixtop_msg_body_t *body,
                              const linkaddr_t *dest_addr,
                              sixtop_return_t status)
{
  assert(dest_addr != NULL && body != NULL);

  if(status == SIXTOP_RETURN_SUCCESS) {
    remove_links_to_schedule(body->cell_list, body->cell_list_len);
  }
}

static void
add_req_input(const sixtop_msg_body_t *body, const linkaddr_t *peer_addr)
{
  uint8_t i;
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int feasible_link;
  sixtop_msg_body_t res;

  assert(body != NULL && peer_addr != NULL);

  PRINTF("6top-sf-simple: Received a 6P Add Request for %d links from node %d with LinkList : ",
         body->num_cells, peer_addr->u8[7]);
  print_cell_list(body->cell_list, body->cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  if(body->num_cells > 0 && body->cell_list_len > 0) {
    memset(&res, 0, sizeof(res));

    /* checking availability for requested slots */
    for(i = 0, feasible_link = 0;
        (i < body->cell_list_len) && (feasible_link < body->num_cells);
        i++) {
      read_cell((const uint8_t *)&body->cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) == NULL) {
        memcpy(&res.cell_list[feasible_link], &cell, sizeof(cell));
        res.cell_list_len++;
        feasible_link++;
      }
    }

    if(feasible_link == body->num_cells) {
      /* Links are feasible. Create Link Response packet */
      PRINTF("6top-sf-simple: Send Link Response to node %d\n", peer_addr->u8[7]);
      sixtop_response_output(sf_simple_sfid, SIXTOP_RC_SUCCESS, &res, peer_addr,
                             add_response_sent_callback);
    }
  }
}

static void
delete_req_input(const sixtop_msg_body_t *body, const linkaddr_t *peer_addr)
{
  uint8_t i;
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  sixtop_msg_body_t res;
  int removed_link;

  PRINTF("6top-sf-simple: Received a 6P Delete Request for %d links from node %d with LinkList : ",
         body->num_cells, peer_addr->u8[7]);
  print_cell_list(body->cell_list, body->cell_list_len);
  PRINTF("\n");

  slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  if(slotframe == NULL) {
    return;
  }

  memset(&res, 0, sizeof(res));

  if(body->num_cells > 0 && body->cell_list_len > 0) {
    /* ensure before delete */
    for(i = 0, removed_link = 0; i < body->cell_list_len; i++) {
      read_cell((const uint8_t *)&body->cell_list[i], &cell);
      if(tsch_schedule_get_link_by_timeslot(slotframe,
                                            cell.timeslot_offset) != NULL) {
        memcpy(&res.cell_list[removed_link], &cell, sizeof(cell));
        res.cell_list_len++;
        removed_link++;
      }
    }
  }

  /* Links are feasible. Create Link Response packet */
  PRINTF("6top-sf-simple: Send Link Response to node %d\n", peer_addr->u8[7]);
  sixtop_response_output(sf_simple_sfid, SIXTOP_RC_SUCCESS, &res, peer_addr,
                         delete_response_sent_callback);
}

static void
request_input(uint8_t cmd,
              const sixtop_msg_body_t *body, const linkaddr_t *peer_addr)
{
  switch(cmd) {
  case SIXTOP_CMD_ADD:
    add_req_input(body, peer_addr);
    break;
  case SIXTOP_CMD_DELETE:
    delete_req_input(body, peer_addr);
    break;
  default:
    /* unsupported request */
    break;
  }
}
static void
response_input(uint8_t cmd, uint8_t return_code,
               const sixtop_msg_body_t *body, const linkaddr_t *peer_addr)
{
  if(return_code == SIXTOP_RC_SUCCESS) {
    switch(cmd) {
    case SIXTOP_CMD_ADD:
      PRINTF("6top-sf-simple: Received a 6P Add Response with LinkList : ");
      print_cell_list(body->cell_list, body->cell_list_len);
      PRINTF("\n");
      add_links_to_schedule(peer_addr, LINK_OPTION_TX,
                            body->cell_list, body->cell_list_len);
      break;
    case SIXTOP_CMD_DELETE:
      PRINTF("6top-sf-simple: Received a 6P Delete Response with LinkList : ");
      print_cell_list(body->cell_list, body->cell_list_len);
      PRINTF("\n");
      remove_links_to_schedule(body->cell_list, body->cell_list_len);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link addition
 */
static int
add_links(linkaddr_t *peer_addr, uint8_t num_Links)
{
  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);

  sixtop_msg_body_t req;
  sf_simple_cell_t cell_list[SIXTOP_IE_MAX_CELLS];

  /* Flag to prevent repeated slots */
  uint8_t slot_check = 1;
  uint16_t random_slot = 0;

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
        PRINTF("6top-sf-simple:! Number of trials for free slot exceeded...\n");
        return -1;
        break; /* exit while loop */
      }
    }
  } while(index < SIXTOP_IE_MAX_CELLS);

  /* Create a Sixtop Add Request. Return 0 if Success */
  if(index == 0 ) {
    return -1;
  }

  memset(&req, 0, sizeof(req));
  req.num_cells = num_Links;
  req.cell_list_len = index;
  memcpy(req.cell_list, cell_list, req.cell_list_len * sizeof(uint32_t));
  sixtop_request_output(sf_simple_sfid, SIXTOP_CMD_ADD, &req, peer_addr, NULL);
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Initiates a Sixtop Link deletion
 */
static int
remove_link(linkaddr_t *peer_addr, uint8_t num_links)
{

  uint8_t i = 0, index = 0;
  struct tsch_slotframe *sf =
    tsch_schedule_get_slotframe_by_handle(slotframe_handle);
  struct tsch_link *l;

  sixtop_msg_body_t req;
  sf_simple_cell_t cell;

  if(num_links != 1) {
    return -1;
  }

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

  memset(&req, 0, sizeof(req));
  req.num_cells = 1;
  req.cell_list_len = 1;
  memcpy(req.cell_list, &cell, sizeof(cell));
  sixtop_request_output(sf_simple_sfid, SIXTOP_CMD_DELETE, &req, peer_addr, NULL);
  return 0;
}

const sixtop_sf_t sf_simple_driver = {
  SIXTOP_SFID_SF_SIMPLE,
  CLOCK_SECOND,
  NULL,
  request_input,
  response_input,
  add_links,
  remove_link
};
