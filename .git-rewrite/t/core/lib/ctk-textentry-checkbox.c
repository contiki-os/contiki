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
 * $Id: ctk-textentry-checkbox.c,v 1.1 2006/06/17 22:41:17 adamdunkels Exp $
 *
 */

#include "ctk/ctk.h"

#include "ctk-textentry-checkbox.h"

/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_textentry_checkbox_input(ctk_arch_key_t c,
			     struct ctk_textentry *t)
{
  switch(c) {
  case 'X':
  case 'x':
    *t->text = 'x';
    return 1;

  case ' ':
    *t->text = *t->text == 'x'? 0:'x';
    return 1;

  case CH_DEL:
  case CH_ENTER:
  case CTK_CONF_WIDGETDOWN_KEY:
  case CTK_CONF_WIDGETUP_KEY:
    return 0;
  }
  return 1;
}
/*-----------------------------------------------------------------------------------*/
