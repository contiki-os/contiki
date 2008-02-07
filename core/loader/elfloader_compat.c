/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: elfloader_compat.c,v 1.7 2008/02/07 15:53:43 oliverschmidt Exp $
 */

/*
 * This code is plug-in compatible with elfloader.c and is an example
 * of how the Contiki dynamic Link Editor (CLE) can be used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contiki.h"

#include "loader/elfloader_compat.h"
#include "loader/cle.h"

#include "lib/malloc.h"
#include "dev/rom.h"
#include "dev/xmem.h"

#define NDEBUG
#include "lib/assert.h"

#ifdef NDEBUG
#define PRINTF(...) do {} while (0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

struct process *elfloader_loaded_process;
void (*elfloader_fini)(void);

#define IMAX(a, b) (((a) > (b)) ? (a) : (b))

unsigned char *datamemory;

#ifdef __AVR__
extern int __data_load_end;
#define TEXTMEMORY (((cle_addr)(&__data_load_end) + ROM_ERASE_UNIT_SIZE) \
		    & ~(ROM_ERASE_UNIT_SIZE - 1))
#else
#include <sys/unistd.h>
#define TEXTMEMORY \
    (cle_addr)(((uintptr_t)(&_etext + 1)                        \
		+ (uintptr_t)&_edata - (uintptr_t)&__data_start \
		+ ROM_ERASE_UNIT_SIZE)                          \
	       & ~(ROM_ERASE_UNIT_SIZE - 1))
#endif

char elfloader_unknown[30];	/* Name that caused link error. */

/*---------------------------------------------------------------------------*/
int
elfloader_load(off_t eepromaddr)
{
  struct cle_info h;
  int ret;

  void (*elfloader_init)(void);

  elfloader_unknown[0] = 0;

  /* The ELF header is located at the start of the buffer. */
  ret = cle_read_info(&h, xmem_pread, eepromaddr);

  if(ret != ELFLOADER_OK) {
    memcpy(elfloader_unknown, h.name, sizeof(elfloader_unknown));
    elfloader_unknown[sizeof(elfloader_unknown) - 1] = 0;
    return ret;
  }

  if(datamemory != NULL) {
    free(datamemory);
  }

  /* We are making semi-permanent allocations, first compact heap! */
  /* malloc_compact(); */
  datamemory = malloc(IMAX(h.textsize, h.datasize + h.bsssize));
  if(datamemory == NULL) {
    return ELFLOADER_DATA_TO_LARGE; /* XXX or text to large */
  }

  h.data = datamemory;
  h.bss = datamemory + h.datasize;
  h.text = TEXTMEMORY;

  PRINTF("elfloader: copy text segment to RAM %p %p\n",
	 h.data, h.data + h.textsize);
  ret = xmem_pread(datamemory, h.textsize, eepromaddr + h.textoff); 
  assert(ret > 0);
  if(h.textrelasize > 0) {
    PRINTF("elfloader: relocate text in RAM\n");
    ret = cle_relocate(&h,
		       xmem_pread,
		       eepromaddr,
		       datamemory,
		       h.textrelaoff, h.textrelasize);
    if(ret != ELFLOADER_OK) {
      memcpy(elfloader_unknown, h.name, sizeof(elfloader_unknown));
      elfloader_unknown[sizeof(elfloader_unknown) - 1] = 0;
      return ret;
    }
  }
  PRINTF("elfloader: copy text segment to ROM 0x%lx 0x%lx\n",
	 (unsigned long)h.text,
	 (unsigned long)h.text + h.textsize);

  ret = rom_erase((h.textsize+ROM_ERASE_UNIT_SIZE) & ~(ROM_ERASE_UNIT_SIZE-1),
		  h.text);
  assert(ret > 0);
  ret = rom_pwrite(datamemory, h.textsize, h.text);
  assert(ret > 0);

  PRINTF("elfloader: copy data segment to RAM %p %p\n",
	 h.data, h.data + h.datasize);
  ret = xmem_pread(datamemory, h.datasize, eepromaddr + h.dataoff); 
  assert(ret >= h.datasize);
  if(h.datarelasize > 0) {
    PRINTF("elfloader: relocate data segment\n");
    ret = cle_relocate(&h,
		       xmem_pread,
		       eepromaddr,
		       datamemory,
		       h.datarelaoff, h.datarelasize);
    if(ret != ELFLOADER_OK) {
      memcpy(elfloader_unknown, h.name, sizeof(elfloader_unknown));
      elfloader_unknown[sizeof(elfloader_unknown) - 1] = 0;
      return ret;
    }
  }

  PRINTF("elfloader: zero bss %p %p\n", h.bss, h.bss + h.bsssize);
  memset(h.bss, 0, h.bsssize);

  /* Find _init, _fini, and loaded_process. */
  elfloader_loaded_process = cle_lookup(&h, xmem_pread, eepromaddr,
					"autostart_processes");
  elfloader_fini = cle_lookup(&h, xmem_pread, eepromaddr, "_fini");
  elfloader_init = cle_lookup(&h, xmem_pread, eepromaddr, "_init");

  if(elfloader_init != NULL) {
    PRINTF("init=%p fini=%p\n", elfloader_init, elfloader_fini);
    (*elfloader_init)();
    elfloader_loaded_process = NULL;
    return ELFLOADER_OK;
  }

  if(elfloader_loaded_process != NULL) {
    PRINTF("elfloader: launch program\n");
    process_start(elfloader_loaded_process, NULL);
    elfloader_fini = NULL;
    return ELFLOADER_OK;
  } else {
    return ELFLOADER_NO_STARTPOINT;
  }
}
/*---------------------------------------------------------------------------*/
void
elfloader_unload(void)
{
  if(elfloader_fini != NULL) {
    (*elfloader_fini)();
    elfloader_fini = NULL;
  } else if(elfloader_loaded_process != NULL) {
    process_exit(elfloader_loaded_process);
    elfloader_loaded_process = NULL;
  }
  if(datamemory != NULL) {
    free(datamemory);
    datamemory = NULL;
  }
}
