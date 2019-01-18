/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki OS
 *
 * $Id: slip-drv.c,v 1.1 2007/11/19 23:00:56 oliverschmidt Exp $
 *
 */

#include "contiki-net.h"
#include "rs232dev.h"

static u8_t output(void);

SERVICE(slip_drv_service, packet_service, { output };);

PROCESS(slip_drv_process, "SLIP driver");

/*---------------------------------------------------------------------------*/
static u8_t
output(void)
{
  rs232dev_send();
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{  
  uip_len = rs232dev_poll();
  if(uip_len > 0) {
    tcpip_input();
  }

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(slip_drv_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());
  PROCESS_EXITHANDLER(rs232dev_unload());
  
  PROCESS_BEGIN();

  rs232dev_init();
  
  SERVICE_REGISTER(slip_drv_service);

  process_poll(&slip_drv_process);
  
  while(1) {
    PROCESS_YIELD();
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
