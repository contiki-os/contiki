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

#define DEBUG 1
#if DEBUG
#define dbg_putchr(...) putchr(__VA_ARGS__)
#define dbg_putstr(...) putstr(__VA_ARGS__)
#define dbg_put_hex(...) put_hex(__VA_ARGS__)
#define dbg_put_hex16(...) put_hex16(__VA_ARGS__)
#define dbg_put_hex32(...) put_hex32(__VA_ARGS__)
#else
#define dbg_putchr(...)
#define dbg_putstr(...)
#define dbg_put_hex(...)
#define dbg_put_hex16(...)
#define dbg_put_hex32(...)
#endif

uint8_t getc(void) 
{
	volatile uint8_t c;
	while(*UART1_URXCON == 0);
	
	c = *UART1_UDATA;
	return c;
}


void flushrx(void);
uint32_t to_u32(volatile uint32_t *c);

enum parse_states {
	SCAN_X,
	READ_CHARS,
	PROCESS,
	MAX_STATE,
};

#define NVM_BASE 0x1E000
static char nvm_base[0x100];

int main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	volatile uint8_t c;
	volatile uint32_t i;
	volatile uint32_t buf[4];
	volatile uint32_t len=0;
	volatile uint32_t state = SCAN_X;
	volatile uint32_t addr,data;


	uart_init(INC, MOD, SAMP);
	disable_irq(UART1);

	vreg_init();

	dbg_putstr("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	dbg_putstr("nvm_detect returned: 0x");
	dbg_put_hex(err);
	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");
	
	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)nvm_base, NVM_BASE, 0x100);
	dbg_putstr("nvm_read returned: 0x");
	dbg_put_hex(err);
	dbg_putstr("\n\r");

	/* erase the flash */
	nvm_setsvar(0);
	err = nvm_erase(gNvmInternalInterface_c, type, 0x40000000);

	dbg_putstr("nvm_erase returned: 0x");
	dbg_put_hex(err);
	dbg_putstr("\n\r");

	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)nvm_base, NVM_BASE, 0x100);
	dbg_putstr("nvm_write returned: 0x");
	dbg_put_hex(err);
	dbg_putstr("\n\r");

	/* say we are ready */
	len = 0;
	putstr("ready");
	flushrx();

	/* read the length */
	for(i=0; i<4; i++) {
		c = uart1_getc();
		/* bail if the first byte of the length is zero */
		len += (c<<(i*8));
	}

	dbg_putstr("len: ");
	dbg_put_hex32(len);
	dbg_putstr("\n\r");
	
	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");

	putstr("flasher done\n\r");

	state = SCAN_X; addr=0;
	while((c=getc())) {
		if(state == SCAN_X) {
			/* read until we see an 'x' */
			if(c==0) { break; }
			if(c!='x'){ continue; } 	
			/* go to read_chars once we have an 'x' */
			state = READ_CHARS;
			i = 0; 
		}
		if(state == READ_CHARS) {
			/* read all the chars up to a ',' */
			((uint8_t *)buf)[i++] = c;
			/* after reading a ',' */
			/* goto PROCESS state */
			if((c == ',') || (c == 0)) { state = PROCESS; }				
		}
		if(state == PROCESS) {
			if(addr==0) {
				/*interpret the string as the starting address */
				addr = to_u32(buf);				
			} else {
				/* string is data to write */
				data = to_u32(buf);
				putstr("writing addr ");
				put_hex32(NVM_BASE+addr);
				putstr(" data ");
				put_hex32(data);
				err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)&data, NVM_BASE+addr, 4);
				addr += 4;
				putstr(" err ");
				put_hex32(err);
				putstr("\n\r");
			}
			/* look for the next 'x' */
			state=SCAN_X;
		}
	}
        putstr("process flasher done\n\r");

	while(1) {continue;};
}

void flushrx(void)
{
	volatile uint8_t c;
	while(*UART1_URXCON !=0) {
		c = *UART1_UDATA;
	}
}

/* Convert from ASCII hex.  Returns                                                                                                      
   the value, or 16 if it was space/newline, or                                                                                          
   32 if some other character. */
uint8_t from_hex(uint8_t ch)
{
        if(ch==' ' || ch=='\r' || ch=='\n')
                return 16;

        if(ch < '0')
                goto bad;
        if(ch <= '9')
                return ch - '0';
        ch |= 0x20;
        if(ch < 'a')
                goto bad;
        if(ch <= 'f')
                return ch - 'a' + 10;
bad:
        return 32;
}

uint32_t to_u32(volatile uint32_t *c) 
{
	volatile uint32_t ret=0;
	volatile uint32_t i,val;
	
	/* c should be /x\d+,/ */
	i=1; /* skip x */
	while(((uint8_t *)c)[i] != ',') {
		ret = ret<<4;
		val = from_hex(((uint8_t *)c)[i++]);
		ret += val;
	}
	return ret;
}



