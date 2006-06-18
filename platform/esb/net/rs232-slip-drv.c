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
 * @(#)$Id: rs232-slip-drv.c,v 1.3 2006/06/18 08:19:28 adamdunkels Exp $
 */

#include "contiki.h"
#include "uip.h"
#include "tcpip.h"
#include "rs232-slip.h"

EK_POLLHANDLER(pollhandler);
EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "SLIP driver", EK_PRIO_HIGH,
	   eventhandler, pollhandler, NULL);
static ek_id_t id = EK_ID_NONE;


/*---------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{
  uip_len = rs232_slip_poll();
  tcpip_input();
}
/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(rs232_slip_drv_init, arg)
{
  arg_free(arg);

  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
}
/*---------------------------------------------------------------------------*/
void
rs232_slip_drv_request_poll(void)
{
  ek_request_poll(id);
}
/*---------------------------------------------------------------------------*/
