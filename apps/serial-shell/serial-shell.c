/**
 * \addtogroup shell
 * @{
 */

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
 */

/**
 * \file
 *         A shell back-end for the serial port
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include "dev/serial-line.h"
#include "net/rime.h"

#include <stdio.h>
#include <string.h>


/*---------------------------------------------------------------------------*/
PROCESS(serial_shell_process, "Contiki serial shell");
/*---------------------------------------------------------------------------*/
void
shell_default_output(const char *text1, int len1, const char *text2, int len2)
{
  int i;
  if(text1 == NULL) {
    text1 = "";
    len1 = 0;
  }
  if(text2 == NULL) {
    text2 = "";
    len2 = 0;
  }

  /* Precision (printf("%.Ns", text1)) not supported on all platforms.
     putchar(c) not be supported on all platforms. */
  for(i = 0; i < len1; i++) {
    printf("%c", text1[i]);
  }
  for(i = 0; i < len2; i++) {
    printf("%c", text2[i]);
  }
  printf("\r\n");
}
/*---------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  printf("%d.%d: %s\r\n", rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 str);
}
/*---------------------------------------------------------------------------*/
void
shell_exit(void)
{
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_shell_process, ev, data)
{
  PROCESS_BEGIN();

  shell_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
    shell_input(data, strlen(data));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
serial_shell_init(void)
{
  process_start(&serial_shell_process, NULL);
}
/*---------------------------------------------------------------------------*/
/** @} */
