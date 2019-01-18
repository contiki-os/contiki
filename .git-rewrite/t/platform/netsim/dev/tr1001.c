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
 * $Id: tr1001.c,v 1.2 2006/10/10 15:58:31 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <stdio.h>
#include "tr1001.h"
/*---------------------------------------------------------------------------*/
void
tr1001_init(void)
{
  printf("tr1001_init()\n");
}
/*---------------------------------------------------------------------------*/
unsigned short
tr1001_poll(void)
{
  printf("tr1001_poll()\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
u8_t
tr1001_send(u8_t *packet, u16_t len)
{
  printf("tr1001_send(%p, %d)\n", packet, len);
  return 0;
}
/*---------------------------------------------------------------------------*/
void
tr1001_set_txpower(unsigned char p)
{
  printf("tr1001_set_txpower(%d)\n", p);
}
/*---------------------------------------------------------------------------*/
unsigned short
tr1001_sstrength(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short packets_ok, packets_err;
unsigned short
tr1001_packets_ok(void)
{
  return packets_ok;
}
/*---------------------------------------------------------------------------*/
unsigned short
tr1001_packets_dropped(void)
{
  return packets_err;
}
/*---------------------------------------------------------------------------*/
void
tr1001_clear_packets(void)
{
  packets_ok = packets_err = 0;
}
/*---------------------------------------------------------------------------*/
void
tr1001_clear_active(void)
{
}
/*---------------------------------------------------------------------------*/
int
tr1001_active(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned short
tr1001_sstrength_value(unsigned int type)
{
  printf("tr1001_sstrength_value(%d)\n", type);
  return 0;
}
/*---------------------------------------------------------------------------*/
