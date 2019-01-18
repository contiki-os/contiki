/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: ctk-graphic.c,v 1.1 2007/05/23 23:19:13 oliverschmidt Exp $
 *
 */

#include "ctk-draw-service.h"

#include "ctk-graphic-draw.h"

#include "ctk-graphic.h"

#include <string.h>


#define WINDOWBORDER_WIDTH  1
#define WINDOWBORDER_HEIGHT 1
#define WINDOWTITLE_HEIGHT  3
#define MENU_HEIGHT         1
#define MENUBAR_HEIGHT      12

#define FONT_BASELINE 12

#define MENUBAR_FONT_BASELINE 7

#define RASTER_X 8
#define RASTER_Y 14

#define BUTTON_HEIGHT 12
#define BUTTON_X_BORDER 4
#define BUTTON_Y_BORDER 1

#define TEXTENTRY_HEIGHT 12
#define TEXTENTRY_X_BORDER 4
#define TEXTENTRY_Y_BORDER 1

static GdkGC *white, *lightgray, *midgray, *darkgray, *ddarkgray, *black,
  *blue;

static GdkPixbuf *background, *windowborder, *menubar;

static void
draw_widget(struct ctk_widget *w,
	    unsigned char winx,
	    unsigned char winy,
	    unsigned char clipx,
	    unsigned char clipy,
	    unsigned char clipy1,
	    unsigned char clipy2,
	    unsigned char focus);


