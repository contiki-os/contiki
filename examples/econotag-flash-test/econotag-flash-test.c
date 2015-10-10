/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *      Flash test
 *
 *      This file contains tests for econotag flash app
 *
 * \author
 *      Lars Schmertmann <SmallLars@t-online.de>
 */

#include "flash.h"
#include "contiki.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../../tools/blaster/blaster.h"
#include "econotag-flash-test.h"

void
output_result(uint32_t i, uint32_t fail)
{
  if(fail) {
    printf("  Test %u         failed!\n", i);
  } else { printf("  Test %u succeed!\n", i);
  }
}
void
test_flash_1()
{
  uint8_t buffer[12];
  uint32_t check_int, my_int = 12345678;

  flash_setVar("Hello World!", RES_MY_STRING_1, LEN_MY_STRING_1);

  flash_getVar(buffer, RES_MY_STRING_1, LEN_MY_STRING_1);
  output_result(1, memcmp(buffer, "Hello World!", 12));

  flash_setVar("Heureka!", RES_MY_STRING_2, LEN_MY_STRING_2);

  flash_getVar(buffer, RES_MY_STRING_1, LEN_MY_STRING_1);
  output_result(2, memcmp(buffer, "Hello World!", 12));

  flash_getVar(buffer, RES_MY_STRING_2, LEN_MY_STRING_2);
  output_result(3, memcmp(buffer, "Heureka!", 8));

  flash_setVar(&my_int, RES_MY_INTEGER, LEN_MY_INTEGER);

  flash_getVar(&check_int, RES_MY_INTEGER, LEN_MY_INTEGER);
  output_result(4, check_int != my_int);

  flash_getVar(buffer, RES_MY_STRING_1, LEN_MY_STRING_1);
  output_result(5, memcmp(buffer, "Hello World!", 12));

  flash_getVar(buffer, RES_MY_STRING_2, LEN_MY_STRING_2);
  output_result(6, memcmp(buffer, "Heureka!", 8));
}
void
test_flash_2()
{
  uint8_t buffer[12];
  uint32_t check_int, my_int = 12345678;

  flash_getVar(&check_int, RES_MY_INTEGER, LEN_MY_INTEGER);
  output_result(1, check_int != my_int);

  flash_getVar(buffer, RES_MY_STRING_1, LEN_MY_STRING_1);
  output_result(2, memcmp(buffer, "Hello World!", 12));

  flash_getVar(buffer, RES_MY_STRING_2, LEN_MY_STRING_2);
  output_result(3, memcmp(buffer, "Heureka!", 8));

  /* Block 1 max usage is 30 Byte -> Optimisation in Makefile */
  output_result(4, flash_setVar("test", 0, 1) != gNvmErrInvalidPointer_c);
  output_result(5, flash_setVar("test", 30, 1) != gNvmErrInvalidPointer_c);
  output_result(6, flash_setVar("test", 29, 2) != gNvmErrAddressSpaceOverflow_c);

  /* Block 2 max usage is 10 Byte -> Optimisation in Makefile */
  output_result(7, flash_setVar("test", 4096, 1) != gNvmErrInvalidPointer_c);
  output_result(8, flash_setVar("test", 4096 + 10, 1) != gNvmErrInvalidPointer_c);
  output_result(9, flash_setVar("test", 4096 + 9, 2) != gNvmErrAddressSpaceOverflow_c);
}
void
test_flash_blaster()
{
  uint8_t buffer[64];

  flash_getVar(buffer, RES_NAME, LEN_NAME);
  output_result(1, memcmp(buffer, "Econotag Flash Test Device", 27));

  flash_getVar(buffer, RES_MODEL, LEN_MODEL);
  output_result(2, memcmp(buffer, "Model 1234 for testing purposes only", 37));
}
void
test_flash_stack()
{
  uint8_t buffer[32];
  flash_stack_init();

  output_result(1, flash_stack_size() != 0);

  flash_stack_push("Hello World!", 12);
  output_result(2, flash_stack_size() != 12);

  flash_stack_read(buffer, 0, 12);
  output_result(3, memcmp(buffer, "Hello World!", 12));

  flash_stack_push("I love Contiki!", 15);
  output_result(4, flash_stack_size() != 27);

  flash_stack_read(buffer, 0, 12);
  output_result(5, memcmp(buffer, "Hello World!", 12));

  flash_stack_read(buffer, 12, 15);
  output_result(6, memcmp(buffer, "I love Contiki!", 15));

  flash_stack_init();
  output_result(7, flash_stack_size() != 0);

  uint32_t i;
  for(i = 1; i < 256; i++) {
    flash_stack_push("I love Contiki! ", 16);
  }
  output_result(8, flash_stack_size() != 4080);

  output_result(9, flash_stack_push("1I love Contiki! ", 17) != gNvmErrAddressSpaceOverflow_c);
}
/* Start Process */
PROCESS(server_firmware, "Server Firmware");
AUTOSTART_PROCESSES(&server_firmware);

PROCESS_THREAD(server_firmware, ev, data) {
  PROCESS_BEGIN();

  if(flash_cmp("\001", RES_DONTCLEAR, LEN_DONTCLEAR)) {
    printf("Initializing flash ... ");
    flash_init();
    printf("DONE\n");
    flash_setVar("\001", RES_DONTCLEAR, LEN_DONTCLEAR);
    printf("Starting flash tests 1:\n");
    test_flash_1();
    int i;
    for(i = 0; i < 1024; i++) {
      printf("Reboot ...\r");
    }
    soft_reset();
  } else {
    printf("Initialization not wished\n");
  }
  printf("Starting flash tests 2:\n");
  test_flash_2();

  printf("Starting flash stack tests:\n");
  test_flash_stack();

  printf("Starting flash blaster tests:\n");
  test_flash_blaster();

  PROCESS_END();
}
