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
 * @(#)$Id: elfloader_compat.h,v 1.1 2006/12/20 13:38:33 bg- Exp $
 */
#ifndef __ELFLOADER_H__
#define __ELFLOADER_H__

#define ELFLOADER_OK                  0
#define ELFLOADER_BAD_ELF_HEADER      1
#define ELFLOADER_NO_SYMTAB           2
#define ELFLOADER_NO_STRTAB           3
#define ELFLOADER_NO_TEXT             4
#define ELFLOADER_SYMBOL_NOT_FOUND    5
#define ELFLOADER_SEGMENT_NOT_FOUND   6
#define ELFLOADER_NO_STARTPOINT       7
#define ELFLOADER_TEXT_TO_LARGE       8
#define ELFLOADER_DATA_TO_LARGE       9
#define ELFLOADER_BSS_TO_LARGE       10

int elfloader_load(off_t eepromaddr);
void elfloader_unload(void);

extern struct process *elfloader_loaded_process;
extern void (*elfloader_fini)(void);
extern char elfloader_unknown[30];

#endif /* __ELFLOADER_H__ */
