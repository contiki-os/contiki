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
 * $Id: ctk-conio-service.c,v 1.1 2007/05/23 23:19:10 oliverschmidt Exp $
 *
 */

#include <conio.h>

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"

#include "ctk/ctk-draw-service.h"

#include "contiki-conf.h"
#include <string.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

static unsigned char sizex, sizey;

/*unsigned char ctk_draw_windowborder_height = 1;
unsigned char ctk_draw_windowborder_width = 1;
unsigned char ctk_draw_windowtitle_height = 1;*/


/*-----------------------------------------------------------------------------------*/
static void
cputsn(char *str, unsigned char len)
{
  char c;

  while(len > 0) {
    --len;
    c = *str;
    if(c == 0) {
      break;
    }
    cputc(c);
    ++str;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_init(void)
{
  (void)bgcolor(SCREENCOLOR);
  (void)bordercolor(BORDERCOLOR);
  screensize(&sizex, &sizey);
  ctk_draw_clear(0, sizey);
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
    (void)textcolor(WIDGETCOLOR_FWIN);
    if(focus & CTK_FOCUS_WIDGET) {
      (void)textcolor(WIDGETCOLOR_FOCUS);
      wfocus = 1;
    }
  } else if(focus & CTK_FOCUS_DIALOG) {
    (void)textcolor(WIDGETCOLOR_DIALOG);
    if(focus & CTK_FOCUS_WIDGET) {
      (void)textcolor(WIDGETCOLOR_FOCUS);
      wfocus = 1;
    }
  } else {
    (void)textcolor(WIDGETCOLOR);
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
	cputsn(text, w->w);
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
      revers(wfocus != 0);
      cputcxy(xpos, ypos, '[');
      cputsn(w->widget.button.text, w->w);
      cputc(']');
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    if(ypos >= clipy1 && ypos < clipy2) {
      revers(wfocus == 0);
      gotoxy(xpos, ypos);
      (void)textcolor(WIDGETCOLOR_HLINK);
      cputsn(w->widget.button.text, w->w);
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
            revers(i == w->widget.textentry.xpos - xscroll);
	    if(c == 0) {
	      cputc(' ');
	    } else {
	      cputc(c);
	    }
	  }
	  revers(0);
	  cputc('<');
	} else {
	  revers(wfocus != 0 && j == w->widget.textentry.ypos);
	  cvlinexy(xpos, ypos, 1);
	  gotoxy(xpos + 1, ypos);          
	  cputsn(text, w->w);
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
      revers(wfocus != 0);
      gotoxy(xpos, ypos);
      if(w->widget.icon.textmap != NULL) {
	for(i = 0; i < 3; ++i) {
	  gotoxy(xpos, ypos);
	  if(ypos >= clipy1 && ypos < clipy2) {
	    cputc(w->widget.icon.textmap[0 + 3 * i]);
	    cputc(w->widget.icon.textmap[1 + 3 * i]);
	    cputc(w->widget.icon.textmap[2 + 3 * i]);
	  }
	  ++ypos;
	}
      }
      x = xpos;
  
      len = (unsigned char)strlen(w->widget.icon.title);
      if(x + len >= sizex) {
	x = sizex - len;
      }

      gotoxy(x, ypos);
      if(ypos >= clipy1 && ypos < clipy2) {
	cputs(w->widget.icon.title);
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

  posx = win->x + 1;
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
    (void)textcolor(WINDOWCOLOR_FOCUS);
  } else {
    (void)textcolor(WINDOWCOLOR);
  }
    
  h = window->y + 2 + window->h;

  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      cclearxy(window->x + 1, i, window->w);
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
    
  x = window->x;
  y = window->y + 1;
  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + window->w;
  y2 = y1 + window->h;

  if(draw_borders) {  

    /* Draw window frame. */
    if(focus & CTK_FOCUS_WINDOW) {
      (void)textcolor(WINDOWCOLOR_FOCUS);
    } else {
      (void)textcolor(WINDOWCOLOR);
    }

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
  }

  draw_window_contents(window, focus, clipy1, clipy2,
		       x1, x2, y + 1, y2);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  unsigned char x1, y1, x2, y2;
  
  (void)textcolor(DIALOGCOLOR);

  x = dialog->x;
  y = dialog->y + 1;

  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;

  /* Draw dialog frame. */
  cvlinexy(x, y1,
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
  
  /* Clear dialog contents. */
  for(i = y1; i < y2; ++i) {
    cclearxy(x1, i, dialog->w);
  }

  draw_window_contents(dialog, CTK_FOCUS_DIALOG, 0, sizey,
		       x1, x2, y1, y2);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;
 
  (void)textcolor(BACKGROUNDCOLOR);
  for(i = y1; i < y2; ++i) {
    cclearxy(0, i, sizex);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m, unsigned char open)
{
  unsigned char x, x2, y;

  if(open) {
    x = x2 = wherex();
    if(x2 + CTK_CONF_MENUWIDTH > sizex) {
      x2 = sizex - CTK_CONF_MENUWIDTH;
    }

    for(y = 0; y < m->nitems; ++y) {
      if(y == m->active) {
	(void)textcolor(ACTIVEMENUITEMCOLOR);
	revers(0);
      } else {
	(void)textcolor(MENUCOLOR);	  
	revers(1);
      }
      gotoxy(x2, y + 1);
      if(m->items[y].title[0] == '-') {
	chline(CTK_CONF_MENUWIDTH);
      } else {
	cputs(m->items[y].title);
      }
      if(x2 + CTK_CONF_MENUWIDTH > wherex()) {
	cclear(x2 + CTK_CONF_MENUWIDTH - wherex());
      }
    }

    gotoxy(x, 0);
    (void)textcolor(OPENMENUCOLOR);
    revers(0);
  }

  cputs(m->title);
  cputc(' ');
  (void)textcolor(MENUCOLOR);
  revers(1);
}
/*-----------------------------------------------------------------------------------*/
static void
s_ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;

  /* Draw menus */
  (void)textcolor(MENUCOLOR);
  gotoxy(0, 0);
  revers(1);
  cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    draw_menu(m, m == menus->open);
  }

  /* Draw desktopmenu */
  if(wherex() + strlen(menus->desktopmenu->title) + 1 >= sizex) {
    gotoxy(sizex - (unsigned char)strlen(menus->desktopmenu->title) - 1, 0);
  } else {
    cclear(sizex - wherex() -
	   (unsigned char)strlen(menus->desktopmenu->title) - 1);
  }
  draw_menu(menus->desktopmenu, menus->desktopmenu == menus->open);

  revers(0);
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_height(void)
{
  return sizey;
}
/*-----------------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_width(void)
{
  return sizex;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_xtoc(unsigned short x)
{
  return x / 8;
}
/*-----------------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_ytoc(unsigned short y)
{
  return y / 8;
}
/*-----------------------------------------------------------------------------------*/

SERVICE(ctk_conio_service, ctk_draw_service, 
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

PROCESS(ctk_conio_service_process, "CTK CONIO service");

PROCESS_THREAD(ctk_conio_service_process, ev, data) {
  PROCESS_BEGIN();

  s_ctk_draw_init();
  ctk_restore();

  SERVICE_REGISTER(ctk_conio_service);

  while(ev != PROCESS_EVENT_SERVICE_REMOVED &&
	ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT();
  }

  SERVICE_REMOVE(ctk_conio_service);
  
  PROCESS_END();
}
/*--------------------------------------------------------------------------*/
