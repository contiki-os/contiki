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
 * $Id: testcfs.c,v 1.3 2009/03/02 09:44:07 fros4943 Exp $
 */

#include "contiki.h"
#include "sys/etimer.h"
#include "sys/clock.h"
#include "cfs/cfs.h"

#include <stdio.h>

PROCESS(test_cfs_process, "Test CFS process");
AUTOSTART_PROCESSES(&test_cfs_process);

PROCESS_THREAD(test_cfs_process, ev, data)
{
  static struct etimer et;
  static int fd;
  static uint16_t counter;
  static char buf[30];

  PROCESS_BEGIN();

  printf("Starting CFS test process\n");

  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Write to filesystem */
    sprintf(buf, "filedata%04ifiledata%04i", counter, counter);
    fd = cfs_open("filename", CFS_READ | CFS_WRITE);
    cfs_seek(fd, 0, CFS_SEEK_SET);
    cfs_write(fd, buf, 24);
    cfs_close(fd);
    printf("Wrote to filesystem: '%s'\n", buf);
    counter++;

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Read from filesystem */
    fd = cfs_open("file1", CFS_READ | CFS_WRITE);
    cfs_seek(fd, 4, CFS_SEEK_SET);
    cfs_read(fd, buf, 12);
    cfs_close(fd);
    buf[12] = '\0';
    printf("Read from filesystem: '%s'\n", buf);

  }

  PROCESS_END();
}
