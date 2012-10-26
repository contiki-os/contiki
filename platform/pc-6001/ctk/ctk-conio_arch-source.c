/*
 * Copyright (c) 2007, Takahide Matsutsuka.
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
 *
 */

/*
 * \file
 * 	This is an original source file for
 *      ctk-conio_arch.c and ctk-conio_arch-asm.S
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */
 

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "contiki-conf.h"
#include "ctk/ctk_arch.h"
#include <string.h>

#ifndef NULL
#define NULL (void *)0
#endif

/*---------------------------------------------------------------------------*/
static char cursx, cursy;
static unsigned char reversed;
static char cy1, cy2;

unsigned char ctk_draw_windowtitle_height = 1;

/*---------------------------------------------------------------------------*/
#define revers(c) reversed = c
#define clip(y1, y2) cy1 = y1; cy2 = y2
#define OFFSET(x, y) y * SCREEN_WIDTH + x

/*---------------------------------------------------------------------------*/
void cputc(char c) {
  if (cursy >= cy1 && cursy <= cy2
      && cursy >= 0 && cursy <= SCREEN_HEIGHT
      && cursx >= 0 && cursx < SCREEN_WIDTH) {
    unsigned int offset = OFFSET(cursx, cursy);
    *((char *) VRAM_CHAR + offset) = c;
    *((char *) VRAM_ATTR + offset) = reversed ? COLOR_REVERSED : COLOR_NORMAL;
  }
  cursx++;
}

/*---------------------------------------------------------------------------*/
void cputs(char *str) {
  while (*str != 0) {
    cputc(*str);
    str++;
  }
}
/*---------------------------------------------------------------------------*/
void cputsn(char *str, unsigned char len) {
  while ((len > 0) && (*str != 0)) {
    cputc(*str);
    str++;
    len--;
  }
}
/*---------------------------------------------------------------------------*/
void chline(unsigned char length) {
  while (length > 0) {
    cputc(CH_HOLILINE);
    length--;
  }
}
/*---------------------------------------------------------------------------*/
/*  #define cvline(l)   while (l > 0) { cputc(CH_VERTLINE); */
/*      ++cursy; --cursx; l--; } */

void cvline(unsigned char length) {
  while (length > 0) {
    cputc(CH_VERTLINE);
    ++cursy;
    --cursx;
    length--;
  }
}
/*---------------------------------------------------------------------------*/
void gotoxy(unsigned char x, unsigned char y) {
  cursx = x;
  cursy = y;
}
/*---------------------------------------------------------------------------*/
void clearTo(char x) {
  while (cursx < x) {
    cputc(CH_SPACE);
  }
  cursx = x;
}

/*---------------------------------------------------------------------------*/
void ctk_draw_clear(unsigned char clipy1, unsigned char clipy2) {
  while (clipy1 < clipy2) {
    gotoxy(0, clipy1);
    clearTo(SCREEN_WIDTH);
    clipy1++;
  }
}

/*---------------------------------------------------------------------------*/
void ctk_draw_init(void) {
  clip(0, SCREEN_HEIGHT);
  ctk_draw_clear(0, SCREEN_HEIGHT);
}
/*---------------------------------------------------------------------------*/
/*
 * w: widget
 * x, y:  screen position of client drawing area (left, top)
 * clipx, clipy: screen position of client drawing area (right, bottom)
 * clipy1, clipy2: min/max y position of screen
 * focus: boolean
 */
void
draw_widget(struct ctk_widget *w,
	    unsigned char x, unsigned char y,
	    unsigned char focus) {
  unsigned char xpos, ypos, xscroll;
  unsigned char i, j;
  char c, *text;
#if CTK_CONF_ICONS
  unsigned char len;
#endif /* CTK_CONF_ICONS */

  xpos = x + w->x;
  ypos = y + w->y;
    
  revers(focus & CTK_FOCUS_WIDGET);
  gotoxy(xpos, ypos);

  if (w->type == CTK_WIDGET_SEPARATOR) {
    chline(w->w);
  } else if (w->type == CTK_WIDGET_LABEL) {
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      gotoxy(xpos, ypos);
      cputsn(text, w->w);
      clearTo(xpos + w->w);
      ++ypos;
      text += w->w;
    }
  } else if (w->type == CTK_WIDGET_BUTTON) {
    cputc('[');
    cputsn(w->widget.button.text, w->w);
    cputc(']');
  } else if (w->type == CTK_WIDGET_HYPERLINK) {
    cputsn(w->widget.hyperlink.text, w->w);
  } else if (w->type == CTK_WIDGET_TEXTENTRY) {
    text = w->widget.textentry.text;
    xscroll = 0;
    if(w->widget.textentry.xpos >= w->w - 1) {
      xscroll = w->widget.textentry.xpos - w->w + 1;
    }
    for(j = 0; j < w->h; ++j) {
      gotoxy(xpos, ypos);
      if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	 w->widget.textentry.ypos == j) {
	revers(0);
	cputc('>');
	for(i = 0; i < w->w; ++i) {
	  c = text[i + xscroll];
	  revers(i == w->widget.textentry.xpos - xscroll);
	  cputc((c == 0) ? CH_SPACE : c);
	}
	revers(0);
	cputc('<');
      } else {
	cputc(CH_VERTLINE);
	cputsn(text, w->w);
	clearTo(xpos + w->w + 1);
	cputc(CH_VERTLINE);
      }
      ++ypos;
      text += w->w;
    }
