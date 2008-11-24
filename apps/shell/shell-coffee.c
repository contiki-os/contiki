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
 * $Id: shell-coffee.c,v 1.3 2008/11/24 15:18:27 nvt-se Exp $
 */

/**
 * \file
 *         CFS Coffee-specific Contiki shell commands
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-coffee.h"

#include "cfs/cfs-coffee.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_format_process, "format");
SHELL_COMMAND(format_command,
	      "format",
	      "format: format the flash-based Coffee file system",
	      &shell_format_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_format_process, ev, data)
{
  struct shell_input *input;
  PROCESS_BEGIN();

  shell_output_str(&format_command, "format: this command will erase all files. Do you want to format? [y/n]", "");
  
  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
  input = data;
  
  if(input->len1 > 0 &&
     (input->data1[0] == 'y' || input->data1[0] == 'Y')) {
    
    shell_output_str(&format_command, "format: formatting file system, please wait...", "");
    if(cfs_coffee_format() == 0) {
      shell_output_str(&format_command, "format: formatting complete", "");
    } else {
      shell_output_str(&format_command, "format: formatting failed", "");
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_coffee_init(void)
{
  shell_register_command(&format_command);
}
/*---------------------------------------------------------------------------*/
