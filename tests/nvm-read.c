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

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[READ_NBYTES/4];
	uint32_t i;

	uart_init(UART1, 115200);

	print_welcome("nvm-read");

	vreg_init();
//	buck_init();
	while(CRM->STATUSbits.VREG_1P5V_RDY == 0) { continue; }
	while(CRM->STATUSbits.VREG_1P8V_RDY == 0) { continue; }

	printf("Sys cntl %08x\n\r",  (unsigned int)CRM->SYS_CNTL);
	printf("vreg cntl %08x\n\r", (unsigned int)CRM->VREG_CNTL);
	printf("crm status %08x\n\r", (unsigned int)CRM->STATUS);

	if(NVM_INTERFACE == gNvmInternalInterface_c)
	{
		printf("Detecting internal nvm\n\r");
	} else {
		printf("Setting up gpio\r\n");
		/* set SPI func */
		GPIO->FUNC_SEL.GPIO_04 = 1;
		GPIO->FUNC_SEL.GPIO_05 = 1;
		GPIO->FUNC_SEL.GPIO_06 = 1;
		GPIO->FUNC_SEL.GPIO_07 = 1;
		printf("Detecting external nvm\n\r");
	}

	err = nvm_detect(NVM_INTERFACE, &type);
		
	printf("nvm_detect returned: 0x%02x type is: 0x%08x\r\n", err, (unsigned int)type);

	nvm_setsvar(0);

	err = nvm_read(NVM_INTERFACE, type, (uint8_t *)buf, READ_ADDR, READ_NBYTES);
	printf("nvm_read returned: 0x%02x\r\n", err);

	for(i=0; i<16/4; i++) {
		printf("0x%08x\r\n", (unsigned int)buf[i]);
	}
		

	while(1) {continue;};
}

