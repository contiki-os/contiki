/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Test for an SD driver.
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 */


#include "contiki.h"
#include "dev/sd.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

PROCESS(sd_test, "SD test process");
AUTOSTART_PROCESSES(&sd_test);

#define BUF_SIZE	64

PROCESS_THREAD(sd_test, event, data)
{
  static unsigned long iter;
  static unsigned long offset;
  char buf[BUF_SIZE];
  static struct etimer et;
  int r, buflen;

  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND / 16);

  offset = 0;
  for(iter = 1;; iter++) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    memset(buf, 0, sizeof(buf));
    buflen = sprintf(buf, "(%ld) Testing the SD card (%ld)", iter, iter);

    if((iter & 7) == 0) {
      offset = random_rand() & 0xffff;
    } else {
      offset += random_rand() & 0xff;
    }

    r = sd_write(offset, buf, buflen + 1);
    if(r > 0) {
      memset(buf, 0, sizeof(buf));
      r = sd_read(offset, buf, buflen + 1);
      if(r > 0) {
        printf("read %s (offset %lu)\n", buf, offset);
      } else {
        printf("read error: %d (%s)\n", r, sd_error_string(r));
      }
    } else {
      printf("write error: %d (%s)\n", r, sd_error_string(r));
    }
    etimer_restart(&et);
  }

  PROCESS_END();
}
