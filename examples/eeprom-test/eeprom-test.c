/*
 * Copyright (c) 2013, Robert Quattlebaum
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
 *         A very simple Contiki application showing how to read and write
 *         data using Contiki's EEPROM API.
 * \author
 *         Robert Quattlebaum <darco@deepdarc.com>
 */

#include "contiki.h"
#include "dev/eeprom.h"

#include <stdio.h>              /* For printf() */

void
print_content()
{
  eeprom_addr_t addr_row = 0, j;
  uint8_t i;
  uint8_t byte;

  printf("\t");
  for(i = 0; i < 16; i++)
    printf("0x%x\t", i);
  printf
    ("\n-----------------------------------------------------------------------------------------------------------------------------------------\n");

  for(addr_row = 0; addr_row < EEPROM_SIZE / 16; ++addr_row) {
    printf("0x%x\t|", addr_row * 16);

    for(j = 0; j < 16; j++) {
      eeprom_read(addr_row * 16 + j, &byte, 1);
      printf("0x%x\t", byte);
    }
    printf("\n");
  }
}

void
erase_content()
{
  static eeprom_addr_t addr = 0;

  for(addr = 0; addr < EEPROM_SIZE; ++addr) {
    eeprom_write(addr, 0, 1);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS(eeprom_test_process, "EEPROM Test Process");
AUTOSTART_PROCESSES(&eeprom_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(eeprom_test_process, ev, data)
{
  static uint8_t counter = 0, error = 0;
  static eeprom_addr_t addr = 0;
  uint8_t byte;

  uint8_t buffer[] =
    { 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB, 0xCC, 0xCC, 0xCC, 0xBB,
    0xBB, 0xBB, 0xAA, 0xAA, 0xAA, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB,
    0xBB, 0xBB, 0xCC, 0xCC, 0xCC, 0xBB, 0xBB, 0xBB, 0xAA, 0xAA,
    0xAA, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB, 0xCC, 0xCC,
    0xCC, 0xBB, 0xBB, 0xBB, 0xAA, 0xAA, 0xAA, 0xFF
  };

  PROCESS_BEGIN();

  printf("eeprom-test: Size = %d bytes\n", EEPROM_SIZE);

  print_content();

  printf("\nErase EEPROM content\n");
  erase_content();

  print_content();

  counter = 0;
  for(addr = 0; addr < EEPROM_SIZE; ++addr) {
    eeprom_write(addr, &counter, 1);
    counter += 1;
  }

  counter = 0;
  for(addr = 0; addr < EEPROM_SIZE; ++addr) {
    byte = 0;
    eeprom_read(addr, &byte, 1);
    if(byte != counter) {
      error++;
      eeprom_read(addr, &byte, 1);
      printf
        ("eeprom-test: EEPROM write failure! 0x%x =/= 0x%x at address 0x%x\n",
         byte, counter, addr);
      break;
    }
    counter += 1;
  }

  if(error)
    printf("eeprom-test: EEPROM write test FAIL!\n%d errors", error);
  else
    printf("eeprom-test: EEPROM write test success!\n");


  print_content();

  printf("Fill memory with buffer\n");

  for(addr = 0; addr < EEPROM_SIZE; addr += sizeof(buffer)) {
    eeprom_write(addr, ((unsigned char *)buffer), sizeof(buffer));
  }
/*
  printf("Write data buffer %d at address 0x0\n", sizeof(buffer));
  eeprom_write(0x0, ((unsigned char *)buffer), sizeof(buffer));

  printf("Write data buffer %d at address 0x40\n", sizeof(buffer));
  eeprom_write(0x40, ((unsigned char *)buffer), sizeof(buffer));

  printf("Write data buffer %d at address 0x95\n", sizeof(buffer));
  eeprom_write(0x95, ((unsigned char *)buffer), sizeof(buffer));
*/
  print_content();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
