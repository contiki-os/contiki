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
 *         Reboot Contiki shell command
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "dev/leds.h"
#include "dev/watchdog.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_reboot_process, "reboot");
SHELL_COMMAND(reboot_command,
	      "reboot",
	      "reboot: reboot the system",
	      &shell_reboot_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_reboot_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_EXITHANDLER(leds_off(LEDS_ALL);)
  
  PROCESS_BEGIN();

  shell_output_str(&reboot_command,
		   "Rebooting the node in four seconds...", "");

  etimer_set(&etimer, CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  leds_on(LEDS_RED);
  etimer_reset(&etimer);
  PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  leds_on(LEDS_GREEN);
  etimer_reset(&etimer);
  PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  leds_on(LEDS_BLUE);
  etimer_reset(&etimer);
  PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  
  watchdog_reboot();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_reboot_init(void)
{
  shell_register_command(&reboot_command);
}
/*---------------------------------------------------------------------------*/
