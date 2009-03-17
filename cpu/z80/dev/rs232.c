/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: rs232.c,v 1.5 2009/03/17 20:20:45 adamdunkels Exp $
 *
 */
/*
 * \file
 * 	This is RS-232C process based on polling.
 * 	Note that rs232.c and rs232-slip.c cannot be used at the same time.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#include "contiki.h"
#include "dev/slip.h"
#include "dev/serial-line.h"
#include "dev/rs232.h"

PROCESS(rs232_process, "RS-232C polling process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rs232_process, ev, data)
{
  static struct etimer timer;
  char ch;
  unsigned char i, stat;
  PROCESS_BEGIN();

  rs232_arch_init(RS232_BAUD_RATE);
  etimer_set(&timer, CLOCK_SECOND / 16);

  while(1) {
    PROCESS_WAIT_EVENT();

    if (etimer_expired(&timer)) {
      for (i = 0; i < RS232_BUFSIZE; i++) {
	ch = rs232_arch_poll(&stat);
	if (stat == 0) {
	  break;
	}
	/* We have an input data */
	RS232_CALLBACK(ch);
      }
      etimer_reset(&timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
