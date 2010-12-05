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
 */

/**
 * \file
 *         Sample Contiki kernel for STK 501 development board
 *
 * \author
 *         Simon Barner <barner@in.tum.de
 */

 /* Patched to allow hello-world ipv4 and ipv6 build */

#include <avr/pgmspace.h>
#include <stdio.h>
#include "net/uip_arp.h"

#include "contiki-stk501.h"
//#include "../core/cfs/cfs-eeprom.h"
#include "cfs/cfs.h"
#include "dev/eeprom.h"
#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"
#include "../apps/codeprop/codeprop.h"
#include "sys/mt.h"

/* Uncomment to enable demonstration of multi-threading libary */
/* #define MT_DEMO */

//TODO: What happened to cfs_eeprom_process?
//PROCINIT(&etimer_process, &tcpip_process, &uip_fw_process, &cfs_eeprom_process);
#if UIP_CONF_IPV6
PROCINIT(&etimer_process, &tcpip_process);
#else
PROCINIT(&etimer_process, &tcpip_process, &uip_fw_process);
#endif

#ifdef MT_DEMO
static struct mt_thread threads[3];

static
void thread_handler1 (void* data) {
  while (1) {
    rs232_print_p (RS232_PORT_1, PSTR ("Thread 1. Data: ") );
    rs232_printf (RS232_PORT_1, "0x%x, %d\n", data, *(uint8_t*)data );
    mt_yield ();
  }
}

static
void thread_handler2 (void* data) {
  while (1) {
    rs232_print_p (RS232_PORT_1, PSTR ("Thread 2. Data: "));
    rs232_printf (RS232_PORT_1, "0x%x, %d\n", data, *(uint8_t*)data );
    mt_yield ();
  }
}
#endif

PROCESS(contiki_stk501_main_init_process, "Contiki STK501 init process");
PROCESS_THREAD(contiki_stk501_main_init_process, ev, data)
{
  PROCESS_BEGIN();

  /* Network support (uIP) */
  init_net();

  /* Initalize heap allocator */
  mmem_init ();

  /* Code propagator */
  /* TODO: The core elfloader-avr.c has 16/32 bit pointer problems so this won't build */
//process_start(&codeprop_process, NULL);

  /* Multi-threading support */
#ifdef MT_DEMO
  mt_init ();
#endif

  PROCESS_END();
}

#ifdef MT_DEMO
static uint8_t d1=1, d2=2, d3=3;
#endif

int
main(void)
{
  /*
   * GCC depends on register r1 set to 0.
   */
  asm volatile ("clr r1");

  /* Initialize hardware */
  init_lowlevel();

  /* Clock */
  clock_init();

  /* Process subsystem */
  process_init();

  /* Register initial processes */
  procinit_init();

  /* Perform rest of initializations */
  process_start(&contiki_stk501_main_init_process, NULL);

  rs232_print_p (RS232_PORT_1, PSTR ("Initialized.\n"));

#ifdef MT_DEMO
  mt_start (&threads[0], thread_handler1, &d1);
  mt_start (&threads[1], thread_handler2, &d2);
  mt_start (&threads[2], thread_handler2, &d3);

  uint8_t i;
#endif

  /* Main scheduler loop */
  while(1) {

    process_run();

#ifdef MT_DEMO
    for (i=0; i<3; ++i) {
      mt_exec (&threads[i]);
    }
#endif
  }

  return 0;
}
