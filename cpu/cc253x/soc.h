/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Header file with cc253x SoC-specific defines and prototypes
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */


#ifndef SOC_H_
#define SOC_H_


#ifndef CC2530_LAST_FLASH_BANK
#define CC2530_LAST_FLASH_BANK 7 /* Default to F256 */
#endif

#if CC2530_LAST_FLASH_BANK==7 /* F256 */
#define CC2530_FLAVOR_STRING "F256"
#elif CC2530_LAST_FLASH_BANK==3 /* F128 */
#define CC2530_FLAVOR_STRING "F128"
#elif CC2530_LAST_FLASH_BANK==1 /* F64 */
#define CC2530_FLAVOR_STRING "F64"
#elif CC2530_LAST_FLASH_BANK==0 /* F32 */
#define CC2530_FLAVOR_STRING "F32"
#else
#error "Unknown SoC Type specified. Check the value of HIGH_FLASH_BANK in your"
#error "Makefile. Valid values are 0, 1, 3, 7"
#endif

void soc_init();

#endif /* SOC_H_ */
