c/*
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
 */

/**
 * \file
 *         Implementation of the raw packet API with broadcast UDP packets
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"

#include "net/rawpacket-udp.h"

#define PORT 8096

/*---------------------------------------------------------------------------*/
void
rawpacket_udp_init(void)
{

}
/*---------------------------------------------------------------------------*/
struct rawpacket_conn *
rawpacket_setup(int id)
{
  return (struct rawpacket_conn *)udp_broadcast_new(UIP_HTONS(PORT + id), NULL);
}
/*---------------------------------------------------------------------------*/
void
rawpacket_send(struct rawpacket_conn *c, char *data, int datalen)
{
  uip_udp_packet_send((struct uip_udp_conn *)c, data, datalen);
}
/*---------------------------------------------------------------------------*/
int
rawpacket_received(struct rawpacket_conn *c)
{
  return uip_newdata() && (struct uip_udp_conn *)c == uip_udp_conn;
}
/*---------------------------------------------------------------------------*/