/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_init(void)
{
  GtkWidget *image;
  
  image = gtk_image_new_from_file("background.png");
  background = gtk_image_get_pixbuf(GTK_IMAGE(image));

  image = gtk_image_new_from_file("windowborder.png");
  windowborder = gtk_image_get_pixbuf(GTK_IMAGE(image));

  image = gtk_image_new_from_file("menubar.png");
  menubar = gtk_image_get_pixbuf(GTK_IMAGE(image));

  ctk_graphic_draw_init();
}
/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_clear(unsigned char y1, unsigned char y2)
{
  if(background == NULL) {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       darkgray,
		       TRUE,
		       0,
		       y1 * RASTER_Y + MENUBAR_HEIGHT,
		       CTK_GTKSIM_SCREEN_WIDTH,
		       (y2 - y1) * RASTER_Y);
  } else {
    gdk_draw_pixbuf(ctk_gtksim_pixmap,
		    darkgray,
		    background,
		    0,
		    y1 * RASTER_Y,
		    0,
		    y1 * RASTER_Y + MENUBAR_HEIGHT,
		    CTK_GTKSIM_SCREEN_WIDTH,
		    (y2 - y1) * RASTER_Y,
		    NULL, 0, 0);    
  }
}
/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_clear_window(struct ctk_window *window,
			unsigned char focus,
			unsigned char clipy1,
			unsigned char clipy2)
{
  int x, y;

  x = window->x;
  y = window->y + MENU_HEIGHT;  

  /* Clear window */
  if(focus == CTK_FOCUS_WINDOW) {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       white,
		       TRUE,
		       x * RASTER_X - 1,
		       y * RASTER_Y - 1 + MENUBAR_HEIGHT,
		       (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X + 2, 
		       (window->h + WINDOWTITLE_HEIGHT +
			            WINDOWBORDER_HEIGHT) * RASTER_Y + 2);
  } else {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       lightgray,
		       TRUE,
		       x * RASTER_X - 1,
		       y * RASTER_Y - 1 + MENUBAR_HEIGHT,
		       (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X + 2, 
		       (window->h + WINDOWTITLE_HEIGHT +
			            WINDOWBORDER_HEIGHT) * RASTER_Y + 2);    
  }
}
/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_window(struct ctk_window *window,
		  unsigned char focus,
		  unsigned char clipy1,
		  unsigned char clipy2)
{
  struct ctk_widget *w;
  unsigned char wfocus;
  int x, y, x1, y1, x2, y2;

  x = window->x;
  y = window->y + MENU_HEIGHT;
  
  x1 = x + WINDOWBORDER_WIDTH;
  y1 = y + WINDOWTITLE_HEIGHT;
  x2 = x1 + window->w;
  y2 = y1 + window->h;

  /* Draw window border */
  gdk_draw_rectangle(ctk_gtksim_pixmap,
		     darkgray,
		     FALSE,
		     x * RASTER_X,
		     y * RASTER_Y + MENUBAR_HEIGHT,
		     (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X,
		     (window->h + WINDOWTITLE_HEIGHT +
		                  WINDOWBORDER_HEIGHT) * RASTER_Y);

  if(ctk_mode_get() == CTK_MODE_WINDOWMOVE) {
    if(focus == CTK_FOCUS_WINDOW) {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 white,
			 TRUE,
			 x * RASTER_X + 1,
			 y * RASTER_Y + MENUBAR_HEIGHT + 1,
			 (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X - 2,
			 (window->h + WINDOWTITLE_HEIGHT +
			  WINDOWBORDER_HEIGHT) * RASTER_Y - 2);
    } else {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 midgray,
			 TRUE,
			 x * RASTER_X + 1,
			 y * RASTER_Y + MENUBAR_HEIGHT + 1,
			 (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X - 2,
			 (window->h + WINDOWTITLE_HEIGHT +
			  WINDOWBORDER_HEIGHT) * RASTER_Y - 2);
    }
    return;
  }
  
  if(windowborder == NULL) {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       midgray,
		       TRUE,
		       x * RASTER_X,
		       y * RASTER_Y + MENUBAR_HEIGHT,
		       (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X,
		       WINDOWTITLE_HEIGHT * RASTER_Y);
  } else {
    gdk_draw_pixbuf(ctk_gtksim_pixmap,
		    midgray,
		    windowborder,
		    0, 0,
		    x * RASTER_X,
		    y * RASTER_Y + MENUBAR_HEIGHT,
		    (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X,
		    WINDOWTITLE_HEIGHT * RASTER_Y,
		    NULL, 0, 0);    
  }
  
  
  gdk_draw_line(ctk_gtksim_pixmap,
		darkgray,
		x * RASTER_X,
		(y + WINDOWTITLE_HEIGHT) * RASTER_Y - 1 + MENUBAR_HEIGHT,
		(x + window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X,
		(y + WINDOWTITLE_HEIGHT) * RASTER_Y - 1 + MENUBAR_HEIGHT);

  
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

  ctk_gtksim_set_redrawflag();
}
/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_dialog(struct ctk_window *window)
{
  struct ctk_widget *w;
  unsigned char wfocus, focus;
  int x, y, x1, y1, x2, y2;

  focus = CTK_FOCUS_DIALOG;

  x = window->x;
  y = window->y + MENU_HEIGHT;
  
  x1 = x + WINDOWBORDER_WIDTH;
  y1 = y + WINDOWTITLE_HEIGHT;
  x2 = x1 + window->w;
  y2 = y1 + window->h;
  
  /* Draw window border */
  gdk_draw_rectangle(ctk_gtksim_pixmap,
		     black,
		     FALSE,
		     x * RASTER_X,
		     y * RASTER_Y + MENUBAR_HEIGHT,
		     (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X,
		     (window->h + WINDOWTITLE_HEIGHT +
		                  WINDOWBORDER_HEIGHT) * RASTER_Y);

  
  
  gdk_draw_rectangle(ctk_gtksim_pixmap,
		     white,
		     TRUE,
		     x * RASTER_X + 1,
		     y * RASTER_Y + 1 + MENUBAR_HEIGHT,
		     (window->w + 2 * WINDOWBORDER_WIDTH) * RASTER_X - 2,
		     (window->h + WINDOWTITLE_HEIGHT +
                                  WINDOWBORDER_HEIGHT) * RASTER_Y - 2);
  
  /* Draw inactive widgets. */
  for(w = window->inactive; w != NULL; w = w->next) {
    draw_widget(w, x1, y1, x2, y2,
		y1, y2,
		focus);
  }
  
  /* Draw active widgets. */
  for(w = window->active; w != NULL; w = w->next) {  
    wfocus = focus;
    if(w == window->focused) {
      wfocus |= CTK_FOCUS_WIDGET;
    }

   draw_widget(w, x1, y1, x2, y2, 
	       y1, y2,
	       wfocus);
  }

  ctk_gtksim_set_redrawflag();
  
}
/*--------------------------------------------------------------------------*/
static void
draw_widget(struct ctk_widget *w,
	    unsigned char winx, unsigned char winy,
	    unsigned char clipx,
	    unsigned char clipy,
	    unsigned char clipy1,
	    unsigned char clipy2,
	    unsigned char focus)
{
  char text[1000];
  unsigned char x, y;
  int width, xpos;
  int i;
  GdkGC *bgcol, *buttoncol;
  struct ctk_gtksim_draw_font *buttonfont, *textfont, *textfont_bold;
  int monospace;
  
  x = winx + w->x;
  y = winy + w->y;

  if(focus & CTK_FOCUS_WINDOW) {
    bgcol = white;
  } else {
    bgcol = lightgray;
  }

  if(focus & CTK_FOCUS_WINDOW) {
    buttoncol = lightgray;
  } else {
    buttoncol = midgray;
  }

  monospace = w->flags & CTK_WIDGET_FLAG_MONOSPACE;
  if(monospace) {
    textfont = &ctk_gtksim_draw_font_monospace;
    textfont_bold = &ctk_gtksim_draw_font_monospace_bold;
  } else {
    textfont = &ctk_gtksim_draw_font_normal;
    textfont_bold = &ctk_gtksim_draw_font_bold;
  }
  
  switch(w->type) {
  case CTK_WIDGET_SEPARATOR:
    gdk_draw_line(ctk_gtksim_pixmap,
		  darkgray,
		  x * RASTER_X,
		  y * RASTER_Y + RASTER_Y / 2 + MENUBAR_HEIGHT,
		  (x + w->w) * RASTER_X,
		  y * RASTER_Y + RASTER_Y / 2 + MENUBAR_HEIGHT);
    break;
  case CTK_WIDGET_LABEL:
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       bgcol,
		       TRUE,
		       x * RASTER_X,
		       y * RASTER_Y + MENUBAR_HEIGHT,
		       w->w * RASTER_X,
		       w->h * RASTER_Y);
    for(i = 0; i < w->h; ++i) {
      strncpy(text, &w->widget.label.text[i * w->w], w->w);
      text[w->w] = 0;
      ctk_gtksim_draw_string(textfont,
			     &ctk_gtksim_draw_color_black,
			     RASTER_X * x,
			     RASTER_Y * (y + i) + MENUBAR_HEIGHT,
			     text, monospace);
    }

    break;
  case CTK_WIDGET_BUTTON:
    if(w == (struct ctk_widget *)&w->window->titlebutton) {
      buttonfont = &ctk_gtksim_draw_font_bold;
    } else {
      buttonfont = textfont;
    }

    ++x;
    
    width = ctk_gtksim_draw_string_width(buttonfont,
					 w->widget.button.text,
					 monospace);
    if(focus == (CTK_FOCUS_WIDGET|CTK_FOCUS_WINDOW)) {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 black,
			 FALSE,
			 x * RASTER_X - BUTTON_X_BORDER,
			 y * RASTER_Y - BUTTON_Y_BORDER + MENUBAR_HEIGHT,
			 w->w * RASTER_X + BUTTON_X_BORDER * 2,
			 BUTTON_HEIGHT + BUTTON_Y_BORDER);
      /*      gdk_draw_rectangle(ctk_gtksim_pixmap,
		    ctk_gtksim_drawing_area->style->black_gc,
			 FALSE,
			 x * RASTER_X - BUTTON_X_BORDER - 1,
			 y * RASTER_Y - BUTTON_Y_BORDER - 1,
			 w->w * RASTER_X + BUTTON_X_BORDER * 2 + 1,
			 RASTER_Y + BUTTON_Y_BORDER * 2 + 1);*/
    } else {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 darkgray,
			 FALSE,
			 x * RASTER_X - BUTTON_X_BORDER,
			 y * RASTER_Y - BUTTON_Y_BORDER + MENUBAR_HEIGHT,
			 w->w * RASTER_X + BUTTON_X_BORDER * 2,
			 BUTTON_HEIGHT + BUTTON_Y_BORDER);
    }
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       buttoncol,
		       TRUE,
		       x * RASTER_X - BUTTON_X_BORDER + 1,
		       y * RASTER_Y + MENUBAR_HEIGHT,
		       w->w * RASTER_X + BUTTON_X_BORDER * 2 - 2,
		       BUTTON_HEIGHT);

    gdk_draw_line(ctk_gtksim_pixmap,
		  ddarkgray,		  
		  x * RASTER_X - BUTTON_X_BORDER + 1,
		  y * RASTER_Y + BUTTON_HEIGHT - 1 + MENUBAR_HEIGHT,
		  x * RASTER_X + w->w * RASTER_X + BUTTON_X_BORDER - 1,
		  y * RASTER_Y + BUTTON_HEIGHT - 1 + MENUBAR_HEIGHT);
    gdk_draw_line(ctk_gtksim_pixmap,
		  ddarkgray,		  		  
		  x * RASTER_X + w->w * RASTER_X + BUTTON_X_BORDER - 1,
		  y * RASTER_Y + BUTTON_HEIGHT - 1 + MENUBAR_HEIGHT,
		  x * RASTER_X + w->w * RASTER_X + BUTTON_X_BORDER - 1,
		  y * RASTER_Y + MENUBAR_HEIGHT);

    ctk_gtksim_draw_string(buttonfont,
			   &ctk_gtksim_draw_color_black,
			   RASTER_X * x +
			   (w->w * RASTER_X) / 2 - width / 2,
			   RASTER_Y * y + MENUBAR_HEIGHT,
			   w->widget.button.text,
			   monospace);
    break;
  case CTK_WIDGET_HYPERLINK:
    strncpy(text, w->widget.hyperlink.text, w->w);
    text[w->w] = 0;
    width = ctk_gtksim_draw_string_width(textfont, text, monospace);
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       bgcol,
		       TRUE,
		       x * RASTER_X,
		       y * RASTER_Y + MENUBAR_HEIGHT,
		       width,
		       RASTER_Y);
    if(focus & CTK_FOCUS_WIDGET) {
      ctk_gtksim_draw_string(textfont_bold,
			     &ctk_gtksim_draw_color_blue,
			     RASTER_X * x,
			     RASTER_Y * y + MENUBAR_HEIGHT,
			     text, monospace);
    } else {
      ctk_gtksim_draw_string(textfont,
			     &ctk_gtksim_draw_color_blue,
			     RASTER_X * x,
			     RASTER_Y * y + MENUBAR_HEIGHT,
			     text, monospace);
    }
    gdk_draw_line(ctk_gtksim_pixmap,
		  blue,		  
		  x * RASTER_X,
		  y * RASTER_Y + FONT_BASELINE + 1 + MENUBAR_HEIGHT,
		  x * RASTER_X + width,
		  y * RASTER_Y + FONT_BASELINE + 1 + MENUBAR_HEIGHT);

    break;
  case CTK_WIDGET_TEXTENTRY:
    strncpy(text, w->widget.textentry.text, sizeof(text));
    text[w->widget.textentry.xpos] = 0;
    width = ctk_gtksim_draw_string_width(textfont, text, monospace);
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       darkgray,
		       FALSE,
		       x * RASTER_X - TEXTENTRY_X_BORDER + RASTER_X,
		       y * RASTER_Y - TEXTENTRY_Y_BORDER + MENUBAR_HEIGHT,
		       w->w * RASTER_X + TEXTENTRY_X_BORDER * 2 - 1,
		       TEXTENTRY_HEIGHT + TEXTENTRY_Y_BORDER);
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       bgcol,
		       TRUE,
		       x * RASTER_X + RASTER_X,
		       y * RASTER_Y + MENUBAR_HEIGHT,
		       w->w * RASTER_X,
		       TEXTENTRY_HEIGHT);
    ctk_gtksim_draw_string(textfont,
			   &ctk_gtksim_draw_color_black,
			   RASTER_X * x + RASTER_X,
			   RASTER_Y * y + MENUBAR_HEIGHT,
			   w->widget.textentry.text,
			   monospace);
    if(focus == (CTK_FOCUS_WIDGET|CTK_FOCUS_WINDOW)) {
      gdk_draw_line(ctk_gtksim_pixmap,
		    black,
		    x * RASTER_X + width + RASTER_X,
		    y * RASTER_Y + MENUBAR_HEIGHT,
		    x * RASTER_X + width + RASTER_X,
		    y * RASTER_Y + TEXTENTRY_HEIGHT - 1 + MENUBAR_HEIGHT);
    }
    break;
  case CTK_WIDGET_ICON:
    width = ctk_gtksim_draw_string_width(&ctk_gtksim_draw_font_normal,
					 w->widget.icon.title, monospace);

    if(x * RASTER_X + width >= CTK_GTKSIM_SCREEN_WIDTH - RASTER_X) {
      xpos = CTK_GTKSIM_SCREEN_WIDTH - width - RASTER_X; 
    } else {
      xpos = x * RASTER_X;
    }
    if((focus & CTK_FOCUS_WIDGET) == 0) {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 bgcol,
			 TRUE,
			 x * RASTER_X,
			 y * RASTER_Y + MENUBAR_HEIGHT,
			 24, 24);
    } else {
      gdk_draw_rectangle(ctk_gtksim_pixmap,
			 black,
			 TRUE,
			 x * RASTER_X,
			 y * RASTER_Y + MENUBAR_HEIGHT,
			 24, 24);
    }
    
    /*      gdk_draw_rectangle(ctk_gtksim_pixmap,
	    white,
	    TRUE,
	    xpos,
	    RASTER_Y * y + 24 + 1 + MENUBAR_HEIGHT,
	    width,
	    RASTER_Y);*/
    
    ctk_gtksim_draw_string(textfont,
			   &ctk_gtksim_draw_color_black,
			   xpos + 1,
			   RASTER_Y * y + 24 + 1 + MENUBAR_HEIGHT,
			   w->widget.icon.title,
			   monospace);
    ctk_gtksim_draw_string(textfont,
			   &ctk_gtksim_draw_color_white,
			   xpos,
			   RASTER_Y * y + 24 + MENUBAR_HEIGHT,
			   w->widget.icon.title,
			   monospace);


    break;
  default:
    break;
  }
}

