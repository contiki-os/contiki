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
 * $Id: slip-dump-drv.c,v 1.1 2007/11/19 23:00:56 oliverschmidt Exp $
 *
 */

#include "contiki.h"
#include "rs232dev.h"

#include "packet-service.h"

#include "tcpdump.h"
#include <string.h>
#include "ctk/ctk.h"

static void output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen);

static const struct packet_service_state state =
  {
    PACKET_SERVICE_VERSION,
    output
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_POLLHANDLER(pollhandler);
EK_PROCESS(proc, PACKET_SERVICE_NAME ": SLIP", EK_PRIO_HIGH,
	   eventhandler, pollhandler, (void *)&state);

#define DUMP_WIDTH 38
#define DUMP_HEIGHT 20
static struct ctk_window window;
static char dump[DUMP_WIDTH * DUMP_HEIGHT];
static struct ctk_label dumplabel =
  {CTK_LABEL(0, 0, DUMP_WIDTH, DUMP_HEIGHT, dump)};
static void
dump_packet(void)
{
  memcpy(dump, &dump[DUMP_WIDTH], DUMP_WIDTH * (DUMP_HEIGHT - 1));
  tcpdump_print(&dump[DUMP_WIDTH * (DUMP_HEIGHT - 1)], DUMP_WIDTH);
  CTK_WIDGET_REDRAW(&dumplabel);
}
/*---------------------------------------------------------------------------*/
LOADER_INIT_FUNC(slip_service_init, arg)
{
  arg_free(arg);
  ek_service_start(PACKET_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
static void
output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen)
{
  rs232dev_send();
  dump_packet();
}
/*---------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    ctk_window_new(&window, DUMP_WIDTH, DUMP_HEIGHT, "SLIP dump");
    CTK_WIDGET_ADD(&window, &dumplabel);
    ctk_window_open(&window);
    rs232dev_init();
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ctk_window_close(&window);
    ek_replace((struct ek_proc *)data, NULL);
    rs232dev_unload();
    LOADER_UNLOAD();
    break;
  case EK_EVENT_REQUEST_EXIT:
    ctk_window_close(&window);
    ek_exit();
    rs232dev_unload();
    LOADER_UNLOAD();
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{  
  uip_len = rs232dev_poll();
  if(uip_len > 0) {
    dump_packet();
    tcpip_input();
  }

}
/*---------------------------------------------------------------------------*/
