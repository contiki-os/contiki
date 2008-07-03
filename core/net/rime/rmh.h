/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimermh Best-effort multihop forwarding
 * @{
 *
 * The rmh module implements a multihop forwarding mechanism. Routes
 * must have already been setup with the route_add() function. Setting
 * up routes is done with another Rime module such as the \ref
 * routediscovery "route-discovery module".
 *
 * \section channels Channels
 *
 * The rmh module uses 1 channel.
 *
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: rmh.h,v 1.5 2008/07/03 21:52:25 adamdunkels Exp $
 */

/**
 * \file
 *         Multihop forwarding header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RMH_H__
#define __RMH_H__

#include "net/rime/runicast.h"
#include "net/rime/rimeaddr.h"

struct rmh_conn;

struct rmh_callbacks {
  void (* recv)(struct rmh_conn *ptr, rimeaddr_t *sender, uint8_t hops);
  rimeaddr_t *(* forward)(struct rmh_conn *ptr,
			  rimeaddr_t *originator,
			  rimeaddr_t *dest,
			  rimeaddr_t *prevhop,
			  uint8_t hops);
};

struct rmh_conn {
  struct runicast_conn c;
  const struct rmh_callbacks *cb;
  uint8_t num_rexmit;
};

void rmh_open(struct rmh_conn *c, uint16_t channel,
	      const struct rmh_callbacks *u);
void rmh_close(struct rmh_conn *c);
int rmh_send(struct rmh_conn *c, rimeaddr_t *to, uint8_t num_rexmit,
	     uint8_t max_hops);

#endif /* __RMH_H__ */
/** @} */
/** @} */
