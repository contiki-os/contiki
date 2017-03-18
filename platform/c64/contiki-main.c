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

#include <stdlib.h>

#include "contiki-net.h"
#include "ctk/ctk.h"
#include "sys/log.h"
#include "lib/config.h"
#include "dev/slip.h"
#include "net/ethernet-drv.h"

#if WITH_SLIP
#define DRIVER_PROCESS &slip_process,
#define SLIP_INIT() slip_arch_init(0)
#define SLIP_POLL() slip_arch_poll()
#else /* WITH_SLIP */
#define DRIVER_PROCESS &ethernet_process,
#define SLIP_INIT()
#define SLIP_POLL()
#endif /* WITH_SLIP */

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
         DRIVER_PROCESS
         &tcpip_process
         RESOLV_PROCESS);

void slip_arch_poll(void);

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

#if WITH_80COL
  _heapadd((void *)0x0400, 0x0400);
#endif /* WITH_80COL */

  config_read("contiki.cfg");

  SLIP_INIT();

  process_init();
  procinit_init();
  autostart_start(autostart_processes);

  log_message("Contiki up and running ...", "");

  while(1) {

    process_run();

    etimer_request_poll();

    SLIP_POLL();
  }
}
/*-----------------------------------------------------------------------------------*/
