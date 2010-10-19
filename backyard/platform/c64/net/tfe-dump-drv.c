/*
 * Copyright (c) 2001-2004, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: tfe-dump-drv.c,v 1.2 2010/10/19 18:29:04 adamdunkels Exp $
 *
 */

#include "contiki-net.h"
#include "cs8900a.h"

static void output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen);

/* 00:0E:3A is the OUI of Cirrus Logic, 64:64:64 just means C64 */
static const struct uip_eth_addr addr =
  {{0x00,0x0e,0x3a,0x64,0x64,0x64}};

static const struct packet_service_state state =
  {
    PACKET_SERVICE_VERSION,
    output
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_POLLHANDLER(pollhandler);
EK_PROCESS(proc, PACKET_SERVICE_NAME ": TFE", EK_PRIO_NORMAL,
	   eventhandler, pollhandler, (void *)&state);

#include "tcpdump.h"
#include <string.h>
#include "ctk/ctk.h"

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
LOADER_INIT_FUNC(tfe_dump_drv_init, arg)
{
  arg_free(arg);
  ek_service_start(PACKET_SERVICE_NAME, &proc);
}
/*---------------------------------------------------------------------------*/
static void
output(u8_t *hdr, u16_t hdrlen, u8_t *data, u16_t datalen)
{
  uip_arp_out();
  cs8900a_send();
  dump_packet();
}
/*---------------------------------------------------------------------------*/

EK_EVENTHANDLER(eventhandler, ev, data)
{
  switch(ev) {
  case EK_EVENT_INIT:
  case EK_EVENT_REPLACE:
    ctk_window_new(&window, DUMP_WIDTH, DUMP_HEIGHT, "TFE dump");
    CTK_WIDGET_ADD(&window, &dumplabel);
    ctk_window_open(&window);
    uip_setethaddr(addr);
    cs8900a_init();
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ctk_window_close(&window);
    ek_replace((struct ek_proc *)data, NULL);
    LOADER_UNLOAD();
    break;
  case EK_EVENT_REQUEST_EXIT:
    ctk_window_close(&window);
    ek_exit();
    LOADER_UNLOAD();
    break;
  default:
    break;
  }
}

/*---------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
  
  /* Poll Ethernet device to see if there is a frame avaliable. */
  uip_len = cs8900a_poll();
  if(uip_len > 0) {
    dump_packet();
    /* A frame was avaliable (and is now read into the uip_buf), so
       we process it. */
    if(BUF->type == UIP_HTONS(UIP_ETHTYPE_IP)) {
      uip_arp_ipin();
      uip_len -= sizeof(struct uip_eth_hdr);
      tcpip_input();
    } else if(BUF->type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
      uip_arp_arpin();
      /* If the above function invocation resulted in data that
         should be sent out on the network, the global variable
         uip_len is set to a value > 0. */
      if(uip_len > 0) {
        cs8900a_send();
      }
    }
  }

}
/*---------------------------------------------------------------------------*/
