/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: bus.c,v 1.2 2010/01/25 23:12:09 anthony-a Exp $
 */

/**
 * \file
 *         Initialization functions for the 8051 bus
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "banked.h"
#include "cc2430_sfr.h"
#include "dev/bus.h"
#include "sys/clock.h"

/*---------------------------------------------------------------------------*/
void
bus_init (void) __banked
{
  CLKCON = (0x00 | OSC32K); 			/* 32k internal */
  while(CLKCON != (0x00 | OSC32K));

  P1DIR |= 0x0E;
  IEN0_EA = 1;

  /* Initialize the clock */
  clock_init();
}
/*---------------------------------------------------------------------------*/
/**
 * Read a block of code memory.
 * The code must be placed in the lowest bank of flash.
 *
 * \param address address to read from flash
 * \param buffer  buffer to store data
 * \param size    number of bytes to read
 */
void
flash_read (uint8_t *buf, uint32_t address, uint8_t size) __banked
{
  buf;	 	/*dptr0*/
  address; 	/*stack-6*/
  size;		/*stack-7*/
	
  buf;
  
  DISABLE_INTERRUPTS();
  __asm
			mov dpl, r2
			mov dph, r3
			mov a, r0
			push acc
			mov a, r2
			push acc
			mov a, _MEMCTR
			push acc

			mov a, _bp
			add a, #0xf9 		;stack - 7 = size
			mov r0,a
			mov a, @r0  		;r2 = size
			mov r2, a   		;r2 = size

			inc r0
			mov a, @r0
			mov _DPL1, a		;DPTR1 = address & 0x7FFF | 0x8000
			inc r0
			mov a, @r0
			orl a, #0x80
			mov _DPH1, a
			inc r0					;MEMCTR = ((address >> 15 & 3) << 4) | 0x01 (bank select)
			mov a, @r0
			dec r0
			rrc a
			mov a, @r0
			rrc a
			rr a
			rr a
			anl a, #0x30
			orl a, #1
			mov _MEMCTR,a
lp1:
			mov _DPS, #1		;active DPTR = 1
			clr a
			movc a, @a+dptr			;read flash (DPTR1)
			inc dptr
			mov _DPS, #0 				;active DPTR = 0
			movx @dptr,a				;write to DPTR0
			inc dptr
			djnz r2,lp1					;while (--size)

			pop acc
			mov _MEMCTR, a	;restore bank

			pop acc
			mov r2,a
			pop acc
			mov r0,a
  __endasm;
  ENABLE_INTERRUPTS();
  DPL1 = *buf++;
}
/*---------------------------------------------------------------------------*/
