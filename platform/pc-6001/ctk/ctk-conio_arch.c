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
 * $Id: ctk-conio_arch.c,v 1.1 2007/09/11 12:12:59 matsutsuka Exp $
 *
 */

/*
 * \file
 * 	This is architecture-depend contiki toolkit for PC-6001 family.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */
 

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "contiki-conf.h"
#include "ctk/ctk_arch.h"
#include "ctk/ctk-conio_arch.h"
#include <stddef.h>

struct screen_info {
  char cursx; // +0
  char cursy; // +1
  char cy1; // +2
  char cy2; // +3
};
struct screen_info info;
#if 0
const unsigned char ctk_arch_window_x = offsetof(struct ctk_window, x);
const unsigned char ctk_arch_window_y = offsetof(struct ctk_window, y);
const unsigned char ctk_arch_window_h = offsetof(struct ctk_window, h);
const unsigned char ctk_arch_window_w = offsetof(struct ctk_window, w);
#endif

unsigned char ctk_draw_windowtitle_height = 1;

/*---------------------------------------------------------------------------*/
/*
 * w: widget
 * x, y:  screen position of client drawing area (left, top)
 * focus: boolean
 */
static void draw_widget(struct ctk_widget *w,
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
    
  revers_arch(focus & CTK_FOCUS_WIDGET);
  gotoxy_arch(xpos, ypos);

  if (w->type == CTK_WIDGET_SEPARATOR) {
    chline_arch(w->w);
  } else if (w->type == CTK_WIDGET_LABEL) {
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      gotoxy_arch(xpos, ypos);
      cputsn_arch(text, w->w);
      cclear_arch(xpos + w->w);
      ++ypos;
      text += w->w;
    }
  } else if (w->type == CTK_WIDGET_BUTTON) {
    cputc_arch('[');
    cputsn_arch(w->widget.button.text, w->w);
    cputc_arch(']');
  } else if (w->type == CTK_WIDGET_HYPERLINK) {
    cputsn_arch(w->widget.button.text, w->w);
  } else if (w->type == CTK_WIDGET_TEXTENTRY) {
    text = w->widget.textentry.text;
    xscroll = 0;
    if(w->widget.textentry.xpos >= w->w - 1) {
      xscroll = w->widget.textentry.xpos - w->w + 1;
    }
    for(j = 0; j < w->h; ++j) {
      gotoxy_arch(xpos, ypos);
      if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	 w->widget.textentry.ypos == j) {
	revers_arch(0);
	cputc_arch('>');
	for(i = 0; i < w->w; ++i) {
	  c = text[i + xscroll];
	  revers_arch(i == w->widget.textentry.xpos - xscroll);
	  cputc_arch((c == 0) ? CH_SPACE : c);
	}
	revers_arch(0);
	cputc_arch('<');
      } else {
	cputc_arch(CH_VERTLINE);
	cputsn_arch(text, w->w);
	cclear_arch(xpos + w->w + 1);
	cputc_arch(CH_VERTLINE);
      }
      ++ypos;
      text += w->w;
    }
#if CTK_CONF_ICONS
  } else if (w->type == CTK_WIDGET_ICON) {
    if(w->widget.icon.textmap != NULL) {
      for(i = 0; i < 3; ++i) {
	gotoxy_arch(xpos, ypos);
	cputc_arch(w->widget.icon.textmap[0 + 3 * i]);
	cputc_arch(w->widget.icon.textmap[1 + 3 * i]);
	cputc_arch(w->widget.icon.textmap[2 + 3 * i]);
	++ypos;
      }
      x = xpos;
  
      len = strlen(w->widget.icon.title);
      if(x + len >= SCREEN_WIDTH) {
	x = SCREEN_WIDTH - len;
      }
      gotoxy_arch(x, ypos);
      cputs_arch(w->widget.icon.title);
    }
#endif /* CTK_CONF_ICONS */
  }
  revers_arch(0);

}
/*---------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *w,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2) {

  struct ctk_window *win = w->window;
  unsigned char posx, posy;
  clip_arch(clipy1, clipy2);

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
  clip_arch(clipy1, clipy2);

  for(i = y1; i < y2; ++i) {
    gotoxy_arch(x1, i);
    cclear_arch(x2);
  }
}
/*---------------------------------------------------------------------------*/
static void draw_window_sub(struct ctk_window *window, unsigned char focus) {

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
  gotoxy_arch(x, y1);
  cvline_arch(window->h);
  cputc_arch(CH_LLCORNER);
  chline_arch(window->w);
  cputc_arch(CH_LRCORNER);

  // -
  gotoxy_arch(x, y);
  cputc_arch(CH_ULCORNER);
  chline_arch(window->w);
  cputc_arch(CH_URCORNER);
  // |
  gotoxy_arch(x2, y1);
  cvline_arch(window->h);

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
void ctk_draw_window(struct ctk_window *window, unsigned char focus,
		     unsigned char clipy1, unsigned char clipy2,
		     unsigned char draw_borders) {
  clip_arch(clipy1, clipy2);

  focus = focus & CTK_FOCUS_WINDOW;
  draw_window_sub(window, focus);
}
/*---------------------------------------------------------------------------*/
void ctk_draw_dialog(struct ctk_window *dialog) {
  clip_arch(0, SCREEN_HEIGHT);

  ctk_draw_clear_window(dialog, 0, 0, SCREEN_HEIGHT);
  draw_window_sub(dialog, CTK_FOCUS_DIALOG);
}
/*---------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
static void draw_menu(struct ctk_menu *m, struct ctk_menu *open) {
#if CC_CONF_UNSIGNED_CHAR_BUGS
  unsigned char x2;
  unsigned int x, y;
#else
  unsigned char x2;
  unsigned char x, y;
#endif
  x = info.cursx;
  cputs_arch(m->title);
  cputc_arch(CH_SPACE);

  if (m == open) {
    x2 = info.cursx;
    if(x + CTK_CONF_MENUWIDTH > SCREEN_WIDTH) {
      x = SCREEN_WIDTH - CTK_CONF_MENUWIDTH;
    }
  
    for(y = 0; y < m->nitems; y++) {
      if(y == m->active) {
	revers_arch(0);
      }
      gotoxy_arch(x, y + 1);
      if(m->items[y].title[0] == '-') {
	chline_arch(CTK_CONF_MENUWIDTH);
      } else {
	cputs_arch(m->items[y].title);
      }
      cclear_arch(x + CTK_CONF_MENUWIDTH);
      revers_arch(1);
    }
    gotoxy_arch(x2, 0);
  }
}
/*---------------------------------------------------------------------------*/
void ctk_draw_menus(struct ctk_menus *menus) {
  struct ctk_menu *m;  
  
  clip_arch(0, SCREEN_HEIGHT);
  /* Draw menus */
  gotoxy_arch(0, 0);
  revers_arch(1);
  for(m = menus->menus->next; m != NULL; m = m->next) {
    draw_menu(m, menus->open);
  }

  cclear_arch(SCREEN_WIDTH - strlen(menus->desktopmenu->title) - 1);

  /* Draw desktopmenu */
  draw_menu(menus->desktopmenu, menus->open);

  revers_arch(0);
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
