/*
 * Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: display.c,v 1.10 2010/02/23 18:44:08 adamdunkels Exp $
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "dev/leds.h"
#include "display.h"
#include "nodes.h"
#include "node.h"
#include "ether.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "sensor.h"

#include <gtk/gtk.h>

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static GdkPixmap *pixmap = NULL;
static GtkWidget *drawing_area;
static GdkFont *font;

#define DISPLAY_WIDTH 400
#define DISPLAY_HEIGHT 400

#define BASESTATION_SIZE 4

#define MAPSCALE 20
#define SCALE 2

#define MARK_SIZE 8

#define RADIO_SIZE 20

#define DOT_SIZE ether_strength()
#define DOT_INTENSITY 3

struct dot {
  struct dot *next;
  int x, y;
  int destx, desty;
  int size;
  int intensity;
};

MEMB(dotsmem, struct dot, 20000);
LIST(dots);
LIST(tempdots);

static int window_is_open;

static GdkGC *intensity_gcs[DOT_INTENSITY];

static GdkGC *intensity_clusterhead;
static GdkGC *intensity_clusterhead_lightgray;
static GdkGC *intensity_clusterhead_red;

static GdkGC *green, *red, *yellow, *black, *white;

static struct nodes_node *marked_node;

/*-----------------------------------------------------------------------------------*/
void
display_redraw(void)
{
  int i;
  struct nodes_node *n;
  int x, y;
  struct dot *d;

  if(!window_is_open) {
    return;
  }
  
  gdk_draw_rectangle(pixmap,
		     white,
		     TRUE,
		     0, 0,
		     drawing_area->allocation.width,
		     drawing_area->allocation.height);
  

  for(i = 0; i < nodes_num(); ++i) {
    n = nodes_node(i);
    x = n->x;
    y = n->y;
    
    /*    if(n->type == NODE_TYPE_CLUSTERHEAD) {
      gdk_draw_arc(pixmap,
		   intensity_clusterhead_lightgray,
		   TRUE,
		   x * SCALE - DOT_SIZE * SCALE,
		   y * SCALE - DOT_SIZE * SCALE,
		   DOT_SIZE * 2 * SCALE, DOT_SIZE * 2 * SCALE,
		   0, 360 * 64);
		   }*/

    if(n == marked_node) {
      gdk_draw_arc(pixmap,
		   red,
		   FALSE,
		   x * SCALE - MARK_SIZE * SCALE,
		   y * SCALE - MARK_SIZE * SCALE,
		   MARK_SIZE * 2 * SCALE, MARK_SIZE * 2 * SCALE,
		   0, 360 * 64);
    }

  }
    
  for(i = 0; i < nodes_num(); ++i) {
    n = nodes_node(i);
    x = n->x;
    y = n->y;
    
    /*    if(n->type == NODE_TYPE_CLUSTERHEAD) {
      gdk_draw_rectangle(pixmap,
			 intensity_clusterhead_red,
			 TRUE,
			 x * SCALE,
			 y * SCALE,
			 3, 3);
      for(j = 0; j < nodes_num(); ++j) {
	m = nodes_node(j);
	if(m->type == NODE_TYPE_CLUSTERHEAD &&
	   ((x - m->x) * (x - m->x) +
	    (y - m->y) * (y - m->y) < ether_strength() * ether_strength())) {
	  gdk_draw_line(pixmap,
			intensity_clusterhead,
			x * SCALE,
			y * SCALE,
			m->x * SCALE,
			m->y * SCALE);
	  
	  
	}
	}
	} else */ {

      if(strlen(n->text) > 0) {
	gdk_draw_string(pixmap,
			font,
			black,
			x * SCALE + 10,
			y * SCALE + 7,
			n->text);
	
      }
      gdk_draw_rectangle(pixmap,
			 black,
			 TRUE,
			 x * SCALE,
			 y * SCALE,
			 2, 2);
      /*      gdk_draw_rectangle(pixmap,
			 drawing_area->style->white_gc,
			 TRUE,
			 x * SCALE,
			 y * SCALE,
			 2, 2);*/
      if(n->leds & LEDS_GREEN) {
	gdk_draw_rectangle(pixmap,
			   green,
			   TRUE,
			   x * SCALE + 2,
			   y * SCALE,
			   4, 4);
      }
      if(n->leds & LEDS_YELLOW) {
	gdk_draw_rectangle(pixmap,
			   yellow,
			   TRUE,
			   x * SCALE,
			   y * SCALE + 2,
			   4, 4);
      }
      if(n->leds & LEDS_RED) {
	gdk_draw_rectangle(pixmap,
			   red,
			   TRUE,
			   x * SCALE + 2,
			   y * SCALE + 2,
			   4, 4);
      }
      if(n->linex != 0 && n->liney != 0) {
	gdk_draw_line(pixmap,
		      green,
		      x * SCALE,
		      y * SCALE,
		      n->linex * SCALE,
		      n->liney * SCALE);
	gdk_draw_rectangle(pixmap,
			   green,
			   TRUE,
			   n->linex * SCALE - 2,
			   n->liney * SCALE - 2,
			   4, 4);
      }

      if(n->radio_status) {
	gdk_draw_arc(pixmap,
		     green,
		     FALSE,
		     x * SCALE - RADIO_SIZE * SCALE,
		     y * SCALE - RADIO_SIZE * SCALE,
		     RADIO_SIZE * 2 * SCALE, RADIO_SIZE * 2 * SCALE,
		     0, 360 * 64);
      }


    }

  }

  for(d = list_head(dots); d != NULL; d = d->next) {
    gdk_draw_arc(pixmap,
		 intensity_gcs[d->intensity - 1],
		 FALSE,
		 d->x * SCALE - d->size * SCALE,
		 d->y * SCALE - d->size * SCALE,
		 d->size * 2 * SCALE, d->size * 2 * SCALE,
		 0, 360 * 64);
  }
  
  
  gtk_widget_draw(drawing_area, NULL);

}
/*-----------------------------------------------------------------------------------*/
void
display_tick(void)
{
  struct dot *d, *e;
  struct ether_packet *p;

  if(!window_is_open) {
    return;
  }
  
  /* Fade out active dots. The intensity value of each dot is counted
     downwards, and those dots that still have an intensity are placed
     in a temporary list. The temporary list is then copied into the
     list of all dots. */

  list_init(tempdots);
  
  for(d = list_head(dots);
      d != NULL;
      d = e) {
    if(d != NULL) {
      e = d->next;
    } else {
      e = NULL;
    }
    if(d->size > 20) {
      d->size /= 2;
    } else {
      d->size -= 4;
    }
    /*    --(d->intensity);*/
    if(d->size > 0) {
      list_push(tempdots, d);
    } else {
      memb_free(&dotsmem, (void *)d);
    }
  }
  list_copy(dots, tempdots);
  
  /* Check if there are any new dots that should be placed in the list. */
  for(p = ether_packets(); p != NULL; p = p->next) {
    d = (struct dot *)memb_alloc(&dotsmem);
    
    if(d != NULL) {
      d->x = p->x;
      d->y = p->y;
      d->size = DOT_SIZE;
      d->intensity = DOT_INTENSITY;
      list_push(dots, d);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static gint
configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
  if(pixmap != NULL) {
    gdk_pixmap_unref(pixmap);
  }

  pixmap = gdk_pixmap_new(widget->window,
			  widget->allocation.width,
			  widget->allocation.height,
			  -1);

  if(pixmap == NULL) {
    printf("gdk_pixmap_new == NULL\n");
    exit(1);
  }
  gdk_draw_rectangle(pixmap,
		     widget->style->black_gc,
		     TRUE,
		     0, 0,
		     widget->allocation.width,
		     widget->allocation.height);
  /*  draw_screen();*/
  return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gint
expose_event (GtkWidget * widget, GdkEventExpose * event)
{
  /*  draw_screen();*/
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);
  return FALSE;
}

static gint
key_press_event (GtkWidget * widget, GdkEventKey * event)
{
  /*  if(event->keyval == GDK_Shift_L ||
     event->keyval == GDK_Shift_R) {
     return TRUE;
  }
  keys[lastkey] = event->keyval;
  ++lastkey;
  if(lastkey >= NUMKEYS) {
    lastkey = 0;
    }*/

  if(event->keyval == 'q') {
    gtk_exit(0);
    /*   exit(0);*/
  }
  if(event->keyval == 'p') {
    display_output_fig();
  }
  return TRUE;
}

static gint
key_release_event (GtkWidget * widget, GdkEventKey * event)
{
  return TRUE;
}

static gint
button_press_event (GtkWidget * widget, GdkEventKey * event)
{
  struct dot *d;
  struct sensor_data s;
  GdkModifierType state;
  int x, y;

  gdk_window_get_pointer (event->window, &x, &y, &state);
  
  x = ((GdkEventButton*)event)->x / SCALE;
  y = ((GdkEventButton*)event)->y / SCALE;

  if(state & GDK_BUTTON1_MASK) {
    d = (struct dot *)memb_alloc(&dotsmem);
    
    if(d != NULL) {
      d->x = x;
      d->y = y;
      d->size = sensor_strength();
      d->intensity = DOT_INTENSITY - 2;
      list_push(dots, d);
    }
    sensor_data_init(&s);
    s.pir = 1;
    s.button = 0;
    s.vib = 0;
    ether_send_sensor_data(&s, x, y, sensor_strength());
  } else if(state & GDK_BUTTON2_MASK) {
    sensor_data_init(&s);
    s.pir = 0;
    s.button = 1;
    s.vib = 0;
    if(marked_node != NULL) {
      ether_send_sensor_data(&s, marked_node->x, marked_node->y, 1);
    }
  } else if(state & GDK_BUTTON3_MASK) {
    sensor_data_init(&s);
    s.pir = 0;
    s.button = 0;
    s.vib = 1;
    if(marked_node != NULL) {
      ether_send_sensor_data(&s, marked_node->x, marked_node->y, 1);
    }
  }
  
  return TRUE;
}

static gint
pointer_motion_event (GtkWidget * widget, GdkEventMotion * event)
{
  struct dot *d;
  struct sensor_data s;
  GdkModifierType state;

  int x, y;
  struct nodes_node *node, *closest;
  int nodex, nodey;
  unsigned long dist;
  int i;
  
  if(event->is_hint) {
    return TRUE;
  }
  
  gdk_window_get_pointer (event->window, &x, &y, &state);
  x /= SCALE;
  y /= SCALE;
   
  
  if(state & GDK_BUTTON1_MASK) {
    d = (struct dot *)memb_alloc(&dotsmem);

    if(d != NULL) {
      d->x = x;
      d->y = y;
      d->size = sensor_strength();
      d->intensity = DOT_INTENSITY - 2;
      list_push(dots, d);
    }
    sensor_data_init(&s);
    s.pir = 1;
    ether_send_sensor_data(&s, x, y, sensor_strength());
  } else {

    
    /* Find the closest node and mark it. */
    closest = NULL;
    dist = 0;
    for(i = 0; i < nodes_num(); ++i) {
      node = nodes_node(i);
      nodex = node->x;
      nodey = node->y;

      if(closest == NULL ||
	 (x - nodex) * (x - nodex) + (y - nodey) * (y - nodey) < dist) {
	dist = (x - nodex) * (x - nodex) + (y - nodey) * (y - nodey);
	closest = node;
      }
    }
    marked_node = closest;
  }
  return TRUE;
}

static void
quit(void)
{
  gtk_exit(0);
}
/*-----------------------------------------------------------------------------------*/
static void (* idle)(void);
static gint
idle_callback(gpointer data)
{
  idle();
  return TRUE;
}
/*-----------------------------------------------------------------------------------*/
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
  
  return gdk_gc_new_with_values(drawing_area->window,
				&values,
				GDK_GC_FOREGROUND);
}
/*-----------------------------------------------------------------------------------*/
static void
stdin_callback(gpointer data, gint source, GdkInputCondition condition)
{
  char buf[1000];
  int len;

  len = read(STDIN_FILENO, &buf, sizeof(buf));
  printf("read len %d\n", len);
  buf[len] = 0;
  ether_send_serial(buf);
}
/*-----------------------------------------------------------------------------------*/
void
display_init(void (* idlefunc)(void), int time, int with_gui)
{
  int i;
  GtkWidget *window;
  GtkWidget *vbox;
  GdkGCValues values;
  GdkColor color;

  memb_init(&dotsmem);
  list_init(dots);
  list_init(tempdots);
  
  gtk_init(NULL, NULL);
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Contiki simulation display");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER (window), vbox);
  gtk_widget_show(vbox);

  gtk_signal_connect(GTK_OBJECT (window), "destroy",
		     GTK_SIGNAL_FUNC (quit), NULL);

  font = gdk_font_load("-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
  
  /* Create the drawing area */

  drawing_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA (drawing_area),
			DISPLAY_WIDTH,
			DISPLAY_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

  gtk_widget_show(drawing_area);
  
  /* Signals used to handle backing pixmap */

  gtk_signal_connect(GTK_OBJECT (drawing_area), "expose_event",
		     (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect(GTK_OBJECT (drawing_area), "configure_event",
		     (GtkSignalFunc) configure_event, NULL);

  /* Event signals */

  gtk_signal_connect(GTK_OBJECT (window), "key_press_event",
		     (GtkSignalFunc) key_press_event, NULL);
  gtk_signal_connect(GTK_OBJECT (window), "key_release_event",
		     (GtkSignalFunc) key_release_event, NULL);

  gtk_signal_connect(GTK_OBJECT (window), "button_press_event",
		     (GtkSignalFunc) button_press_event, NULL);

  gtk_signal_connect(GTK_OBJECT (window), "motion_notify_event",
		     (GtkSignalFunc) pointer_motion_event, NULL);

  gtk_widget_set_events(drawing_area,GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK);

  /*  gtk_window_iconify(window);*/
  if(with_gui) {
    gtk_widget_show(window);
    window_is_open = with_gui;
  }


  idle = idlefunc;
  gtk_timeout_add(time, idle_callback, NULL);

  if(with_gui) {
    
    for(i = 0; i < DOT_INTENSITY; ++i) {
      color.pixel = 0;
      color.red = 0;
      color.green = ((DOT_INTENSITY + 1) * 0xffff) / (i + 1);
      color.blue = ((DOT_INTENSITY + 1) * 0xffff) / (i + 1);

      if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
				  &color, FALSE, TRUE)) {
      }
    
      values.foreground = color;

      intensity_gcs[i] = gdk_gc_new_with_values(drawing_area->window,  &values,
						GDK_GC_FOREGROUND);
    }

    color.pixel = 0;
    color.red = 0xbfff;
    color.green = 0xbfff;
    color.blue = 0xbfff;
  
    if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
				&color, FALSE, TRUE)) {
    }
  
    values.foreground = color;
  
    intensity_clusterhead = gdk_gc_new_with_values(drawing_area->window,  &values,
						   GDK_GC_FOREGROUND);

    color.pixel = 0;
    color.red = 0xefff;
    color.green = 0xefff;
    color.blue = 0xefff;
  
    if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
				&color, FALSE, TRUE)) {
    }
  
    values.foreground = color;
  
    intensity_clusterhead_lightgray = gdk_gc_new_with_values(drawing_area->window,  &values,
							     GDK_GC_FOREGROUND);

    color.pixel = 0;
    color.red = 0xffff;
    color.green = 0;
    color.blue = 0;
  
    if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
				&color, FALSE, TRUE)) {
    }
  
    values.foreground = color;
  
    intensity_clusterhead_red = gdk_gc_new_with_values(drawing_area->window,  &values,
						       GDK_GC_FOREGROUND);


    red = get_color(0xffff, 0, 0);
    green = get_color(0, 0xffff, 0);
    yellow = get_color(0xffff, 0xffff, 0);
    black = get_color(0, 0, 0);
    white = get_color(0xffff, 0xffff, 0xffff);
  }

  gdk_input_add(STDIN_FILENO, GDK_INPUT_READ, stdin_callback, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
display_run(void)
{
  gtk_main();
}
/*-----------------------------------------------------------------------------------*/
void
display_output_fig(void)
{
  int i;
  struct nodes_node *n;
  int x, y;
  int dot_radius = 75;
  int scale = 50;
  FILE *fp;
  char name[40];
  struct timeval tv;

  gettimeofday(&tv, NULL);
  snprintf(name, sizeof(name), "network-%lu.fig", tv.tv_sec);
  
  fp = fopen(name, "w");
  fprintf(fp, "#FIG 3.2\n"
	 "Landscape\n"
	 "Center\n"
	 "Inches\n"
	 "Letter\n"
	 "100.00\n"
	 "Single\n"
	 "-2\n"
	 "1200 2\n"
	 );

  for(i = 0; i < nodes_num(); ++i) {
    n = nodes_node(i);
    x = n->x * scale;
    y = n->y * scale;

    fprintf(fp, "1 3 1 1 0 7 50 -1 0 4.000 1 0.0000 %d %d %d %d %d %d %d %d\n",
	   x, y,
	   dot_radius, dot_radius,
	   x, y,
	   x + dot_radius, y + dot_radius);

    if(strlen(n->text) > 0) {
      fprintf(fp, "4 0 0 50 -1 16 18 0.0000 4 135 720 %d %d %s\\001\n",
	      x + 2 * scale, y, n->text);
    }

    if(n->linex != 0 && n->liney != 0) {
      fprintf(fp, "2 1 1 1 0 7 50 -1 -1 0.000 0 0 -1 0 1 2\n"
	      "1 1 4.00 60.00 120.00\n"
	      "%d %d %d %d\n",
	      x, y,
	      n->linex * scale, n->liney * scale);
    }

  }

  fclose(fp);
}
/*-----------------------------------------------------------------------------------*/
