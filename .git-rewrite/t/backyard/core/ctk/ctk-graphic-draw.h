/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ctk-graphic-draw.h,v 1.1 2007/05/23 23:19:13 oliverschmidt Exp $
 */
#ifndef __CTK_GRAPHIC_DRAW_H__
#define __CTK_GRAPHIC_DRAW_H__

enum {
  CTK_GRAPHIC_DRAW_FONT_NORMAL,
  CTK_GRAPHIC_DRAW_FONT_BOLD,
  CTK_GRAPHIC_DRAW_FONT_MENU,
  CTK_GRAPHIC_DRAW_FONT_MONOSPACE,
  CTK_GRAPHIC_DRAW_FONT_MONOSPACE_BOLD,  
};

enum {
  CTK_GRAPHIC_DRAW_COLOR_WHITE,
  CTK_GRAPHIC_DRAW_COLOR_LIGHTGRAY,
  CTK_GRAPHIC_DRAW_COLOR_MIDGRAY,
  CTK_GRAPHIC_DRAW_COLOR_DARKGRAY,
  CTK_GRAPHIC_DRAW_COLOR_BLACK,
  CTK_GRAPHIC_DRAW_COLOR_BLUE,  
}

/*struct ctk_graphic_draw_font;

extern struct ctk_graphic_draw_font ctk_graphic_draw_font_normal,
  ctk_graphic_draw_font_bold,
  ctk_graphic_draw_font_menu,
  ctk_graphic_draw_font_monospace,
  ctk_graphic_draw_font_monospace_bold;

struct ctk_graphic_draw_color;

extern struct ctk_graphic_draw_color ctk_graphic_draw_color_white,
  ctk_graphic_draw_color_lightgray,
  ctk_graphic_draw_color_midgray,
  ctk_graphic_draw_color_darkgray,
  ctk_graphic_draw_color_ddarkgray,
  ctk_graphic_draw_color_black,
  ctk_graphic_draw_color_blue;
*/

void ctk_graphic_draw_init(void);

int ctk_graphic_draw_string_width(unsigned char font,
				  char *str, unsigned char monospace);

void ctk_graphic_draw_string(unsigned char font,
			     unsigned char color,
			     unsigned int x,
			     unsigned int y,
			     char *str, 
			     unsigned char monospace);

void ctk_graphic_draw_line(unsigned char color,
			   unsigned int x1, unsigned int y1,
			   unsigned int x2, unsigned int y2);

void ctk_graphic_draw_rect(unsigned char color,
			   unsigned char filled,
			   unsigned int x1, unsigned int y1,
			   unsigned int width, unsigned int height);
			   

#endif /* __CTK_GRAPHIC_DRAW_H__ */
