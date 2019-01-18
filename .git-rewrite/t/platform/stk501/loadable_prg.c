/*
 * Copyright (c) 2006, Technical University of Munich
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
 * @(#)$$
 *
 */

/**
 * \file
 *       Sample loadable module
 *
 * \author
 *       Simon Barner <barner@in.tum.de>
 */
#include <stdio.h>
#include "dev/rs232.h"
#include "contiki.h"

PROCESS(test_process1, "Test process");
PROCESS_THREAD(test_process1, ev, data)
{
  static struct etimer etimer;

  PROCESS_BEGIN();

  rs232_print (RS232_PORT_1, "test_process 1 starting\n");

  while(1) {
    etimer_set(&etimer, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    rs232_print (RS232_PORT_1, "Tick\n");
    etimer_set(&etimer, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    rs232_print (RS232_PORT_1, "Tack\n");
  }

  PROCESS_END();
}

const struct process *autostart_processes[] = {&test_process1};
