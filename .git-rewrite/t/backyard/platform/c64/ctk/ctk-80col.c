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
 * $Id: ctk-80col.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 */

#include "contiki.h"

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "ctk/ctk-draw-service.h"

#include "ctk-80col-asm.h"
#include "ctk-80col-theme.h"

#include <string.h>

unsigned char ctk_80col_cursx, ctk_80col_cursy;
unsigned char ctk_80col_reversed;
unsigned char ctk_80col_color;
unsigned char *ctk_80col_bitmapptr;
unsigned char ctk_80col_underline = 0;

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  80

#define SCREENADDR 0xdc00
#define HIRESADDR  0xe000

unsigned char ctk_80col_lefttab[256];
unsigned char ctk_80col_righttab[256];

#define COLOR(bg, fg) ((fg << 4) | (bg))

#define COLOR_DIALOG            0x01
#define COLOR_FOCUS_WINDOW      0xbf
#define COLOR_BACKGROUND_WINDOW 0x0c

#define color(c) ctk_80col_color = c

#define BGCOLOR1 7
#define BGCOLOR2 10
#define BGCOLOR3 4
#define BGCOLOR4 6

unsigned char ctk_80col_screencolors[25] =
  {COLOR(BGCOLOR4,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR2,BGCOLOR1),COLOR(BGCOLOR2,BGCOLOR1),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR3,BGCOLOR2),COLOR(BGCOLOR3,BGCOLOR2),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,BGCOLOR3),COLOR(BGCOLOR4,BGCOLOR3),
   COLOR(BGCOLOR4,1)};

unsigned char ctk_80col_screenpattern[25*8] =
  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xdd,0xff,0x77,0xff,0xdd,0xff,0x77,
   0xff,0x55,0xff,0x55,0xff,0x55,0xff,0x55,
   0xee,0x55,0xbb,0x55,0xee,0x55,0xbb,0x55,
   0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,
   0xaa,0x44,0xaa,0x11,0xaa,0x44,0xaa,0x11,
   0xaa,0x00,0xaa,0x00,0xaa,0x00,0xaa,0x00,
   0x88,0x00,0x22,0x00,0x88,0x00,0x22,0x00,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xdd,0xff,0x77,0xff,0xdd,0xff,0x77,
   0xff,0x55,0xff,0x55,0xff,0x55,0xff,0x55,
   0xee,0x55,0xbb,0x55,0xee,0x55,0xbb,0x55,
   0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,
   0xaa,0x44,0xaa,0x11,0xaa,0x44,0xaa,0x11,
   0xaa,0x00,0xaa,0x00,0xaa,0x00,0xaa,0x00,
   0x88,0x00,0x22,0x00,0x88,0x00,0x22,0x00,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xdd,0xff,0x77,0xff,0xdd,0xff,0x77,
   0xff,0x55,0xff,0x55,0xff,0x55,0xff,0x55,
   0xee,0x55,0xbb,0x55,0xee,0x55,0xbb,0x55,
   0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,
   0xaa,0x44,0xaa,0x11,0xaa,0x44,0xaa,0x11,
   0xaa,0x00,0xaa,0x00,0xaa,0x00,0xaa,0x00,
   0x88,0x00,0x22,0x00,0x88,0x00,0x22,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned short ctk_80col_yscreenaddr[25] =
  {0 * 40 + SCREENADDR, 1 * 40 + SCREENADDR,
   2 * 40 + SCREENADDR, 3 * 40 + SCREENADDR,
   4 * 40 + SCREENADDR, 5 * 40 + SCREENADDR,
   6 * 40 + SCREENADDR, 7 * 40 + SCREENADDR,
   8 * 40 + SCREENADDR, 9 * 40 + SCREENADDR,
   10 * 40 + SCREENADDR, 11 * 40 + SCREENADDR,
   12 * 40 + SCREENADDR, 13 * 40 + SCREENADDR,
   14 * 40 + SCREENADDR, 15 * 40 + SCREENADDR,
   16 * 40 + SCREENADDR, 17 * 40 + SCREENADDR,
   18 * 40 + SCREENADDR, 19 * 40 + SCREENADDR,
   20 * 40 + SCREENADDR, 21 * 40 + SCREENADDR,
   22 * 40 + SCREENADDR, 23 * 40 + SCREENADDR,
   24 * 40 + SCREENADDR};

