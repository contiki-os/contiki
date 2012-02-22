/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 *  Coffee architecture-dependent functionality for the STM32W108-based mb851
 *  platform.
 * \author
 *  Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */


#include "cfs-coffee-arch.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define TESTCOFFEE 1
#define DEBUG_CFS 1
#if TESTCOFFEE
#if DEBUG_CFS
#include <stdio.h>
#define PRINTF_CFS(...) printf(__VA_ARGS__)
#else
#define PRINTF_CFS(...)
#endif

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"
#include <stdio.h>

#define FAIL(x) PRINTF("FAILED\n");error = (x); goto end;

#define FILE_SIZE 512

int
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
  cfs_remove("T4");
  cfs_remove("T5");

  wfd = rfd = afd = -1;

  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = r;
  }
  
  PRINTF("TEST 1\n");

  /* Test 1: Open for writing. */
  wfd = cfs_open("T1", CFS_WRITE);
  if(wfd < 0) {
    FAIL(-1);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("2\n");

  /* Test 2: Write buffer. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-2);
  } else if(r < sizeof(buf)) {
    FAIL(-3);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("3\n");

  /* Test 3: Deny reading. */
  r = cfs_read(wfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-4);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("4\n");

  /* Test 4: Open for reading. */
  rfd = cfs_open("T1", CFS_READ);
  if(rfd < 0) {
    FAIL(-5);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("5\n");

  /* Test 5: Write to read-only file. */
  r = cfs_write(rfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-6);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("7\n");

  /* Test 7: Read the buffer written in Test 2. */
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-8);
  } else if(r < sizeof(buf)) {
    PRINTF_CFS("r=%d\n", r);
    FAIL(-9);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("8\n");

  /* Test 8: Verify that the buffer is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      PRINTF_CFS("r=%d. buf[r]=%d\n", r, buf[r]);
      FAIL(-10);
    }
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("9\n");

  /* Test 9: Seek to beginning. */
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-11);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("10\n");

  /* Test 10: Write to the log. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-12);
  } else if(r < sizeof(buf)) {
    FAIL(-13);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("11\n");

  /* Test 11: Read the data from the log. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-14);
  } else if(r < sizeof(buf)) {
    FAIL(-15);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("12\n");

  /* Test 12: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      FAIL(-16);
    }
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("13\n");

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
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("14\n");

  /* Test 14: Read the reversed buffer. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-21);
  } else if(r < sizeof(buf)) {
    PRINTF_CFS("r = %d\n", r);
    FAIL(-22);
  }
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("15\n");

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
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("16\n");

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
        PRINTF_CFS("buf[%d] != %d\n", i, buf[i]);
        FAIL(-30);
      }
    }
  }
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("17\n");
  /* Test 17: Append data to the same file many times. */
#define APPEND_BYTES 3000
#define BULK_SIZE 10
  for (i = 0; i < APPEND_BYTES; i += BULK_SIZE) {
    afd = cfs_open("T3", CFS_WRITE | CFS_APPEND);
    if (afd < 0) {
      FAIL(-31);
    }
    for (j = 0; j < BULK_SIZE; j++) {
      buf[j] = 1 + ((i + j) & 0x7f);
    }
    if ((r = cfs_write(afd, buf, BULK_SIZE)) != BULK_SIZE) {
      PRINTF_CFS("Count:%d, r=%d\n", i, r);
      FAIL(-32);
    }
    cfs_close(afd);
  }
	
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("18\n");

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
	  PRINTF_CFS("FAIL:-35 r=%d\n",r);
    FAIL(-35);
  }
  if(total_read != APPEND_BYTES) {
	  PRINTF_CFS("FAIL:-35 total_read=%d\n",total_read);
    FAIL(-35);
  }
  cfs_close(afd);
  
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("19\n");

/***************T4********************/
/* file T4 and T5 writing forces to use garbage collector in greedy mode
 * this test is designed for 10kb of file system
 * */
#define APPEND_BYTES_1 2000
#define BULK_SIZE_1 10
  for (i = 0; i < APPEND_BYTES_1; i += BULK_SIZE_1) {
    afd = cfs_open("T4", CFS_WRITE | CFS_APPEND);
    if (afd < 0) {
     FAIL(-36);
    }
    for (j = 0; j < BULK_SIZE_1; j++) {
      buf[j] = 1 + ((i + j) & 0x7f);
    }

   if ((r = cfs_write(afd, buf, BULK_SIZE_1)) != BULK_SIZE_1) {
     PRINTF_CFS("Count:%d, r=%d\n", i, r);
     FAIL(-37);
   }
   cfs_close(afd);
   }

  afd = cfs_open("T4", CFS_READ);
  if(afd < 0) {
    FAIL(-38);
  }
  total_read = 0;
  while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
    for(j = 0; j < r; j++) {
      if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
    	  PRINTF_CFS("FAIL:-39, total_read=%d r=%d\n",total_read,r);
  FAIL(-39);
      }
    }
    total_read += r;
  }
  if(r < 0) {
	  PRINTF_CFS("FAIL:-40 r=%d\n",r);
    FAIL(-40);
  }
  if(total_read != APPEND_BYTES_1) {
	  PRINTF_CFS("FAIL:-41 total_read=%d\n",total_read);
    FAIL(-41);
  }
  cfs_close(afd);
  /***************T5********************/
  PRINTF("PASSED\n");
  PRINTF("TEST ");
  PRINTF("20\n");
