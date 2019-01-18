/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: cle_avr.c,v 1.3 2007/06/04 17:50:25 bg- Exp $
 */

/*
 * The Contiki dynamic Link Editor (CLE), ELF version.
 */

#include <stdio.h>

#include "contiki.h"

#include "loader/elf32.h"
#include "loader/cle.h"

#define NDEBUG
#include "lib/assert.h"

#ifdef NDEBUG
#define PRINTF(...) do {} while (0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

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

/*
 * Writing relocs is machine dependent and this function is AVR
 * specific!
 */
int
cle_write_reloc(void *pos_,
		const struct elf32_rela *rela,
		cle_addr addr,
		const struct cle_info *info)
{
  unsigned char *pos = pos_;
  unsigned char byte;

  switch(ELF32_R_TYPE(rela->r_info)) {
  default:
    PRINTF("cle_upd_reloc: unsupported relocation type: %d\n",
	   ELF32_R_TYPE(rela->r_info));
    return CLE_UNKNOWN_RELOC;

  case R_AVR_7_PCREL:		/* 2 */
    /* Reloc in bits 0x03f8 (0000 00kk kkkk k000). */
    byte = addr - (/* text */ + rela->r_offset + 2);
    byte = byte >> 1;
    pos[0] = (pos[0] & 0x07) | (byte << 3);	/* 0xf8 */
    pos[1] = (pos[1] & 0xfc) | (byte >> 5);	/* 0x03 */
    return CLE_OK;

  case R_AVR_13_PCREL:		/* 3 */
    /* Reloc in bits 0x0fff (0000 kkkk kkkk kkkk). */
    addr = addr - (info->text + rela->r_offset + 2);
    addr = addr >> 1;
    pos[0] = addr;
    pos[1] = (pos[1] & 0xf0) | ((addr >> 8) & 0x0f);
    return CLE_OK;

  case R_AVR_CALL:		/* 18 */
    addr = addr >> 1;
    pos[2] = addr;
    pos[3] = addr >> 8;
    return CLE_OK;

  case R_AVR_16:		/* 4 */
    pos[0] = addr;
    pos[1] = addr >> 8;
    return CLE_OK;

  case R_AVR_16_PM:		/* 5 */
    addr = addr >> 1;
    pos[0] = addr;
    pos[1] = addr >> 8;
    return CLE_OK;

    /*
     * Remaining relocs all have immediate value in bits 0x0f0f.
     */
  case R_AVR_LO8_LDI:		/* 6 */
    byte = addr;
    break;
    
  case R_AVR_HI8_LDI:		/* 7 */
    byte = addr >> 8;
    break;

  case R_AVR_HH8_LDI:		/* 8 */
    byte = addr >> 16;
    break;

  case R_AVR_LO8_LDI_NEG:	/* 9 */
    byte = (-addr);
    break;

  case R_AVR_HI8_LDI_NEG:	/* 10 */
    byte = (-addr) >> 8;
    break;

  case R_AVR_HH8_LDI_NEG:	/* 11 */
    byte = (-addr) >> 16;
    break;

  case R_AVR_LO8_LDI_PM:	/* 12 */
    byte = addr >> 1;
    break;

  case R_AVR_HI8_LDI_PM:	/* 13 */
    byte = addr >> 9;
    break;

  case R_AVR_HH8_LDI_PM:	/* 14 */
    byte = addr >> 17;
    break;

  case R_AVR_LO8_LDI_PM_NEG:	/* 15 */
    byte = (-addr) >> 1;
    break;

  case R_AVR_HI8_LDI_PM_NEG:	/* 16 */
    byte = (-addr) >> 9;
    break;

  case R_AVR_HH8_LDI_PM_NEG:	/* 17 */
    byte = (-addr) >> 17;
    break;
  }
  /* Relocation in bits 0x0f0f (0000 kkkk 0000 kkkk). */
  pos[0] = (pos[0] & 0xf0) | (byte & 0x0f);
  pos[1] = (pos[1] & 0xf0) | (byte >> 4);

  return CLE_OK;
}
