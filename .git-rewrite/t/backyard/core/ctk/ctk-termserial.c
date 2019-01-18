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
 * @(#)$Id: ctk-termserial.c,v 1.1 2007/05/26 21:46:28 oliverschmidt Exp $
 */
#include "contiki.h"
#include "loader.h"
#include "ctk-term.h"
#include "serial32.h"

/*-----------------------------------------------------------------------------------*/
/* 
 * #defines and enums
 */
/*-----------------------------------------------------------------------------------*/
#define SIO_POLL(c)  (SerialRead(&c) == 1)
#define SIO_SEND(c)  SerialWrite(c)
#define SIO_INIT     SerialOpenPort("COM1")

#ifdef CTK_TERM_CONF_SERIAL_BUFFER_SIZE
#define OUTPUT_BUFFER_SIZE CTK_TERM_CONF_SERIAL_BUFFER_SIZE
#else
#define OUTPUT_BUFFER_SIZE 200
#endif

/*-----------------------------------------------------------------------------------*/
/* 
 * Local variables
 */
/*-----------------------------------------------------------------------------------*/
static ek_id_t id = EK_ID_NONE;

static struct ctk_term_state* termstate;
static unsigned char outbuffer[OUTPUT_BUFFER_SIZE];

EK_POLLHANDLER(pollhandler);
EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "CTK serial server", EK_PRIO_NORMAL,
	   eventhandler, pollhandler, NULL);

/*-----------------------------------------------------------------------------------*/
/* 
 * Idle function
 */
/*-----------------------------------------------------------------------------------*/
EK_POLLHANDLER(pollhandler)
{
  unsigned char c;
  unsigned short len;
  while(SIO_POLL(c)) {
    ctk_term_input(termstate, c);
  }

  len = ctk_term_send(termstate, outbuffer, OUTPUT_BUFFER_SIZE);
  if (len > 0) {
    unsigned short i;
    for (i=0; i < len; ++i) {
      SIO_SEND(outbuffer[i]);
    }
    ctk_term_sent(termstate);
  }
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Init function
 */
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ctk_termserial_init, arg)
{
  arg_free(arg);
  SIO_INIT;
  termstate = ctk_term_alloc_state();
  if (termstate == NULL) return;
  if(id == EK_ID_NONE) {
    id = ek_start(&p);
  }
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{

}
