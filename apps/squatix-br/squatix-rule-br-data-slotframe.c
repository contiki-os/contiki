#include "contiki.h"
#include "squatix-br.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"
#include "lib/random.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#if SQUATIX_DATA_SENDER_BASED && SQUATIX_COLLISION_FREE_HASH
#define DATA_SLOT_SHARED_FLAG    ((SQUATIX_DATA_PERIOD < (SQUATIX_MAX_HASH + 1)) ? LINK_OPTION_SHARED : 0)
#else
#define DATA_SLOT_SHARED_FLAG      LINK_OPTION_SHARED
#endif

static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;

static struct tsch_slotframe *sf_data;

// Because of the One-SF design, use the common shared cell for both EB and non-EB traffic \
// (inteded for RPL packets but not CoAP packets)
#define SQUATIX_DATA_SLOTFRAME_TYPE              LINK_TYPE_NORMAL

/////////End of declaration/////////////////

/*--------------------------------)-------------------------------------------*/
// static uint16_t
// get_node_timeslot(const linkaddr_t *addr)
// {
// 	uint16_t next_timeslot[7],i,j; 
// 	int k=0;
// #if SQUATIX_ONE_SLOTFRAME > 0
// 	for (i=2;i!=SQUATIX_ONE_SLOTFRAME; i++){
// 		if (i==2){
// 			next_timeslot[k]=i;
// 			k++;
// 		}
// 		else { 
// 			for (j=2;j!=i;j++){
// 				if ((i%j)==0 ){
// 					break;
// 					}
// 				else{
// 					next_timeslot[k]=i;
// 					k++;
// 					break;
// 				}
// 			}
// 		}
// 	}
//  return next_timeslot[(int)(random_rand() % sizeof(next_timeslot))];
// #else
//  return 0xffff;
// #endif
// }
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot(const linkaddr_t *addr)
{
  if(addr != NULL && SQUATIX_DATA_SLOTFRAME_PERIOD > 0) {
    PRINTF("hash: [%d]",SQUATIX_LINKADDR_HASH(addr));
    return SQUATIX_LINKADDR_HASH(addr) % SQUATIX_DATA_SLOTFRAME_PERIOD;
  } else {
    return 0xffff;
  }
}

