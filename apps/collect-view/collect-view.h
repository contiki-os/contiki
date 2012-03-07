#ifndef COLLECT_VIEW_H
#define COLLECT_VIEW_H

#include "contiki-conf.h"
#include "net/rime/rimeaddr.h"
#include "net/rime/collect.h"

struct collect_view_data_msg {
  uint16_t len;
  uint16_t clock;
  uint16_t timesynch_time;
  uint16_t cpu;
  uint16_t lpm;
  uint16_t transmit;
  uint16_t listen;
  uint16_t parent;
  uint16_t parent_etx;
  uint16_t current_rtmetric;
  uint16_t num_neighbors;
  uint16_t beacon_interval;

  uint16_t sensors[10];
};

void collect_view_construct_message(struct collect_view_data_msg *msg,
                                    const rimeaddr_t *parent,
                                    uint16_t etx_to_parent,
                                    uint16_t current_rtmetric,
                                    uint16_t num_neighbors,
                                    uint16_t beacon_interval);

void collect_view_arch_read_sensors(struct collect_view_data_msg *msg);

#endif /* COLLECT_VIEW_H */
