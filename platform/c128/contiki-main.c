/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#include <string.h>

#include "contiki-net.h"
#include "ctk/ctk.h"
#include "sys/log.h"
#include "lib/config.h"
#include "net/ethernet-drv.h"

#if WITH_GUI
#define CTK_PROCESS &ctk_process,
#else /* WITH_GUI */
#define CTK_PROCESS
#endif /* WITH_GUI */

#if WITH_DNS
#define RESOLV_PROCESS ,&resolv_process
#else /* WITH_DNS */
#define RESOLV_PROCESS
#endif /* WITH_DNS */

PROCINIT(&etimer_process,
         CTK_PROCESS
         &tcpip_process
         RESOLV_PROCESS);

static struct ethernet_config *ethernet_config;

/*-----------------------------------------------------------------------------------*/
#if WITH_ARGS

int contiki_argc;
char **contiki_argv;

void
main(int argc, char **argv)
{
  contiki_argc = argc;
  contiki_argv = argv;

#else /* WITH_ARGS */

void
main(void)
{

#endif /* WITH_ARGS */

  videomode(VIDEOMODE_80COL);

  process_init();

#if 1
  ethernet_config = config_read("contiki.cfg");
#else
  {
    static struct ethernet_config config = {0xDE08, "cs8900a.eth"};
    uip_ipaddr_t addr;

    uip_ipaddr(&addr, 192,168,0,128);
    uip_sethostaddr(&addr);

    uip_ipaddr(&addr, 255,255,255,0);
    uip_setnetmask(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    uip_setdraddr(&addr);

    uip_ipaddr(&addr, 192,168,0,1);
    uip_nameserver_update(&addr, UIP_NAMESERVER_INFINITE_LIFETIME);

    ethernet_config = &config;
  }
#endif

#if (WITH_GUI && WITH_MOUSE)
  {
    static const uint8_t mouse_sprite[64] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x0F, 0xE0, 0x00, 0x0F, 0xC0, 0x00, 0x0F,
      0x80, 0x00, 0x0F, 0xC0, 0x00, 0x0D, 0xE0, 0x00,
      0x08, 0xF0, 0x00, 0x00, 0x78, 0x00, 0x00, 0x3C,
      0x00, 0x00, 0x1E, 0x00, 0x00, 0x0F, 0x00, 0x00,
      0x07, 0x80, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy((void*)0x0E00, mouse_sprite, sizeof(mouse_sprite));
    *(uint8_t*)0x07F8 = 0x0E00 / 64;
    VIC.spr0_color = COLOR_WHITE;
  }
#endif /* WITH_GUI && WITH_MOUSE */

  procinit_init();

  process_start((struct process *)&ethernet_process, (void *)ethernet_config);

  autostart_start(autostart_processes);

  log_message("Contiki up and running ...", "");

  while(1) {

    process_run();

    etimer_request_poll();
  }
}
/*-----------------------------------------------------------------------------------*/
