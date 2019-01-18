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
 * $Id: test-cfs.c,v 1.3 2007/11/17 18:10:54 adamdunkels Exp $
 */

/**
 * \file
 *         A quick program for testing the CFS xmem driver
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "cfs/cfs.h"

#include <stdio.h>

PROCESS(cfs_process, "Test CFS process");
AUTOSTART_PROCESSES(&cfs_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cfs_process, ev, data)
{
  PROCESS_BEGIN();

  {
    int i, j, fd;
    int errors = 0;
    int filesize = 65000;
#define CHUNKSIZE 128

    fd = cfs_open("hej", CFS_WRITE);
    if(fd < 0) {
      printf("could not open file for writing, aborting\n");
    } else {
      unsigned char buf[CHUNKSIZE];
      for(i = 0; i < filesize; i += CHUNKSIZE) {
	for(j = 0; j < CHUNKSIZE; ++j) {
	  buf[j] = i + j;
	}
	cfs_write(fd, buf, CHUNKSIZE);
      }
      cfs_close(fd);
    }

    fd = cfs_open("hej", CFS_READ);
    if(fd < 0) {
      printf("could not open file for reading, aborting\n");
    } else {
      for(i = 0; i < filesize; ++i) {
	unsigned char buf;
	cfs_read(fd, &buf, 1);
	if(buf != i) {
	  errors++;
	  printf("error: diff at %d, %d != %d\n", i, i, buf);
	}
      }
    }
    printf("CFS xmem test 1 completed with %d errors\n", errors);

    fd = cfs_open("hej", CFS_WRITE);
    if(fd < 0) {
      printf("could not open file for writing, aborting\n");
    } else {
      unsigned char buf[CHUNKSIZE];
      for(i = 0; i < filesize; i += CHUNKSIZE) {
	for(j = 0; j < CHUNKSIZE; ++j) {
	  buf[j] = i + j + 1;
	}
	cfs_write(fd, buf, CHUNKSIZE);
      }
      cfs_close(fd);
    }

    fd = cfs_open("hej", CFS_READ);
    if(fd < 0) {
      printf("could not open file for reading, aborting\n");
    } else {
      for(i = 0; i < filesize; ++i) {
	unsigned char buf;
	cfs_read(fd, &buf, 1);
	if(buf != i + 1) {
	  errors++;
	  printf("error: diff at %d, %d != %d\n", i, i, buf);
	}
      }
    }
    printf("CFS xmem test 2 completed with %d errors\n", errors);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