/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_widget(struct ctk_widget *w,
		  unsigned char focus,
		  unsigned char clipy1,
		  unsigned char clipy2)
{
  struct ctk_window *win = w->window;
  unsigned char posx, posy;

  posx = win->x + WINDOWBORDER_WIDTH;
  posy = win->y + WINDOWTITLE_HEIGHT + MENU_HEIGHT;

  if(w == win->focused) {
    focus |= CTK_FOCUS_WIDGET;
  }
  
  draw_widget(w, posx, posy,
	      posx + win->w,
	      posy + win->h,
	      clipy1, clipy2,
	      focus);
  
  ctk_gtksim_set_redrawflag();
}
/*--------------------------------------------------------------------------*/
static void
draw_menu(struct ctk_menu *m, int x, int open)
{
  int i;
  int xpos;
  
  if(x >= CTK_GTKSIM_SCREEN_WIDTH / RASTER_X - 16) {
    xpos = CTK_GTKSIM_SCREEN_WIDTH / RASTER_X - 16;
  } else {
    xpos = x;
  }
  
  if(open) {

    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       black,
		       TRUE,
		       RASTER_X * x, 0,
		       m->titlelen * RASTER_X,
		       RASTER_Y + MENUBAR_HEIGHT);
    
    ctk_gtksim_draw_string(&ctk_gtksim_draw_font_menu,
			   &ctk_gtksim_draw_color_white,
			   RASTER_X * x + 2,
			   MENUBAR_FONT_BASELINE,
			   m->title, 0);      

    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       white,
		       TRUE,
		       xpos * RASTER_X,
		       RASTER_Y * MENU_HEIGHT + MENUBAR_HEIGHT,
		       (xpos + 16) * RASTER_X,
		       (m->nitems) * RASTER_Y);
    
    for(i = 0; i < m->nitems; ++i) {
      if(i == m->active) {
	gdk_draw_rectangle(ctk_gtksim_pixmap,
			   black,
			   TRUE,
			   RASTER_X * xpos,
			   (MENU_HEIGHT + i) * RASTER_Y + MENUBAR_HEIGHT,
			   (xpos + 16) * RASTER_X,
			   RASTER_Y);
	ctk_gtksim_draw_string(&ctk_gtksim_draw_font_normal,
			       &ctk_gtksim_draw_color_white,
			       RASTER_X * xpos + 2,
			       (MENU_HEIGHT + i) * RASTER_Y +
			       MENUBAR_HEIGHT,
			       m->items[i].title, 0);

      } else {
	ctk_gtksim_draw_string(&ctk_gtksim_draw_font_normal,
			       &ctk_gtksim_draw_color_black,
			       RASTER_X * xpos + 2,
			       (MENU_HEIGHT + i) * RASTER_Y + MENUBAR_HEIGHT,
			       m->items[i].title, 0);
      }

    }
  } else {
    /*    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       white,
		       TRUE,
		       RASTER_X * x, 0,
		       m->titlelen * RASTER_X, RASTER_Y);*/
    
    ctk_gtksim_draw_string(&ctk_gtksim_draw_font_menu,
			   &ctk_gtksim_draw_color_black,
			   RASTER_X * x, MENUBAR_FONT_BASELINE,
			   m->title, 0);      

  }
}
/*--------------------------------------------------------------------------*/
static void
s_ctk_draw_menus(struct ctk_menus *menus)
{
  struct ctk_menu *m;
  int x;
  
  if(menubar != NULL) {
    gdk_draw_pixbuf(ctk_gtksim_pixmap,
		    darkgray,
		    menubar,
		    0, 0,
		    0, 0,
		    CTK_GTKSIM_SCREEN_WIDTH,
		    RASTER_Y + MENUBAR_HEIGHT,
		    NULL, 0, 0);
  }
  
  x = 1;
  for(m = menus->menus->next; m != NULL; m = m->next) {
    draw_menu(m, x, m == menus->open);
    x += strlen(m->title);
  }

  x = CTK_GTKSIM_SCREEN_WIDTH / RASTER_X - strlen(menus->menus->title);
  draw_menu(menus->menus, x, menus->menus == menus->open);
  
  

  /*  gdk_draw_line(ctk_gtksim_pixmap,
		ctk_gtksim_drawing_area->style->black_gc,
		0, RASTER_Y,
		CTK_GTKSIM_SCREEN_WIDTH,
		RASTER_Y);*/

  ctk_gtksim_set_redrawflag();
}
/*--------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_width(void)
{
  return CTK_GTKSIM_SCREEN_WIDTH / RASTER_X;
}
/*--------------------------------------------------------------------------*/
static unsigned char
s_ctk_draw_height(void)
{
  return CTK_GTKSIM_SCREEN_HEIGHT / RASTER_Y;
}
/*--------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_xtoc(unsigned short x)
{
  return x / RASTER_X;
}
/*--------------------------------------------------------------------------*/
static unsigned short
s_ctk_mouse_ytoc(unsigned short y)
{
  if(y < MENUBAR_HEIGHT) {
    return 0;
  } else {
    return (y - MENUBAR_HEIGHT) / RASTER_Y;
  }
}
/*--------------------------------------------------------------------------*/
static const struct ctk_draw_service_interface interface =
  {CTK_DRAW_SERVICE_VERSION,
   WINDOWBORDER_WIDTH,
   WINDOWBORDER_HEIGHT,
   WINDOWTITLE_HEIGHT,
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
   s_ctk_mouse_ytoc,
  };

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(proc, CTK_DRAW_SERVICE_NAME, EK_PRIO_NORMAL,
	   eventhandler, NULL, (void *)&interface);

