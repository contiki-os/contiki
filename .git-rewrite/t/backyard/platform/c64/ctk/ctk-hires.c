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
 * $Id: ctk-hires.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

#include "contiki-version.h"

#include "ctk/ctk.h"
#include "ctk-draw.h"
#include "ctk-hires.h"
#include "ctk-hires-asm.h"

#include "ctk-hires-theme.h"

#include <string.h>
#include <ctype.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  40

#define SCREENADDR 0xdc00
#define HIRESADDR  0xe000

static unsigned char lineptr;

unsigned char ctk_hires_cursx, ctk_hires_cursy;
unsigned char ctk_hires_reversed;
unsigned char ctk_hires_color;
unsigned char ctk_hires_underline;

/*static unsigned char cchar;

static unsigned char tmp01;
static unsigned char tmph, tmpl, tmpborder;
static unsigned char *tmpptr;*/


static unsigned char x, y, i;
/*static unsigned char h;*/
static unsigned char wfocus;
static unsigned char x1, y1, x2, y2;

struct ctk_hires_windowparams ctk_hires_windowparams;
unsigned char *ctk_hires_bitmapptr;

unsigned char ctk_draw_windowborder_height = 1;
unsigned char ctk_draw_windowborder_width = 1;
unsigned char ctk_draw_windowtitle_height = 1;

/*-----------------------------------------------------------------------------------*/
/* Tables. */

unsigned short ctk_hires_yscreenaddr[25] =
  {0 * SCREEN_WIDTH + SCREENADDR, 1 * SCREEN_WIDTH + SCREENADDR,
   2 * SCREEN_WIDTH + SCREENADDR, 3 * SCREEN_WIDTH + SCREENADDR,
   4 * SCREEN_WIDTH + SCREENADDR, 5 * SCREEN_WIDTH + SCREENADDR,
   6 * SCREEN_WIDTH + SCREENADDR, 7 * SCREEN_WIDTH + SCREENADDR,
   8 * SCREEN_WIDTH + SCREENADDR, 9 * SCREEN_WIDTH + SCREENADDR,
   10 * SCREEN_WIDTH + SCREENADDR, 11 * SCREEN_WIDTH + SCREENADDR,
   12 * SCREEN_WIDTH + SCREENADDR, 13 * SCREEN_WIDTH + SCREENADDR,
   14 * SCREEN_WIDTH + SCREENADDR, 15 * SCREEN_WIDTH + SCREENADDR,
   16 * SCREEN_WIDTH + SCREENADDR, 17 * SCREEN_WIDTH + SCREENADDR,
   18 * SCREEN_WIDTH + SCREENADDR, 19 * SCREEN_WIDTH + SCREENADDR,
   20 * SCREEN_WIDTH + SCREENADDR, 21 * SCREEN_WIDTH + SCREENADDR,
   22 * SCREEN_WIDTH + SCREENADDR, 23 * SCREEN_WIDTH + SCREENADDR,
   24 * SCREEN_WIDTH + SCREENADDR};

unsigned short ctk_hires_yhiresaddr[25] =
  {0 * 320 + HIRESADDR, 1 * 320 + HIRESADDR,
   2 * 320 + HIRESADDR, 3 * 320 + HIRESADDR,
   4 * 320 + HIRESADDR, 5 * 320 + HIRESADDR,
   6 * 320 + HIRESADDR, 7 * 320 + HIRESADDR,
   8 * 320 + HIRESADDR, 9 * 320 + HIRESADDR,
   10 * 320 + HIRESADDR, 11 * 320 + HIRESADDR,
   12 * 320 + HIRESADDR, 13 * 320 + HIRESADDR,
   14 * 320 + HIRESADDR, 15 * 320 + HIRESADDR,
   16 * 320 + HIRESADDR, 17 * 320 + HIRESADDR,
   18 * 320 + HIRESADDR, 19 * 320 + HIRESADDR,
   20 * 320 + HIRESADDR, 21 * 320 + HIRESADDR,
   22 * 320 + HIRESADDR, 23 * 320 + HIRESADDR,
   24 * 320 + HIRESADDR};
