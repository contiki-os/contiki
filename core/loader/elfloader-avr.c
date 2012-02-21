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

#include <stdio.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "elfloader-arch.h"
#include "lib/mmem.h"
#include <string.h> //memset

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

#define DEBUG 0
#if DEBUG
#include <avr/pgmspace.h>
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif

static struct mmem module_heap;
/*---------------------------------------------------------------------------*/
void*
elfloader_arch_allocate_ram(int size)
{
  /* Free previously allocated memory */
  /* TODO Assumes memory address 0 can't be allocated, use flag instead? */
  if (MMEM_PTR(&module_heap) != 0) {
    mmem_free(&module_heap);
  }
  
  /* Allocate RAM for module */
  if (mmem_alloc (&module_heap, size) ==  0) {
    return NULL;
  }
  
  return (char*)MMEM_PTR(&module_heap);
}

/*---------------------------------------------------------------------------*/
/* TODO: Currently, modules are written to the fixed address 0x10000. Since
 *        flash rom uses word addresses on the AVR, we return 0x8000 here
 */
void*
elfloader_arch_allocate_rom(int size)
{
  return (void *)0x8000;
}

/*---------------------------------------------------------------------------*/
/* Eliminate compiler warnings for (non-functional) code when flash requires 32 bit addresses and pointers are 16 bit */
#define INCLUDE_APPLICATE_SOURCE 1
#ifdef __GNUC__
#if (FLASHEND > USHRT_MAX) && (__SIZEOF_POINTER__ <= 2)
#undef INCLUDE_APPLICATE_SOURCE
#define INCLUDE_APPLICATE_SOURCE 0
#endif
#if (__SIZEOF_POINTER__ > 2)
#define INCLUDE_32BIT_CODE 1
#endif
#endif
#if INCLUDE_APPLICATE_SOURCE

BOOTLOADER_SECTION void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
    unsigned char   buf[SPM_PAGESIZE];
    unsigned short* flashptr = (unsigned short *) mem;


    // Sanity-check size of loadable module
    if (size <= 0)
	return;

  
    // Seek to patched module and burn it to flash (in chunks of
    // size SPM_PAGESIZE, i.e. 256 bytes on the ATmega128)
    cfs_seek(fd, textoff, CFS_SEEK_SET);
    for (flashptr=(unsigned short *)mem; flashptr < (unsigned short *) mem + size; flashptr += SPM_PAGESIZE) {
	memset (buf, 0, SPM_PAGESIZE);
	cfs_read(fd, buf, SPM_PAGESIZE);

	// Disable interrupts
	uint8_t sreg;
	sreg = SREG;
	cli ();
  
	// Erase flash page
	boot_page_erase (flashptr);
	boot_spm_busy_wait ();
	
	unsigned short *origptr =  flashptr;

	int i;	
	// Store data into page buffer
	for(i = 0; i < SPM_PAGESIZE; i+=2) {
	    boot_page_fill (flashptr, (uint16_t)((buf[i+1] << 8) | buf[i]));
	    PORTB = 0xff - 7;
	    ++flashptr;
	}
	
	// Burn page
	boot_page_write (origptr);
	boot_spm_busy_wait();
	
	// Reenable RWW sectin
	boot_rww_enable ();
	boot_spm_busy_wait ();	

	// Restore original interrupt settings
	SREG = sreg;
    }
}
#endif /* INCLUDE_APPLICATE_SOURCE */

