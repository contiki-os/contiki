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
 * $Id: shell-rime-sniff.c,v 1.1 2008/02/04 23:42:17 adamdunkels Exp $
 */

/**
 * \file
 *         Contiki shell Rime sniffer application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "shell.h"
#include "net/rime.h"
/*---------------------------------------------------------------------------*/
PROCESS(shell_sniff_process, "sniff");
SHELL_COMMAND(sniff_command,
	      "sniff",
	      "sniff: dump incoming packets",
	      &shell_sniff_process);
/*---------------------------------------------------------------------------*/
enum {
  SNIFFER_INPUT,
  SNIFFER_OUTPUT,
};
/*---------------------------------------------------------------------------*/
static void
input_sniffer(void)
{
  struct {
    uint16_t len;
    uint16_t flags;
  } msg;
  msg.len = rimebuf_totlen() / 2 + 1;
  msg.flags = SNIFFER_INPUT;
  shell_output(&sniff_command, &msg, sizeof(msg),
	       rimebuf_dataptr(), (rimebuf_datalen() & 0xfffe) +
	       2 * (rimebuf_totlen() & 1));
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(void)
{
  struct {
    uint16_t len;
    uint16_t flags;
  } msg;
  msg.len = rimebuf_totlen() / 2 + 1;
  msg.flags = SNIFFER_OUTPUT;
  shell_output(&sniff_command, &msg, sizeof(msg),
	       rimebuf_hdrptr(), (rimebuf_totlen() & 0xfffe) +
	       2 * (rimebuf_totlen() & 1));
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sniff_process, ev, data)
{
  RIME_SNIFFER(s, input_sniffer, output_sniffer);
  PROCESS_EXITHANDLER(goto exit;);
  PROCESS_BEGIN();

  rime_sniffer_add(&s);
  
  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);

 exit:
  rime_sniffer_remove(&s);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_rime_sniff_init(void)
{
  shell_register_command(&sniff_command);
}
/*---------------------------------------------------------------------------*/
