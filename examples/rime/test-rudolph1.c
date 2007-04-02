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
 * $Id: test-rudolph1.c,v 1.4 2007/04/02 10:04:37 adamdunkels Exp $
 */

/**
 * \file
 *         Testing the rudolph1 code in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rudolph1.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "cfs/cfs-ram.h"

#include <stdio.h>

#define FILESIZE 200

/*---------------------------------------------------------------------------*/
PROCESS(test_rudolph1_process, "Rudolph1 test");
AUTOSTART_PROCESSES(&test_rudolph1_process);
/*---------------------------------------------------------------------------*/
static void
write_chunk(struct rudolph1_conn *c, int offset, int flag,
	    char *data, int datalen)
{
  int fd;
  
  if(flag == RUDOLPH1_FLAG_NEWFILE) {
    /*printf("+++ rudolph1 new file incoming at %lu\n", clock_time());*/
    leds_on(LEDS_RED);
    fd = cfs_open("codeprop.out", CFS_WRITE);
  } else {
    fd = cfs_open("codeprop.out", CFS_WRITE + CFS_APPEND);
  }
  
  if(datalen > 0) {
    int ret;
    cfs_seek(fd, offset);
    ret = cfs_write(fd, data, datalen);
    /*    printf("write_chunk wrote %d bytes at %d, %d\n", ret, offset, (unsigned char)data[0]);*/
  }

  cfs_close(fd);

  if(flag == RUDOLPH1_FLAG_LASTCHUNK) {
    int i;
    /*    printf("+++ rudolph1 entire file received at %lu\n", clock_time());*/
    leds_off(LEDS_RED);
    leds_on(LEDS_YELLOW);

    fd = cfs_open("hej", CFS_READ);
    for(i = 0; i < FILESIZE; ++i) {
      unsigned char buf;
      cfs_read(fd, &buf, 1);
      if(buf != (unsigned char)i) {
	printf("error: diff at %d, %d != %d\n", i, i, buf);
	break;
      }
    }
#if NETSIM
    ether_send_done();
#endif
    cfs_close(fd);
  }
}
static int
read_chunk(struct rudolph1_conn *c, int offset, char *to, int maxsize)
{
  int fd;
  int ret;
  
  fd = cfs_open("hej", CFS_READ);

  cfs_seek(fd, offset);
  ret = cfs_read(fd, to, maxsize);
  /*  printf("read_chunk %d bytes at %d, %d\n", ret, offset, (unsigned char)to[0]);*/
  cfs_close(fd);
  return ret;
}
const static struct rudolph1_callbacks rudolph1_call = {write_chunk,
							read_chunk};
static struct rudolph1_conn rudolph1;
/*---------------------------------------------------------------------------*/
#include "node-id.h"

PROCESS_THREAD(test_rudolph1_process, ev, data)
{
  static int fd;
  PROCESS_EXITHANDLER(rudolph1_close(&rudolph1);)
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  
  rudolph1_open(&rudolph1, 128, &rudolph1_call);
  button_sensor.activate();

  PROCESS_PAUSE();
  
  if(rimeaddr_node_addr.u8[0] == 1 &&
     rimeaddr_node_addr.u8[1] == 1) {
    {
      int i;
 
      fd = cfs_open("hej", CFS_WRITE);
      for(i = 0; i < FILESIZE; i++) {
	unsigned char buf = i;
	cfs_write(fd, &buf, 1);
      }
      cfs_close(fd);
    }
    rudolph1_send(&rudolph1, CLOCK_SECOND / 2);

  }
  
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);
    {
      int i;
      
      fd = cfs_open("hej", CFS_WRITE);
      for(i = 0; i < FILESIZE; i++) {
	unsigned char buf = i;
	cfs_write(fd, &buf, 1);
      }
      cfs_close(fd);
    }
    rudolph1_send(&rudolph1, CLOCK_SECOND / 2);

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);
    rudolph1_stop(&rudolph1);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
