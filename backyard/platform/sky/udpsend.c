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
 * @(#)$Id: udpsend.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 */

/* Set the IP destination address to something different from the
 * broadcast address. Use commas (",") rather than the normal dots
 * (".") in addresses.
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/uip.h"

#include "dev/leds.h"
#include "dev/light.h"

PROCESS(udpsend_process, "UDP send process");

PROCESS_THREAD(udpsend_process, ev, data)
{
  static struct etimer etimer;
  static struct uip_udp_conn *c;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("udpsend_process starting\n");

  {
    uip_ipaddr_t addr;
    uip_ipaddr(&addr, 255,255,255,255); /* Change address here! */
    c = udp_new(&addr, UIP_HTONS(4321), NULL);
    c->ttl = 1;			/* One hop only. */
  }

  while(1) {
    etimer_set(&etimer, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etimer));

    tcpip_poll_udp(c);
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    char buf[64];
    sprintf(buf, "light sensors %d %d", sensors_light1(), sensors_light2());
    uip_send(buf, strlen(buf) + 1);
  }

 exit:
  /* Contiki does automatic garbage collection of uIP state and we
   * need not worry about that. */
  printf("udpsend_process exiting\n");
  PROCESS_END();
}

/*
 * Initialize this kernel module.
 */
void
_init(void)
{
  process_start(&udpsend_process, NULL);
}

/*
 * When this module is unloaded we must clean up!
 */
void
_fini(void)
{
  process_exit(&udpsend_process);
}
