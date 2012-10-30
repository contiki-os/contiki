/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 *
 */
#ifndef __LOADER_ARCH_H__
#define __LOADER_ARCH_H__

/**
 * Load a program from EEPROM.
 *
 * This function loads a program from EEPROM into the flash ROM (code) and RAM (data).
 *
 * \param code The address of the first byte of code in the EEPROM.
 *
 * \param codelen The length of the code.
 *
 * \param data The address of the first byte of data in the EEPROM.
 *
 * \param datalen The length of the data.
 *
 */
void loader_arch_load(unsigned short startaddr);

/**
 * Unload a program.
 *
 * This function should not be called directly, but accessed through
 * the macro LOADER_UNLOAD(). The function unloads a program from both
 * RAM and ROM.
 *
 * \param codeaddr A pointer to the first byte of program code.
 *
 * \param dataaddr A pointer to the first byte of program data.
 */
void loader_arch_free(void *codeaddr, void *dataaddr);

extern void *loader_arch_codeaddr, *loader_arch_dataaddr;

#ifndef LOADER_UNLOAD
#define LOADER_UNLOAD() loader_arch_free(&loader_arch_codeaddr, &loader_arch_dataaddr)
#endif

#define LOADER_ARCH_MAGIC 0x373a
#define LOADER_ARCH_VERSION 0x0001

#endif /* __LOADER_ARCH_H__ */
