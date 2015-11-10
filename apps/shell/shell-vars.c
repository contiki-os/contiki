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
#include "contiki-conf.h"
#include "shell-vars.h"

#include "loader/symbols.h"

#include <stdio.h>
#include <string.h>

#ifdef SHELL_VARS_CONF_RAM_BEGIN
#define SHELL_VARS_RAM_BEGIN SHELL_VARS_CONF_RAM_BEGIN
#define SHELL_VARS_RAM_END   SHELL_VARS_CONF_RAM_END
#else /* SHELL_VARS_CONF_RAM_BEGIN */
#define SHELL_VARS_RAM_BEGIN 0
#define SHELL_VARS_RAM_END (uintptr_t)-1
#endif /* SHELL_VARS_CONF_RAM_BEGIN */

/*---------------------------------------------------------------------------*/
PROCESS(shell_vars_process, "vars");
SHELL_COMMAND(vars_command,
	      "vars",
	      "vars: list all variables in RAM",
	      &shell_vars_process);
/*---------------------------------------------------------------------------*/
PROCESS(shell_var_process, "var");
SHELL_COMMAND(var_command,
	      "var",
	      "var <variable>: show content of a variable",
	      &shell_var_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_vars_process, ev, data)
{
  int i;
  
  PROCESS_BEGIN();

  shell_output_str(&vars_command, "Variables in RAM:", "");
  
  for(i = 0; i < symbols_nelts; ++i) {
    if(symbols[i].name != NULL &&
       (uintptr_t)symbols[i].value >= SHELL_VARS_RAM_BEGIN &&
       (uintptr_t)symbols[i].value <= SHELL_VARS_RAM_END) {
      shell_output_str(&vars_command, (char *)symbols[i].name, "");
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_var_process, ev, data)
{
  int i;
  int j;
  char numbuf[32];
  
  PROCESS_BEGIN();

  if(data == NULL) {
    shell_output_str(&var_command, "syntax: var <variable name>", "");
  } else {
    for(i = 0; i < symbols_nelts; ++i) {
      if(symbols[i].name != NULL &&
	 strncmp(symbols[i].name, data, strlen(symbols[i].name)) == 0) {
	
	sprintf(numbuf, " %d", *((int *)symbols[i].value));
	shell_output_str(&var_command, (char *)symbols[i].name, numbuf);

	for(j = 0; j < 8 * 8; j += 8) {
	  sprintf(numbuf, "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
		  ((unsigned char *)symbols[i].value)[j],
		  ((unsigned char *)symbols[i].value)[j + 1],
		  ((unsigned char *)symbols[i].value)[j + 2],
		  ((unsigned char *)symbols[i].value)[j + 3],
		  ((unsigned char *)symbols[i].value)[j + 4],
		  ((unsigned char *)symbols[i].value)[j + 5],
		  ((unsigned char *)symbols[i].value)[j + 6],
		  ((unsigned char *)symbols[i].value)[j + 7]);
	  shell_output_str(&var_command, numbuf, "");
	}
	PROCESS_EXIT();
      }
    }
    shell_output_str(&var_command, data, ": variable name not found");
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_vars_init(void)
{
  shell_register_command(&var_command);
  shell_register_command(&vars_command);
}
/*---------------------------------------------------------------------------*/
