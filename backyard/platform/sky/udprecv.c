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
 * @(#)$Id: udprecv.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/uip.h"

PROCESS(udprecv_process, "UDP recv process");

PROCESS_THREAD(udprecv_process, ev, data)
{
  static struct uip_udp_conn *c;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("udprecv_process starting\n");

  {
    uip_ipaddr_t any;
    uip_ipaddr(&any, 0,0,0,0);
    c = udp_new(&any, UIP_HTONS(0), NULL);
    uip_udp_bind(c, UIP_HTONS(4321));
  }
  
  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event && uip_newdata()) {
      u8_t *src = ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->srcipaddr.u8;
      printf("%d.%d.%d.%d: %s\n",
	     src[0], src[1], src[2], src[3], (char *)uip_appdata);
    }
  }

 exit:
  /* Contiki does automatic garbage collection of uIP state and we
   * need not worry about that. */
  printf("udprecv_process exiting\n");
  PROCESS_END();
}

/*
 * Initialize this kernel module.
 */
void
_init(void)
{
  process_start(&udprecv_process, NULL);
}

/*
 * When this module is unloaded we must clean up!
 */
void
_fini(void)
{
  process_exit(&udprecv_process);
}
