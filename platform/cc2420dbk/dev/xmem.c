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
 * @(#)$Id: xmem.c,v 1.1 2007/02/02 14:08:22 bg- Exp $
 */

/*
 * External memory built from the AVR eeprom. It is incredibly slow!
 */

#include <stdio.h>

#include <avr/eeprom.h>

#include "contiki.h"

#include "dev/xmem.h"

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

/*
 */
void
xmem_init(void)
{
  PRINTF("xmem_init\n");
}

int
xmem_pread(void *_p, int size, off_t offset)
{
  PRINTF("xmem_pread(%p, %d, %06lx)\n", _p, size, offset);

  spl_t s = splhigh();
  eeprom_busy_wait();
  eeprom_read_block(_p, (void *)(uintptr_t)offset, size);
  splx(s);
  return size;
}

int
xmem_pwrite(const void *_p, int size, off_t offset)
{
  PRINTF("xmem_pwrite(%p, %d, %06lx)\n", _p, size, offset);

  spl_t s = splhigh();
  eeprom_busy_wait();
  eeprom_write_block(_p, (void *)(uintptr_t)offset, size);
  splx(s);
  return size;
}

int
xmem_erase(long size, off_t offset)
{
  /*
   * AVR internal eeprom has a kind of auto erase, thus nothing is
   * done here.
   */
  PRINTF("xmem_erase(%ld, %06lx)\n", size, offset);

  return size;
}
