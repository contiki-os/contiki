/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details. 
 *
 *
 */

#include <mc1322x.h>
#include <stdio.h>

#include "put.h"
#include "tests.h"

void print_welcome(char* testname) {
	printf("mc1322x-test: %s\n\r",testname);
	printf("board: %s\n\r", STR2(BOARD));
}

void print_packet(volatile packet_t *p) { 
	volatile uint8_t i,j,k; 
#define PER_ROW 16
	if(p) {
		printf("len 0x%02x lqi 0x%02x rx_time 0x%08x", p->length, p->lqi, (int)p->rx_time);
		for(j=0, k=0; j <= ( (p->length) / PER_ROW ); j++) {
			printf("\n\r");
			for(i=0; i < PER_ROW; i++, k++) {
				if(k >= p->length ) { 
					printf("\n\r");
					return; 
				} 
				printf("%02x ",p->data[j*PER_ROW + i + p->offset]);
			}
		}
	}
	printf("\n\r");
	return; 
}

void dump_regs(uint32_t base, uint32_t len) {
	volatile uint32_t i;
	
	printf("base +0       +4       +8       +c       +10      +14      +18      +1c      \n\r");
	for (i = 0; i < len; i ++) {
		if ((i & 7) == 0) {
			printf("%02lx",(uint32_t)(4 * i));
		}
		printf(" %08lx",(uint32_t)*mem32(base+(4*i)));
		if ((i & 7) == 7)
			printf(NL);
	}
	printf(NL); 
}
