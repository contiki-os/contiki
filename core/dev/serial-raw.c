/*
 * Copyright (c) 2017, Peter Sjödin, KTH Royal Institute of Technology
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
#include "dev/serial-raw.h"
#include <stdio.h>

#include "lib/ringbuf.h"

#ifdef SERIAL_RAW_CONF_BUFSIZE
#define BUFSIZE SERIAL_RAW_CONF_BUFSIZE
#else /* SERIAL_RAW_CONF_BUFSIZE */
#define BUFSIZE 128
#endif /* SERIAL_RAW_CONF_BUFSIZE */

#if (BUFSIZE & (BUFSIZE - 1)) != 0
#error SERIAL_RAW_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change SERIAL_RAW_CONF_BUFSIZE in contiki-conf.h.
#endif

static struct ringbuf rxbuf;
static uint8_t rxbuf_data[BUFSIZE];

PROCESS(serial_raw_process, "Serial driver for raw unbuffered input");
AUTOSTART_PROCESSES(&serial_raw_process);

process_event_t serial_raw_event_message;
/*---------------------------------------------------------------------------*/
int
serial_raw_input_byte(unsigned char c)
{
  /* Add char to buffer. Unlike serial line input, ignore buffer overflow */
  (void) ringbuf_put(&rxbuf, c);
  /* Wake up consumer process */
  process_poll(&serial_raw_process);
  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_raw_process, ev, data)
{
  static uint8_t buf[1];

  PROCESS_BEGIN();
  
  while(1) {
    int c = ringbuf_get(&rxbuf);
    if (c == -1) {
      PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    }
    else {
      buf[0] = c;
      /* Broadcast event */
      process_post(PROCESS_BROADCAST, serial_raw_event_message, buf);

      /* Wait until all processes have handled the serial line event */
      if(PROCESS_ERR_OK ==
         process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL)) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
      }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void
serial_raw_init(void)
{
  ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));
  process_start(&serial_raw_process, NULL);
}
/*---------------------------------------------------------------------------*/
