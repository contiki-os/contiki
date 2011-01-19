/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *	A test program for the unit test library.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "unit-test.h"

/* Register two unit tests that will be executed by using 
   the UNIT_TEST_RUN macro. */
UNIT_TEST_REGISTER(arithmetic, "Arith ops");
UNIT_TEST_REGISTER(string, "String ops");

/* arithmetic: Demonstrates a test that succeeds. The exit point will be 
   the line where UNIT_TEST_END is called. */
UNIT_TEST(arithmetic)
{
  int a, b;

  UNIT_TEST_BEGIN();

  a = 1;
  b = 2;

  UNIT_TEST_ASSERT(a + b == 3);

  UNIT_TEST_END();
}

/* string: Demonstrates a test that fails. The exit point will be 
   the line where the call to UNIT_TEST_ASSERT fails. */
UNIT_TEST(string)
{
  char str1[] = "A";

  UNIT_TEST_BEGIN();

  UNIT_TEST_ASSERT(str1[0] == 'B');

  UNIT_TEST_END();
}

PROCESS(test_process, "Unit testing");
AUTOSTART_PROCESSES(&test_process);

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();

  UNIT_TEST_RUN(arithmetic);
  UNIT_TEST_RUN(string);

  PROCESS_END();
}