#define APPEND_BYTES_2 1000
#define BULK_SIZE_2 10
    for (i = 0; i < APPEND_BYTES_2; i += BULK_SIZE_2) {
      afd = cfs_open("T5", CFS_WRITE | CFS_APPEND);
      if (afd < 0) {
        FAIL(-42);
      }
      for (j = 0; j < BULK_SIZE_2; j++) {
        buf[j] = 1 + ((i + j) & 0x7f);
      }

      if ((r = cfs_write(afd, buf, BULK_SIZE_2)) != BULK_SIZE_2) {
        PRINTF_CFS("Count:%d, r=%d\n", i, r);
        FAIL(-43);
      }

      cfs_close(afd);
    }

    afd = cfs_open("T5", CFS_READ);
    if(afd < 0) {
      FAIL(-44);
    }
    total_read = 0;
    while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
      for(j = 0; j < r; j++) {
        if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
      	  PRINTF_CFS("FAIL:-45, total_read=%d r=%d\n",total_read,r);
    FAIL(-45);
        }
      }
      total_read += r;
    }
    if(r < 0) {
  	  PRINTF_CFS("FAIL:-46 r=%d\n",r);
      FAIL(-46);
    }
    if(total_read != APPEND_BYTES_2) {
  	  PRINTF_CFS("FAIL:-47 total_read=%d\n",total_read);
      FAIL(-47);
    }
    cfs_close(afd);
    
    PRINTF("PASSED\n");

  error = 0;
end:
  cfs_close(wfd); cfs_close(rfd); cfs_close(afd);
  return error;
}
#endif /* TESTCOFFEE */

void stm32w_flash_read(int32u address, void * data, int32u length)
{
  int8u	* pdata = (int8u *)address;
  ENERGEST_ON(ENERGEST_TYPE_FLASH_READ);
  memcpy(data, pdata, length);
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_READ);
}

void stm32w_flash_erase(int8u sector)
{
  //halInternalFlashErase(MFB_PAGE_ERASE, COFFEE_START + (sector) * COFFEE_SECTOR_SIZE);
  
  int16u data = 0;
  int32u addr = COFFEE_START + (sector) * COFFEE_SECTOR_SIZE;
  int32u end = addr + COFFEE_SECTOR_SIZE;
  
  /* This prevents from accidental write to CIB. */
  if (!(addr >= MFB_BOTTOM && end <= MFB_TOP + 1)) {
    return;
  }
  
  for(; addr < end; addr += 2){
    halInternalFlashWrite(addr, &data, 1);
  }
}

// Allocates a buffer of FLASH_PAGE_SIZE bytes statically (rather than on the stack).
#ifndef STATIC_FLASH_BUFFER
#define STATIC_FLASH_BUFFER 1
#endif

void stm32w_flash_write(int32u address, const void * data, int32u length)
{
  const int32u end = address + length;
  int32u i;
  int32u next_page, curr_page;
  int16u offset;
  
#if STATIC_FLASH_BUFFER
  static int8u buf[FLASH_PAGE_SIZE];
#else
  int8u buf[FLASH_PAGE_SIZE];
#endif
  
  for(i = address; i < end;) {
    next_page = (i | (FLASH_PAGE_SIZE-1)) + 1;
    curr_page = i & ~(FLASH_PAGE_SIZE-1);
    offset = i-curr_page;
    if(next_page > end) {
      next_page = end;
    }
    
    // Read a page from flash and put it into a mirror buffer.
    stm32w_flash_read(curr_page, buf, FLASH_PAGE_SIZE);
    // Update flash mirror data with new data.
    memcpy(buf + offset, data, next_page - i);
    // Erase flash page.    
    ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
    halInternalFlashErase(MFB_PAGE_ERASE, i);
    // Write modified data form mirror buffer into the flash.   
    halInternalFlashWrite(curr_page, (int16u *)buf, FLASH_PAGE_SIZE/2);
    ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
    
    data = (uint8_t *)data + next_page - i;
    i = next_page;
  }
  
}

