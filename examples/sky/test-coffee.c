/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: test-coffee.c,v 1.12 2009/09/17 14:32:57 nvt-se Exp $
 */

/**
 * \file
 *         Basic test for CFS/Coffee.
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

PROCESS(testcoffee_process, "Test CFS/Coffee process");
AUTOSTART_PROCESSES(&testcoffee_process);

#define FAIL(x) 	error = (x); goto end;

#define FILE_SIZE	4096

/*---------------------------------------------------------------------------*/
static int
coffee_gc_test(void)
{
  int i;

  cfs_remove("alpha");
  cfs_remove("beta");


  for (i = 0; i < 100; i++) {
    if (i & 1) {
      if(cfs_coffee_reserve("alpha", random_rand() & 0xffff) < 0) {
	return -i;
      }
      cfs_remove("beta");
    } else {
      if(cfs_coffee_reserve("beta", 93171) < 0) {
	return -1;
      }
      cfs_remove("alpha");
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
coffee_file_test(void)
{
  int error;
  int wfd, rfd, afd;
  unsigned char buf[256], buf2[11];
  int r, i, j, total_read;
  unsigned offset;

  cfs_remove("T1");
  cfs_remove("T2");
  cfs_remove("T3");

  wfd = rfd = afd = -1;

  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = r;
  }

  /* Test 1: Open for writing. */
  wfd = cfs_open("T1", CFS_WRITE);
  if(wfd < 0) {
    FAIL(-1);
  }

  /* Test 2: Write buffer. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-2);
  } else if(r < sizeof(buf)) {
    FAIL(-3);
  }

  /* Test 3: Deny reading. */
  r = cfs_read(wfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-4);
  }

  /* Test 4: Open for reading. */
  rfd = cfs_open("T1", CFS_READ);
  if(rfd < 0) {
    FAIL(-5);
  }

  /* Test 5: Write to read-only file. */
  r = cfs_write(rfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-6);
  }

  /* Test 7: Read the buffer written in Test 2. */
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-8);
  } else if(r < sizeof(buf)) {
    printf("r=%d\n", r);
    FAIL(-9);
  }

  /* Test 8: Verify that the buffer is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      printf("r=%d. buf[r]=%d\n", r, buf[r]);
      FAIL(-10);
    }
  }

  /* Test 9: Seek to beginning. */
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-11);
  }

  /* Test 10: Write to the log. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-12);
  } else if(r < sizeof(buf)) {
    FAIL(-13);
  }

  /* Test 11: Read the data from the log. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-14);
  } else if(r < sizeof(buf)) {
    FAIL(-15);
  }

  /* Test 12: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      FAIL(-16);
    }
  }

  /* Test 13: Write a reversed buffer to the file. */
  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = sizeof(buf) - r - 1;
  }
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-17);
  }
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-18);
  } else if(r < sizeof(buf)) {
    FAIL(-19);
  }
  if(cfs_seek(rfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-20);
  }

  /* Test 14: Read the reversed buffer. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-21);
  } else if(r < sizeof(buf)) {
    printf("r = %d\n", r);
    FAIL(-22);
  }

  /* Test 15: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != sizeof(buf) - r - 1) {
      FAIL(-23);
    }
  }

  cfs_close(rfd);
  cfs_close(wfd);

  if(cfs_coffee_reserve("T2", FILE_SIZE) < 0) {
    FAIL(-24);
  }

  /* Test 16: Test multiple writes at random offset. */
  for(r = 0; r < 100; r++) {
    wfd = cfs_open("T2", CFS_WRITE | CFS_READ);
    if(wfd < 0) {
      FAIL(-25);
    }

    offset = random_rand() % FILE_SIZE;

    for(r = 0; r < sizeof(buf); r++) {
      buf[r] = r;
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      FAIL(-26);
    }

    if(cfs_write(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      FAIL(-27);
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      FAIL(-28);
    }

    memset(buf, 0, sizeof(buf));
    if(cfs_read(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      FAIL(-29);
    }

    for(i = 0; i < sizeof(buf); i++) {
      if(buf[i] != i) {
        printf("buf[%d] != %d\n", i, buf[i]);
        FAIL(-30);
      }
    }
  }

  /* Test 17: Append data to the same file many times. */
#define APPEND_BYTES 1000
#define BULK_SIZE 10
  for(i = 0; i < APPEND_BYTES; i += BULK_SIZE) {
    afd = cfs_open("T3", CFS_WRITE | CFS_APPEND);
    if(afd < 0) {
      FAIL(-31);
    }
    for(j = 0; j < BULK_SIZE; j++) {
      buf[j] = 1 + ((i + j) & 0x7f);
    }
    if((r = cfs_write(afd, buf, BULK_SIZE)) != BULK_SIZE) {
      printf("r=%d\n", r);
      FAIL(-32);
    }
    cfs_close(afd);
  }

  /* Test 18: Read back the data written in Test 17 and verify that it 
     is correct. */
  afd = cfs_open("T3", CFS_READ);
  if(afd < 0) {
    FAIL(-33);
  }
  total_read = 0;
  while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
    for(j = 0; j < r; j++) {
      if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
	FAIL(-34);
      }
    }
    total_read += r;
  }
  if(r < 0) {
    FAIL(-35);
  }
  if(total_read != APPEND_BYTES) {
    FAIL(-36);
  }
  cfs_close(afd);

  error = 0;
end:
  cfs_close(wfd); cfs_close(rfd); cfs_close(afd);
  return error;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(testcoffee_process, ev, data)
{
  int start;

  PROCESS_BEGIN();

  start = clock_seconds();
  printf("Coffee format: %d\n", cfs_coffee_format());
  printf("Coffee file test: %d\n", coffee_file_test());
  printf("Coffee garbage collection test: %d\n", coffee_gc_test());
  printf("Test time: %d seconds\n", (int)(clock_seconds() - start));

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