unsigned short ctk_80col_yhiresaddr[25] =
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


struct ctk_80col_theme ctk_80col_theme =
  {
    /* Version string. */
    /*    char version[8]; */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

    /* Window borders patterns. */
    /* unsigned char ulcorner[8], */ /* Upper left corner. */
    {0xff,0xc0,0x80,0xbf,0x80,0xbf,0x80,0x80},
    
    /* titlebar[8], */            /* Title bar pattern. */
    {0xff,0x00,0x00,0xff,0x00,0xff,0x00,0x00},
    
    /* urcorner[8],       */       /* Upper right corner. */
    {0xff,0x03,0x01,0xfd,0x01,0xfd,0x01,0x01},
    
    /* rightborder[8],     */      /* Right border. */
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
    
    /* lrcorner[8],      */        /* Lower right corner. */
    {0x01,0x01,0x01,0x01,0x01,0x01,0x03,0xff},
    
    /* lowerborder[8], */          /* Lower border. */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},
    
    /* llcorner[8],  */            /* Lower left corner. */
    {0x80,0x80,0x80,0x80,0x80,0x80,0xc0,0xff},
    
    /* leftborder[8]; */          /* Left border. */
    {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},

    /* Button corner patterns. */
    /*    unsigned char buttonleft[8], */
    {0xc5,0xdf,0xdf,0xdf,0xdf,0xdf,0xdf,0xdf},
    
    /* buttonright[8]; */
    {0xfb,0xfb,0xfb,0xfb,0xfb,0xfb,0xfb,0xa3},

      /* Menu border patterns. */
    /*    unsigned char menuleftpattern[8], */
    {0x0f,0x3f,0x3f,0x7f,0x7f,0xff,0xff,0xff},
    
    /* menurightpatterns[8]; */
    {0xf0,0xfc,0xfc,0xfe,0xfe,0xff,0xff,0xff},
    
    /* Window and widget colors. */
    /*    unsigned char windowcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_BLACK),
     COLOR(COLOR_GRAY2, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_GRAY1),
     COLOR(COLOR_GRAY3, COLOR_GRAY1),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK)},
    
    /* separatorcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY3, COLOR_GRAY2),
     COLOR(COLOR_GRAY3, COLOR_GRAY2),
     COLOR(COLOR_WHITE, COLOR_GRAY1),
     COLOR(COLOR_WHITE, COLOR_GRAY1)},
    
    /* labelcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK)},
    
    /* buttoncolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_GRAY2),
     COLOR(COLOR_GRAY3, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_GRAY3),
     COLOR(COLOR_GRAY3, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_GRAY3)},
    
    /* hyperlinkcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
     COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
     COLOR(COLOR_WHITE, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_WHITE),
     COLOR(COLOR_WHITE, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_WHITE)},
    
    /* textentrycolors[6],  */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY2, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK)},
    
    /* bitmapcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_GRAY1),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK)},
    
    /* textmapcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK)},
    
    /* iconcolors[6]; */
    {COLOR(COLOR_GRAY3, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_GRAY2),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_WHITE),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_BLACK, COLOR_YELLOW)},
    
    /* Menu colors. */
    /*    unsigned char menucolor,*/
    COLOR(COLOR_YELLOW, COLOR_BLACK),
    
    /*    openmenucolor, */
    COLOR(COLOR_WHITE, COLOR_BLACK),
    
    /* activemenucolor; */
    COLOR(COLOR_BLACK, COLOR_WHITE),
    
  };
char ctk_80col_versionstring[] = CONTIKI_VERSION_STRING;
char ctk_80col_versionstring_len = sizeof(CONTIKI_VERSION_STRING) - 1;

struct ctk_80col_windowparams ctk_80col_windowparams;