extern struct ctk_hires_theme ctk_hires_theme;
struct ctk_hires_theme *ctk_hires_theme_ptr = &ctk_hires_theme;
/*-----------------------------------------------------------------------------------*/
#define hires_wherex() ctk_hires_cursx
#define hires_revers(c)   ctk_hires_reversed = c
#define hires_color(c)   ctk_hires_color = c
#define hires_underline(c)   ctk_hires_underline = c
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_cvline(unsigned char length)
{
  static unsigned char i;
  
  for(i = 0; i < length; ++i) {
    ctk_hires_cputc('|');
    --ctk_hires_cursx;
    ++ctk_hires_cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_gotoxy(unsigned char x, unsigned char y)
{
  ctk_hires_cursx = x;
  ctk_hires_cursy = y;
}
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  ctk_hires_cclear(length);
}
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  ctk_hires_chline(length);
}
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  hires_gotoxy(x, y);
  hires_cvline(length);
}
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
hires_cputcxy(unsigned char x, unsigned char y, char c)
{
  hires_gotoxy(x, y);
  ctk_hires_cputc(c);
}
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void 
clear_line(unsigned char line)
{
  lineptr = line;
  asm("lda %v", lineptr);
  asm("asl");
  asm("tax");
  asm("lda %v,x", ctk_hires_yhiresaddr);
  asm("sta ptr2");
  asm("lda %v+1,x", ctk_hires_yhiresaddr);
  asm("sta ptr2+1");
  asm("lda %v,x", ctk_hires_yscreenaddr);
  asm("sta ptr1");
  asm("lda %v+1,x", ctk_hires_yscreenaddr);
  asm("sta ptr1+1");

  
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldy #39");
  asm("ldx %v", lineptr);
  asm("lda %v+%w,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpatterncolors));
  asm("clearlineloop1:");
  asm("sta (ptr1),y");
  asm("dey");
  asm("bpl clearlineloop1");
  asm("pla");
  asm("sta $01");
  asm("cli");


  asm("lda %v", lineptr);
  /*  asm("and #7");*/
  asm("asl");
  asm("asl");
  asm("asl");
  asm("tax");
  asm("ldy #0");
  asm("clearlineloop2:");
  asm("lda %v+%w+0,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+1,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+2,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+3,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+4,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+5,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+6,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+7,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("bne clearlineloop2");
  
  asm("inc ptr2+1");

  asm("ldy #0");
  asm("clearlineloop3:");
  asm("lda %v+%w+0,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+1,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+2,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+3,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+4,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+5,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+6,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("lda %v+%w+7,x", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, backgroundpattern));
  asm("sta (ptr2),y");
  asm("iny");
  asm("cpy #$40");
  asm("bne clearlineloop3");

  
  if(lineptr == 24) {
    hires_color(ctk_hires_theme.backgroundpatterncolors[24]);
    hires_gotoxy(0, 24);
    ctk_hires_cputsn(CONTIKI_VERSION_STRING,
		     sizeof(CONTIKI_VERSION_STRING) - 1);
  }
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
nmi2(void)
{
  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("rti");
}  
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
nmi(void)
{
  asm("sei");
  asm("pha");
  asm("inc $d020");
  asm("lda $01");
  asm("pha");
  asm("lda #$36");
  asm("sta $01");
  asm("lda #>_nmi2");
  asm("pha");
  asm("lda #<_nmi2");
  asm("pha");
  asm("php");
  asm("jmp ($0318)");

  nmi2();
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
setup_nmi(void)
{
  asm("lda #<_nmi");
  asm("sta $fffa");
  asm("lda #>_nmi");
  asm("sta $fffb");
  return;
  nmi();
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
ctk_draw_init(void)
{
  unsigned char i, *ptr1, *ptr2;

  
  setup_nmi();
  
  /* Turn on hires mode, bank 0 ($c000 - $ffff) and $e000/$c000 for
     hires/colors. */
  VIC.ctrl1 = 0x3b;  /* $D011 */
  VIC.addr  = 0x78;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x00;  /* $DD00 */

  VIC.bordercolor = ctk_hires_theme.bordercolor; /* $D020 */
  VIC.bgcolor0 = ctk_hires_theme.screencolor; /* $D021 */  

  /* Fill color memory. */
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldx #0");
  asm("lda #$c0");
  asm("fillcolorloop:");
  asm("sta $dc00,x");
  asm("sta $dd00,x");
  asm("sta $de00,x");
  asm("sta $df00,x");
  asm("inx");
  asm("bne fillcolorloop");

  /* Setup sprite pointers */
  asm("ldx #$fd");
  asm("stx $dff8");
  asm("inx");
  asm("stx $dff9");
  asm("pla");
  asm("sta $01");
  asm("cli");

  /* Fill hires memory with 0. */

  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");
  asm("lda #0");
  asm("sta $fd");
  asm("lda #$e0");
  asm("sta $fe");
  asm("ldy #0");
  asm("lda #0");
  asm("clrscrnloop:");
  asm("lda #$55");
  asm("sta ($fd),y");
  asm("iny");
  asm("lda #$aa");
  asm("sta ($fd),y");
  asm("iny");
  asm("bne clrscrnloop");
  asm("inc $fe");
  asm("lda $fe");
  asm("cmp #$ff");
  asm("bne clrscrnloop");

  asm("ldy #$00");
  asm("clrscrnloop2:");
  asm("lda #$55");
  asm("sta $ff00,y");
  asm("iny");
  asm("lda #$aa");
  asm("sta $ff00,y");
  asm("iny");
  asm("cpy #$40");
  asm("bne clrscrnloop2");

  
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");

  
  ctk_draw_clear(0, SCREEN_HEIGHT);

  /* Setup mouse pointer sprite. */
  asm("lda %v+%w", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, pointermaskcolor));
  asm("sta $d027");
  asm("lda %v+%w", ctk_hires_theme,
      offsetof(struct ctk_hires_theme, pointercolor));
  asm("sta $d028");

  ptr1 = ctk_hires_theme.pointer;
  ptr2 = (unsigned char *)0xff40;
  
  for(i = 0; i < 0x80; ++i) {
    *ptr2++ = *ptr1++;
  }

  return;
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
static void __fastcall__
draw_widget(register struct ctk_widget *w,
	    unsigned char x, unsigned char y,
	    unsigned char clipy1, unsigned char clipy2,
	    unsigned char afocus)
{
  static unsigned char xpos, ypos, xscroll;
  static unsigned char i;
  static char c;
  static unsigned char len;
  static unsigned char tmp;
  static unsigned char yclipped;
  static unsigned char focus;
  char *text;
  
  xpos = x + w->x;
  ypos = y + w->y;  

  yclipped = 0;  
  if(ypos >= clipy1 && ypos < clipy2) {
    yclipped = 1;
  }
  focus = afocus;
  
  
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    hires_color(ctk_hires_theme.separatorcolors[focus]);
    if(yclipped) {
      hires_chlinexy(xpos, ypos, w->w);
    }
    break;
  case CTK_WIDGET_LABEL:
    hires_color(ctk_hires_theme.labelcolors[focus]);
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	hires_gotoxy(xpos, ypos);
	ctk_hires_cputsn(text, w->w);
	tmp = w->w - (hires_wherex() - xpos);
	if(tmp > 0) {
	  ctk_hires_cclear(tmp);
	}
      }
      ++ypos;
      text += w->w;
    }
    break;
  case CTK_WIDGET_BUTTON:
    if(yclipped) {
      hires_color(ctk_hires_theme.buttonleftcolors[focus]);
      hires_gotoxy(xpos, ypos);
      ctk_hires_draw_buttonleft();
      hires_color(ctk_hires_theme.buttoncolors[focus]);
      hires_gotoxy(xpos + 1, ypos);
      ctk_hires_cputsn(w->widget.button.text, w->w);
      hires_color(ctk_hires_theme.buttonrightcolors[focus]);
      ctk_hires_draw_buttonright();
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(yclipped) {
      hires_color(ctk_hires_theme.hyperlinkcolors[focus]);
      hires_underline(1);
      hires_gotoxy(xpos, ypos);
      ctk_hires_cputsn(w->widget.button.text, w->w);
      hires_underline(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    if(yclipped) {
      hires_color(ctk_hires_theme.textentrycolors[focus]);
      
      if((focus & CTK_FOCUS_WIDGET) &&
	 w->widget.textentry.state != CTK_TEXTENTRY_EDIT) {
	hires_revers(1);
      } else {
	hires_revers(0);
      }
      xscroll = 0;
      tmp = w->w - 1;
      if(w->widget.textentry.xpos >= tmp) {
	xscroll = w->widget.textentry.xpos - tmp;
      }
      text = w->widget.textentry.text;
      if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT) {
	hires_revers(0);
	hires_cputcxy(xpos, ypos, '>');
	text += xscroll;
	for(i = 0; i < w->w; ++i) {
	  c = *text;
	  if(i == w->widget.textentry.xpos - xscroll) {
	    hires_revers(1);
	  } else {
	    hires_revers(0);
	  }
	  if(c == 0) {
	    ctk_hires_cputc(' ');
	  } else {
	    ctk_hires_cputc(c);
	  }
	  hires_revers(0);
	  ++text;
	}
	ctk_hires_cputc('<');
      } else {
	hires_cputcxy(xpos, ypos, '|');
	/*	  hires_gotoxy(xpos + 1, ypos);          */
	ctk_hires_cputsn(text, w->w);
	i = hires_wherex();
	tmp = i - xpos - 1;
	if(tmp < w->w) {
	  ctk_hires_cclear(w->w - tmp);
	}
	ctk_hires_cputc('|');
      }
    }
    hires_revers(0);
    break;
  case CTK_WIDGET_ICON:
    if(yclipped) {
      hires_color(ctk_hires_theme.iconcolors[focus]);

      x = xpos;
      len = strlen(w->widget.icon.title);
      if(x + len >= SCREEN_WIDTH) {
	x = SCREEN_WIDTH - len;
      }

      tmp = ypos + 3;
	
      if(tmp < clipy2) {
	hires_gotoxy(x, tmp);
	ctk_hires_cputsn(w->widget.icon.title, len);
      }

      hires_gotoxy(xpos, ypos);
      if(w->widget.icon.bitmap != NULL) {
	ctk_hires_bitmapptr = w->widget.icon.bitmap;
	for(i = 0; i < 3; ++i) {
	  if(ypos >= clipy1 && ypos < clipy2) {
	    hires_gotoxy(xpos, ypos);
	    ctk_hires_draw_bitmapline(3);
	  }
	  ctk_hires_bitmapptr += 3 * 8;
	  ++ypos;
	}
	
	/*	draw_bitmap_icon(w->widget.icon.bitmap);*/
      }

    }
    break;
  case CTK_WIDGET_BITMAP:
    hires_color(ctk_hires_theme.bitmapcolors[focus]);
    ctk_hires_bitmapptr = w->widget.bitmap.bitmap;
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	hires_gotoxy(xpos, ypos);
	ctk_hires_draw_bitmapline(w->w);
      }
      ctk_hires_bitmapptr += w->w * 8;
      ++ypos;
    }
    break;

  default:
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *w,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_window *win = w->window;
  unsigned char posx, posy;

  posx = win->x + 1;
  posy = win->y + 2;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy,
	      clipy1, clipy2,
	      focus);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear_window(register struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  static unsigned char h;

  hires_color(ctk_hires_theme.windowcolors[focus]);
  
  h = window->y + 2 + window->h;
  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i <= clipy2) {
      hires_cclearxy(window->x + 1, i, window->w);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_window(register struct ctk_window *window,
		unsigned char focus,
		unsigned char clipy1, unsigned char clipy2,
		unsigned char draw_borders)
{
  register struct ctk_widget *w;

  x = window->x;
  y = window->y + 1;
  
  ++clipy2;
  
  if(clipy2 <= y) {
    return;
  }
  
  /*  hires_color(ctk_hires_theme.windowcolors[focus+1]);*/
  
  x1 = x + 1;
  y1 = y + 1;
  /*  x2 = x1 + window->w;
      y2 = y1 + window->h;*/
    
  hires_gotoxy(x, y);
  ctk_hires_windowparams.w = window->w;
  ctk_hires_windowparams.h = window->h;
  if(clipy1 < y) {
    ctk_hires_windowparams.clipy1 = 0;
  } else {
    ctk_hires_windowparams.clipy1 = clipy1 - y;
  }
  ctk_hires_windowparams.clipy2 = clipy2 - y;
  ctk_hires_windowparams.color1 = ctk_hires_theme.windowcolors[focus+1];
  ctk_hires_windowparams.color2 = ctk_hires_theme.windowcolors[focus];
  ctk_hires_windowparams.title = window->title;
  ctk_hires_windowparams.titlelen = window->titlelen;

  if(ctk_hires_windowparams.clipy1 < ctk_hires_windowparams.clipy2 &&
     ctk_hires_windowparams.clipy2 > 0) {
    ctk_hires_draw_windowborders();
  }
  
  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1,
		clipy1, clipy2,
		focus);
  }
  
  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {  
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x1, y1,
		clipy1, clipy2,
		wfocus);
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_dialog(register struct ctk_window *dialog)
{
  register struct ctk_widget *w;

  hires_color(ctk_hires_theme.windowcolors[CTK_FOCUS_DIALOG]);

  /*  x = (SCREEN_WIDTH - dialog->w) / 2;
      y = (SCREEN_HEIGHT - 1 - dialog->h) / 2; */
  x = dialog->x;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  hires_cvlinexy(x, y1, dialog->h);
  hires_cvlinexy(x2, y1, dialog->h);

  hires_chlinexy(x1, y, dialog->w);
  hires_chlinexy(x1, y2, dialog->w);

  hires_cputcxy(x, y, CH_ULCORNER);
  hires_cputcxy(x, y2, CH_LLCORNER);
  hires_cputcxy(x2, y, CH_URCORNER);
  hires_cputcxy(x2, y2, CH_LRCORNER);
  
  
  /* Clear window contents. */
  for(i = y1; i < y2; ++i) {
    hires_cclearxy(x1, i, dialog->w);
  }
  
  /* Draw inactive widgets. */
  for(w = dialog->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1,
		0, SCREEN_HEIGHT, CTK_FOCUS_DIALOG);
  }


  /* Draw active widgets. */
  for(w = dialog->active; w != NULL; w = w->next) {
    wfocus = CTK_FOCUS_DIALOG;
    if(w == dialog->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x1, y1, 
		0, SCREEN_HEIGHT, wfocus);
  }

}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  for(i = y1; i < y2; ++i) {
    clear_line(i);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(register struct ctk_menu *m)
{
  static unsigned char x, x2, y;
  
  hires_color(ctk_hires_theme.openmenucolor);
  x = hires_wherex();
  ctk_hires_cputsn(m->title, m->titlelen);
  ctk_hires_cputc(' ');
  x2 = hires_wherex();
  if(x + CTK_CONF_MENUWIDTH > SCREEN_WIDTH) {
    x = SCREEN_WIDTH - CTK_CONF_MENUWIDTH;
  }      
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      hires_color(ctk_hires_theme.activemenucolor);
    } else {
      hires_color(ctk_hires_theme.menucolor);
    }
    hires_gotoxy(x, y + 1);
    if(m->items[y].title[0] == '-') {
      ctk_hires_chline(CTK_CONF_MENUWIDTH);
    } else {
      ctk_hires_cputsn(m->items[y].title,
			  strlen(m->items[y].title));
    }
    ctk_hires_cclear(x + CTK_CONF_MENUWIDTH - hires_wherex());
    hires_revers(0);
  }
  hires_gotoxy(x2, 0);
  hires_color(ctk_hires_theme.menucolor);  
}
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;
  
  /* Draw menus */
  hires_color(ctk_hires_theme.menucolor); 
  hires_gotoxy(0, 0);
  hires_revers(0);
  ctk_hires_cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
      ctk_hires_cputsn(m->title, m->titlelen);
      ctk_hires_cputc(' ');
    } else {     
      draw_menu(m);
    }
  }
  ctk_hires_cclear(SCREEN_WIDTH - hires_wherex() -
		      strlen(menus->desktopmenu->title) - 1);
  
  /* Draw desktopmenu */  
  if(menus->desktopmenu != menus->open) {
    ctk_hires_cputsn(menus->desktopmenu->title,
			menus->desktopmenu->titlelen);
    ctk_hires_cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  return SCREEN_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  return SCREEN_WIDTH;
}
/*-----------------------------------------------------------------------------------*/
/*unsigned char
ctk_arch_isprint(char c)
{
  return isprint(c);
}*/
