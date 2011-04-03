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
 * $Id: contiki-main.c,v 1.21 2010/10/27 22:17:39 oliverschmidt Exp $
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

#include "contiki-net.h"

#include "sys/clock.h"
#include "ctk/ctk.h"
#include "ctk/ctk-console.h"

#include "../../apps/directory/directory-dsc.h"
#include "../../apps/webbrowser/www-dsc.h"

#include "sys/etimer.h"
#include "net/wpcap-drv.h"

#ifdef PLATFORM_BUILD
#include "program-handler.h"
#endif  /* PLATFORM_BUILD */

#if WITH_GUI
#define CTK_PROCESS &ctk_process,
#else /* WITH_GUI */
#define CTK_PROCESS
#endif /* WITH_GUI */

PROCINIT(&etimer_process,
	 &wpcap_process,
	 CTK_PROCESS
	 &tcpip_process,
	 &resolv_process);

/*-----------------------------------------------------------------------------------*/
void
debug_printf(char *format, ...)
{
  va_list argptr;
  char buffer[1024];

  va_start(argptr, format);
  vsprintf(buffer, format, argptr);
  va_end(argptr);

#if WITH_GUI
  OutputDebugString(buffer);
#else /* WITH_GUI */
  fputs(buffer, stderr);
#endif /* WITH_GUI */
}
/*-----------------------------------------------------------------------------------*/
void
uip_log(char *message)
{
  debug_printf("%s\n", message);
}
/*-----------------------------------------------------------------------------------*/
void
log_message(const char *part1, const char *part2)
{
  debug_printf("%s%s\n", part1, part2);
}
/*-----------------------------------------------------------------------------------*/
int
main(void)
{
  process_init();

  procinit_init();

#ifdef PLATFORM_BUILD
  program_handler_add(&directory_dsc, "Directory",   1);
  program_handler_add(&www_dsc,       "Web browser", 1);
#endif /* PLATFORM_BUILD */

  autostart_start(autostart_processes);

#if 1
  {
    uip_ipaddr_t addr;
    uip_ipaddr(&addr, 10,1,1,1);
    uip_sethostaddr(&addr);
    log_message("IP Address:  ", inet_ntoa(*(struct in_addr*)&addr));

    uip_ipaddr(&addr, 255,0,0,0);
    uip_setnetmask(&addr);
    log_message("Subnet Mask: ", inet_ntoa(*(struct in_addr*)&addr));

    uip_ipaddr(&addr, 10,1,1,100);
    uip_setdraddr(&addr);
    log_message("Def. Router: ", inet_ntoa(*(struct in_addr*)&addr));

    uip_ipaddr(&addr, 10,1,1,100);
    resolv_conf(&addr);
    log_message("DNS Server:  ", inet_ntoa(*(struct in_addr*)&addr));
  }
#endif

  while(1) {

    process_run();

    etimer_request_poll();

    /* Allow user-mode APC to execute. */
    SleepEx(10, TRUE);

#if WITH_GUI
    if(console_resize()) {
	ctk_restore();
    }
#endif /* WITH_GUI */
  }
}
/*-----------------------------------------------------------------------------------*/
