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
 * @(#)$Id: elfloader-msp430.c,v 1.5 2009/03/26 12:25:05 fros4943 Exp $
 */
#include "elfloader-arch.h"

#include "dev/flash.h"

static uint16_t datamemory_aligned[ELFLOADER_DATAMEMORY_SIZE/2+1];
static uint8_t* datamemory = (uint8_t *)datamemory_aligned;
#if ELFLOADER_CONF_TEXT_IN_ROM
static const char textmemory[ELFLOADER_TEXTMEMORY_SIZE] = {0};
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
static char textmemory[ELFLOADER_TEXTMEMORY_SIZE];
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
  return datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
#if ELFLOADER_CONF_TEXT_IN_ROM
  /* Return an 512-byte aligned pointer. */
  return (char *)
    ((unsigned long)&textmemory[0] & 0xfffffe00) +
    (((unsigned long)&textmemory[0] & 0x1ff) == 0? 0: 0x200);
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
  return textmemory;
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
}
/*---------------------------------------------------------------------------*/
#define READSIZE 32
void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
#if ELFLOADER_CONF_TEXT_IN_ROM
  int i;
  unsigned int ptr;
  unsigned short *flashptr;

  flash_setup();

  flashptr = (unsigned short *)mem;

  cfs_seek(fd, textoff, CFS_SEEK_SET);
  for(ptr = 0; ptr < size; ptr += READSIZE) {

    /* Read data from file into RAM. */
    cfs_read(fd, (unsigned char *)datamemory, READSIZE);

    /* Clear flash page on 512 byte boundary. */
    if((((unsigned short)flashptr) & 0x01ff) == 0) {
      flash_clear(flashptr);
    }

    /* Burn data from RAM into flash ROM. Flash is burned one 16-bit
       word at a time, so we need to be careful when incrementing
       pointers. The flashptr is already a short pointer, so
       incrementing it by one will actually increment the address by
       two. */
    for(i = 0; i < READSIZE / 2; ++i) {
      flash_write(flashptr, ((unsigned short *)datamemory)[i]);
      ++flashptr;
    }
  }

  flash_done();
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  cfs_read(fd, (unsigned char *)mem, size);
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  addr += rela->r_addend;

  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
  cfs_write(fd, (char *)&addr, 2);
}
/*---------------------------------------------------------------------------*/
