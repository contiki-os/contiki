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
 * $Id: contiki-serial-main.c,v 1.5 2009/12/11 14:59:31 matsutsuka Exp $
 *
 */

/*
 * \file
 * 	This is a sample main file with serial.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#include "contiki.h"

/* devices */
#include "dev/serial-line.h"
#include "ctk/libconio_arch-small.h"

#undef RS232_INTR
#ifdef RS232_INTR
void rs232_arch_writeb(uint8_t ch);
void rs232_arch_init(int (* callback)(unsigned char), unsigned long ubr);
#else
#include "dev/rs232.h"
#endif

PROCESS(stest_process, "Serial test process");
/*---------------------------------------------------------------------------*/
static void
rs232_print(char* str) {
  while (*str != 0) {
    rs232_arch_writeb(*str++);
  }
}
/*---------------------------------------------------------------------------*/
static void
log(char* str) {
  while (*str != 0) {
    libputc_arch(*str++);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stest_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  clrscr_arch();
#ifdef RS232_INTR
  rs232_arch_init(serial_line_input_byte, 0);
#endif

  etimer_set(&timer, CLOCK_SECOND);

  log("Starting serial test process");
  while(1) {
    PROCESS_WAIT_EVENT();

    if (etimer_expired(&timer)) {
      log("Sending serial data now");
      rs232_print("GNU's not Unix\n");
      etimer_reset(&timer);
    }

    if(ev == serial_line_event_message) {
      log(data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
main(void)
{
  /* initialize process manager. */
  process_init();

  /* start services */
  process_start(&etimer_process, NULL);
  process_start(&serial_line_process, NULL);
#ifndef RS232_INTR
  process_start(&rs232_process, NULL);
#endif
  process_start(&stest_process, NULL);

  while(1) {
    process_run();
    etimer_request_poll();
  }
}
