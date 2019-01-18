/**
 * \addtogroup rimeaddr
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
 * $Id: rimeaddr.c,v 1.6 2007/08/30 14:39:17 matsutsuka Exp $
 */

/**
 * \file
 *         Functions for manipulating Rime addresses
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/rimeaddr.h"

rimeaddr_t rimeaddr_node_addr;
const rimeaddr_t rimeaddr_null = { { 0, 0 } };

/*---------------------------------------------------------------------------*/
void
rimeaddr_copy(rimeaddr_t *dest, const rimeaddr_t *src)
{
  dest->u16[0] = src->u16[0];
}
/*---------------------------------------------------------------------------*/
int
rimeaddr_cmp(const rimeaddr_t *addr1, const rimeaddr_t *addr2)
{
  return addr1->u16[0] == addr2->u16[0];
}
/*---------------------------------------------------------------------------*/
void
rimeaddr_set_node_addr(rimeaddr_t *t)
{
  rimeaddr_copy(&rimeaddr_node_addr, t);
}
/*---------------------------------------------------------------------------*/
/** @} */
