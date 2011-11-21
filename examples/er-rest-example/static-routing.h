/*
 * static-routing.h
 *
 *  Created on: Oct 12, 2010
 *      Author: simonduq
 */

#ifndef STATICROUTING_H_
#define STATICROUTING_H_

#include "contiki.h"

extern int node_rank;
extern uint32_t get_mote_suffix(int id);
extern int get_mote_id(uint32_t suffix);
extern void add_route(int dest, int next);
extern void set_global_address(void);
extern void configure_routing(void);

#endif /* STATICROUTING_H_ */