#if CTK_CONF_ICONS
  } else if (w->type == CTK_WIDGET_ICON) {
    if(w->widget.icon.textmap != NULL) {
      for(i = 0; i < 3; ++i) {
	gotoxy(xpos, ypos);
	cputc(w->widget.icon.textmap[0 + 3 * i]);
	cputc(w->widget.icon.textmap[1 + 3 * i]);
	cputc(w->widget.icon.textmap[2 + 3 * i]);
	++ypos;
      }
      x = xpos;
  
      len = strlen(w->widget.icon.title);
      if(x + len >= SCREEN_WIDTH) {
	x = SCREEN_WIDTH - len;
      }
      gotoxy(x, ypos);
      cputs(w->widget.icon.title);
    }
#endif /* CTK_CONF_ICONS */
  }
  revers(0);

}
/*---------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *w,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2) {

  struct ctk_window *win = w->window;
  unsigned char posx, posy;
  clip(clipy1, clipy2);

  posx = win->x + 1;
  posy = win->y + 2;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy, focus);
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2) {

  unsigned char i;
  unsigned char x1, x2, y1, y2;
  x1 = window->x + 1;
  x2 = x1 + window->w;
  y1 = window->y + 2;
  y2 = y1 + window->h;

  for(i = y1; i < y2; ++i) {
    gotoxy(x1, i);
    clearTo(x2);
  }
}
/*---------------------------------------------------------------------------*/
void draw_window_sub(struct ctk_window *window, unsigned char focus) {

  unsigned char x, y;
  unsigned char x1, y1, x2;
  struct ctk_widget *w;
  unsigned char wfocus;

  x = window->x;
  y = window->y + 1;

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;

  // |_
  gotoxy(x, y1);
  cvline(window->h);
  cputc(CH_LLCORNER);
  chline(window->w);
  cputc(CH_LRCORNER);

  // -
  gotoxy(x, y);
  cputc(CH_ULCORNER);
  chline(window->w);
  cputc(CH_URCORNER);
  // |
  gotoxy(x2, y1);
  cvline(window->h);

  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1, focus);
  }

  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }
    draw_widget(w, x1, y1, wfocus);
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window, unsigned char focus,
		unsigned char clipy1, unsigned char clipy2, unsigned char draw_borders) {
  clip(clipy1, clipy2);

  focus = focus & CTK_FOCUS_WINDOW;
  draw_window_sub(window, focus);
}
/*---------------------------------------------------------------------------*/
void ctk_draw_dialog(struct ctk_window *dialog) {
  clip(0, SCREEN_HEIGHT);

  ctk_draw_clear_window(dialog, 0, 0, SCREEN_HEIGHT);
  draw_window_sub(dialog, CTK_FOCUS_DIALOG);
}
/*---------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
void draw_menu(struct ctk_menu *m, struct ctk_menu *open) {
#if CC_CONF_UNSIGNED_CHAR_BUGS
  unsigned char x2;
  unsigned int x, y;
#else
  unsigned char x2;
  unsigned char x, y;
#endif
  x = cursx;
  cputs(m->title);
  cputc(CH_SPACE);

  if (m == open) {
    x2 = cursx;
    if(x + CTK_CONF_MENUWIDTH > SCREEN_WIDTH) {
      x = SCREEN_WIDTH - CTK_CONF_MENUWIDTH;
    }
  
    for(y = 0; y < m->nitems; y++) {
      if(y == m->active) {
	revers(0);
      }
      gotoxy(x, y + 1);
      if(m->items[y].title[0] == '-') {
	chline(CTK_CONF_MENUWIDTH);
      } else {
	cputs(m->items[y].title);
      }
      clearTo(x + CTK_CONF_MENUWIDTH);
      revers(1);
    }
    gotoxy(x2, 0);
  }
}
/*---------------------------------------------------------------------------*/
void ctk_draw_menus(struct ctk_menus *menus) {
  struct ctk_menu *m;  
  
  clip(0, SCREEN_HEIGHT);
  /* Draw menus */
  gotoxy(0, 0);
  revers(1);
  for(m = menus->menus->next; m != NULL; m = m->next) {
    draw_menu(m, menus->open);
  }

  clearTo(SCREEN_WIDTH - strlen(menus->desktopmenu->title) - 1);

  /* Draw desktopmenu */
  draw_menu(menus->desktopmenu, menus->open);

  revers(0);
}
#endif /* CTK_CONF_MENUS */
/*---------------------------------------------------------------------------*/
/* Returns width and height of screen. */
unsigned char ctk_draw_width(void) {
  return SCREEN_WIDTH;
}
unsigned char ctk_draw_height(void) {
  return SCREEN_HEIGHT;
}