/*---------------------------------------------------------------------------*/
static void
write_ldi(int fd, unsigned char *instr, unsigned char byte)
{
  instr[0] = (instr[0] & 0xf0) | (byte & 0x0f);
  instr[1] = (instr[1] & 0xf0) | (byte >> 4);
  cfs_write (fd, instr, 2);
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
	//			struct elf32_rela *rela, elf32_addr addr)
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  unsigned char instr[4];

  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
  cfs_read(fd, instr, 4);
  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
  
  type = ELF32_R_TYPE(rela->r_info);

  addr += rela->r_addend;

  switch(type) {
  case R_AVR_NONE:
  case R_AVR_32:
    PRINTF(PSTR ("elfloader-avr.c: unsupported relocation type: "));
    PRINTF("%d\n", type);
    break;

  case R_AVR_7_PCREL: { /* 4 */
    /*
     * Relocation is relative to PC. -2: branch instructions add 2 to PC.
     * Do not use >> 1 for division because branch instructions use
     * signed offsets.
     */
    int16_t a = (((int)addr - rela->r_offset -2) / 2);
    instr[0] |= (a << 3) & 0xf8;
    instr[1] |= (a >> 5) & 0x03;
    cfs_write(fd, instr, 2);
  }
    break;
  case R_AVR_13_PCREL: { /* 3 */
    /*
     * Relocation is relative to PC. -2: RJMP adds 2 to PC.
     * Do not use >> 1 for division because RJMP uses signed offsets.
     */
    int16_t a = (int)addr / 2;
    a -= rela->r_offset / 2;
    a--;
    instr[0] |= a & 0xff;
    instr[1] |= (a >> 8) & 0x0f;
    cfs_write(fd, instr, 2);
  }
    break;

  case R_AVR_16:    /* 4 */
    instr[0] = (int)addr  & 0xff;
    instr[1] = ((int)addr >> 8) & 0xff;

    cfs_write(fd, instr, 2);
    break;

  case R_AVR_16_PM: /* 5 */
    addr = (char *)((int)addr >> 1);
    instr[0] = (int)addr  & 0xff;
    instr[1] = ((int)addr >> 8) & 0xff;

    cfs_write(fd, instr, 2);
    break;

  case R_AVR_LO8_LDI: /* 6 */
    write_ldi(fd, instr, (int)addr);
    break;
  case R_AVR_HI8_LDI: /* 7 */
    write_ldi(fd, instr, (int)addr >> 8);
    break;

#if INCLUDE_32BIT_CODE       /* 32 bit AVRs */
  case R_AVR_HH8_LDI: /* 8 */
    write_ldi(fd, instr, (int)addr >> 16);
    break;
#endif

  case R_AVR_LO8_LDI_NEG: /* 9 */
    addr = (char *) (0 - (int)addr);
    write_ldi(fd, instr, (int)addr);
    break;
  case R_AVR_HI8_LDI_NEG: /* 10 */
    addr = (char *) (0 - (int)addr);
    write_ldi(fd, instr, (int)addr >> 8);
    break;
    
#if INCLUDE_32BIT_CODE         /* 32 bit AVRs */
  case R_AVR_HH8_LDI_NEG: /* 11 */
    addr = (char *)(0 - (int)addr);
    write_ldi(fd, instr, (int)addr >> 16);
    break;
#endif

  case R_AVR_LO8_LDI_PM: /* 12 */
    write_ldi(fd, instr, (int)addr >> 1);
    break;
  case R_AVR_HI8_LDI_PM: /* 13 */
    write_ldi(fd, instr, (int)addr >> 9);
    break;

#if INCLUDE_32BIT_CODE         /* 32 bit AVRs */
  case R_AVR_HH8_LDI_PM: /* 14 */
    write_ldi(fd, instr, (int)addr >> 17);
    break;
#endif

  case R_AVR_LO8_LDI_PM_NEG: /* 15 */
    addr = (char *) (0 - (int)addr);
    write_ldi(fd, instr, (int)addr >> 1);
    break;
  case R_AVR_HI8_LDI_PM_NEG: /* 16 */
    addr = (char *) (0 - (int)addr);
    write_ldi(fd, instr, (int)addr >> 9);
    break;
    
#if INCLUDE_32BIT_CODE         /* 32 bit AVRs */
  case R_AVR_HH8_LDI_PM_NEG: /* 17 */
    addr = (char *) (0 - (int)addr);
    write_ldi(fd, instr, (int)addr >> 17);
    break;
#endif

  case R_AVR_CALL: /* 18 */
  	/* old solution: 
     addr = ((int16_t)addr >> 1);
     instr[2] = (int16_t)addr & 0xff;
     instr[3] = (int16_t)addr >> 8;
	*/

	/* new solution */
    instr[2] = (uint8_t) ((int)addr) & 0xff;
    instr[3] = ((int)addr) >> 8;
    cfs_write(fd, instr, 4);
    break;

  default:
    PRINTF(PSTR ("Unknown relocation type!\n"));
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
elfloader_unload(void) {
}
