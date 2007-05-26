/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: ctk-term.c,v 1.1 2007/05/26 21:54:33 oliverschmidt Exp $
 */
#include "ctk/ctk.h"
#include "ctk-draw.h"

#include "contiki.h"
#include "loader.h"
#include "ctk-term.h"
#include "ctk-term-int.h"
#include "ctk-term-out.h"
#include "contiki-conf.h"
#include "libconio.h"

#define PRINTF(x)

#define reverse(x)

/*-----------------------------------------------------------------------------------*/
/* 
 * #defines and enums
 */
/*-----------------------------------------------------------------------------------*/
#define CH_ULCORNER '+' //0x00
#define CH_TITLEBAR '-' //0x01
#define CH_URCORNER '+' //0x02
#define CH_WINDOWRBORDER '|' //0x03
#define CH_LRCORNER '+' //0x04
#define CH_WINDOWLOWERBORDER '-' //0x05
#define CH_LLCORNER '+' //0x06
#define CH_WINDOWLBORDER '|' //0x07

#define CH_DIALOG_ULCORNER '+' //0x12
#define CH_DIALOGUPPERBORDER '-' //0x09
#define CH_DIALOG_URCORNER '+' //0x0a
#define CH_DIALOGRBORDER '|' //0x0b
#define CH_DIALOG_LRCORNER '+' //0x0c
#define CH_DIALOGLOWERBORDER '-' //0x0d
#define CH_DIALOG_LLCORNER '+' //0x0e
#define CH_DIALOGLBORDER '|' //0x0f

#define CH_BUTTONLEFT  '[' //0x10
#define CH_BUTTONRIGHT ']' //0x11

#define CH_SEPARATOR   '=' //0x13

#ifdef CTK_TERM_CONF_MAX_CLIENTS
#define CTK_TERM_NUMCONNS CTK_TERM_CONF_MAX_CLIENTS
#else
#define CTK_TERM_NUMCONNS 1
#endif

unsigned char ctk_draw_windowborder_height = 1;
unsigned char ctk_draw_windowborder_width = 1;
unsigned char ctk_draw_windowtitle_height = 1;


/* Term context states */
enum {
  TERM_DEALLOCATED,
  TERM_ALLOCATED
};

/*-----------------------------------------------------------------------------------*/
/* 
 * Local variables
 */
/*-----------------------------------------------------------------------------------*/

static unsigned char sizex, sizey;
static struct ctk_term_state conns[CTK_TERM_NUMCONNS];

/*-----------------------------------------------------------------------------------*/
/* 
 * Unconditionally add an update
 */
