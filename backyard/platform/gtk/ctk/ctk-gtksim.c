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
 * $Id: ctk-gtksim.c,v 1.1 2008/01/05 21:08:26 oliverschmidt Exp $
 *
 */

/* This file provides a very simple implementation of CTK using the
   GTK (Gimp toolkit) under FreeBSD/Linux. */


#include "lib/libconio.h"

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "ctk/ctk-mouse.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FONT_HEIGHT 14
#define FONT_WIDTH  8
#define FONT_HEIGHT_BASELINE 11

GdkPixmap *ctk_gtksim_pixmap = NULL;
GtkWidget *ctk_gtksim_drawing_area;
static GdkFont *font;

static int mouse_x, mouse_y, mouse_button;

static int redrawflag;

void
ctk_gtksim_redraw(void)
{
  if(redrawflag) {
    gdk_draw_pixmap(ctk_gtksim_drawing_area->window,
		    ctk_gtksim_drawing_area->style->fg_gc[GTK_WIDGET_STATE (ctk_gtksim_drawing_area)],
		    ctk_gtksim_pixmap,
		    0,0,0,0,
		    CTK_GTKSIM_SCREEN_WIDTH, CTK_GTKSIM_SCREEN_HEIGHT);
    redrawflag = 0;
  }

}
/*-----------------------------------------------------------------------------------*/
void
ctk_gtksim_set_redrawflag(void)
{
  redrawflag = 1;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_arch_draw_char(char c,
		   unsigned char x, unsigned char y,
		   unsigned char reversed,
		   unsigned char color)
{
  char str[2];

  str[0] = c;
  str[1] = 0;
  
  if(reversed) {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       ctk_gtksim_drawing_area->style->black_gc,
		       TRUE,
		       x * FONT_WIDTH,
		       y * FONT_HEIGHT,
		       FONT_WIDTH, FONT_HEIGHT);

    gdk_draw_string(ctk_gtksim_pixmap,
		    font,
		    ctk_gtksim_drawing_area->style->white_gc,
		    x * FONT_WIDTH, FONT_HEIGHT_BASELINE + y * FONT_HEIGHT,
		    str);
  } else {
    gdk_draw_rectangle(ctk_gtksim_pixmap,
		       ctk_gtksim_drawing_area->style->white_gc,
		       TRUE,
		       x * FONT_WIDTH,
		       y * FONT_HEIGHT,
		       FONT_WIDTH, FONT_HEIGHT);
    
    gdk_draw_string(ctk_gtksim_pixmap,
		    font,
		    ctk_gtksim_drawing_area->style->black_gc,
		    x * FONT_WIDTH, FONT_HEIGHT_BASELINE + y * FONT_HEIGHT,
		    str);
  }

  gdk_draw_pixmap(ctk_gtksim_drawing_area->window,
		  ctk_gtksim_drawing_area->style->fg_gc[GTK_WIDGET_STATE (ctk_gtksim_drawing_area)],
		  ctk_gtksim_pixmap,
		  x * FONT_WIDTH,
		  y * FONT_HEIGHT,
		  x * FONT_WIDTH,
		  y * FONT_HEIGHT,
		  FONT_WIDTH, FONT_HEIGHT);

}
/*-----------------------------------------------------------------------------------*/
#define NUMKEYS 100
static ctk_arch_key_t keys[NUMKEYS];
static int firstkey, lastkey;

unsigned char
ctk_arch_keyavail(void)
{
  return firstkey != lastkey;
}
/*-----------------------------------------------------------------------------------*/
ctk_arch_key_t
ctk_arch_getkey(void)
{
  ctk_arch_key_t key;
  key = keys[firstkey];

  if(firstkey != lastkey) {
    ++firstkey;
    if(firstkey >= NUMKEYS) {
      firstkey = 0;
    }
  }
  
  return key;  
}
/*-----------------------------------------------------------------------------------*/
static gint
configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
  if(ctk_gtksim_pixmap != NULL) {
    gdk_pixmap_unref(ctk_gtksim_pixmap);
  }
  
    ctk_gtksim_pixmap = gdk_pixmap_new(widget->window,
			  widget->allocation.width,
			  widget->allocation.height,
			    -1);

  if(ctk_gtksim_pixmap == NULL) {
    printf("gdk_pixmap_new == NULL\n");
    exit(1);
  }
  gdk_draw_rectangle(ctk_gtksim_pixmap,
		     widget->style->white_gc,
		     TRUE,
		     0, 0,
		     widget->allocation.width,
		     widget->allocation.height);
  
  return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gint
