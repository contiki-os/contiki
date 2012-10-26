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
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"

#include "net/rime/timesynch.h"

#include "sys/profile.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(test_shell_process, "Test Contiki shell");
AUTOSTART_PROCESSES(&test_shell_process);
/*---------------------------------------------------------------------------*/
#if 0
PROCESS(shell_xmacprofile_process, "xmacprofile");
SHELL_COMMAND(xmacprofile_command,
	      "xmacprofile",
	      "xmacprofile: show aggregate time",
	      &shell_xmacprofile_process);
/*---------------------------------------------------------------------------*/
#define xmac_timetable_size 256
TIMETABLE_DECLARE(xmac_timetable);
PROCESS_THREAD(shell_xmacprofile_process, ev, data)
{
  int i;
  char buf[40];
  PROCESS_BEGIN();

  for(i = 0; i < xmac_timetable_size; ++i) {
    if(xmac_timetable.timestamps[i].id != NULL) {
      snprintf(buf, sizeof(buf), "%u %u \"%s\"",
	       xmac_timetable.timestamps[i].time + timesynch_offset(),
	       (unsigned short)xmac_timetable.timestamps[i].id,
	       xmac_timetable.timestamps[i].id);
      shell_output_str(&xmacprofile_command, buf, "");
    }
  }
  /*  xmacprofile_aggregate_print_detailed();*/

  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_shell_process, ev, data)
{
  PROCESS_BEGIN();

  serial_shell_init();

  shell_coffee_init();
  shell_exec_init();
  shell_file_init();
  shell_netfile_init();
  shell_ps_init();
  shell_rime_init();
  shell_rime_netcmd_init();
  shell_text_init();

#if 0
  shell_register_command(&xmacprofile_command);
#endif
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
