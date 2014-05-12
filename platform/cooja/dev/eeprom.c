/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 */

#include "dev/eeprom.h"

#include "lib/simEnvChange.h"

const struct simInterface eeprom_interface;

#define EEPROM_BUF_SIZE 1024 /* Configure EEPROM size here and in ContikiEeprom.java */

unsigned char simEEPROMData[EEPROM_BUF_SIZE];
char simEEPROMChanged = 0;
int simEEPROMRead = 0;
int simEEPROMWritten = 0;

void 
eeprom_init(void) 
{
}

void
eeprom_read(eeprom_addr_t addr, unsigned char *buf, int len)
{
    if (addr >= EEPROM_BUF_SIZE) {
        return;
    }
    
    if(addr + len >= EEPROM_BUF_SIZE) {
		len = EEPROM_BUF_SIZE - addr;
	}

	memcpy(buf, &simEEPROMData[addr], len);
	
    simEEPROMChanged = 1;
    simEEPROMRead += len;
}

void
eeprom_write(eeprom_addr_t addr, unsigned char *buf, int len)
{    
    if (addr >= EEPROM_BUF_SIZE) {
        return;
    }

    if(addr + len >= EEPROM_BUF_SIZE) {
		len = EEPROM_BUF_SIZE - addr;
	}
    
    
    memcpy(&simEEPROMData[addr], buf, len);
    
    simEEPROMChanged = 1;
    simEEPROMWritten += len;
    
}

/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(eeprom_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
