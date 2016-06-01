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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Basic test for CFS/Coffee.
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
PROCESS(testcoffee_process, "Test CFS/Coffee process");
AUTOSTART_PROCESSES(&testcoffee_process);
/*---------------------------------------------------------------------------*/
#define TEST_FAIL(x) 	error = (x); goto end;
#define FILE_SIZE	4096
/*---------------------------------------------------------------------------*/
static int
coffee_test_basic(void)
{
  int error;
  int wfd, rfd, afd;
  unsigned char buf[256];
  int r;

  wfd = rfd = afd = -1;

  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = r;
  }

  /* Test 1: Open for writing. */
  wfd = cfs_open("T1", CFS_WRITE);
  if(wfd < 0) {
    TEST_FAIL(1);
  }

  /* Test 2 and 3: Write buffer. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(2);
  } else if(r < sizeof(buf)) {
    TEST_FAIL(3);
  }

  /* Test 4: Deny reading. */
  r = cfs_read(wfd, buf, sizeof(buf));
  if(r >= 0) {
    TEST_FAIL(4);
  }

  /* Test 5: Open for reading. */
  rfd = cfs_open("T1", CFS_READ);
  if(rfd < 0) {
    TEST_FAIL(5);
  }

  /* Test 6: Write to read-only file. */
  r = cfs_write(rfd, buf, sizeof(buf));
  if(r >= 0) {
    TEST_FAIL(6);
  }

  /* Test 7 and 8: Read the buffer written in Test 2. */
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(7);
  } else if(r < sizeof(buf)) {
    printf("r=%d\n", r);
    TEST_FAIL(8);
  }

  /* Test 9: Verify that the buffer is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      printf("r=%d. buf[r]=%d\n", r, buf[r]);
      TEST_FAIL(9);
    }
  }

  /* Test 10: Seek to beginning. */
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    TEST_FAIL(10);
  }

  /* Test 11 and 12: Write to the log. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(11);
  } else if(r < sizeof(buf)) {
    TEST_FAIL(12);
  }

  /* Test 13 and 14: Read the data from the log. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(13);
  } else if(r < sizeof(buf)) {
    TEST_FAIL(14);
  }

  /* Test 16: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      TEST_FAIL(15);
    }
  }

  /* Test 17 to 20: Write a reversed buffer to the file. */
  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = sizeof(buf) - r - 1;
  }
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    TEST_FAIL(16);
  }
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(17);
  } else if(r < sizeof(buf)) {
    TEST_FAIL(18);
  }
  if(cfs_seek(rfd, 0, CFS_SEEK_SET) != 0) {
    TEST_FAIL(19);
  }

  /* Test 21 and 22: Read the reversed buffer. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    TEST_FAIL(20);
  } else if(r < sizeof(buf)) {
    printf("r = %d\n", r);
    TEST_FAIL(21);
  }

  /* Test 23: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != sizeof(buf) - r - 1) {
      TEST_FAIL(22);
    }
  }

  error = 0;
end:
  cfs_close(wfd);
  cfs_close(rfd);
  cfs_remove("T1");
  return error;
}
/*---------------------------------------------------------------------------*/
static int
coffee_test_append(void)
{
  int error;
  int afd;
  unsigned char buf[256], buf2[11];
  int r, i, j, total_read;
#define APPEND_BYTES 1000
#define BULK_SIZE 10

  /* Test 1 and 2: Append data to the same file many times. */
  for(i = 0; i < APPEND_BYTES; i += BULK_SIZE) {
    afd = cfs_open("T2", CFS_WRITE | CFS_APPEND);
    if(afd < 0) {
      TEST_FAIL(1);
    }
    for(j = 0; j < BULK_SIZE; j++) {
      buf[j] = 1 + ((i + j) & 0x7f);
    }
    if((r = cfs_write(afd, buf, BULK_SIZE)) != BULK_SIZE) {
      printf("r=%d\n", r);
      TEST_FAIL(2);
    }
    cfs_close(afd);
  }

  /* Test 3-6: Read back the data written previously and verify that it
     is correct. */
  afd = cfs_open("T2", CFS_READ);
  if(afd < 0) {
    TEST_FAIL(3);
  }
  total_read = 0;
  while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
    for(j = 0; j < r; j++) {
      if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
	TEST_FAIL(4);
      }
    }
    total_read += r;
  }
  if(r < 0) {
    TEST_FAIL(5);
  }
  if(total_read != APPEND_BYTES) {
    TEST_FAIL(6);
  }
  cfs_close(afd);

  error = 0;
end:
  cfs_close(afd);
  cfs_remove("T2");
  return error;
}
/*---------------------------------------------------------------------------*/
static int
coffee_test_modify(void)
{
  int error;
  int wfd;
  unsigned char buf[256];
  int r, i;
  unsigned offset;

  wfd = -1;

  if(cfs_coffee_reserve("T3", FILE_SIZE) < 0) {
    TEST_FAIL(1);
  }

  if(cfs_coffee_configure_log("T3", FILE_SIZE / 2, 11) < 0) {
    TEST_FAIL(2);
  }

  /* Test 16: Test multiple writes at random offset. */
  for(r = 0; r < 100; r++) {
    wfd = cfs_open("T3", CFS_WRITE | CFS_READ);
    if(wfd < 0) {
      TEST_FAIL(3);
    }

    offset = random_rand() % FILE_SIZE;

    for(i = 0; i < sizeof(buf); i++) {
      buf[i] = i;
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      TEST_FAIL(4);
    }

    if(cfs_write(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      TEST_FAIL(5);
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      TEST_FAIL(6);
    }

    memset(buf, 0, sizeof(buf));
    if(cfs_read(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      TEST_FAIL(7);
    }

    for(i = 0; i < sizeof(buf); i++) {
      if(buf[i] != i) {
        printf("buf[%d] != %d\n", i, buf[i]);
        TEST_FAIL(8);
      }
    }

    cfs_close(wfd);
  }

  error = 0;
end:
  cfs_close(wfd);
  cfs_remove("T3");
  return error;
}
/*---------------------------------------------------------------------------*/
static int
coffee_test_gc(void)
{
  int i;

  for (i = 0; i < 100; i++) {
    if (i & 1) {
      if(cfs_coffee_reserve("beta", random_rand() & 0xffff) < 0) {
	return i;
      }
      cfs_remove("alpha");
    } else {
      if(cfs_coffee_reserve("alpha", 93171) < 0) {
	return i;
      }
      cfs_remove("beta");
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
print_result(const char *test_name, int result)
{
  printf("%s: ", test_name);
  if(result == 0) {
    printf("OK\n");
  } else {
    printf("ERROR (test %d)\n", result);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(testcoffee_process, ev, data)
{
  int start;
  int result;

  PROCESS_BEGIN();

  start = clock_seconds();

  printf("Coffee test started\n");

  result = cfs_coffee_format();
  print_result("Formatting", result);

  result = coffee_test_basic();
  print_result("Basic operations", result);

  result = coffee_test_append();
  print_result("File append", result);

  result = coffee_test_modify();
  print_result("File modification", result);

  result = coffee_test_gc();
  print_result("Garbage collection", result);

  printf("Coffee test finished. Duration: %d seconds\n",
         (int)(clock_seconds() - start));

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
