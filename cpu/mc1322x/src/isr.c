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
#include <stdint.h>

static void (*tmr_isr_funcs[4])(void) = {
	tmr0_isr,
	tmr1_isr,
	tmr2_isr,
	tmr3_isr
};

void irq_register_timer_handler(int timer, void (*isr)(void))
{
	tmr_isr_funcs[timer] = isr;
}


__attribute__ ((section (".irq")))
__attribute__ ((interrupt("IRQ"))) 
void irq(void)
{
	uint32_t pending;

	while ((pending = *NIPEND)) {
		
		if(bit_is_set(pending, INT_NUM_TMR)) { 
			/* dispatch to individual timer isrs if they exist */
			/* timer isrs are responsible for determining if they
			 * caused an interrupt */
			/* and clearing their own interrupt flags */
			if (tmr_isr_funcs[0] != 0) { (tmr_isr_funcs[0])(); }
			if (tmr_isr_funcs[1] != 0) { (tmr_isr_funcs[1])(); }
			if (tmr_isr_funcs[2] != 0) { (tmr_isr_funcs[2])(); }
			if (tmr_isr_funcs[3] != 0) { (tmr_isr_funcs[3])(); }
		}

		if(bit_is_set(pending, INT_NUM_MACA)) {
	 		if(maca_isr != 0) { maca_isr(); } 
		}
		if(bit_is_set(pending, INT_NUM_UART1)) {
	 		if(uart1_isr != 0) { uart1_isr(); } 
		}
		if(bit_is_set(pending, INT_NUM_UART2)) {
	 		if(uart2_isr != 0) { uart2_isr(); } 
		}
		if(bit_is_set(pending, INT_NUM_CRM)) {
			if(rtc_wu_evt() && (rtc_isr != 0)) { rtc_isr(); }
			if(kbi_evnt(4) && (kbi4_isr != 0)) { kbi4_isr(); }
			if(kbi_evnt(5) && (kbi5_isr != 0)) { kbi5_isr(); }
			if(kbi_evnt(6) && (kbi6_isr != 0)) { kbi6_isr(); }
			if(kbi_evnt(7) && (kbi7_isr != 0)) { kbi7_isr(); }

			if (CRM->STATUSbits.CAL_DONE && CRM->CAL_CNTLbits.CAL_IEN && cal_isr)
			{
				CRM->STATUSbits.CAL_DONE = 0;
				cal_isr();
			}
		}
		if(bit_is_set(pending, INT_NUM_ASM)) {
			if(asm_isr != 0) { asm_isr(); }
		}
		if (bit_is_set(pending, INT_NUM_I2C)) {
			if (i2c_isr != 0) { i2c_isr(); }
		}

		*INTFRC = 0; /* stop forcing interrupts */

	}	
}