/*--------------------------------------------------------------------------*/
EK_PROCESS_INIT(ctk_gtksim_service_init, arg)
{
  ek_service_start(CTK_DRAW_SERVICE_NAME, &proc);
}
/*--------------------------------------------------------------------------*/
static GdkGC *
get_color(unsigned short r, unsigned short g, unsigned short b)
{
  GdkGCValues values;
  GdkColor color;

  color.pixel = 0;
  color.red = r;
  color.green = g;
  color.blue = b;

  if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
			      &color, FALSE, TRUE)) {
  }
  
  values.foreground = color;
  
  return gdk_gc_new_with_values(ctk_gtksim_drawing_area->window,
				&values,
				GDK_GC_FOREGROUND);  
}
/*--------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  EK_EVENTHANDLER_ARGS(ev, data);
  
  switch(ev) {
  case EK_EVENT_INIT:
    blue = get_color(0, 0, 0xffff);
    white = get_color(0xffff, 0xffff, 0xffff);
    lightgray = get_color(0xefff, 0xefff, 0xefff);
    midgray = get_color(0xdfff, 0xdfff, 0xdfff);
    darkgray = get_color(0xcfff, 0xcfff, 0xcfff);
    ddarkgray = get_color(0xafff, 0xafff, 0xafff);
    black = get_color(0, 0, 0);
    break;
  case EK_EVENT_REQUEST_REPLACE:
    ek_replace((struct ek_proc *)data, NULL);
    LOADER_UNLOAD();
    break;    
  }
}
/*--------------------------------------------------------------------------*/
