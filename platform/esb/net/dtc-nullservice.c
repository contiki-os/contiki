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
 * @(#)$Id: dtc-nullservice.c,v 1.1 2006/06/18 07:49:34 adamdunkels Exp $
 */
#include "contiki.h"
#include "dtc-service.h"
#include "uip-fw.h"

static u8_t input(void) {return UIP_FW_OK;}
static u8_t output(void) {return UIP_FW_OK;}

static const struct dtc_service_interface state =
  {
    DTC_SERVICE_VERSION,
    input,
    output
  };

EK_EVENTHANDLER(eventhandler, ev, data);
/*EK_POLLHANDLER(pollhandler);*/
EK_PROCESS(p, DTC_SERVICE_NAME ": null", EK_PRIO_NORMAL,
	   eventhandler, NULL/*pollhandler*/, (void *)&state);

/*---------------------------------------------------------------------------*/
EK_PROCESS_INIT(dtc_service_init, arg)
{
  ek_service_start(DTC_SERVICE_NAME, &p);
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, NULL);
    break;
  case EK_EVENT_REQUEST_EXIT:
    ek_exit();
    break;
  default:
    break;
  }

}
/*---------------------------------------------------------------------------*/
/*EK_POLLHANDLER(pollhandler)
{

}*/
/*---------------------------------------------------------------------------*/
