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
 * @(#)$Id: symtab.c,v 1.5 2006/12/20 14:41:28 bg- Exp $
 */

#include "symtab.h"

#include "loader/symbols-def.h"

#include <string.h>

#define SYMTAB_CONF_BINARY_SEARCH 0

/*---------------------------------------------------------------------------*/
#if SYMTAB_CONF_BINARY_SEARCH
const char *
symtab_lookup(const char *name)
{
  int start, middle, end;
  int r;
  
  start = 0;
  end = symbols_nelts;

  do {
    /* Check middle, divide */
    middle = (end + start) / 2;

    if(symbols[middle].name == NULL) {
      return NULL;
    }
    r = strcmp(name, symbols[middle].name);
    if(r == 0) {
      return symbols[middle].value;   
    }
    if(end == middle || start == middle) {
      return NULL;
    }
    if(r < 0) {
      end = middle;
    }
    if(r > 0) {
      start = middle;
    }
  } while(1);

}
#else /* SYMTAB_CONF_BINARY_SEARCH */
const char *
symtab_lookup(const char *name)
{
  const struct symbols *s;
  for(s = symbols; s->name != NULL; ++s) {
    if(strcmp(name, s->name) == 0) {
      return s->value;
    }
  }
  return 0;
}
#endif /* SYMTAB_CONF_BINARY_SEARCH */
/*---------------------------------------------------------------------------*/
