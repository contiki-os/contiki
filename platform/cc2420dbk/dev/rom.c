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
 * @(#)$Id: rom.c,v 1.1 2007/02/02 14:08:22 bg- Exp $
 */

#include <stdio.h>

#include <avr/boot.h>
#include <avr/pgmspace.h>

#include "contiki.h"

#include "lib/assert.h"

#include "dev/rom.h"

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

BOOTLOADER_SECTION
int
rom_erase(long nbytes, off_t offset)
{
  long nb = nbytes;

  if(nbytes % ROM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  if(offset % ROM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  PRINTF("rom_erase(%ld, %06lx)\n", nbytes, offset);

  while (nbytes > 0) {
    spl_t s = splhigh();

    eeprom_busy_wait();

    boot_page_erase(offset);
    boot_spm_busy_wait();

    boot_rww_enable();		/* Before return or intr. */

    splx(s);

    nbytes -= ROM_ERASE_UNIT_SIZE;
    offset += ROM_ERASE_UNIT_SIZE;
  }

  return nb;
}

int
rom_pread(void *buf, int nbytes, off_t offset)
{
  PRINTF("rom_pread(%p, %d, %06lx)\n", buf, nbytes, offset);

  assert(offset == (uintptr_t)offset);
  assert((offset + nbytes) == (uintptr_t)(offset + nbytes));
  memcpy_P(buf, (PGM_P)(uintptr_t)offset, nbytes);
  return nbytes;
}

BOOTLOADER_SECTION
int
rom_pwrite(const void *buf, int nbytes, off_t offset)
{
  long nb = nbytes;
  const unsigned char *from = buf;

  PRINTF("rom_pwrite(%p, %d, %06lx)\n", buf, nbytes, offset);

  while(nbytes > 0) {
    int i, n = (nbytes > ROM_ERASE_UNIT_SIZE) ? ROM_ERASE_UNIT_SIZE : nbytes;
    spl_t s = splhigh();

    eeprom_busy_wait();

    for (i = 0; i < n; i += 2) {
      uint16_t w = *from++;
      w |= (*from++) << 8;
      boot_page_fill(offset + i, w);
    }
    boot_page_write(offset);
    boot_spm_busy_wait();

    boot_rww_enable();		/* Before return or intr. */

    splx(s);

    nbytes -= ROM_ERASE_UNIT_SIZE;
    offset += ROM_ERASE_UNIT_SIZE;
  }

  return nb;
}
