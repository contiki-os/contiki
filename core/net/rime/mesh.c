/**
 * \addtogroup rimemesh
 * @{
 */

/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: mesh.c,v 1.11 2008/01/08 07:55:56 adamdunkels Exp $
 */

/**
 * \file
 *         A mesh routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/route.h"
#include "net/rime/mesh.h"

#include <stddef.h> /* For offsetof */

struct data_hdr {
  rimeaddr_t dest;
  rimeaddr_t originator;
  u16_t seqno;
};

#define PACKET_TIMEOUT (CLOCK_SECOND * 4)

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
data_packet_received(struct mh_conn *mh, rimeaddr_t *from, u8_t hops)
{
  struct mesh_conn *c = (struct mesh_conn *)
    ((char *)mh - offsetof(struct mesh_conn, mh));

  if(c->cb->recv) {
    c->cb->recv(c, from, hops);
  }
}
/*---------------------------------------------------------------------------*/
static rimeaddr_t *
data_packet_forward(struct mh_conn *mh, rimeaddr_t *originator,
		    rimeaddr_t *dest, rimeaddr_t *prevhop, u8_t hops)
{
  struct route_entry *rt;

  rt = route_lookup(dest);

  if(rt == NULL) {
    return NULL;
  }
  
  return &rt->nexthop;
}
/*---------------------------------------------------------------------------*/
static void
found_route(struct route_discovery_conn *rdc, rimeaddr_t *dest)
{
  struct mesh_conn *c = (struct mesh_conn *)
    ((char *)rdc - offsetof(struct mesh_conn, route_discovery_conn));

  if(c->queued_data != NULL &&
     rimeaddr_cmp(dest, &c->queued_data_dest)) {
    queuebuf_to_rimebuf(c->queued_data);
    queuebuf_free(c->queued_data);
    c->queued_data = NULL;
    mh_send(&c->mh, dest);
  }
}
/*---------------------------------------------------------------------------*/
static void
route_timed_out(struct route_discovery_conn *rdc)
{
  struct mesh_conn *c = (struct mesh_conn *)
    ((char *)rdc - offsetof(struct mesh_conn, route_discovery_conn));

  if(c->queued_data != NULL) {
    queuebuf_free(c->queued_data);
    c->queued_data = NULL;
  }

  if(c->cb->timedout) {
    c->cb->timedout(c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct mh_callbacks data_callbacks = { data_packet_received,
						    data_packet_forward };
static const struct route_discovery_callbacks route_discovery_callbacks =
  { found_route, route_timed_out };
/*---------------------------------------------------------------------------*/
void
mesh_open(struct mesh_conn *c, u16_t channels,
	  const struct mesh_callbacks *callbacks)
{
  mh_open(&c->mh, channels, &data_callbacks);
  route_discovery_open(&c->route_discovery_conn,
		       CLOCK_SECOND / 2,
		       channels + 1,
		       &route_discovery_callbacks);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
mesh_close(struct mesh_conn *c)
{
  mh_close(&c->mh);
  route_discovery_close(&c->route_discovery_conn);
}
/*---------------------------------------------------------------------------*/
int
mesh_send(struct mesh_conn *c, rimeaddr_t *to)
{
  int could_send;

  could_send = mh_send(&c->mh, to);

  if(!could_send) {
    if(c->queued_data != NULL) {
      queuebuf_free(c->queued_data);
    }

    PRINTF("mesh_send: queueing data, sending rreq\n");
    c->queued_data = queuebuf_new_from_rimebuf();
    rimeaddr_copy(&c->queued_data_dest, to);
    route_discovery_discover(&c->route_discovery_conn, to,
			     PACKET_TIMEOUT);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
/** @} */
