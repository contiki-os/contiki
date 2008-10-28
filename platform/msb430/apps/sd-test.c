/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: sd-test.c,v 1.1 2008/10/28 12:56:57 nvt-se Exp $
 *
 * \file
 *	A simple example of using the SD card on the MSB430 platform.
 * \author
 *	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "dev/sd/sd.h"
#include "dev/msb430-uart1.h"

#include <stdio.h>

#define BLOCK_SIZE	512

/*---------------------------------------------------------------------------*/
PROCESS(test_sd_process, "SD test");
AUTOSTART_PROCESSES(&test_sd_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_sd_process, ev, data)
{
  static char buf[BLOCK_SIZE];
  static struct etimer et;
  static int r;
  static unsigned iter;

  PROCESS_BEGIN();

  printf("starting the SD test\n");

  while(1) {
    printf("\n\nIteration %u\n", ++iter);
    sprintf(buf, "Testing the SD memory #%u.", iter);

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    r = sd_write_block(BLOCK_SIZE, &buf);
    if(r != SD_WRITE_SUCCESS) {
      printf("sd_write_block failed: %d\n", r);
    }
	  
    memset(buf, 0, sizeof (buf));
    r = sd_read_block(&buf, BLOCK_SIZE);
    printf("Read %d bytes\n", r);
    if(r > 0) {
      buf[sizeof(buf) - 1] = '\0';
      printf("Contents of the buffer: %s\n", buf);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
