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
 * $Id: test-rudolph1.c,v 1.1 2007/03/21 23:24:24 adamdunkels Exp $
 */

/**
 * \file
 *         Testing the rudolph1 code in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "rudolph1.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "cfs/cfs-ram.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(test_rudolph1_process, "Rudolph1 test");
AUTOSTART_PROCESSES(&test_rudolph1_process);
/*---------------------------------------------------------------------------*/
static int
newfile(struct rudolph1_conn *c)
{
  printf("+++ rudolph1 new file incoming at %lu\n", clock_time());
  fflush(NULL);
  return cfs_open("hej", CFS_WRITE);
}
static void
recv(struct rudolph1_conn *c, int cfs_fd)
{
  int fd;
  int i;
  
  printf("+++ rudolph1 entire file received at %lu\n", clock_time());
  fflush(NULL);
  cfs_close(cfs_fd);

  
  fd = cfs_open("hej", CFS_READ);
  for(i = 0; i < 200; ++i) {
    unsigned char buf;
    cfs_read(fd, &buf, 1);
    if(buf != i) {
      printf("error: diff at %d, %d != %d\n", i, i, buf);
    }
  }
}
const static struct rudolph1_callbacks rudolph1_call = {newfile,
							recv};
static struct rudolph1_conn rudolph1;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_rudolph1_process, ev, data)
{
  PROCESS_BEGIN();

  process_start(&cfs_ram_process, NULL);
  PROCESS_PAUSE();

  {
    int i, fd;
    
    fd = cfs_open("hej", CFS_WRITE);
    for(i = 0; i < 200; i++) {
      unsigned char buf = i;
      cfs_write(fd, &buf, 1);
    }
    cfs_close(fd);
  }
  
  rudolph1_open(&rudolph1, 128, &rudolph1_call);
  button_sensor.activate();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);

    rudolph1_send(&rudolph1, cfs_open("hej", CFS_READ));

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
