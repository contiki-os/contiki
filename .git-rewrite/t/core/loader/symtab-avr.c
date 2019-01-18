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
 * @(#)$Id: symtab-avr.c,v 1.3 2007/01/24 16:13:50 adamdunkels Exp $
 */

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "symtab.h"
#include "loader/symbols.h"

#define SYMTAB_CONF_BINARY_SEARCH 0

/*---------------------------------------------------------------------------*/
void *
symtab_lookup(const char *name)
{
  uint16_t i=0;
  void* name_addr;

  for(name_addr = (void*)pgm_read_word(&symbols[0].name);
      name_addr != NULL;
      name_addr = (void*)pgm_read_word(&symbols[++i].name)) {

    if(strcmp_P (name, (const char*)name_addr) == 0) {
      return (void*)pgm_read_word(&symbols[i].value);
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

#if 0
#define SYMTAB_PRINT_BUFFER_SIZE 30
void
symtab_print (void)
{
  uint16_t i=0;
  const char* name_addr;
  char buf[SYMTAB_PRINT_BUFFER_SIZE];

  for(name_addr = (const char*)pgm_read_word(&symbols[0].name);
      name_addr != NULL;
      name_addr = pgm_read_word(&symbols[++i].name)) {

    strncpy_P (buf, (const char*)name_addr, SYMTAB_PRINT_BUFFER_SIZE);
    buf [SYMTAB_PRINT_BUFFER_SIZE - 1] = '\0';
    uint16_t value = pgm_read_word(&symbols[i].value);
    printf ("%s -> 0x%x\n", buf, value);
  }
}
#endif
