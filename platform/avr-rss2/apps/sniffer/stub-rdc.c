/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         Definition of a fake RDC driver to be used with passive
 *         examples. The sniffer will never send packets and it will never
 *         push incoming packets up the stack. We do this by defining this
 *         driver as our RDC. We then drop everything
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "net/mac/mac.h"
#include "net/mac/rdc.h"
/*---------------------------------------------------------------------------*/
static void
send(mac_callback_t sent, void *ptr)
{
  if(sent) {
    sent(ptr, MAC_TX_OK, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *list)
{
  if(sent) {
    sent(ptr, MAC_TX_OK, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
input(void)
{
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return keep_radio_on;
}
/*---------------------------------------------------------------------------*/
static unsigned short
cca(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver stub_rdc_driver = {
  "stub-rdc",
  init,
  send,
  send_list,
  input,
  on,
  off,
  cca,
};
/*---------------------------------------------------------------------------*/
