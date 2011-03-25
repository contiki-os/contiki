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

#ifndef ASM_H
#define ASM_H

/* Structure-based register definitions */
/* Example use:
        ASM->KEY0 = 0xaabbccdd;
	ASM->CONTROL1bits = (struct ASM_CONTROL1) {
	         .MASK_IRQ = 1,
		 .CBC = 1,
	};
	ASM->CONTROL1bits.SELF_TEST = 1;
*/

struct ASM_struct {
	uint32_t KEY0;
	uint32_t KEY1;
	uint32_t KEY2;
	uint32_t KEY3;
	uint32_t DATA0;
	uint32_t DATA1;
	uint32_t DATA2;
	uint32_t DATA3;
	uint32_t CTR0;
	uint32_t CTR1;
	uint32_t CTR2;
	uint32_t CTR3;
	uint32_t CTR0_RESULT;
	uint32_t CTR1_RESULT;
	uint32_t CTR2_RESULT;
	uint32_t CTR3_RESULT;
	uint32_t CBC0_RESULT;
	uint32_t CBC1_RESULT;
	uint32_t CBC2_RESULT;
	uint32_t CBC3_RESULT;

	union {
		uint32_t CONTROL0;
		struct ASM_CONTROL0 {
		        uint32_t :24;
			uint32_t START:1;
			uint32_t CLEAR:1;
			uint32_t LOAD_MAC:1;
       		        uint32_t :4;
			uint32_t CLEAR_IRQ:1;
		} CONTROL0bits;
	};
	union {
		uint32_t CONTROL1;
		struct ASM_CONTROL1 {
			uint32_t ON:1;
			uint32_t NORMAL_MODE:1;
			uint32_t BYPASS:1;
		        uint32_t :21;
			uint32_t CBC:1;
			uint32_t CTR:1;
			uint32_t SELF_TEST:1;
		        uint32_t :4;
			uint32_t MASK_IRQ:1;
		} CONTROL1bits;
	};
	union {
		uint32_t STATUS;
		struct ASM_STATUS {
   		        uint32_t :24;
			uint32_t DONE:1;
			uint32_t TEST_PASS:1;
		        uint32_t :6;
		} STATUSbits;
	};

	uint32_t reserved;

	uint32_t MAC0;
	uint32_t MAC1;
	uint32_t MAC2;
	uint32_t MAC3;
};

static volatile struct ASM_struct * const ASM = (void *) (0x80008000);

#endif
