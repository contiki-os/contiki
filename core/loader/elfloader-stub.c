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
 * @(#)$Id: elfloader-stub.c,v 1.3 2007/11/17 10:46:41 adamdunkels Exp $
 */
#include "elfloader-arch.h"

#include <stdio.h>

static char datamemory[ELFLOADER_DATAMEMORY_SIZE];
static const char textmemory[ELFLOADER_TEXTMEMORY_SIZE] = {0};
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
  return (void *)datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
  return (void *)textmemory;
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
  printf("elfloader_arch_write_rom: size %d, offset %i, mem %p\n", size, textoff, mem);
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  printf("elfloader_arch_relocate: sectionoffset 0x%04x, sectionaddr %p, r_offset 0x%04x, r_info 0x%04x, r_addend 0x%04x, addr %p\n",
	 sectionoffset, sectionaddr,
	 (unsigned int)rela->r_offset, (unsigned int)rela->r_info,
	 (unsigned int)rela->r_addend, addr);
}
/*---------------------------------------------------------------------------*/
