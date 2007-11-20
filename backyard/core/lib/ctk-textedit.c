/**
 * \file
 * An experimental CTK text edit widget.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This module contains an experimental CTK widget which is
 * implemented in the application process rather than in the CTK
 * process. The widget is instantiated in a similar fashion as other
 * CTK widgets, but is different from other widgets in that it
 * requires a signal handler function to be called by the process
 * signal handler function.
 * 
 */

/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment
 *
 * $Id: ctk-textedit.c,v 1.1 2007/11/20 20:41:54 oliverschmidt Exp $
 *
 */


#include "ctk-textedit.h"

#include <string.h>

/*-----------------------------------------------------------------------------------*/
void
ctk_textedit_init(struct ctk_textedit *t)
{
  t->xpos = t->ypos = 0;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Add a CTK textedit widget to a window.
 *
 * \param w A pointer to the window to which the entry is to be added.
 * \param t A pointer to the CTK textentry structure.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_textedit_add(struct ctk_window *w,
		 struct ctk_textedit *t)
{
  CTK_WIDGET_SET_FLAG(t, CTK_WIDGET_FLAG_MONOSPACE);
  CTK_WIDGET_ADD(w, t);
}
/*-----------------------------------------------------------------------------------*/
/**
 * The CTK textedit signal handler.
 *
 * This function must be called as part of the normal signal handler
 * of the process that contains the CTK textentry structure.
 *
 * \param t A pointer to the CTK textentry structure.
 * \param s The signal number.
 * \param data The signal data.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_textedit_eventhandler(struct ctk_textedit *t,
			  process_event_t s,
			  process_data_t data)
{
  char *textptr, *textptr2;
  unsigned char len;
  
  if(s == ctk_signal_keypress) {
    CTK_WIDGET_FOCUS(t->label.window, &t->label);
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr &= 0x7f;
    switch((ctk_arch_key_t)data) {
    case CH_CURS_DOWN:
      if(t->ypos < t->label.h - 1) {
	++t->ypos;
      }
      break; 
    case CH_CURS_UP:
      if(t->ypos > 0) {
	--t->ypos;
      }
      break; 
    case CH_CURS_RIGHT:
      len = strlen(&t->label.text[t->ypos * t->label.w]);
      if(t->xpos < len) {
	/*      if(t->xpos < t->label.w) {*/
	++t->xpos;
      } else {
	t->xpos = len;
      }
      break; 
    case CH_CURS_LEFT:
      if(t->xpos > 0) {
	--t->xpos;
      } else {
	if(t->ypos > 0) {
	  --t->ypos;
	  t->xpos = t->label.w - 1;
	}       
      }
      break;
    case CH_ENTER:
      t->xpos = 0;
      if(t->ypos < t->label.h - 1) {
	++t->ypos;
      }
      break;
    case CH_DEL:
      len = t->label.w - t->xpos;
      if(t->xpos > 0 && len > 0) {
	strncpy(textptr - 1, textptr,
		len);
	*(textptr + len - 1) = 0;
	--t->xpos;
      }
      break;      
    default:
      len = t->label.w - t->xpos;
      if(len > 0) {
	textptr2 = textptr + len - 1;
	while(textptr2 + 1 > textptr) {
	  *(textptr2 + 1) = *textptr2;
	  --textptr2;
	}
	
	*textptr = (char)data;
	++t->xpos;
	if(t->xpos == t->label.w) {
	  t->xpos = 0;
	  if(t->ypos < t->label.h - 1) {
	    ++t->ypos;
	  }
	}
      }
      break;
    }
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&t->label);
  } else if(s == ctk_signal_widget_activate &&
	    data == (process_data_t)t) {
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr &= 0x7f;
    t->xpos = 0;
    if(t->ypos < t->label.h - 1) {
      ++t->ypos;
    }
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&t->label);
  }
}
/*-----------------------------------------------------------------------------------*/
