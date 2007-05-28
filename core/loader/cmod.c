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
 * @(#)$Id: cmod.c,v 1.4 2007/05/28 16:22:15 bg- Exp $
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "loader/elf32.h"
#include "loader/cle.h"
#include "loader/cmod.h"

#include "lib/malloc.h"

#include "lib/assert.h"

#if 1
#define PRINTF(...) do {} while (0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

#ifndef CMOD_NMODULES
#define CMOD_NMODULES 4
#endif
struct cmod_info cmod_module[CMOD_NMODULES];

int
cmod_load(unsigned imod,
	  cle_scratch scratch,
	  int (*pread)(void *, int, off_t),
	  off_t off)
{
  struct cle_info h;
  int ret;
  void (*init)(void);

  if(imod >= CMOD_NMODULES) {
    PRINTF("imod to large");
    return 100;
  }

  if(cmod_module[imod].ram != NULL || cmod_module[imod].fini != NULL) {
    PRINTF("module busy\n");
    return 101;
  }

  /* The (ELF) header is located at the start of the buffer. */
  ret = cle_read_info(&h, pread, off);

  if(ret != CLE_OK) {
    strcpy(scratch, h.name);
    return ret;
  }

  cmod_module[imod].ram = malloc(h.datasize + h.bsssize + h.textsize);
  if(cmod_module[imod].ram == NULL) {
    return CMOD_DATA_TO_LARGE;
  }

  /*
   * Here we specify where we want to relocate to.
   */
  h.data = cmod_module[imod].ram;
  h.bss = h.data + h.datasize;
  h.text = (cle_addr)h.bss + h.bsssize;

  PRINTF("cmod: copy text segment to RAM %p %p\n",
	 h.text, h.text + h.textsize);
  ret = pread((void *)h.text, h.textsize, off + h.textoff); 
  assert(ret > 0);
  if(h.textrelasize > 0) {
    PRINTF("cmod: relocate text in RAM\n");
    ret = cle_relocate(&h,
		       pread,
		       off,
		       (void *)h.text,
		       h.textrelaoff, h.textrelasize);
    if(ret != CLE_OK) {
      strcpy(scratch, h.name);
      return ret;
    }
  }

  PRINTF("cmod: copy data segment to RAM %p %p\n",
	 h.data, h.data + h.datasize);
  ret = pread(h.data, h.datasize, off + h.dataoff); 
  assert(ret >= 0);
  if(h.datarelasize > 0) {
    PRINTF("cmod: relocate data segment\n");
    ret = cle_relocate(&h,
		       pread,
		       off,
		       h.data,
		       h.datarelaoff, h.datarelasize);
    if(ret != CLE_OK) {
      strcpy(scratch, h.name);
      return ret;
    }
  }

  PRINTF("cmod: zero bss %p %p\n", h.bss, h.bss + h.bsssize);
  memset(h.bss, 0, h.bsssize);

  cmod_module[imod].fini = cle_lookup(&h, pread, off, "_fini");
  init = cle_lookup(&h, pread, off, "_init");

  if(init != NULL) {
    PRINTF("init=%p fini=%p\n", init, cmod_module[imod].fini);
    (*init)();
    return CLE_OK;
  } else
    return CMOD_NO_STARTPOINT;
}

void
cmod_unload(int imod)
{
  if(cmod_module[imod].fini != NULL) {
    (*cmod_module[imod].fini)();
    cmod_module[imod].fini = NULL;
  }
  if(cmod_module[imod].ram != NULL) {
    free(cmod_module[imod].ram);
    cmod_module[imod].ram = NULL;
  }
}

#if 0
void
cmod_status(void)
{
  unsigned i;
  PRINTF("Id Module   Address Fini\n");
  for(i = 0; i < CMOD_NMODULES; i++)
    if(cmod_module[i].ram != NULL)
      PRINTF("%2d %-8s %7p %4p\n", i,
	     cmod_module[i].name, cmod_module[i].ram, cmod_module[i].fini);
}
#endif
