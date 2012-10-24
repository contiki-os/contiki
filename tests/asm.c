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
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

void asm_isr(void) {
	printf("asm isr\n\r");
	ASM->CONTROL0bits.CLEAR_IRQ = 1;
}


void main(void) {
	volatile int i;

	/* trim the reference osc. to 24MHz */
	trim_xtal();

	uart_init(UART1, 115200);

	vreg_init();

	enable_irq(ASM);

	print_welcome("asm");

	printf("ASM Control 0: %08x\n\r", (unsigned int)ASM->CONTROL0);
	printf("ASM Control 1: %08x\n\r", (unsigned int)ASM->CONTROL1);
	printf("ASM Status: %08x\n\r", (unsigned int)ASM->STATUS);
	printf("ASM Test pass: %d\n\r", ASM->STATUSbits.TEST_PASS);

	/* ASM module is disabled until self-test passes */
	printf("\n\r*** ASM self-test ***\n\r");
	
	ASM->CONTROL1bits.ON = 1;
	ASM->CONTROL1bits.SELF_TEST = 1;
	ASM->CONTROL0bits.START = 1;

	/* Self test takes 3330 periph. clocks (default 24Mhz) */
	/* to complete. This doesn't wait 3330 exactly, but should be long enough */

	for(i = 0; i < 3330; i++) { continue; }

	printf("ASM Test pass: %d\n\r", ASM->STATUSbits.TEST_PASS);

	/* must clear the self test bit when done */
	ASM->CONTROL1bits.SELF_TEST = 0;

	
	/* ASM starts in "BOOT" mode which uses an internal secret key
	 * to load encrypted data from an external source */
	/* must set to NORMAL mode */
	ASM->CONTROL1bits.NORMAL_MODE = 1;

	/* setting the bypass bit will disable the encryption */
	/* bypass defaults to off */
	ASM->CONTROL1bits.BYPASS = 0;

	printf("\n\r*** set ASM key ***\n\r");
	ASM->KEY0 = 0xccddeeff;
	ASM->KEY1 = 0x8899aabb;
	ASM->KEY2 = 0x44556677;
	ASM->KEY3 = 0x00112233;

	/* KEY registers appear to be write-only (which is a good thing) */
	/* even though the datasheet says you can read them */
	printf("ASM Key [3,2,1,0]       : 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->KEY3, 
	       (unsigned int) ASM->KEY2, 
	       (unsigned int) ASM->KEY1, 
	       (unsigned int) ASM->KEY0); 
		
	printf("\n\r*** CTR test ***\n\r");
	printf("Encrypt\n\r");
	ASM->CONTROL1bits.CTR = 1;

	ASM->DATA0 = 0xdeaddead;
	ASM->DATA1 = 0xbeefbeef;
	ASM->DATA2 = 0xfaceface;
	ASM->DATA3 = 0x01234567;
	
	printf("ASM Data [3,2,1,0]      : 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->DATA3, 
	       (unsigned int) ASM->DATA2, 
	       (unsigned int) ASM->DATA1, 
	       (unsigned int) ASM->DATA0); 

	ASM->CTR0 = 0x33333333;
	ASM->CTR1 = 0x22222222;
	ASM->CTR2 = 0x11111111;
	ASM->CTR3 = 0x00000000;
	
	printf("ASM CTR [3,2,1,0]       : 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CTR3, 
	       (unsigned int) ASM->CTR2, 
	       (unsigned int) ASM->CTR1, 
	       (unsigned int) ASM->CTR0);

	ASM->CONTROL0bits.START = 1;
	while(ASM->STATUSbits.DONE == 0) { continue; }

	printf("ASM CTR RESULT [3,2,1,0]: 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CTR3_RESULT, 
	       (unsigned int) ASM->CTR2_RESULT, 
	       (unsigned int) ASM->CTR1_RESULT, 
	       (unsigned int) ASM->CTR0_RESULT);

	printf("Decrypt\n\r");

	ASM->DATA0 = ASM->CTR0_RESULT;
	ASM->DATA1 = ASM->CTR1_RESULT;
	ASM->DATA2 = ASM->CTR2_RESULT;
	ASM->DATA3 = ASM->CTR3_RESULT;

	ASM->CONTROL0bits.START = 1;
	while(ASM->STATUSbits.DONE == 0) { continue; }

	printf("ASM CTR RESULT [3,2,1,0]: 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CTR3_RESULT, 
	       (unsigned int) ASM->CTR2_RESULT, 
	       (unsigned int) ASM->CTR1_RESULT, 
	       (unsigned int) ASM->CTR0_RESULT);

	printf("\n\r*** CBC MAC generation ***\n\r");

	ASM->CONTROL1bits.CTR = 0;
	ASM->CONTROL1bits.CBC = 1;

	/* CBC is like a hash */
	/* it doesn't use the CTR data */
	/* the accumulated MAC is in the MAC registers */
	/* you must use the CLEAR bit to reset the MAC state */

	ASM->DATA0 = 0xdeaddead;
	ASM->DATA1 = 0xbeefbeef;
	ASM->DATA2 = 0xfaceface;
	ASM->DATA3 = 0x01234567;

	ASM->CONTROL0bits.CLEAR  = 1;

	ASM->CONTROL0bits.START = 1;
	while(ASM->STATUSbits.DONE == 0) { continue; }

	printf("ASM CBC RESULT [3,2,1,0]: 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CBC3_RESULT, 
	       (unsigned int) ASM->CBC2_RESULT, 
	       (unsigned int) ASM->CBC1_RESULT, 
	       (unsigned int) ASM->CBC0_RESULT);

	printf("\n\r*** CCM (CTR+CBC) ***\n\r");

	ASM->CONTROL1bits.CTR = 1;
	ASM->CONTROL1bits.CBC = 1;
	ASM->CONTROL0bits.CLEAR  = 1;

	ASM->CONTROL0bits.START = 1;
	while(ASM->STATUSbits.DONE == 0) { continue; }

	printf("ASM CTR RESULT [3,2,1,0]: 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CTR3_RESULT, 
	       (unsigned int) ASM->CTR2_RESULT, 
	       (unsigned int) ASM->CTR1_RESULT, 
	       (unsigned int) ASM->CTR0_RESULT);

	printf("ASM CBC RESULT [3,2,1,0]: 0x%08x%08x%08x%08x\n",
	       (unsigned int) ASM->CBC3_RESULT, 
	       (unsigned int) ASM->CBC2_RESULT, 
	       (unsigned int) ASM->CBC1_RESULT, 
	       (unsigned int) ASM->CBC0_RESULT);

	while(1) {		
	}
}
