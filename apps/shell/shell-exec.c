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
#include "shell-exec.h"
#include "loader/elfloader.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_exec_process, "exec");
SHELL_COMMAND(exec_command,
	      "exec",
	      "exec <filename>: load and execute the ELF file filename",
	      &shell_exec_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_exec_process, ev, data)
{
  char *name;
  int fd;

  PROCESS_BEGIN();

  name = data;
  if(name == NULL || strlen(name) == 0) {
    shell_output_str(&exec_command,
		     "exec <file>: filename must be given", "");
    PROCESS_EXIT();
  }

  /* Kill any old processes. */
  if(elfloader_autostart_processes != NULL) {
    autostart_exit(elfloader_autostart_processes);
  }

  fd = cfs_open(name, CFS_READ | CFS_WRITE);
  if(fd < 0) {
    shell_output_str(&exec_command,
		     "exec: could not open ", name);
  } else {
    int ret;
    char *print, *symbol;

    ret = elfloader_load(fd);
    cfs_close(fd);
    symbol = "";

    switch(ret) {
    case ELFLOADER_OK:
      print = "OK";
      break;
    case ELFLOADER_BAD_ELF_HEADER:
      print = "Bad ELF header";
      break;
    case ELFLOADER_NO_SYMTAB:
      print = "No symbol table";
      break;
    case ELFLOADER_NO_STRTAB:
      print = "No string table";
      break;
    case ELFLOADER_NO_TEXT:
      print = "No text segment";
      break;
    case ELFLOADER_SYMBOL_NOT_FOUND:
      print = "Symbol not found: ";
      symbol = elfloader_unknown;
      break;
    case ELFLOADER_SEGMENT_NOT_FOUND:
      print = "Segment not found: ";
      symbol = elfloader_unknown;
      break;
    case ELFLOADER_NO_STARTPOINT:
      print = "No starting point";
      break;
    default:
      print = "Unknown return code from the ELF loader (internal bug)";
      break;
    }
    shell_output_str(&exec_command, print, symbol);

    if(ret == ELFLOADER_OK) {
#if !PROCESS_CONF_NO_PROCESS_NAMES
      int i;
      for(i = 0; elfloader_autostart_processes[i] != NULL; ++i) {
	shell_output_str(&exec_command, "exec: starting process ",
			 elfloader_autostart_processes[i]->name);
      }
#endif
      autostart_start(elfloader_autostart_processes);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_exec_init(void)
{
  elfloader_init();
  shell_register_command(&exec_command);
}
/*---------------------------------------------------------------------------*/