/*-----------------------------------------------------------------------------------*/
void ctk_term_update_add(struct ctk_term_state *ts, struct ctk_term_update *a)
{
  /* XXX: test both head and tail placement!*/
  a->next = ts->updates_pending;
  ts->updates_pending = a;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Allocate an update from the update pool
 */
/*-----------------------------------------------------------------------------------*/
struct ctk_term_update *
ctk_term_update_alloc(struct ctk_term_state *vs)
{
  struct ctk_term_update *a;

  a = vs->updates_free;
  if(a == NULL) {
    return NULL;
  }
  vs->updates_free = a->next;
  a->next = NULL;
  return a;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Return an update to the pool
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_term_update_free(struct ctk_term_state *ts, struct ctk_term_update *a)
{
  a->next = ts->updates_free;
  ts->updates_free = a;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Fetch update from the update list
 */
/*-----------------------------------------------------------------------------------*/
struct ctk_term_update *
ctk_term_update_dequeue(struct ctk_term_state *ts)
{
  struct ctk_term_update *a;

  a = ts->updates_pending;
  if(a == NULL) {
    return a;
  }
  ts->updates_pending = a->next;
  a->next = NULL;
  return a;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Remove an update from the update list
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_term_update_remove(struct ctk_term_state *ts, struct ctk_term_update *a)
{
  struct ctk_term_update *b, *c;

  if(a == ts->updates_pending) {
    ts->updates_pending = a->next;
  } else {
    b = ts->updates_pending;
    for(c = ts->updates_pending; c != a; b = c, c = c->next);

    b->next = a->next;
  }
}
/*-----------------------------------------------------------------------------------*/
/* 
 * Add an area update for a specific connection. Overlapping updates are merged
 */
/*-----------------------------------------------------------------------------------*/
static void
update_area_connection(struct ctk_term_state *ts,
		    unsigned char  x, unsigned char  y, unsigned char  w, unsigned char  h)
{
  unsigned char  x2, y2, ax2, ay2;
  struct ctk_term_update *a;

  PRINTF(("update_area_connection: should update (%d:%d) (%d:%d)\n",
	 x, y, w, h));
  
  /* First check if we already have a full update queued. If so, there
     is no need to put this update on the list. If there is a full
     update, it is always the first one on the list, so there is no
     need to go step the list in search for it. */

  if(ts->updates_pending != NULL &&
     ts->updates_pending->type == UPDATE_FULL) {
    PRINTF(("Update_area_connecion: full update already queued...\n"));
    return;
  }

again:
  
  /* Check that we don't update the same area twice by going through
     the list and search for an update with the same coordinates. */
  for(a = ts->updates_pending; a != NULL; a = a->next) {
    if(a->x == x && a->y == y &&
       a->w == w && a->h == h) {
      PRINTF(("Update_area_connecion: found equal area\n"));
      return;
    }    
  }

  /* Next we check if this update covers an existing update. If so, we
     remove the old update, expand this update so that it covers both
     areas to be updated and run through the process again. */
  for(a = ts->updates_pending; a != NULL; a = a->next) {      
    x2 = x + w;
    y2 = y + h;
    
    ax2 = a->x + a->w;
    ay2 = a->y + a->h;

    /* Test if updates overlaps */
    if(((x < ax2) && (a->x < x2)) &&
      ((y < ay2) && (a->y < y2))) {

      /* Remove the old update from the list. */
      ctk_term_update_remove(ts, a);

      /* Put it on the free list. */
      ctk_term_update_free(ts, a);

      PRINTF(("update_area_connection: inside (%d:%d, %d:%d)\n",
	     a->x, a->y, ax2, ay2));
      
      /* Find the area that covers both updates. */
#define MIN(a,b) ((a) < (b)? (a): (b))
#define MAX(a,b) ((a) > (b)? (a): (b))
      x = MIN(a->x, x);
      y = MIN(a->y, y);
      ax2 = MAX(ax2, x2);
      ay2 = MAX(ay2, y2);
      w = ax2 - x;
      h = ay2 - y;

      /* This should really be done by a recursive call to this
	 function: update_area_connection(vs, x, y, w, h); but because
	 some compilers might not be able to optimize away the
	 recursive call, we do it using a goto instead. */
      PRINTF(("Update_area_connecion: trying larger area (%d:%d) (%d:%d)\n", x, y, w, h));
      goto again;
    }
  }
  
  /* Allocate an update object by pulling it off the free list. If
     there are no free objects, we go for a full update instead. */

  a = ctk_term_update_alloc(ts);
  if(a == NULL) {
    PRINTF(("Update_area_connecion: no free updates, doing full\n"));
    /* Put all pending updates, except for one, on the free list. Use
       the remaining update as a full update. */
    while(ts->updates_pending != NULL) {
      a = ts->updates_pending;
      ctk_term_update_remove(ts, a);
      ctk_term_update_free(ts, a);
    }

    a = ctk_term_update_alloc(ts);
    a->type = UPDATE_FULL;
    ctk_term_update_add(ts, a);
					

  } else {
    
    PRINTF(("Update_area_connecion: allocated update for (%d:%d) (%d:%d)\n", x, y, w, h));
  /* Else, we put the update object at the end of the pending
     list. */
    a->type = UPDATE_PARTS;
    a->x = x;
    a->y = y;
    a->w = w;
    a->h = h;
    ctk_term_update_add(ts, a);
  }
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Update an area for all connections.
 */
/*-----------------------------------------------------------------------------------*/
static void
update_area(unsigned char  x, unsigned char  y, unsigned char  w, unsigned char  h)
{
  unsigned char  i;
  
  if(h == 0 || w == 0) {
    return;
  }

  if ((x+w) > sizex) {
    w = sizex - x;
  }

  if ((y+h) > sizey) {
    h = sizey - y;
  }
  
  /* Update for all active terminal connections. */
  for(i = 0; i < CTK_TERM_NUMCONNS; ++i) {
    if(conns[i].state != TERM_DEALLOCATED) {
      update_area_connection(&conns[i],x, y, w, h);
    }
  }

}
/*-----------------------------------------------------------------------------------*/
/** 
 * Request a full update for a specific connections. Usefull when a new client is
 * connected through telnet for example.
 *
 * \param ts Terminal connection state
 */
/*-----------------------------------------------------------------------------------*/
void ctk_term_redraw(struct ctk_term_state *ts)
{
  update_area_connection(ts,0,0,ts->width, ts->height);
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Initialize a terminal state structure
 */
/*-----------------------------------------------------------------------------------*/
static void
init_state(struct ctk_term_state *ts)
{
  unsigned char i;
  
  ts->width = sizex;
  ts->height = sizey;
  ts->x = ts->y = ts->x1 = ts->y1 = ts->x2 = ts->y2 = 0;
  ts->c1 = ts->c2 = 0;
  ts->w = sizex;
  ts->h = sizey;
  ts->state = TERM_ALLOCATED;
  ts->inputstate = ANS_IDLE;

  /* Initialize the linked list of updates. */
  for(i = 0; i < CTK_TERM_MAX_UPDATES - 1; ++i) {
    ts->updates_pool[i].next = &(ts->updates_pool[i + 1]);    
  }
  ts->updates_pool[CTK_TERM_MAX_UPDATES-1].next = NULL;

  ts->updates_free = &ts->updates_pool[0];
  ts->updates_pending = ts->updates_current = NULL;
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Allocate a new state structure. Returns NULL if none available
 */
/*-----------------------------------------------------------------------------------*/
struct ctk_term_state *
ctk_term_alloc_state(void)
{
  unsigned char i;
  for(i = 0; i < CTK_TERM_NUMCONNS; ++i) {
    if(conns[i].state == TERM_DEALLOCATED) {
      init_state(&conns[i]);
      return &conns[i];
    }
  }
  return NULL;
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Free a state structure.
 *
 * \param ts Terminal connection state
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_term_dealloc_state(struct ctk_term_state *s)
{
  s->state = TERM_DEALLOCATED;
}
/*-----------------------------------------------------------------------------------*/
static char tmp[40];
static void
cputsn(char *str, unsigned char len)
{
  strncpy(tmp, str, len);
  tmp[len] = 0;
  cputs(tmp);
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Initialize the terminal ctk-draw module. Called by the CTK module.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  int i;
  bgcolor(TERM_BACKGROUNDCOLOR);
  screensize(&sizex, &sizey);
  ctk_draw_clear(0, sizey);
  ctk_term_input_init();
  for(i = 0; i < CTK_TERM_NUMCONNS; ++i) {
    conns[i].state = TERM_DEALLOCATED;
  }
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
  unsigned char len;

  xpos = x + w->x;
  ypos = y + w->y;
    
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    textcolor((unsigned char)(TERM_SEPARATORCOLOR + focus));
    if(ypos >= clipy1 && ypos < clipy2) {
      gotoxy(xpos, ypos);
      for(i = 0; i < w->w; ++i) {
	cputc(CH_SEPARATOR);
      }
    }
    break;
  case CTK_WIDGET_LABEL:
    textcolor((unsigned char)(TERM_LABELCOLOR + focus));
    text = w->widget.label.text;
    for(i = 0; i < w->h; ++i) {
      if(ypos >= clipy1 && ypos < clipy2) {
	gotoxy(xpos, ypos);
	cputsn(text, w->w);
	if(w->w - (wherex() - xpos) > 0) {
	  cclear((unsigned char)(w->w - (wherex() - xpos)));
	}
      }
      ++ypos;
      text += w->w;
    }
    break;
  case CTK_WIDGET_BUTTON:
    textcolor((unsigned char)(TERM_BUTTONCOLOR + focus));
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus & CTK_FOCUS_WIDGET) {
	revers(1);
      } else {
	revers(0);
      }
      cputcxy(xpos, ypos, CH_BUTTONLEFT);
      cputsn(w->widget.button.text, w->w);
      cputc(CH_BUTTONRIGHT);
      revers(0);
    }
    break;
  case CTK_WIDGET_HYPERLINK:
    textcolor((unsigned char)(TERM_HYPERLINKCOLOR + focus));
    if(ypos >= clipy1 && ypos < clipy2) {
      if(focus & CTK_FOCUS_WIDGET) {
	revers(0);	
      } else {
	revers(1);
      }
      gotoxy(xpos, ypos);
      cputsn(w->widget.button.text, w->w);
      revers(0);
    }
    break;
  case CTK_WIDGET_TEXTENTRY:
    textcolor((unsigned char)(TERM_TEXTENTRYCOLOR + focus));
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
	      textcolor((unsigned char)(TERM_TEXTENTRYCOLOR + (focus ^ 0x01)));
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
	    textcolor((unsigned char)(TERM_TEXTENTRYCOLOR + focus));
	  }
	  cputc('<');
	} else {
	  if(focus & CTK_FOCUS_WIDGET && j == w->widget.textentry.ypos) {
	    revers(1);
	  } else {
	    revers(0);
	  }
	  cvlinexy(xpos, ypos, 1);
	  gotoxy((unsigned char)(xpos + 1), ypos);          
	  cputsn(text, w->w);
	  i = wherex();
	  if(i - xpos - 1 < w->w) {
	    cclear((unsigned char)(w->w - (i - xpos) + 1));
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
      textcolor((unsigned char)(TERM_ICONCOLOR + focus));
      if(focus & 1) {
	revers(1);
      } else {
	revers(0);
      }

      x = xpos;
      len = strlen(w->widget.icon.title);
      if(x + len >= sizex) {
	x = sizex - len;
      }

      gotoxy(x, (unsigned char)(ypos + 3));
      if(ypos >= clipy1 && ypos < clipy2) {
	cputs(w->widget.icon.title);
      }

      gotoxy(xpos, ypos);

      if (w->widget.icon.textmap != NULL) {
	for(i = 0; i < 3; ++i) {
	  
	  if(ypos >= clipy1 && ypos < clipy2) {
	    gotoxy(xpos,ypos);
	    cputc(w->widget.icon.textmap[0 + 3 * i]);
	    cputc(w->widget.icon.textmap[1 + 3 * i]);
	    cputc(w->widget.icon.textmap[2 + 3 * i]);
	  }
	  ++ypos;
	}
      }
      x = xpos;
      revers(0);
    }
    break;

  default:
    break;
  }
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Draw a widget on the VNC screen. Called by the CTK module.
 *
 * \param w The widget to be drawn.
 * \param focus The focus of the widget.
 * \param clipy1 The lower y coordinate bound.
 * \param clipy2 The upper y coordinate bound.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *w,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  struct ctk_window *win = w->window;
  struct ctk_icon *icon;
  unsigned char posx, posy, x, len;

  posx = win->x + 1;
  posy = win->y + 2;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy,
	      (unsigned char)(posx + win->w),
	      (unsigned char)(posy + win->h),
	      clipy1, clipy2,
	      focus);

  if(w->type != CTK_WIDGET_ICON) {
    update_area((unsigned char)(posx + w->x),
		(unsigned char)(posy + w->y), 
		(unsigned char)(w->w + 2), 
		w->h);
  } else {
    icon = (struct ctk_icon *)w;

    len = strlen(icon->title);
    x = posx + w->x;
    if(x + len >= sizex) {
      x = sizex - len;
    }

    update_area(x, 
      (unsigned char)(posy + w->y), 
      (unsigned char)(len > 4? len: 4), 
      w->h);    
  }
  
#ifdef CTK_CONIO_CONF_UPDATE
  CTK_CONIO_CONF_UPDATE();
#endif /* CTK_CONIO_CONF_UPDATE */
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Clear a window on the terminal screen. Called by the CTK module.
 *
 * \param window The window to be cleared.
 * \param focus The focus of the window.
 * \param clipy1 The lower y coordinate bound.
 * \param clipy2 The upper y coordinate bound.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  unsigned char i;
  unsigned char h;

  textcolor((unsigned char)(TERM_WINDOWCOLOR + focus));
  
  h = window->y + 2 + window->h;
  /* Clear window contents. */
  for(i = window->y + 2; i < h; ++i) {
    if(i >= clipy1 && i < clipy2) {
      cclearxy((unsigned char)(window->x + 1), i, window->w);
    }
  }

  update_area((unsigned char)(window->x + 1),
    (unsigned char)(window->y + 2), 
    window->w, window->h);
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
/** 
 * Draw a window on the terminal screen. Called by the CTK module.
 *
 * \param window The window to be drawn.
 * \param focus The focus of the window.
 * \param clipy1 The lower y coordinate bound.
 * \param clipy2 The upper y coordinate bound.
 * \param draw_borders The flag for border drawing.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window, unsigned char focus,
		unsigned char clipy1, unsigned char clipy2,
		unsigned char draw_borders)
{
  unsigned char x, y;
  unsigned char h;
  unsigned char x1, y1, x2, y2;
  unsigned char i;
  

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
    textcolor((unsigned char)(TERM_WINDOWCOLOR + focus));

    if(y >= clipy1) {
      cputcxy(x, y, CH_ULCORNER);
      for(i = wherex() + window->titlelen + CTK_CONF_WINDOWMOVE * 2; i < x2; ++i) {
	cputcxy(i, y, CH_TITLEBAR);
      }
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
  
    for(i = y1; i < y1 + h; ++i) {
      cputcxy(x, i, CH_WINDOWLBORDER);
      cputcxy(x2, i, CH_WINDOWRBORDER);
    }

    if(y2 >= clipy1 &&
       y2 < clipy2) {
      cputcxy(x, y2, CH_LLCORNER);
      for(i = x1; i < x2; ++i) {
	cputcxy(i, y2, CH_WINDOWLOWERBORDER);
      }
      cputcxy(x2, y2, CH_LRCORNER);
    }
  }

  draw_window_contents(window, focus, clipy1, clipy2,
		       x1, x2, (unsigned char)(y + 1), y2);

  update_area(window->x, window->y, 
    (unsigned char)(window->w + 2), 
    (unsigned char)(window->h + 2));
}
/*-----------------------------------------------------------------------------------*/
/** 
 * Draw a dialog on the terminal screen. Called by the CTK module.
 *
 * \param dialog The dialog to be drawn.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  unsigned char x, y;
  unsigned char i;
  unsigned char x1, y1, x2, y2;

  textcolor(TERM_WINDOWCOLOR + CTK_FOCUS_DIALOG);

  x = dialog->x;
  y = dialog->y + 1;


  x1 = x + 1;
  y1 = y + 1;
  x2 = x1 + dialog->w;
  y2 = y1 + dialog->h;


  /* Draw dialog frame. */
  
  for(i = y1; i < y1 + dialog->h; ++i) {
    cputcxy(x, i, CH_DIALOGLBORDER);
    cputcxy(x2, i, CH_DIALOGRBORDER);
  }
  
  for(i = x1; i < x2; ++i) {
    cputcxy(i, y, CH_DIALOGUPPERBORDER);
    cputcxy(i, y2, CH_DIALOGLOWERBORDER);
  }

  cputcxy(x, y, CH_DIALOG_ULCORNER);
  cputcxy(x, y2, CH_DIALOG_LLCORNER);
  cputcxy(x2, y, CH_DIALOG_URCORNER);
  cputcxy(x2, y2, CH_DIALOG_LRCORNER);
  
  
  /* Clear dialog contents. */
  for(i = y1; i < y2; ++i) {
    cclearxy(x1, i, dialog->w);
  }

  draw_window_contents(dialog, CTK_FOCUS_DIALOG, 0, sizey,
		       x1, x2, y1, y2);

  update_area(dialog->x, dialog->y, 
    (unsigned char)(dialog->w + 4), 
    (unsigned char)(dialog->h + 4));
}
/*-----------------------------------------------------------------------------------*/
/** 
 * Clear parts of the terminal desktop. Called by the CTK module.
 *
 * \param y1 The lower y coordinate bound.
 * \param y2 The upped y coordinate bound.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  unsigned char i;

  textcolor(TERM_BACKGROUNDCOLOR);
  for(i = y1; i < y2; ++i) {
    cclearxy(0, i, sizex);
  }

  update_area(0, y1, sizex, (unsigned char)(y2 - y1));
}
/*-----------------------------------------------------------------------------------*/
/** \internal
 * Draw one menu on the termainl desktop.
 *
 * \param m The CTK menu to be drawn.
 */
/*-----------------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m)
{
  unsigned char x, x2, y;

  textcolor(TERM_MENUCOLOR);
  x = wherex();
  cputs(m->title);
  cputc(' ');
  x2 = wherex();
  if(x + CTK_CONF_MENUWIDTH > sizex) {
    x = sizex - CTK_CONF_MENUWIDTH;
  }
  
  
  for(y = 0; y < m->nitems; ++y) {
    if(y == m->active) {
      textcolor(TERM_ACTIVEMENUCOLOR);
      revers(0);
    } else {
      textcolor(TERM_MENUCOLOR);	  
    }
    gotoxy(x, (unsigned char)(y + 1));
    if(m->items[y].title[0] == '-') {
      chline(CTK_CONF_MENUWIDTH);
    } else {
      cputs(m->items[y].title);
    }
    if(x + CTK_CONF_MENUWIDTH > wherex()) {
      cclear((unsigned char)(x + CTK_CONF_MENUWIDTH - wherex()));
    }
    revers(1);
  }
  
  gotoxy(x2, 0);
  textcolor(TERM_MENUCOLOR);  

  update_area(x, 0, CTK_CONF_MENUWIDTH, (unsigned char)(m->nitems + 1));
}
/*-----------------------------------------------------------------------------------*/
/** 
 * Draw the menus on the terminal desktop. Called by the CTK module.
 *
 * \param menus The CTK menubar.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;  

  
  /* Draw menus */
  textcolor(TERM_MENUCOLOR);
  gotoxy(0, 0);
  revers(1);
  cputc(' ');
  for(m = menus->menus->next; m != NULL; m = m->next) {
    if(m != menus->open) {
      update_area(wherex(), 0, (unsigned char)(strlen(m->title) + 1), 1);
      cputs(m->title);
      cputc(' ');
    } else {
      draw_menu(m);
    }
  }


  if(wherex() + strlen(menus->desktopmenu->title) + 1>= sizex) {
    gotoxy((unsigned char)(sizex - strlen(menus->desktopmenu->title) - 1), 0);
  } else {
    cclear((unsigned char)(sizex - wherex() - strlen(menus->desktopmenu->title) - 1));
    update_area(wherex(), 0, 
      (unsigned char)(sizex - wherex() -strlen(menus->desktopmenu->title) - 1), 
      1);
  }
  
  /* Draw desktopmenu */
  if(menus->desktopmenu != menus->open) {
    update_area(wherex(), 0, (unsigned char)(strlen(menus->desktopmenu->title) + 1), 1);
    cputs(menus->desktopmenu->title);
    cputc(' ');
  } else {
    draw_menu(menus->desktopmenu);
  }

  revers(0);
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Obtain the height of the terminal desktop. Called by the CTK module.
 *
 * \return The height of the terminal desktop, in characters.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  return sizey;
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Obtain the height of the terminal desktop. Called by the CTK module.
 *
 * \return The height of the terminal desktop, in characters.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  return sizex;
}

/*-----------------------------------------------------------------------------------*/
/**
 * Draws a character on the virtual screen. Called by the libconio module.
 *
 * \param c The character to be drawn.
 * \param xpos The x position of the character.
 * \param ypos The y position of the character.
 * \param reversedflag Determines if the character should be reversed or not.
 * \param color The color of the character.
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_arch_draw_char(char c,
		   unsigned char xpos,
		   unsigned char ypos,
		   unsigned char reversedflag,
		   unsigned char color)
{
  /* Check if out of bounds */
  if (xpos >= sizex || ypos >= sizey) {
    return;
  }
  ctk_term_out_update_screen(xpos,ypos, (unsigned char)(c & 0x7f), color);
}
