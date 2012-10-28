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
 */

#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

#include "loader/sym.h"

#include <string.h>

static union sym_value
sym_lookup(const char *name, const struct sym_bol *symbols, int nelts)
{
  union sym_value ret;
  int start, middle, end;
  int r;
  
  start = 0;
  end = nelts - 1;

  while(start <= end) {
    /* Check middle, divide */
    middle = (start + end) / 2;
#ifdef __AVR__
    PGM_P addr = (PGM_P)pgm_read_word(&symbols[middle].name);
    r = strcmp_P(name, addr);
#else
    r = strcmp(name, symbols[middle].name);
#endif
    if(r < 0) {
      end = middle - 1;
    } else if(r > 0) {
      start = middle + 1;
    } else {
#ifdef __AVR__
      ret.func = (sym_func_t)pgm_read_word(&symbols[middle].value);
      return ret;
#else
      return symbols[middle].value;   
#endif
    }
  }
  ret.obj = NULL;
  ret.func = NULL;
  return ret;
}

/* Lookup a pointer to an ANSI C object. */
void *
sym_object(const char *name)
{
  union sym_value ret;
  ret = sym_lookup(name, sym_obj, sym_obj_nelts);
  if(ret.obj != NULL)
    return ret.obj;

  /*
   * If the implementation puts constants into the text segment, this
   * is where to find them!
   */
  ret = sym_lookup(name, sym_func, sym_func_nelts);
  return ret.obj;
}

/* Lookup a pointer to an ANSI C function. */
sym_func_t
sym_function(const char *name)
{
  return sym_lookup(name, sym_func, sym_func_nelts).func;
}
