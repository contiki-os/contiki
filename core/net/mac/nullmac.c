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
 * $Id: nullmac.c,v 1.6 2007/12/23 14:56:54 oliverschmidt Exp $
 */

/**
 * \file
 *         A MAC protocol that does not do anything.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/mac/nullmac.h"
#include "net/rime/rimebuf.h"

static const struct radio_driver *radio;
static void (* receiver_callback)(const struct mac_driver *);
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  return radio->send(rimebuf_hdrptr(), rimebuf_totlen());
}
/*---------------------------------------------------------------------------*/
static void
input_packet(const struct radio_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&nullmac_driver);
  }
}
/*---------------------------------------------------------------------------*/
static int
read_packet(void)
{
  int len;
  rimebuf_clear();
  len = radio->read(rimebuf_dataptr(), RIMEBUF_SIZE);
  rimebuf_set_datalen(len);
  return len;
}
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return radio->on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  return radio->off();
}
/*---------------------------------------------------------------------------*/
const struct mac_driver nullmac_driver = {
  send_packet,
  read_packet,
  set_receive_function,
  on,
  off,
};
/*---------------------------------------------------------------------------*/
const struct mac_driver *
nullmac_init(const struct radio_driver *d)
{
  radio = d;
  radio->set_receive_function(input_packet);
  radio->on();
  return &nullmac_driver;
}
/*---------------------------------------------------------------------------*/
