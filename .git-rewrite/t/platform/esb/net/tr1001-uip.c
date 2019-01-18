/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: tr1001-uip.c,v 1.2 2007/08/07 11:14:39 nifi Exp $
 */

#include "contiki-esb.h"
#include "net/hc.h"

/*---------------------------------------------------------------------------*/
static void
tr1001_uip_callback(const struct radio_driver *driver)
{
  uip_len = driver->read(&uip_buf[UIP_LLH_LEN], UIP_BUFSIZE - UIP_LLH_LEN);
  if(uip_len > 0) {
    uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
  }
}
/*---------------------------------------------------------------------------*/
void
tr1001_uip_init()
{
  tr1001_init();
  tr1001_driver.set_receive_function(tr1001_uip_callback);
}
/*---------------------------------------------------------------------------*/
u8_t
tr1001_uip_send(void)
{
  uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
  if (tr1001_driver.send(&uip_buf[UIP_LLH_LEN], uip_len) == 0) {
    return UIP_FW_OK;
  } else {
    return UIP_FW_DROPPED;
  }
}
/*---------------------------------------------------------------------------*/
