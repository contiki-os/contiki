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
 * @(#)$Id: tcp_loader.c,v 1.4 2010/10/19 18:29:03 adamdunkels Exp $
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "sys/etimer.h"
#include "loader/elfloader_compat.h"

#include "net/uip.h"

#include "dev/xmem.h"

#include "codeprop.h"

#define PRINTF(x)

PROCESS(tcp_loader_process, "TCP loader");

static
struct codeprop_state {
  uint16_t addr;
  uint16_t len;
  struct pt tcpthread_pt;
} s;

/*---------------------------------------------------------------------*/
static
PT_THREAD(recv_tcpthread(struct pt *pt))
{
  PT_BEGIN(pt);

  /* Read the header. */
  PT_WAIT_UNTIL(pt, uip_newdata() && uip_datalen() > 0);

  if(uip_datalen() < sizeof(struct codeprop_tcphdr)) {
    PRINTF(("codeprop: header not found in first tcp segment\n"));
    uip_abort();
    goto thread_done;
  }

  s.len = uip_htons(((struct codeprop_tcphdr *)uip_appdata)->len);
  s.addr = 0;
  uip_appdata += sizeof(struct codeprop_tcphdr);
  uip_len -= sizeof(struct codeprop_tcphdr);

  xmem_erase(XMEM_ERASE_UNIT_SIZE, EEPROMFS_ADDR_CODEPROP);

  /* Read the rest of the data. */
  do {      
    if(uip_len > 0) {
      xmem_pwrite(uip_appdata, uip_len, EEPROMFS_ADDR_CODEPROP + s.addr);
      s.addr += uip_len;
    }
    if(s.addr < s.len) {
      PT_YIELD_UNTIL(pt, uip_newdata());
    }	
  } while(s.addr < s.len);

  /* Kill old program. */
  elfloader_unload();

  /* Link, load, and start new program. */
  int s;
  static char msg[30 + 10];
  s = elfloader_load(EEPROMFS_ADDR_CODEPROP);
  if (s == ELFLOADER_OK)
    sprintf(msg, "ok\n");
  else
    sprintf(msg, "err %d %s\n", s, elfloader_unknown);
    
  /* Return "ok" message. */
  do {
    s = strlen(msg);
    uip_send(msg, s);
    PT_WAIT_UNTIL(pt, uip_acked() || uip_rexmit() || uip_closed());
  } while(uip_rexmit());

  /* Close the connection. */
  uip_close();
    
 thread_done:;
  PT_END(pt);
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(tcp_loader_process, ev, data)
{
  PROCESS_BEGIN();
  
  tcp_listen(UIP_HTONS(CODEPROP_DATA_PORT));
  
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event && uip_conn->lport == UIP_HTONS(CODEPROP_DATA_PORT)) {
      if(uip_connected()) {	/* Really uip_connecting()!!! */
	if(data == NULL) {
	  PT_INIT(&s.tcpthread_pt);
	  process_poll(&tcp_loader_process);
	  tcp_markconn(uip_conn, &s);
	} else {
	  PRINTF(("codeprop: uip_connected() and data != NULL\n"));
	  uip_abort();
	}
      }
      recv_tcpthread(&s.tcpthread_pt); /* Run thread */

      if(uip_closed() || uip_aborted() || uip_timedout()) {
	PRINTF(("codeprop: connection down\n"));
	tcp_markconn(uip_conn, NULL);
      }
    }
  }

  PROCESS_END();
}