/*---------------------------------------------------------------------------*/
/*void
ctk_arch_draw_char(char c,
		   unsigned char xpos,
		   unsigned char ypos,
		   unsigned char reversedflag,
		   unsigned char color)
{
  ctk_80col_cursx = xpos;
  ctk_80col_cursy = ypos;
  ctk_80col_reversed = reversedflag;
  ctk_80col_color = color;
  
  ctk_80col_cputc(c);
}*/
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
void reset(void);
void
quit(void)
{
  VIC.ctrl1 = 0x1b;  /* $D011 */
  VIC.addr  = 0x17;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x03;  /* $DD00 */

  VIC.bordercolor = 0x0e; /* $D020 */
  VIC.bgcolor0 = 0x06; /* $D021 */

  memset((char *)0xd800, 0x0e, 40*25);

}
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
ctk_80col_init(void)
{
  int i;
  
  setup_nmi();
  
  /* Turn on hires mode, bank 0 ($c000 - $ffff) and $e000/$c000 for
     hires/colors. */
  VIC.ctrl1 = 0x3b;  /* $D011 */
  VIC.addr  = 0x78;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x00;  /* $DD00 */

  VIC.bordercolor = 0x06; /* $D020 */
  VIC.bgcolor0 = 0x0b; /* $D021 */  

  /* Fill color memory. */
  asm("sei");
  asm("lda $01");
  asm("pha");
  asm("lda #$30");
  asm("sta $01");
  asm("ldx #0");
  asm("lda #$0");
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

  memset((char *)0xe000, 0, 8000);

  for(i = 0; i < 256; ++i) {
#if 0
    ctk_80col_lefttab[i] =
      ((i & 0x40) << 1) |
      ((i & 0x20) << 1) |
      ((i & 0x08) << 2) |
      ((i & 0x02) << 3);
    ctk_80col_righttab[i] =
      ((i & 0x40) >> 3) |
      ((i & 0x20) >> 3) |
      ((i & 0x08) >> 2) |
      ((i & 0x02) >> 1);
#else
    ctk_80col_lefttab[i] =
      ((i & 0x40) << 1) |
      ((i & 0x10) << 2) |
      ((i & 0x04) << 3) |
      ((i & 0x01) << 4);
    ctk_80col_righttab[i] =
      ((i & 0x40) >> 3) |
      ((i & 0x10) >> 2) |
      ((i & 0x04) >> 1) |
      ((i & 0x01));
#endif
  }
  
#if 0
  /* Setup mouse pointer sprite. */
  asm("lda %v+%w", ctk_80col_theme,
      offsetof(struct ctk_80col_theme, pointermaskcolor));
  asm("sta $d027");
  asm("lda %v+%w", ctk_80col_theme,
      offsetof(struct ctk_80col_theme, pointercolor));
  asm("sta $d028");

  ptr1 = ctk_80col_theme.pointer;
  ptr2 = (unsigned char *)0xff40;
  
  for(i = 0; i < 0x80; ++i) {
    *ptr2++ = *ptr1++;
  }
#endif
  return;
}
#pragma optimize(pop)
/*---------------------------------------------------------------------------*/
/*static unsigned char cursx, cursy;
  static unsigned char reversed;*/

/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL 
cputc(char c) 
{
  /*  ctk_arch_draw_char(c, cursx, cursy, reversed, 0);*/
  ctk_80col_cputc(c);
  /*  ++cursx;*/
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  return ctk_80col_cursx;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  return ctk_80col_cursy;
}
/*-----------------------------------------------------------------------------------*/
/*void
clrscr(void)
{
  unsigned char x, y;

  for(x = 0; x < SCREEN_WIDTH; ++x) {
    for(y = 0; y < SCREEN_HEIGHT; ++y) {
      gotoxy(x, y);
      cputc(' ');
    }
  }
}*/
/*-----------------------------------------------------------------------------------*/
#define revers(c) ctk_80col_reversed = c
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL 
_cputs(char *str)
{
  char *ptr = str;
  
  while(*ptr != 0) {
    cputc(*ptr++);
  }

  /*  int i;
  for(i = 0; i < strlen(str); ++i) {
    cputc(str[i]);
    }*/
}
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL
cclear(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc(' ');
  } 
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
chline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('-');
  }
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cvline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('|');
    --ctk_80col_cursx;
    ++ctk_80col_cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
