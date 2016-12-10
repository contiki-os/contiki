/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
/* This is the main contiki header, it should be included always */
#include "contiki.h"

/* This is the standard input/output C library, used to print information to the
 * console, amongst other features
 */
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/* We first declare the name or our process, and create a friendly printable
 * name
 */
PROCESS(hello_world_process, "Hello world process");

/* And we tell the system to start this process as soon as the device is done
 * initializing
 */
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
/* We declared the process in the above step, now we are implementing the
 * process.  A process communicates with other processes and, thats why we
 * include as arguments "ev" (event type) and "data" (information associated
 * with the event, so when we receive a message from another process, we can
 * check which type of event is, and depending of the type what data should I
 * receive
 */
PROCESS_THREAD(hello_world_process, ev, data)
{
  /* Every process start with this macro, we tell the system this is the start
   * of the thread
   */
  PROCESS_BEGIN();

  static uint16_t num = 0xABCD;
  static const char *hello = "Hello world, again!";

  /* Now we are printing this message to the console over the USB port, use
   * "make login" and hit the reset button to catch this message, as well as the
   * booting information of the node
   */
  printf("Hello, world\n");

  /* We might as well also use */
  printf("%s\n", hello);

  /* And mix numeric values with strings */
  printf("This is a value in hex 0x%02X, the same as %u\n", num, num);
  
  /* This is the end of the process, we tell the system we are done */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