/*---------------------------------------------------------------------------*/
static int
neighbor_has_uc_link(const linkaddr_t *linkaddr)
{
  if(linkaddr != NULL && !linkaddr_cmp(linkaddr, &linkaddr_null)) {
    if((squatix_parent_knows_us || !SQUATIX_DATA_SENDER_BASED)
       && linkaddr_cmp(&squatix_parent_linkaddr, linkaddr)) {
      return 1;
    }
    if(nbr_table_get_from_lladdr(nbr_routes, (linkaddr_t *)linkaddr) != NULL) {
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
add_uc_link(const linkaddr_t *linkaddr)
{
  if(linkaddr != NULL) {
    uint16_t timeslot = get_node_timeslot(linkaddr);
    uint8_t link_options = SQUATIX_DATA_SENDER_BASED ? LINK_OPTION_RX : LINK_OPTION_TX | DATA_SLOT_SHARED_FLAG;

    if(timeslot == get_node_timeslot(&linkaddr_node_addr)) {
      /* This is also our timeslot, add necessary flags */
      link_options |= SQUATIX_DATA_SENDER_BASED ? LINK_OPTION_TX | DATA_SLOT_SHARED_FLAG: LINK_OPTION_RX;
    }

    /* Add/update link */
    tsch_schedule_add_link(sf_data, link_options, LINK_TYPE_NORMAL, &tsch_broadcast_address,
          timeslot, channel_offset);
  }
}
//  tsch_schedule_add_link(sf_data,link_options, LINK_TYPE_NORMAL, &tsch_broad_cast_address,timeslot,channel_offset);

/*---------------------------------------------------------------------------*/
static void
remove_uc_link(const linkaddr_t *linkaddr)
{
  uint16_t timeslot;
  struct tsch_link *l;

  if(linkaddr == NULL) {
    return;
  }

  timeslot = get_node_timeslot(linkaddr);
  l = tsch_schedule_get_link_by_timeslot(sf_data, timeslot);
  if(l == NULL) {
    return;
  }
  /* Does our current parent need this timeslot? */
  if(timeslot == get_node_timeslot(&squatix_parent_linkaddr)) {
    /* Yes, this timeslot is being used, return */
    return;
  }
  /* Does any other child need this timeslot?
   * (lookup all route next hops) */
  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(item != NULL) {
    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);
    if(timeslot == get_node_timeslot(addr)) {
      /* Yes, this timeslot is being used, return */
      return;
    }
    item = nbr_table_next(nbr_routes, item);
  }

  /* Do we need this timeslot? */
  if(timeslot == get_node_timeslot(&linkaddr_node_addr)) {
    /* This is our link, keep it but update the link options */
    uint8_t link_options = SQUATIX_DATA_SENDER_BASED ? LINK_OPTION_TX | DATA_SLOT_SHARED_FLAG: LINK_OPTION_RX;
    tsch_schedule_add_link(sf_data, link_options, LINK_TYPE_NORMAL, &tsch_broadcast_address,
              timeslot, channel_offset);
  } else {
    /* Remove link */
    tsch_schedule_remove_link(sf_data, l);
  }
}
/*---------------------------------------------------------------------------*/
static void
child_added(const linkaddr_t *linkaddr)
{
  add_uc_link(linkaddr);
}
/*---------------------------------------------------------------------------*/
static void
child_removed(const linkaddr_t *linkaddr)
{
  remove_uc_link(linkaddr);
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  /* Select data packets we have a unicast link to */
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME
     && neighbor_has_uc_link(dest)) {
    if(slotframe != NULL) {
      *slotframe = slotframe_handle;
    }
    if(timeslot != NULL) {
      *timeslot = SQUATIX_DATA_SENDER_BASED ? get_node_timeslot(&linkaddr_node_addr) : get_node_timeslot(dest);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if(new != old) {
    const linkaddr_t *old_addr = old != NULL ? &old->addr : NULL;
    const linkaddr_t *new_addr = new != NULL ? &new->addr : NULL;
    if(new_addr != NULL) {
      linkaddr_copy(&squatix_parent_linkaddr, new_addr);
    } else {
      linkaddr_copy(&squatix_parent_linkaddr, &linkaddr_null);
    }
    remove_uc_link(old_addr);
    add_uc_link(new_addr);
  }
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

// static void
// new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
// {
//   uint16_t old_ts = get_node_timeslot(&old->addr);
//   uint16_t new_ts = get_node_timeslot(&new->addr);

//   if(new_ts == old_ts) {
//     return;
//   }

//   if(old_ts != 0xffff) {
//     /* Stop listening to the old time source's EBs */
//     tsch_schedule_remove_link_by_timeslot(sf_node, old_ts);
//   }
//   if(new_ts != 0xffff) {
//     /* Listen to the time source's EBs */
//     tsch_schedule_add_link(sf_node,
//                            LINK_OPTION_RX,
//                            LINK_TYPE_ADVERTISING_ONLY, NULL,
//                            new_ts, channel_offset);
//   }
// }
/*---------------------------------------------------------------------------*/
//DESIGN SHARED CELL FIRST

static void
init(uint16_t sf_handle)
{
  slotframe_handle = sf_handle;
  channel_offset = sf_handle;
  /* Slotframe for unicast transmissions */
  sf_data = tsch_schedule_add_slotframe(slotframe_handle, SQUATIX_DATA_SLOTFRAME_PERIOD);
//  uint16_t timeslot = get_node_timeslot(&linkaddr_node_addr);
  uint16_t timeslot;
  for (timeslot=0x0001; timeslot<SQUATIX_DATA_SLOTFRAME_PERIOD;timeslot++){
    // tsch_schedule_add_link(sf_data,
    //           SQUATIX_DATA_SENDER_BASED ? LINK_OPTION_TX | DATA_SLOT_SHARED_FLAG: LINK_OPTION_RX,
    //           LINK_TYPE_NORMAL, &tsch_broadcast_address,
    //           timeslot, channel_offset);
      tsch_schedule_add_link(sf_data,
              LINK_OPTION_RX,
              LINK_TYPE_NORMAL, &tsch_broadcast_address,
              timeslot, channel_offset);

  }
}

/*---------------------------------------------------------------------------*/
struct squatix_rule data_slotframe = {
  init,
  new_time_source,
  select_packet,
  child_added,
  child_removed,
};

