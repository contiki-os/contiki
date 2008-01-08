/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: serial.c,v 1.3 2008/01/08 07:49:51 adamdunkels Exp $
 */
#include "serial.h"
#include <string.h> /* for memcpy() */

#ifdef SERIAL_CONF_BUFSIZE
#define BUFSIZE SERIAL_CONF_BUFSIZE
#else /* SERIAL_CONF_BUFSIZE */
#define BUFSIZE 80
#endif /* SERIAL_CONF_BUFSIZE */

#define IGNORE_CHAR(c) (c == 0x0d)
#define END 0x0a

static char buffer[BUFSIZE], appbuffer[BUFSIZE];
static volatile unsigned char bufwptr;
static volatile char buffer_full = 0;

PROCESS(serial_process, "Serial driver");

process_event_t serial_event_message;


/*---------------------------------------------------------------------------*/
int
serial_input_byte(unsigned char c)
{
  if(!buffer_full && !IGNORE_CHAR(c)) {
    if(c == END) {
      /* terminate the string */
      buffer[bufwptr++] = '\0';
      buffer_full++;
      process_poll(&serial_process);
      return 1;
    }

    /* reserve space for the terminating zero character */
    if(bufwptr < (BUFSIZE - 1)) {
      buffer[bufwptr++] = c;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_process, ev, data)
{
  PROCESS_BEGIN();

  bufwptr = 0;
  buffer_full = 0;

  serial_event_message = process_alloc_event();

  while(1) {
    PROCESS_YIELD();
    
    if(buffer_full) {
      memcpy(appbuffer, buffer, bufwptr);
      process_post(PROCESS_BROADCAST, serial_event_message, appbuffer);
      bufwptr = 0;
      buffer_full = 0;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
serial_init(void)
{
  process_start(&serial_process, NULL);
}
/*---------------------------------------------------------------------------*/
