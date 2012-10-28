/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */
 
/*
 * \file
 * 	mef.h
 * 	The Micro Executable Format
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */
/*
 * MEF file format:
 * [AreaDecls]
 *   BYTE nAreas (0-15)
 *   struct AreaSize[nAreas]
 * [Data]
 *   binary*
 * [Relocation]
 *   WORD nRelocs
 *   struct Relocation[nRelocs]
 */

#ifndef __MEF_H__
#define __MEF_H__


/*
 * mode
 *  bit 7: read/write (1) / read only (0)
 *  bit 3-0: Area index
 * checksum
 *  just a sum of all data of the area
 */
#define MEF_AREA_RW    0x80
#define MEF_AREA_MAX   0x10

struct Area {
  unsigned char mode;
  uint16_t size;
  uint16_t checksum;
};

/*
 * mode
 * bit 7: Absolute (1) / Relative (0)
 * bit 6: MSB (1) / LSB (0) (in byte mode)
 * bit 5: Byte mode (1) / Word mode (0)
 */
#define MEF_RELOC_ABSOLUTE   0x80
#define MEF_RELOC_MSB_BYTE   0x60
#define MEF_RELOC_LSB_BYTE   0x20

struct Relocation {
  unsigned char mode;
  uint16_t address;
  uint16_t data;
};

unsigned char load_byte();

void mef_load(unsigned char* offset);
unsigned char load_byte();
void mef_reloc(unsigned char* offset, struct Relocation* reloc);

#endif /* __MEF_H__ */
