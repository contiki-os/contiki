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

void default_vreg_init(void) {
	volatile uint32_t i;
	*CRM_SYS_CNTL = 0x00000018; /* set default state */
	*CRM_VREG_CNTL = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*CRM_VREG_CNTL = 0x00000ff8; /* start the regulators */
}

void buck_init(void) {
	CRM->SYS_CNTLbits.PWR_SOURCE = 1;
	CRM->VREG_CNTLbits.BUCK_SYNC_REC_EN = 1;
	CRM->VREG_CNTLbits.BUCK_BYPASS_EN = 0;
	CRM->VREG_CNTLbits.BUCK_EN = 1;
	while(CRM->STATUSbits.VREG_BUCK_RDY == 0) { continue; }
	CRM->VREG_CNTLbits.VREG_1P5V_SEL = 3;
	CRM->VREG_CNTLbits.VREG_1P5V_EN = 3;
	CRM->VREG_CNTLbits.VREG_1P8V_EN = 1;
	while(CRM->STATUSbits.VREG_1P5V_RDY == 0) { continue; }
	while(CRM->STATUSbits.VREG_1P8V_RDY == 0) { continue; }
}

void m12_init(void) {
	/* configure pullups for low power */
	GPIO->FUNC_SEL.GPIO_63 = 3;
	GPIO->PAD_PU_SEL.GPIO_63 = 0;
	GPIO->FUNC_SEL.SS = 3;
	GPIO->PAD_PU_SEL.SS = 1;
	GPIO->FUNC_SEL.VREF2H = 3;
	GPIO->PAD_PU_SEL.VREF2H = 1;
	GPIO->FUNC_SEL.U1RTS = 3;
	GPIO->PAD_PU_SEL.U1RTS = 1;
}