expose_event (GtkWidget * widget, GdkEventExpose * event)
{
  /*  draw_screen();*/
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  ctk_gtksim_pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);
  return FALSE;
}

static gint
key_press_event (GtkWidget * widget, GdkEventKey * event)
{
  if(event->keyval == GDK_Shift_L ||
     event->keyval == GDK_Shift_R) {
    return TRUE;
  }
  keys[lastkey] = event->keyval;
  ++lastkey;
  if(lastkey >= NUMKEYS) {
    lastkey = 0;
  }
    
  return TRUE;
}

static gint
key_release_event (GtkWidget * widget, GdkEventKey * event)
{
  return TRUE;
}

static gint
motion_notify_event (GtkWidget * widget, GdkEventMotion * event)
{
  int x, y;
  GdkModifierType state;
  
  if (event->is_hint) {
    gdk_window_get_pointer (event->window, &x, &y, &state);
  } else {
    x = event->x;
    y = event->y;  
  }
  
  mouse_x = x;
  mouse_y = y;
  return TRUE;
}

static gint
button_press_event (GtkWidget * widget, GdkEventButton * event)
{
  mouse_button = event->button;
  return TRUE;
}

static gint
button_release_event (GtkWidget * widget, GdkEventButton * event)
{
  mouse_button = 0;
  return TRUE;
}

static void
quit(void)
{
  gtk_exit(0);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_gtksim_init(void)
{
  GtkWidget *window;
#if 0
  GtkWidget *vbox;
#endif
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(window, "Contiki GTKsim");

#if 0
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER (window), vbox);
  gtk_widget_show(vbox);
#endif
  gtk_signal_connect(GTK_OBJECT (window), "destroy",
		     GTK_SIGNAL_FUNC (quit), NULL);
  
  /* Create the drawing area */

  ctk_gtksim_drawing_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA (ctk_gtksim_drawing_area),
			CTK_GTKSIM_SCREEN_WIDTH,
			CTK_GTKSIM_SCREEN_HEIGHT);
#if 0
  gtk_box_pack_start(GTK_BOX(vbox), ctk_gtksim_drawing_area, TRUE, TRUE, 0);
#else
  gtk_container_add(GTK_CONTAINER(window), ctk_gtksim_drawing_area);
#endif

  gtk_widget_show(ctk_gtksim_drawing_area);

  /* Load a fixed width font. */
  /*  font = gdk_font_load("-*-gamow-medium-r-*-*-*-90-*-*-*-*-*-*");*/
  /*  font = gdk_font_load("-*-courier-*-r-normal-*-14-*-*-*-m-*-iso8859-1");*/
  font = gdk_font_load("-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
  if(font != NULL) {
    printf("Font loaded OK\n");
  } else {
    printf("Font loading failed\n");
    exit(1);
  }

  
  /* Signals used to handle backing pixmap */

  gtk_signal_connect(GTK_OBJECT (ctk_gtksim_drawing_area), "expose_event",
		     (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect(GTK_OBJECT (ctk_gtksim_drawing_area), "configure_event",
		     (GtkSignalFunc) configure_event, NULL);

  /* Event signals */

  gtk_signal_connect(GTK_OBJECT (window), "key_press_event",
		     (GtkSignalFunc) key_press_event, NULL);
  gtk_signal_connect(GTK_OBJECT (window), "key_release_event",
		     (GtkSignalFunc) key_release_event, NULL);

  gtk_signal_connect(GTK_OBJECT (ctk_gtksim_drawing_area), "motion_notify_event",
		     (GtkSignalFunc) motion_notify_event, NULL);

  gtk_signal_connect(GTK_OBJECT (ctk_gtksim_drawing_area), "button_press_event",
		     (GtkSignalFunc) button_press_event, NULL);

  gtk_signal_connect(GTK_OBJECT (ctk_gtksim_drawing_area), "button_release_event",
		     (GtkSignalFunc) button_release_event, NULL);

  gtk_widget_set_events(ctk_gtksim_drawing_area, gtk_widget_get_events (ctk_gtksim_drawing_area) 
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK);

  gtk_widget_show(window);

}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  return mouse_x;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  return mouse_y;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  return mouse_button;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_hide(void)
{
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_show(void)
{
}
/*-----------------------------------------------------------------------------------*/
