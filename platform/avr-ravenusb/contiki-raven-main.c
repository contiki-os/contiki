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

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>


#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"
#include <stdbool.h>
#include "mac.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "contiki-raven.h"

#include "usb_task.h"
#include "serial/cdc_task.h"
#include "rndis/rndis_task.h"
#include "storage/storage_task.h"

/*----------------------Configuration of the .elf file---------------------*/
typedef struct {unsigned char B2;unsigned char B1;unsigned char B0;} __signature_t;
#define SIGNATURE __signature_t __signature __attribute__((section (".signature")))
SIGNATURE = {
/* Older AVR-GCCs may not define the SIGNATURE_n bytes so use explicit values */
  .B2 = 0x82,//SIGNATURE_2, //AT90USB128x
  .B1 = 0x97,//SIGNATURE_1, //128KB flash
  .B0 = 0x1E,//SIGNATURE_0, //Atmel
};
FUSES ={.low = 0xde, .high = 0x99, .extended = 0xff,};

/* Put default MAC address in EEPROM */
uint8_t mac_address[8] EEMEM = {0x02, 0x12, 0x13, 0xff, 0xfe, 0x14, 0x15, 0x16};
//uint8_t EEMEM mac_address[8];     //The raven webserver uses this EEMEM allocation
//uint8_t EEMEM server_name[16];
//uint8_t EEMEM domain_name[30];

PROCINIT(&etimer_process, &mac_process);

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

  printf_P(PSTR("\n\n\n********BOOTING CONTIKI*********\n"));

  /* Process subsystem */
  process_init();

  /* Register initial processes */
  procinit_init();

  /* Setup USB */
  process_start(&usb_process, NULL);
  process_start(&cdc_process, NULL);
  process_start(&rndis_process, NULL);
  process_start(&storage_process, NULL);

  printf_P(PSTR("System online.\n"));

  //Fix MAC address
  init_net();

   /* Main scheduler loop */
  while(1) {
    process_run();
  }

  return 0;
}





