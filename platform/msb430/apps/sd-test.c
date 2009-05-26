/*
 * Copyright (c) 2007 Swedish Institute of Computer Science.
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
 * $Id: sd-test.c,v 1.4 2009/05/26 13:23:59 nvt-se Exp $
 *
 * \file
 *	A simple example of using the SD card on the MSB430 platform.
 * \author
 *	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "dev/sd/sd.h"
#include "dev/msb430-uart1.h"
#include "lib/random.h"
#include "net/rime.h"

#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE	512UL

#define CALM_MODE	1
#define ALTER_OFFSET	0

/*---------------------------------------------------------------------------*/
PROCESS(test_sd_process, "SD test");
AUTOSTART_PROCESSES(&test_sd_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_sd_process, ev, data)
{
  static char buf[BLOCK_SIZE];
#if CALM_MODE
  static struct etimer et;
#endif
  static int r;
  static unsigned long iter;
  static uint32_t offset;

  PROCESS_BEGIN();

  printf("starting the SD test\n");

  while(1) {
    printf("\n\nIteration %lu\n", ++iter);
    sprintf(buf, "Testing the SD memory #%lu.", iter);

#if CALM_MODE
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
#else
    PROCESS_PAUSE();
#endif
    offset = BLOCK_SIZE;
#if ALTER_OFFSET
    offset *= (random_rand() & 0xff);
#endif

    r = sd_write_block(offset, &buf);
    if(r != SD_WRITE_SUCCESS) {
      printf("writing failed: %d\n", r);
    }

    memset(buf, 0, sizeof (buf));
    r = sd_read_block(&buf, offset);
    if(r > 0) {
      printf("Read %d bytes\n", r);
      buf[sizeof(buf) - 1] = '\0';
      printf("Contents of the buffer: %s\n", buf);
    } else {
      printf("reading failed\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
