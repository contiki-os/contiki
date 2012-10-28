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

#include <string.h>

#include "contiki.h"

#include "shell-run.h"

/*---------------------------------------------------------------------------*/
PROCESS(shell_run_process, "run");
SHELL_COMMAND(run_command,
	      "run",
	      "run: load and run a PRG file",
	      &shell_run_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_run_process, ev, data)
{
  char *name, *print;

  PROCESS_BEGIN();

  name = data;
  if(name == NULL || strlen(name) == 0) {
    shell_output_str(&run_command,
		     "run <file>: filename must be given", "");
    PROCESS_EXIT();
  }

  switch(LOADER_LOAD(name, NULL)) {
  case LOADER_OK:
    print = "OK";
    break;
  case LOADER_ERR_READ:
    print = "Read error";
    break;
  case LOADER_ERR_HDR:
    print = "Header error";
    break;
  case LOADER_ERR_OS:
    print = "Wrong OS";
    break;
  case LOADER_ERR_FMT:
    print = "Data format error";
    break;
  case LOADER_ERR_MEM:
    print = "Not enough memory";
    break;
  case LOADER_ERR_OPEN:
    print = "Could not open file";
    break;
  case LOADER_ERR_ARCH:
    print = "Wrong architecture";
    break;
  case LOADER_ERR_VERSION:
    print = "Wrong OS version";
    break;
  case LOADER_ERR_NOLOADER:
    print = "Program loading not supported";
    break;
  default:
    print = "Unknown return code from the loader (internal bug)";
    break;
  }
  shell_output_str(&run_command, print, ".");
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_run_init(void)
{
  shell_register_command(&run_command);
}
/*---------------------------------------------------------------------------*/
