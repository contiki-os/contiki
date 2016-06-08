/*
 * Copyright (c) 2016, Inria.
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
 */

/**
 * \file
 *         RPL non-storing mode specific functions. Includes support for
 *         source routing.
 *
 * \author Simon Duquennoy <simon.duquennoy@inria.fr>
 */


#ifndef RPL_NS_H
#define RPL_NS_H

#include "rpl-conf.h"

#ifdef RPL_NS_CONF_LINK_NUM
#define RPL_NS_LINK_NUM RPL_NS_CONF_LINK_NUM
#else /* RPL_NS_CONF_LINK_NUM */
#define RPL_NS_LINK_NUM 32
#endif /* RPL_NS_CONF_LINK_NUM */

typedef struct rpl_ns_node {
  struct rpl_ns_node *next;
  uint32_t lifetime;
  rpl_dag_t *dag;
  /* Store only IPv6 link identifiers as all nodes in the DAG share the same prefix */
  unsigned char link_identifier[8];
  struct rpl_ns_node *parent;
} rpl_ns_node_t;

int rpl_ns_num_nodes(void);
void rpl_ns_expire_parent(rpl_dag_t *dag, const uip_ipaddr_t *child, const uip_ipaddr_t *parent);
rpl_ns_node_t *rpl_ns_update_node(rpl_dag_t *dag, const uip_ipaddr_t *child, const uip_ipaddr_t *parent, uint32_t lifetime);
void rpl_ns_init(void);
rpl_ns_node_t *rpl_ns_node_head(void);
rpl_ns_node_t *rpl_ns_node_next(rpl_ns_node_t *item);
rpl_ns_node_t *rpl_ns_get_node(const rpl_dag_t *dag, const uip_ipaddr_t *addr);
int rpl_ns_is_node_reachable(const rpl_dag_t *dag, const uip_ipaddr_t *addr);
void rpl_ns_get_node_global_addr(uip_ipaddr_t *addr, rpl_ns_node_t *node);
void rpl_ns_periodic();

#endif /* RPL_NS_H */
