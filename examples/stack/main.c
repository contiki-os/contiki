/*
 * Copyright (c) 2017, University of Bristol - http://www.bris.ac.uk/
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
 */

/**
 * \file
 *         Test of Contiki system's stack checker functionality
 * \author
 *         Atis Elsts <atis.elsts@bristol.ac.uk>
 */

#include "contiki.h"
#include "sys/stack-check.h"
#include "random.h"

#include <stdio.h>
#include <string.h>
#include <alloca.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_process, "Stack check example");
AUTOSTART_PROCESSES(&example_process);
/*---------------------------------------------------------------------------*/
static void
nested_function(void)
{
  printf("stack usage: %u permitted: %u\n",
         stack_check_get_usage(), stack_check_get_reserved_size());
}
/*---------------------------------------------------------------------------*/
static void
test_function(void)
{
  void *p;
  uint16_t s;

  /* allocate and fill some random bytes */
  s = random_rand() % 1000;
  printf("allocating %u bytes on the stack\n", s);
  p = alloca(s);
  memset(p, 0, s);

  /* call the nested function to print stack usage */
  nested_function();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 2);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    test_function();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
