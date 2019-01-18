/**
 * \file
 * Header file for the experimental application level CTK textedit widget.
 * \author Adam Dunkels <adam@dunkels.com>
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
 * $Id: ctk-textedit.h,v 1.1 2007/11/20 20:41:54 oliverschmidt Exp $
 *
 */
#ifndef __CTK_TEXTEDIT_H__
#define __CTK_TEXTEDIT_H__

#include "ctk/ctk.h"

/**
 * Instantiating macro for the CTK textedit widget.
 *
 *
 * \param tx The x position of the widget.
 * \param ty The y position of the widget.
 * \param tw The width of the widget.
 * \param th The height of the widget.
 * \param ttext The text buffer to be edited.
 */
#define CTK_TEXTEDIT(tx, ty, tw, th, ttext) \
  {CTK_LABEL(tx, ty, tw, th, ttext)}, 0, 0
struct ctk_textedit {
  struct ctk_label label;
  unsigned char xpos, ypos;
};

void ctk_textedit_init(struct ctk_textedit *t);

void ctk_textedit_add(struct ctk_window *w,
		      struct ctk_textedit *t);

void ctk_textedit_eventhandler(struct ctk_textedit *t,
			       process_event_t s,
			       process_data_t data);          

#endif /* __CTK_TEXTEDIT_H__ */
