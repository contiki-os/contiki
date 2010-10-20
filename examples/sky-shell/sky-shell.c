/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: sky-shell.c,v 1.26 2010/10/20 15:22:11 adamdunkels Exp $
 */

/**
 * \file
 *         Tmote Sky-specific Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"
#include "collect-view.h"

#include "net/rime/collect-neighbor.h"
#include "dev/watchdog.h"

#include "net/rime.h"
#include "net/rime/broadcast-announcement.h"
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/light-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/sht11-sensor.h"

#include "net/rime/timesynch.h"


#include <stdio.h>
#include <string.h>

#include <io.h>
#include <signal.h>

/*---------------------------------------------------------------------------*/
PROCESS(sky_shell_process, "Sky Contiki shell");
AUTOSTART_PROCESSES(&sky_shell_process);
/*---------------------------------------------------------------------------*/
static void
periodic_debug(void *ptr)
{
  ctimer_set(ptr, 20 * CLOCK_SECOND, periodic_debug, ptr);
  collect_print_stats();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sky_shell_process, ev, data)
{
  static struct ctimer c;
  PROCESS_BEGIN();

  /*  ctimer_set(&c, 20 * CLOCK_SECOND, periodic_debug, &c);*/

  serial_shell_init();
  shell_blink_init();
  /*  shell_file_init();
      shell_coffee_init();*/
  /*  shell_download_init();
      shell_rime_sendcmd_init();*/
  /*  shell_ps_init();*/
  shell_reboot_init();
  shell_rime_init();
  shell_rime_netcmd_init();
  /*  shell_rime_ping_init();
  shell_rime_debug_init();
  shell_rime_debug_runicast_init();*/
  /*  shell_rime_sniff_init();*/
  shell_sky_init();
  shell_powertrace_init();
  /*  shell_base64_init();*/
  shell_text_init();
  shell_time_init();
  /*  shell_checkpoint_init();*/
  /*  shell_sendtest_init();*/

  shell_collect_view_init();

#if DEBUG_SNIFFERS
  rime_sniffer_add(&s);
#endif /* DEBUG_SNIFFERS */
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
