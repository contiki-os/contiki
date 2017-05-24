/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 */


/**
 * \file
 *         squatix header file
 */

#ifndef __SQUATIX_H__
#define __SQUATIX_H__

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-conf.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "squatix-br-conf.h"

/* The structure of an Squatix rule */
struct squatix_rule {
  void (* init)(uint16_t slotframe_handle);
  void (* new_time_source)(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
  int  (* select_packet)(uint16_t *slotframe, uint16_t *timeslot);
  void (* child_added)(const linkaddr_t *addr);
  void (* child_removed)(const linkaddr_t *addr);
};

//new TSCH schedule
struct squatix_rule one_slotframe;
struct squatix_rule data_slotframe;

extern linkaddr_t squatix_parent_linkaddr;
extern int squatix_parent_knows_us;

/* Call from application to start squatix */
void squatix_init(void);
/* Callbacks requied for squatix to operate */

/* Set with #define TSCH_CALLBACK_PACKET_READY squatix_callback_packet_ready */
void squatix_callback_packet_ready(void);

/* Set with #define TSCH_CALLBACK_NEW_TIME_SOURCE squatix_callback_new_time_source */
void squatix_callback_new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK squatix_callback_child_added */
void squatix_callback_child_added(const linkaddr_t *addr);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK squatix_callback_child_removed */
void squatix_callback_child_removed(const linkaddr_t *addr);

#endif /* __SQUATIX_H__ */