gotoxy(unsigned char x, unsigned char y)
{
  ctk_80col_cursx = x;
  ctk_80col_cursy = y;
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cclear(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  chline(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cvline(length);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
_cputsxy(unsigned char x, unsigned char y, char *str)
{
  gotoxy(x, y);
  _cputs(str);
}
/*-----------------------------------------------------------------------------------*/
void CC_FASTCALL
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  cputc(c);
}
/*-----------------------------------------------------------------------------------*/
/*void CC_FASTCALL
screensize(unsigned char *x, unsigned char *y)
{
  *x = SCREEN_WIDTH;
  *y = SCREEN_HEIGHT;
}*/
/*-----------------------------------------------------------------------------------*/
/*static unsigned char sizex, sizey;*/
/*-----------------------------------------------------------------------------------*/
static void
_cputsn(char *str, unsigned char len)
{
  /*  char c;

  while(len > 0) {
    --len;
    c = *str;
    if(c == 0) {
      break;
    }
    cputc(c);
    ++str;
    }*/  
  ctk_80col_cputsn(str, len);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_init(void)
{
  ctk_80col_init();

  /*  screensize(&sizex, &sizey);*/
  
  ctk_draw_clear(0, SCREEN_HEIGHT);
}
/*-----------------------------------------------------------------------------------*/
static void
draw_widget(struct ctk_widget *w,
	    unsigned char x, unsigned char y,
	    unsigned char clipx,
	    unsigned char clipy,
	    unsigned char clipy1, unsigned char clipy2,
	    unsigned char focus)
{
  unsigned char xpos, ypos, xscroll;
  unsigned char i, j;
  char c, *text;
  unsigned char len, wfocus;

  wfocus = 0;
  if(focus & CTK_FOCUS_WINDOW) {    
    if(focus & CTK_FOCUS_WIDGET) {
      wfocus = 1;
    }
  } else if(focus & CTK_FOCUS_DIALOG) {
    if(focus & CTK_FOCUS_WIDGET) {
      wfocus = 1;
    }
  } else {
  }
  
  xpos = x + w->x;
  ypos = y + w->y;
    
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    if(ypos >= clipy1 && ypos < clipy2) {
      chlinexy(xpos, ypos, w->w);
    }
    break;
  case CTK_WIDGET_LABEL:
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	gotoxy(xpos, ypos);
	_cputsn(text, w->w);
	if(w->w - (wherex() - xpos) > 0) {
	  cclear(w->w - (wherex() - xpos));
	}
      }
      ++ypos;
      text += w->w;
    }
    break;
  case CTK_WIDGET_BUTTON:
    if(ypos >= clipy1 && ypos < clipy2) {
      if(wfocus != 0) {
	revers(1);
      } else {
	revers(0);
      }
      cputcxy(xpos, ypos, '[');
      _cputsn(w->widget.button.text, w->w);
      cputc(']');
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(ypos >= clipy1 && ypos < clipy2) {
      if(wfocus != 0) {
	revers(0);
      } else {
	revers(1);
      }
      gotoxy(xpos, ypos);
      _cputsn(w->widget.button.text, w->w);
      revers(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    text = w->widget.textentry.text;
    xscroll = 0;
    if(w->widget.textentry.xpos >= w->w - 1) {
      xscroll = w->widget.textentry.xpos - w->w + 1;
    }
    for(j = 0; j < w->h; ++j) {
      if(ypos >= clipy1 && ypos < clipy2) {
	if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	   w->widget.textentry.ypos == j) {
	  revers(0);
	  cputcxy(xpos, ypos, '>');
	  c = 1;
	  for(i = 0; i < w->w; ++i) {
	    if(c != 0) {
	      c = text[i + xscroll];
	    }
	    if(i == w->widget.textentry.xpos - xscroll) {
	      revers(1);
	    } else {
	      revers(0);
	    }
	    if(c == 0) {
	      cputc(' ');
	    } else {
	      cputc(c);
	    }
	    revers(0);
	  }
	  cputc('<');
	} else {
	  if(wfocus != 0 && j == w->widget.textentry.ypos) {
	    revers(1);
	  } else {
	    revers(0);
	  }
	  cvlinexy(xpos, ypos, 1);
	  gotoxy(xpos + 1, ypos);          
	  _cputsn(text, w->w);
	  i = wherex();
	  if(i - xpos - 1 < w->w) {
	    cclear(w->w - (i - xpos) + 1);
	  }
	  cvline(1);
	}
      }
      ++ypos;
      text += w->widget.textentry.len + 1;
    }
    revers(0);
    break;
  case CTK_WIDGET_ICON:
    if(ypos >= clipy1 && ypos < clipy2) {
      color(ctk_80col_theme.iconcolors[focus]);
      
      /*      if(focus & 1) {
	
	revers(1);
      } else {
	revers(0);
	}*/
      /*      gotoxy(xpos, ypos);*/
      if(xpos >= 73) {
	xpos = 73;
      }
      if(xpos <= 2) {
	xpos = 2;
      }
      if(w->widget.icon.textmap != NULL) {
	ctk_80col_bitmapptr = w->widget.icon.bitmap;
	for(i = 0; i < 3; ++i) {
	  if(ypos >= clipy1 && ypos < clipy2) {
	    gotoxy(xpos, ypos);
	    ctk_80col_draw_bitmapline(3);	    
	  }
	  ctk_80col_bitmapptr += 3 * 8;
	  ++ypos;
	}
      }
      x = xpos;
  
      len = strlen(w->widget.icon.title);
      if(x + len >= SCREEN_WIDTH) {
	x = SCREEN_WIDTH - len;
      }

      if(ypos >= clipy1 && ypos < clipy2) {
	len = strlen(w->widget.icon.title);
	gotoxy((x & 0xfe) + 1, ypos);
	ctk_80col_cclear((len - 1)/ 2);
	gotoxy(x, ypos);
	ctk_80col_cputsn(w->widget.icon.title, len);
      }
      revers(0);
    }
    break;

  default:
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_widget(struct ctk_widget *w,
		  unsigned char focus,
		  unsigned char clipy1,
		  unsigned char clipy2)
{
  struct ctk_window *win = w->window;
  unsigned char posx, posy;

  posx = (win->x & 0xfe) + 1;
  posy = win->y + 2;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy,
	      posx + win->w,
	      posy + win->h,
	      clipy1, clipy2,
	      focus);
  
#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  unsigned char i;
  unsigned char h;
  
  if(focus & CTK_FOCUS_WINDOW) {
    color(COLOR_FOCUS_WINDOW);
  } else {
    color(COLOR_BACKGROUND_WINDOW);
  }
    
  h = window->y + 2 + window->h;
  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      gotoxy((window->x & 0xfe) + 1, i);
      ctk_80col_cclear((window->w + 1)/2);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_window_contents(struct ctk_window *window, unsigned char focus,
		     unsigned char clipy1, unsigned char clipy2,
		     unsigned char x1, unsigned char x2,
		     unsigned char y1, unsigned char y2)
{
  struct ctk_widget *w;
  unsigned char wfocus;
  
  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1, x2, y2,
		clipy1, clipy2,
		focus);
  }
  
  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {  
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }

   draw_widget(w, x1, y1, x2, y2, 
	       clipy1, clipy2,
	       wfocus);
  }

#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */

}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_window(struct ctk_window *window, unsigned char focus,
		  unsigned char clipy1, unsigned char clipy2,
		  unsigned char draw_borders)
{
  unsigned char x, y;
  unsigned char h;
  unsigned char x1, y1, x2, y2;

  
  if(window->y + 1 >= clipy2) {
    return;
  }
    
  x = window->x & 0xfe;
  y = window->y + 1;
  
/*  if(focus & CTK_FOCUS_WINDOW) {
  } else {
  }*/

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;
  y2 = y1 + window->h;

  /* Draw window frame. */
  gotoxy(x, y);
  ctk_80col_windowparams.w = (window->w-1)/2;
  ctk_80col_windowparams.h = window->h;
  if(clipy1 < y) {
    ctk_80col_windowparams.clipy1 = 0;
  } else {
    ctk_80col_windowparams.clipy1 = clipy1 - y;
  }
  ctk_80col_windowparams.clipy2 = clipy2 - y + 1;
  ctk_80col_windowparams.color1 = ctk_80col_theme.windowcolors[focus];
  ctk_80col_windowparams.color2 = ctk_80col_theme.windowcolors[focus];
  ctk_80col_windowparams.titlecolor = ctk_80col_theme.windowcolors[focus+1];
  ctk_80col_windowparams.title = window->title;
  ctk_80col_windowparams.titlelen = window->titlelen/2;

  if(ctk_80col_windowparams.clipy1 < ctk_80col_windowparams.clipy2 &&
     ctk_80col_windowparams.clipy2 > 0) {
    ctk_80col_draw_windowborders();
  }
  /*  
  if(y >= clipy1) {
    cputcxy(x, y, CH_ULCORNER);
    gotoxy(wherex() + window->titlelen + CTK_CONF_WINDOWMOVE * 2, wherey());
    chline(window->w - (wherex() - x) - 2);
    cputcxy(x2, y, CH_URCORNER);
  }

  h = window->h;
  
  if(clipy1 > y1) {
    if(clipy1 - y1 < h) {
      h = clipy1 - y1;
            y1 = clipy1;
    } else {
      h = 0;
    }
  }

  if(clipy2 < y1 + h) {
    if(y1 >= clipy2) {
      h = 0;
    } else {
      h = clipy2 - y1;
    }
  }

  cvlinexy(x, y1, h);
  cvlinexy(x2, y1, h);  

  if(y + window->h >= clipy1 &&
     y + window->h < clipy2) {
    cputcxy(x, y2, CH_LLCORNER);
    chlinexy(x1, y2, window->w);
    cputcxy(x2, y2, CH_LRCORNER);
  }
  */
  if(ctk_mode_get() != CTK_MODE_WINDOWMOVE) {
    draw_window_contents(window, focus, clipy1, clipy2,
			 x1, x2, y + 1, y2);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  unsigned char x1, y1, x2, y2;
  
  
  x = dialog->x & 0xfe;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  /*  cvlinexy(x, y1,
	   dialog->h);
  cvlinexy(x2, y1,
	   dialog->h);

  chlinexy(x1, y,
	   dialog->w);
  chlinexy(x1, y2,
	   dialog->w);

  cputcxy(x, y, CH_ULCORNER);
  cputcxy(x, y2, CH_LLCORNER);
  cputcxy(x2, y, CH_URCORNER);
  cputcxy(x2, y2, CH_LRCORNER);
  */
  gotoxy(x, y);
  ctk_80col_windowparams.w = (dialog->w-1)/2;
  ctk_80col_windowparams.h = dialog->h;
  ctk_80col_windowparams.clipy1 = 0;
  ctk_80col_windowparams.clipy2 = SCREEN_HEIGHT;
  ctk_80col_windowparams.color1 = ctk_80col_theme.windowcolors[4];
  ctk_80col_windowparams.color2 = ctk_80col_theme.windowcolors[4];
  ctk_80col_windowparams.titlecolor = ctk_80col_theme.windowcolors[5];

  ctk_80col_draw_windowborders();  
  
  /* Clear dialog contents. */
  color(COLOR_DIALOG);
  for(i = y1; i < y2; ++i) {
    gotoxy((x1 & 0xfe) + 2, i);
    ctk_80col_cclear((dialog->w + 1)/2 - 2);
    /*    cclearxy(x1, i, dialog->w);*/
  }

  draw_window_contents(dialog, CTK_FOCUS_DIALOG, 0, SCREEN_HEIGHT,
		       x1, x2, y1, y2);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;

  
  for(i = y1; i < y2; ++i) {
    
    ctk_80col_clear_line(i);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;
  
  color(ctk_80col_theme.openmenucolor);
  
  revers(0);
  x = wherex();
  _cputs(m->title);
  cputc(' ');
  x2 = wherex();
  if(x + CTK_CONF_MENUWIDTH > SCREEN_WIDTH) {
    x = SCREEN_WIDTH - CTK_CONF_MENUWIDTH - 2;
  }
    
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      color(ctk_80col_theme.activemenucolor);
    } else {
      color(ctk_80col_theme.openmenucolor);
    }
    gotoxy(x, y + 1);
    ctk_80col_cclear(CTK_CONF_MENUWIDTH/2);

    gotoxy(x, y + 1);
    if(m->items[y].title[0] == '-') {
      chline(CTK_CONF_MENUWIDTH);
    } else {
      /*      _cputs(m->items[y].title);*/
      ctk_80col_cputsn(m->items[y].title, m->items[y].titlelen);
    }
    /*    if(x + CTK_CONF_MENUWIDTH > wherex()) {
      cclear(x + CTK_CONF_MENUWIDTH - wherex());
      }*/
  
  }
  gotoxy(x2, 0);
  revers(1);

}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;  
  
  memcpy((char *)0xe000, ctk_80col_theme.menuleftpattern, 8);
  /* Draw menus */
  gotoxy(2, 0);

  revers(1); 
  for(m = menus->menus->next; m != NULL; m = m->next) {
    color(ctk_80col_theme.menucolor);
    if(m != menus->open) {
      /*      _cputs(m->title);*/
      ctk_80col_cputsn(m->title, m->titlelen);
      cputc(' ');
    } else {
      draw_menu(m);
    }
  }

  color(ctk_80col_theme.menucolor);
    
  if(wherex() + strlen(menus->desktopmenu->title) + 2 >= SCREEN_WIDTH) {
    gotoxy(SCREEN_WIDTH - strlen(menus->desktopmenu->title) - 2, 0);
  } else {
    cclear(SCREEN_WIDTH - wherex() -
	   strlen(menus->desktopmenu->title) - 2);
  }
  
  /* Draw desktopmenu */
  if(menus->desktopmenu != menus->open) {
    ctk_80col_cputsn(menus->desktopmenu->title, menus->desktopmenu->titlelen);
  } else {
    draw_menu(menus->desktopmenu);
  }

  /*  gotoxy(78, 0);
  color(ctk_80col_screencolors[0]);
  cputc(' ');
  cputc(' ');    */
  
  revers(0);
  memcpy((char *)0xe138, ctk_80col_theme.menurightpattern, 8);
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_height(void)
{
  return SCREEN_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_width(void)
{
  return SCREEN_WIDTH;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_xtoc(unsigned short x)
{
  return x / 4;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_ytoc(unsigned short y)
{
  return y / 8;
}
/*-----------------------------------------------------------------------------------*/

SERVICE(ctk_hires_service, ctk_draw_service, 
{ 1,
  1,
  1,
  s_ctk_draw_init,
  s_ctk_draw_clear,
  s_ctk_draw_clear_window,
  s_ctk_draw_window,
  s_ctk_draw_dialog,
  s_ctk_draw_widget,
  s_ctk_draw_menus,
  s_ctk_draw_width,
  s_ctk_draw_height,
  s_ctk_mouse_xtoc,
  s_ctk_mouse_ytoc });

/*--------------------------------------------------------------------------*/

PROCESS(ctk_hires_service_process, "CTK hires service");

PROCESS_THREAD(ctk_hires_service_process, ev, data) {
  PROCESS_BEGIN();

  s_ctk_draw_init();
  ctk_restore();
  
  SERVICE_REGISTER(ctk_hires_service);

  while(ev != PROCESS_EVENT_SERVICE_REMOVED &&
	ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT();
  }

  SERVICE_REMOVE(ctk_hires_service);

  VIC.ctrl1 = 0x1b;  /* $D011 */
  VIC.addr  = 0x17;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x03;  /* $DD00 */
  
  PROCESS_END();
}
/*--------------------------------------------------------------------------*/
