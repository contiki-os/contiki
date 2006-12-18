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
 * @(#)$Id: elfloader-avr.c,v 1.3 2006/12/18 14:54:04 fros4943 Exp $
 */
#include "elfloader-arch.h"

/*#include "dev/flash.h"*/

#define R_AVR_NONE             0
#define R_AVR_32               1
#define R_AVR_7_PCREL          2
#define R_AVR_13_PCREL         3
#define R_AVR_16               4
#define R_AVR_16_PM            5
#define R_AVR_LO8_LDI          6
#define R_AVR_HI8_LDI          7
#define R_AVR_HH8_LDI          8
#define R_AVR_LO8_LDI_NEG      9
#define R_AVR_HI8_LDI_NEG     10
#define R_AVR_HH8_LDI_NEG     11
#define R_AVR_LO8_LDI_PM      12
#define R_AVR_HI8_LDI_PM      13
#define R_AVR_HH8_LDI_PM      14
#define R_AVR_LO8_LDI_PM_NEG  15
#define R_AVR_HI8_LDI_PM_NEG  16
#define R_AVR_HH8_LDI_PM_NEG  17
#define R_AVR_CALL            18

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

static char datamemory[ELFLOADER_DATAMEMORY_SIZE];
static const char textmemory[ELFLOADER_TEXTMEMORY_SIZE];
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
  /* Return an 512-byte aligned pointer. */
  return (char *)
    ((unsigned long)&textmemory[0] & 0xfffffe00) +
    (((unsigned long)&textmemory[0] & 0x1ff) == 0? 0: 0x200);
}
/*---------------------------------------------------------------------------*/
#define READSIZE 32
void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
#if 0
  int i;
  unsigned int ptr;
  unsigned short *flashptr;

  flash_setup();

  flashptr = (unsigned short *)elfloader_arch_textmemory;
  
  cfs_seek(fd, textoff);
  for(ptr = 0; ptr < size; ptr += READSIZE) {
    
    /* Read data from file into RAM. */
    cfs_read(fd, (unsigned char *)elfloader_arch_datamemory, READSIZE);

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
      flash_write(flashptr, ((unsigned short *)elfloader_arch_datamemory)[i]);
      ++flashptr;
    }
  }

  flash_done();
#endif
}
/*---------------------------------------------------------------------------*/
static void
write_ldi(int fd, unsigned char *instr, unsigned char byte)
{
  instr[0] = (instr[0] & 0xf0) | (byte & 0x0f);
  instr[1] = (instr[0] & 0xf0) | (byte >> 4);
  cfs_write(fd, instr, 2);
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  unsigned char instr[4];

  cfs_seek(fd, sectionoffset + rela->r_offset);
  cfs_read(fd, instr, 4);
  cfs_seek(fd, sectionoffset + rela->r_offset);
  
  addr += rela->r_addend;

  type = ELF32_R_TYPE(rela->r_info);

  printf("elfloader_arch_relocate: type %d\n", type);
  
  switch(type) {
  case R_AVR_NONE:
  case R_AVR_32:
  case R_AVR_7_PCREL:  /* >> 1 */
  case R_AVR_13_PCREL: /* >> 1 */
    printf("elfloader-avr.c: unsupported relocation type %d\n", type);
    break;
    
  case R_AVR_16:
    cfs_write(fd, (char *)addr, 2);
    break;
  case R_AVR_16_PM:
    addr = (char *)((unsigned long)addr >> 1);
    cfs_write(fd, (char *)addr, 2);
    break;

  case R_AVR_LO8_LDI:
    write_ldi(fd, instr, (unsigned long)addr);
    break;
  case R_AVR_HI8_LDI:
    write_ldi(fd, instr, (unsigned long)addr >> 8);
    break;
  case R_AVR_HH8_LDI:
    write_ldi(fd, instr, (unsigned long)addr >> 16);
    break;

  case R_AVR_LO8_LDI_NEG:
    addr = (char *)(0 - (unsigned long)addr);
    write_ldi(fd, instr, (unsigned long)addr);
    break;
  case R_AVR_HI8_LDI_NEG:
    addr = (char *)(0 - (unsigned long)addr);
    write_ldi(fd, instr, (unsigned long)addr >> 8);
    break;
  case R_AVR_HH8_LDI_NEG:
    addr = (char *)(0 - (unsigned long)addr);
    write_ldi(fd, instr, (unsigned long)addr >> 16);
    break;

  case R_AVR_LO8_LDI_PM:
    write_ldi(fd, instr, (unsigned long)addr >> 1);
    break;
  case R_AVR_HI8_LDI_PM:
    write_ldi(fd, instr, (unsigned long)addr >> 9);
    break;
  case R_AVR_HH8_LDI_PM:
    write_ldi(fd, instr, (unsigned long)addr >> 17);
    break;

  case R_AVR_LO8_LDI_PM_NEG:
    addr = (char *)(0 - (unsigned long)addr);
    write_ldi(fd, instr, (unsigned long)addr >> 1);
    break;
  case R_AVR_HI8_LDI_PM_NEG:
    addr = (char *)(0 - (unsigned long)addr);
    write_ldi(fd, instr, (unsigned long)addr >> 9);
    break;
    addr = (char *)(0 - (unsigned long)addr);
  case R_AVR_HH8_LDI_PM_NEG:
    write_ldi(fd, instr, (unsigned long)addr >> 17);
    break;

  case R_AVR_CALL:
    addr = (char *)((unsigned long)addr >> 1);
    instr[3] = (unsigned long)addr >> 8;
    instr[4] = (unsigned long)addr & 0xff;
    printf("R_AVR_CALL:Writing 0x%02x 0x%02x 0x%02x 0x%02x\n",
	   instr[0], instr[1], instr[2], instr[3]);
    cfs_write(fd, instr, 4);
    break;

  }
  
}
/*---------------------------------------------------------------------------*/
