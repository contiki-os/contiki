/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit@wsystem.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Example demonstrating how to use the FAT file system.
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "ff.h"
/*---------------------------------------------------------------------------*/
PROCESS(example_fat_process, "FAT example");
AUTOSTART_PROCESSES(&example_fat_process);
/*---------------------------------------------------------------------------*/
#define TEST_FILENAME   "test.txt"
#define TEST_LINE       "Hello world!"
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_fat_process, ev, data)
{
  static FATFS FatFs; /* Work area (file system object) for logical drive */
  FIL fil;            /* File object */
  char line[82];      /* Line buffer */
  FRESULT fr;         /* FatFs return code */

  PROCESS_BEGIN();

  printf("FAT example\n");

  /* Register work area to the default drive */
  f_mount(&FatFs, "", 0);

  printf("Writing \"%s\" to \"%s\"\n", TEST_LINE, TEST_FILENAME);

  /* Create the test file */
  fr = f_open(&fil, TEST_FILENAME, FA_WRITE | FA_CREATE_ALWAYS);
  if(fr) {
    printf("f_open() error: %d\n", fr);
    PROCESS_EXIT();
  }

  /* Write the test line */
  f_printf(&fil, "%s\n", TEST_LINE);

  /* Close the file */
  f_close(&fil);

  printf("Reading back \"%s\":\n\n", TEST_FILENAME);

  /* Open the test file */
  fr = f_open(&fil, TEST_FILENAME, FA_READ);
  if(fr) {
    printf("f_open() error: %d\n", fr);
    PROCESS_EXIT();
  }

  /* Read all the lines and display them */
  while(f_gets(line, sizeof(line), &fil)) {
    printf(line);
  }

  /* Close the file */
  f_close(&fil);

  printf("\nDone\n");

  PROCESS_END();
}
