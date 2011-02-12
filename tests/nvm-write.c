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

#include "tests.h"
#include "config.h"

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[WRITE_NBYTES/4];
	uint32_t i;

	uart_init(INC, MOD, SAMP);

	print_welcome("nvm-write");

	vreg_init();

	putstr("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	putstr("nvm_detect returned: 0x");
	put_hex(err);
	putstr(" type is: 0x");
	put_hex32(type);
	putstr("\n\r");


	buf[0] = WRITEVAL0;
	buf[1] = WRITEVAL1;

	err = nvm_erase(gNvmInternalInterface_c, type, 0x40000000); /* erase sector 30 --- sector 31 is the 'secret zone' */
	putstr("nvm_erase returned: 0x");
	put_hex(err);
	putstr("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	putstr("nvm_write returned: 0x");
	put_hex(err);
	putstr("\n\r");
	putstr("writing\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		putstr("0x");
		put_hex32(buf[i]);
		putstr("\n\r");
		buf[i] = 0x00000000; /* clear buf for the read */
	}

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	putstr("nvm_read returned: 0x");
	put_hex(err);
	putstr("\n\r");
	putstr("reading\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		putstr("0x");
		put_hex32(buf[i]);
		putstr("\n\r");
	}
		

	while(1) {continue;};
}

