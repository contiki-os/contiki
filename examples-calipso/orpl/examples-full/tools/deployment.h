/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         Macros setting up the deployment, header file for deployment.c
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include "contiki-conf.h"
#include "sys/node-id.h"
#include "net/uip.h"
#include "net/rime/rimeaddr.h"

#define DEPLOYMENT_COOJA        1
#define DEPLOYMENT_MOTES        2
#define DEPLOYMENT_TWIST        3
#define DEPLOYMENT_INDRIYA      4

#define DEPLOYMENT DEPLOYMENT_COOJA

#define IN_COOJA (DEPLOYMENT == DEPLOYMENT_COOJA)
#define IN_MOTES (DEPLOYMENT == DEPLOYMENT_MOTES)
#define IN_TWIST (DEPLOYMENT == DEPLOYMENT_TWIST)
#define IN_INDRIYA (DEPLOYMENT == DEPLOYMENT_INDRIYA)

#if IN_TWIST
#define ROOT_ID 137
#elif IN_INDRIYA
#define ROOT_ID 1
#else
#define ROOT_ID 1
#endif

/* Returns the node's node-id */
uint16_t get_n_nodes();
/* Returns the total number of nodes in the deployment */
uint16_t get_node_id();
/* Returns a node-id from a node's link-layer address */
uint16_t node_id_from_rimeaddr(const rimeaddr_t *addr);
/* Returns a node-id from a node's IPv6 address */
uint16_t node_id_from_ipaddr(const uip_ipaddr_t *addr);
/* Returns a node-id from a node's absolute index in the deployment */
uint16_t get_node_id_from_index(uint16_t index);
/* Sets an IPv6 from a link-layer address */
void set_ipaddr_from_rimeaddr(uip_ipaddr_t *ipaddr, const rimeaddr_t *lladdr);
/* Sets an IPv6 from a link-layer address */
void set_ipaddr_from_id(uip_ipaddr_t *ipaddr, uint16_t id);
/* Sets an rimeaddr from a link-layer address */
void set_rimeaddr_from_id(rimeaddr_t *lladdr, uint16_t id);
/* Initializes global IPv6 and creates DODAG */
void deployment_init(uip_ipaddr_t *ipaddr);

#endif /* DEPLOYMENT_H */
