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

#ifndef NVM_H
#define NVM_H

#include <stdint.h>

typedef enum
{
	gNvmType_NoNvm_c,
	gNvmType_SST_c,
	gNvmType_ST_c,
	gNvmType_ATM_c,
	gNvmType_Max_c
} nvmType_t;


typedef enum
{
	gNvmErrNoError_c = 0,
	gNvmErrInvalidInterface_c,
	gNvmErrInvalidNvmType_c,
	gNvmErrInvalidPointer_c,
	gNvmErrWriteProtect_c,
	gNvmErrVerifyError_c,
	gNvmErrAddressSpaceOverflow_c,
	gNvmErrBlankCheckError_c,
	gNvmErrRestrictedArea_c,
	gNvmErrMaxError_c
} nvmErr_t;

typedef enum
{
	gNvmInternalInterface_c,
	gNvmExternalInterface_c,
	gNvmInterfaceMax_c
} nvmInterface_t;

/* ROM code seems to be THUMB */
/* need to be in a THUMB block before calling them */
extern nvmErr_t (*nvm_detect)(nvmInterface_t nvmInterface,nvmType_t* pNvmType);
extern nvmErr_t (*nvm_read)(nvmInterface_t nvmInterface , nvmType_t nvmType , void *pDest, uint32_t address, uint32_t numBytes);
extern nvmErr_t (*nvm_write)(nvmInterface_t nvmInterface, nvmType_t nvmType ,void *pSrc, uint32_t address, uint32_t numBytes);
/* sector bit field selects which sector to erase */
/* SST flash has 32 sectors 4096 bytes each */
/* bit 0 is the first sector, bit 31 is the last */
extern nvmErr_t (*nvm_erase)(nvmInterface_t nvmInterface, nvmType_t nvmType ,uint32_t sectorBitfield);
extern nvmErr_t (*nvm_verify)(nvmInterface_t nvmInterface, nvmType_t nvmType, void *pSrc, uint32_t address, uint32_t numBytes);
extern void(*nvm_setsvar)(uint32_t zero_for_awesome);
#endif //NVM_H
