/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the "ctk" console GUI toolkit for cc65
 *
 * $Id: ctk-80col-asm.h,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

#ifndef __CTK_80COL_ASM_H__
#define __CTK_80COL_ASM_H__

void __fastcall__ ctk_80col_cclear(unsigned char len);
void __fastcall__ ctk_80col_chline(unsigned char len);
void __fastcall__ ctk_80col_cputc(unsigned char c);
void __fastcall__ ctk_80col_cputsn(unsigned char *str,
				      unsigned char len);

void ctk_80col_draw_buttonleft(void);
void ctk_80col_draw_buttonright(void);

struct ctk_80col_windowparams {
  unsigned char w;
  unsigned char h;
  unsigned char clipy1;
  unsigned char clipy2;
  unsigned char color1;
  unsigned char color2;
  unsigned char titlecolor;
  unsigned char titlelen;
  char *title;
};
extern struct ctk_80col_windowparams ctk_80col_windowparams;
void ctk_80col_draw_windowborders(void);

extern unsigned char ctk_80col_cursx,
  ctk_80col_cursy,
  ctk_80col_color,
  ctk_80col_reversed,
  ctk_80col_underline;


void __fastcall__ ctk_80col_draw_bitmapline(unsigned char len);

void __fastcall__ ctk_80col_clear_line(unsigned char len);

#endif /* __CTK_80COL_ASM_H__ */
