/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: testcfs.c,v 1.1 2006/10/11 14:15:16 fros4943 Exp $
 */

#include <stdio.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/clock.h"
#include "sys/log.h"
#include "cfs/cfs.h"

PROCESS(cfs_test_process, "CFS test process");

PROCESS_THREAD(cfs_test_process, ev, data)
{
  static struct etimer mytimer;
  static char wroteLastTime = 0;
  static int fd;
  static char buf[30];
  
  PROCESS_BEGIN();

  etimer_set(&mytimer, CLOCK_SECOND);
  fd = cfs_open("ignored_name", CFS_READ | CFS_WRITE);

  log_message("Starting CFS test process\n", "");

  while(1) {
    PROCESS_WAIT_EVENT();

    if (etimer_expired(&mytimer)) {
	  if (wroteLastTime) {
	  	wroteLastTime = !wroteLastTime;
	    cfs_seek(fd, 5);
	    cfs_read(fd, &buf[0], 9);
	    buf[10] = '\0';
        log_message("Read from filesystem:", buf);
        log_message("\n", "");
	  } else {
	  	wroteLastTime = !wroteLastTime;
	    cfs_seek(fd, 0);
        cfs_write(fd, "tjobaloba labadobahoba", 22);
        log_message("Wrote to filesystem\n", "");
	  }
      etimer_restart(&mytimer);
    }
  }

  PROCESS_END();
}
