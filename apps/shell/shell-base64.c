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
 *         Base64-related shell commands
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include <ctype.h>
#include <stdio.h>
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */

#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_dec64_process, "dec64");
SHELL_COMMAND(dec64_command,
	      "dec64",
	      "dec64: decode base64 input",
	      &shell_dec64_process);
/*---------------------------------------------------------------------------*/
#define BASE64_MAX_LINELEN 76

struct base64_decoder_state {
  uint8_t data[3 * BASE64_MAX_LINELEN / 4];
  int dataptr;
  unsigned long tmpdata;
  int sextets;
  int padding;
};
/*---------------------------------------------------------------------------*/
static int
base64_decode_char(char c)
{
  if(c >= 'A' && c <= 'Z') {
    return c - 'A';
  } else if(c >= 'a' && c <= 'z') {
    return c - 'a' + 26;
  } else if(c >= '0' && c <= '9') {
    return c - '0' + 52;
  } else if(c == '+') {
    return 62;
  } else if(c == '/') {
    return 63;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
base64_add_char(struct base64_decoder_state *s, char c)
{
  if(isspace((int)c)) {
    return 0;
  }

  if(s->dataptr >= sizeof(s->data)) {
    return 0;
  }
  if(c == '=') {
    ++s->padding;
  }
  
  s->tmpdata = (s->tmpdata << 6) | base64_decode_char(c);
  ++s->sextets;
  if(s->sextets == 4) {
    s->sextets = 0;
    s->data[s->dataptr] = (uint8_t)(s->tmpdata >> 16);
    s->data[s->dataptr + 1] = (uint8_t)(s->tmpdata >> 8);
    s->data[s->dataptr + 2] = (uint8_t)(s->tmpdata);
    s->dataptr += 3;
    if(s->dataptr == sizeof(s->data)) {
      return 0;
    } else {
      return 1;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_dec64_process, ev, data)
{
  struct shell_input *input;
  struct base64_decoder_state s;
  int i;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }

    s.sextets = s.dataptr = s.padding = 0;

    for(i = 0; i < input->len1; ++i) {
      base64_add_char(&s, input->data1[i]);
    }
    for(i = 0; i < input->len2; ++i) {
      base64_add_char(&s, input->data2[i]);
    }
    shell_output(&dec64_command, s.data, s.dataptr - s.padding, "", 0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_base64_init(void)
{
  shell_register_command(&dec64_command);
}
/*---------------------------------------------------------------------------*/
