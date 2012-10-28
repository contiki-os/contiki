/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 * This file is part of the Contiki operating system.
 *
 *
 */

#include <string.h>

#include "ctk/ctk.h"

#include "lib/ctk-textentry-multiline.h"

/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_textentry_multiline_input(ctk_arch_key_t c,
			      struct ctk_textentry *t)
{
  unsigned char len;

  switch(c) {
  case 0:
    t->xpos = (unsigned char)strlen(&t->text[t->ypos * (t->len + 1)]);
    if(t->xpos == t->len) {
      --t->xpos;
    }
    return 1;

  case CH_CURS_UP:
    if(t->ypos == 0) {
      t->xpos = 0;
      return 1;
    }
    --t->ypos;
    break;

  case CH_CURS_DOWN:
    if(t->ypos == t->h - 1) {
      t->xpos = (unsigned char)strlen(&t->text[t->ypos * (t->len + 1)]);
      if(t->xpos == t->len) {
	--t->xpos;
      }
      return 1;
    }
    ++t->ypos;
    break;

  case CH_ENTER:
    if(t->ypos == t->h - 1) {
      return 0;
    }
    ++t->ypos;
    t->xpos = 0;
    return 1;

  default:
    return 0;
  }

  len = (unsigned char)strlen(&t->text[t->ypos * (t->len + 1)]);
  if(t->xpos > len) {
    t->xpos = len;
  }
  return 1;
}
/*-----------------------------------------------------------------------------------*/
