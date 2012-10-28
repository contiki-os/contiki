/**
 * \defgroup ctk CTK graphical user interface
 *
 * The Contiki Toolkit (CTK) provides the graphical user interface for
 * the Contiki system.
 *
 * @{
 */

/**
 * \file
 * The Contiki Toolkit CTK, the Contiki GUI.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 *
 */

#include <string.h>

#include "contiki.h"

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "ctk/ctk-mouse.h"

static unsigned char height, width;

static unsigned char mode;

#if CTK_CONF_WINDOWS
static struct ctk_window desktop_window;
static struct ctk_window *windows;
static struct ctk_window *dialog;
#else /* CTK_CONF_WINDOWS */
static struct ctk_window *window;
#endif /* CTK_CONF_WINDOWS */

#if CTK_CONF_MENUS
static struct ctk_menus menus;
static struct ctk_menu *lastmenu;
static struct ctk_menu desktopmenu;
static unsigned char maxnitems;
#endif /* CTK_CONF_MENUS */

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#define REDRAW_NONE         0
#define REDRAW_ALL          1
#define REDRAW_FOCUS        2
#define REDRAW_WIDGETS      4
#define REDRAW_MENUS        8
#define REDRAW_MENUPART     16

#define MAX_REDRAWWIDGETS 4
static unsigned char redraw;
static struct ctk_widget *redraw_widgets[MAX_REDRAWWIDGETS];
static unsigned char redraw_widgetptr;

#if CTK_CONF_ICONS
static unsigned char iconx, icony;
#define ICONX_START  (width - 6)
#define ICONY_START  (height - 6 - CTK_CONF_MENUS)
#define ICONX_DELTA  -16
#define ICONY_DELTA  -5
#define ICONY_MAX    height
#endif /* CTK_CONF_ICONS */

#ifndef ctk_arch_keyavail
unsigned char ctk_arch_keyavail(void);
#endif /* ctk_arch_keyavail */

#ifndef ctk_arch_getkey
ctk_arch_key_t ctk_arch_getkey(void);
#endif /* ctk_arch_getkey */

#ifndef ctk_arch_isprint
unsigned char ctk_arch_isprint(ctk_arch_key_t key);
#endif /* ctk_arch_isprint */

PROCESS(ctk_process, "CTK Contiki GUI");

/**
 * \defgroup ctkevents CTK events
 * @{
 */
process_event_t

  /**
   * Emitted for every key being pressed.
   *
   * The key is passed as signal data.*/
  ctk_signal_keypress,
  
  /** Emitted when a widget is activated (pressed). A pointer to the
      widget is passed as signal data. */
  ctk_signal_widget_activate,
  
  /** Same as ctk_signal_widget_activate. */
  ctk_signal_button_activate,

  /** Emitted when a widget is selected. A pointer to the widget is
      passed as signal data. */
  ctk_signal_widget_select,
  
  /** Same as ctk_signal_widget_select. */
  ctk_signal_button_hover,

  /** Emitted when a hyperlink is activated. The signal is broadcast
      to all listeners. */
  ctk_signal_hyperlink_activate,

  /** Same as ctk_signal_widget_select. */
  ctk_signal_hyperlink_hover;

  /** Emitted when a menu item is activated. The number of the menu
      item is passed as signal data. */
process_event_t ctk_signal_menu_activate;

  /** Emitted when a window is closed. A pointer to the window is
      passed as signal data. */
process_event_t ctk_signal_window_close;

#if CTK_CONF_MOUSE_SUPPORT
  /** Emitted when the mouse pointer is moved. A NULL pointer is
      passed as signal data and it is up to the listening process to
      check the position of the mouse using the CTK mouse API.*/
process_event_t ctk_signal_pointer_move,
  /** Emitted when a mouse button is pressed. The button is passed as
      signal data to the listening process. */
  ctk_signal_pointer_button;
#endif /* CTK_CONF_MOUSE_SUPPORT */

#if CTK_CONF_SCREENSAVER
/** Emitted when the user has been idle long enough for the
    screensaver to start. */
process_event_t ctk_signal_screensaver_stop,
  /** Emitted when the user presses a key or moves the mouse when the
      screensaver is active. */
  ctk_signal_screensaver_start;
#endif /* CTK_CONF_SCREENSAVER */

/** @} */

#if CTK_CONF_MOUSE_SUPPORT
unsigned short mouse_x, mouse_y, mouse_button;
#endif /* CTK_CONF_MOUSE_SUPPORT */

#if CTK_CONF_SCREENSAVER
static unsigned short screensaver_timer = 0;
unsigned short ctk_screensaver_timeout = (5*60);
static struct timer timer;
#endif /* CTK_CONF_SCREENSAVER */

static void CC_FASTCALL
textentry_input(ctk_arch_key_t c,
		CC_REGISTER_ARG struct ctk_textentry *t);

#if CTK_CONF_MENUS
/*---------------------------------------------------------------------------*/
/**
 * \internal Creates the Desktop menu.
 *
 * Creates the leftmost menu, "Desktop". Since the desktop menu
 * contains the list of all open windows, this function will be called
 * whenever a window is opened or closed.
 */
/*---------------------------------------------------------------------------*/
static void
make_desktopmenu(void)
{
  struct ctk_window *w;
  
  desktopmenu.nitems = 0;
  
  if(windows == NULL) {
    ctk_menuitem_add(&desktopmenu, "(No windows)");
  } else {
    for(w = windows; w != NULL; w = w->next) {
      ctk_menuitem_add(&desktopmenu, w->title);
    }
  }
}
#endif /* CTK_CONF_MENUS */
/*---------------------------------------------------------------------------*/
#if CTK_CONF_ICONS
static void
arrange_icons(void)
{
  struct ctk_widget *icon;

  iconx = ICONX_START;
  icony = ICONY_START;
  
  for(icon = desktop_window.active; icon != NULL; icon = icon->next) {
    
    icon->x = iconx;
    icon->y = icony;
    
    icony += ICONY_DELTA;
    if(icony >= ICONY_MAX) {
      icony = ICONY_START;
      iconx += ICONX_DELTA;
    }
  }
}
#endif /* CTK_CONF_ICONS */
/*---------------------------------------------------------------------------*/
void
ctk_restore(void)
{
  ctk_draw_init();

  height = ctk_draw_height();
  width = ctk_draw_width();

#if CTK_CONF_ICONS
  arrange_icons();
#endif /* CTK_CONF_ICONS */

  redraw = REDRAW_ALL;
}
/*---------------------------------------------------------------------------*/

/**
 * \addtogroup ctkappfunc
 * @{
 */

/*---------------------------------------------------------------------------*/
/**
 * Sets the current CTK mode.
 *
 * The CTK mode can be either CTK_MODE_NORMAL, CTK_MODE_SCREENSAVER or
 * CTK_MODE_EXTERNAL. CTK_MODE_NORMAL is the normal mode, in which
 * keypresses and mouse pointer movements are processed and the screen
 * is redrawn. In CTK_MODE_SCREENSAVER, no screen redraws are
 * performed and the first key press or pointer movement will cause
 * the ctk_signal_screensaver_stop to be emitted. In the
 * CTK_MODE_EXTERNAL mode, key presses and pointer movements are
 * ignored and no screen redraws are made.
 *
 * \param m The mode.
 */
/*---------------------------------------------------------------------------*/
void
ctk_mode_set(unsigned char m) {
  mode = m;
}
/*---------------------------------------------------------------------------*/
/**
 * Retrieves the current CTK mode.
 *
 * \return The current CTK mode.
 */
/*---------------------------------------------------------------------------*/
unsigned char
ctk_mode_get(void) {
  return mode;
}
/*---------------------------------------------------------------------------*/
/**
 * Add an icon to the desktop.
 *
 * \param icon The icon to be added.
 *
 * \param p The process that owns the icon.
 */
/*---------------------------------------------------------------------------*/
void
ctk_icon_add(CC_REGISTER_ARG struct ctk_widget *icon, struct process *p)
{
#if CTK_CONF_ICONS
  icon->widget.icon.owner = p;
  ctk_widget_add(&desktop_window, icon);
  arrange_icons();
#endif /* CTK_CONF_ICONS */
}
#if CTK_CONF_WINDOWS
/*---------------------------------------------------------------------------*/
/**
 * Open a dialog box.
 *
 * \param d The dialog to be opened.
 */
/*---------------------------------------------------------------------------*/
void
ctk_dialog_open(struct ctk_window *d)
{
  dialog = d;
  redraw |= REDRAW_FOCUS;
}
/*---------------------------------------------------------------------------*/
/**
 * Close the dialog box, if one is open.
 *
 */
/*---------------------------------------------------------------------------*/
void
ctk_dialog_close(void)
{
  dialog = NULL;
  redraw |= REDRAW_ALL;
}
#endif /* CTK_CONF_WINDOWS */
/*---------------------------------------------------------------------------*/
/**
 * Open a window, or bring window to front if already open.
 *
 * \param w The window to be opened.
 */
/*---------------------------------------------------------------------------*/
void
ctk_window_open(CC_REGISTER_ARG struct ctk_window *w)
{
#if CTK_CONF_WINDOWS
  struct ctk_window *w2;
  
  /* Check if already open. */
  for(w2 = windows; w2 != w && w2 != NULL; w2 = w2->next);
  if(w2 == NULL) {
   /* Not open, so we add it at the head of the list of open
       windows. */
    w->next = windows;
    if(windows != NULL) {
      windows->prev = w;
    }
    windows = w;
    w->prev = NULL;
  } else {
    /* Window already open, so we move it to the front of the windows
       list. */
    if(w != windows) {
      if(w->next != NULL) {
	w->next->prev = w->prev;
      }
      if(w->prev != NULL) {
	w->prev->next = w->next;
      }
      w->next = windows;
      windows->prev = w;
      windows = w;
      w->prev = NULL;
    }
  }
#else /* CTK_CONF_WINDOWS */
  window = w;
#endif /* CTK_CONF_WINDOWS */

#if CTK_CONF_MENUS
  /* Recreate the Desktop menu's window entries.*/
  make_desktopmenu();
#endif /* CTK_CONF_MENUS */

  redraw |= REDRAW_ALL;
}
/*---------------------------------------------------------------------------*/
/**
 * Close a window if it is open.
 *
 * If the window is not open, this function does nothing.
 *
 * \param w The window to be closed.
 */
/*---------------------------------------------------------------------------*/
void
ctk_window_close(struct ctk_window *w)
{
#if CTK_CONF_WINDOWCLOSE
  static struct ctk_window *w2;

  if(w == NULL) {
    return;
  }
  
  /* Check if the window to be closed is the first window on the list. */
  if(w == windows) {
    windows = w->next;
    if(windows != NULL) {
      windows->prev = NULL;
    }
    w->next = w->prev = NULL;
  } else {
    /* Otherwise we step through the list until we find the window
       before the one to be closed. We then redirect its ->next
       pointer and its ->next->prev. */
    for(w2 = windows; w2 != NULL && w2->next != w; w2 = w2->next);

    if(w2 == NULL) {
      /* The window wasn't open, so there is nothing more for us to do. */
      return;
    }

    if(w->next != NULL) {
      w->next->prev = w->prev;
    }
    w2->next = w->next;
    
    w->next = w->prev = NULL;
  }
  
#if CTK_CONF_MENUS
  /* Recreate the Desktop menu's window entries.*/
  make_desktopmenu();
#endif /* CTK_CONF_MENUS */
  redraw |= REDRAW_ALL;
#endif /* CTK_CONF_WINDOWCLOSE */
}
#if CTK_CONF_WINDOWS
/*---------------------------------------------------------------------------*/
/**
 * \internal Create the move and close buttons on the window titlebar.
 */
/*---------------------------------------------------------------------------*/
static void
make_windowbuttons(CC_REGISTER_ARG struct ctk_window *window)
{
  unsigned char placement;

  if(ctk_draw_windowtitle_height >= 2) {
    placement = -1 - ctk_draw_windowtitle_height/2;
  } else {
    placement = -1;
  }
#if CTK_CONF_WINDOWMOVE
  CTK_BUTTON_NEW(&window->titlebutton, 0, placement,
		 window->titlelen, window->title);
#else
  CTK_LABEL_NEW(&window->titlebutton, 0, placement,
		window->titlelen, 1, window->title);
#endif /* CTK_CONF_WINDOWMOVE */
  CTK_WIDGET_ADD(window, &window->titlebutton);

#if CTK_CONF_WINDOWCLOSE
  CTK_BUTTON_NEW(&window->closebutton, window->w - 3, placement,
		 1, "x");
#else
  CTK_LABEL_NEW(&window->closebutton, window->w - 4, placement,
		3, 1, "   ");
#endif /* CTK_CONF_WINDOWCLOSE */
  CTK_WIDGET_ADD(window, &window->closebutton);
}
#endif /* CTK_CONF_WINDOWS */
/*---------------------------------------------------------------------------*/
/**
 * Remove all widgets from a window.
 *
 * \param w The window to be cleared.
 */
/*---------------------------------------------------------------------------*/
void
ctk_window_clear(struct ctk_window *w)
{
  w->active = w->inactive = w->focused = NULL;
  
#if CTK_CONF_WINDOWS
  make_windowbuttons(w);
#endif /* CTK_CONF_WINDOWS */
}
/*---------------------------------------------------------------------------*/
/**
 * Add a menu to the menu bar.
 *
 * \param menu The menu to be added.
 *
 * \note Do not call this function multiple times for the same menu,
 * as no check is made to see if the menu already is in the menu bar.
 */
/*---------------------------------------------------------------------------*/
void
ctk_menu_add(struct ctk_menu *menu)
{
#if CTK_CONF_MENUS
  struct ctk_menu *m;

  if(lastmenu == NULL) {
    lastmenu = menu;
  }
    
  for(m = menus.menus; m->next != NULL; m = m->next) {
    if(m == menu) {
      return;
    }
  }
  m->next = menu;
  menu->next = NULL;

  redraw |= REDRAW_MENUPART;
#endif /* CTK_CONF_MENUS */
}
/*---------------------------------------------------------------------------*/
/**
 * Remove a menu from the menu bar.
 *
 * \param menu The menu to be removed.
 */
/*---------------------------------------------------------------------------*/
void
ctk_menu_remove(struct ctk_menu *menu)
{
#if CTK_CONF_MENUS
  struct ctk_menu *m;

  for(m = menus.menus; m->next != NULL; m = m->next) {
    if(m->next == menu) {
      m->next = menu->next;
      if(menu == lastmenu) {
	lastmenu = NULL;
      }
      redraw |= REDRAW_MENUPART;
      return;
    }
  }
#endif /* CTK_CONF_MENUS */
}
/*---------------------------------------------------------------------------*/
/**
 * \internal Redraws everything on the screen within the clip
 * interval.
 *
 * \param clipy1 The upper bound of the clip interval
 * \param clipy2 The lower bound of the clip interval
 */
/*---------------------------------------------------------------------------*/
static void CC_FASTCALL
do_redraw_all(unsigned char clipy1, unsigned char clipy2)
{
#if CTK_CONF_WINDOWS
  static struct ctk_widget *widget;
  struct ctk_window *w;
  unsigned char focus;
#endif /* CTK_CONF_WINDOWS */

  if(mode != CTK_MODE_NORMAL && mode != CTK_MODE_WINDOWMOVE) {
    return;
  }
  
  ctk_draw_clear(clipy1, clipy2);

#if CTK_CONF_WINDOWS  
  /* Draw widgets in root window */
  for(widget = desktop_window.active;
      widget != NULL; widget = widget->next) {
    ctk_draw_widget(widget, windows != NULL? 0: CTK_FOCUS_WINDOW, clipy1, clipy2);
  }

  /* Draw windows */
  if(windows != NULL) {
    /* Find the last window.*/
    for(w = windows; w->next != NULL; w = w->next);

    /* Draw the windows from back to front. */
    for(; w != windows; w = w->prev) {
      ctk_draw_clear_window(w, 0, clipy1, clipy2);
      ctk_draw_window(w, 0, clipy1, clipy2, 1);
    }

    /* Draw focused window */
    focus = mode == CTK_MODE_WINDOWMOVE?
	    CTK_FOCUS_WIDGET|CTK_FOCUS_WINDOW:
	    CTK_FOCUS_WINDOW;
    ctk_draw_clear_window(windows, focus, clipy1, clipy2);
    ctk_draw_window(windows, focus, clipy1, clipy2, 1);
  }

  /* Draw dialog (if any) */
  if(dialog != NULL) {
    ctk_draw_dialog(dialog);
  }
#else /* CTK_CONF_WINDOWS */
  if(window != NULL) {
    ctk_draw_clear_window(window, CTK_FOCUS_WINDOW, clipy1, clipy2);
    ctk_draw_window(window, CTK_FOCUS_WINDOW, clipy1, clipy2, 0);
  }
#endif /* CTK_CONF_WINDOWS */

#if CTK_CONF_MENUS
  ctk_draw_menus(&menus);
#endif /* CTK_CONF_MENUS */
}
#if CTK_CONF_WINDOWS
/*---------------------------------------------------------------------------*/
/**
 * Redraw the entire desktop.
 *
 * \param d The desktop to be redrawn.
 *
 * \note Currently the parameter d is not used, but must be set to
 * NULL.
 *
 */
/*---------------------------------------------------------------------------*/
void
ctk_desktop_redraw(struct ctk_desktop *d)
{
  if(PROCESS_CURRENT() == &ctk_process) {
    if(mode == CTK_MODE_NORMAL || mode == CTK_MODE_WINDOWMOVE) {
      do_redraw_all(CTK_CONF_MENUS, height);
    }
  } else {
    height = ctk_draw_height();
    width = ctk_draw_width();
    
    redraw |= REDRAW_ALL;
  }
}
#endif /* CTK_CONF_WINDOWS */
/*---------------------------------------------------------------------------*/
/**
 * Redraw a window.
 *
 * This function redraws the window, but only if it is the foremost
 * one on the desktop.
 *
 * \param w The window to be redrawn.
 */
/*---------------------------------------------------------------------------*/
void
ctk_window_redraw(struct ctk_window *w)
{
  /* Only redraw the window if it is a dialog or if it is the foremost
     window. */
  if(mode != CTK_MODE_NORMAL) {
    return;
  }
  
#if CTK_CONF_WINDOWS
  if(w == dialog) {
    ctk_draw_dialog(w);
  } else if(dialog == NULL &&
#if CTK_CONF_MENUS
	    menus.open == NULL &&
#endif /* CTK_CONF_MENUS */
	    windows == w)
#endif /* CTK_CONF_WINDOWS */
  {
    ctk_draw_window(w, CTK_FOCUS_WINDOW, 0, height, 0);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \internal Creates a new window.
 *
 * \param window The window to be created.
 * \param w The width of the window.
 * \param h The height of the window.
 * \param title The title of the window.
 */
/*---------------------------------------------------------------------------*/
static void
window_new(CC_REGISTER_ARG struct ctk_window *window,
	   unsigned char w, unsigned char h, char *title)
{
#if CTK_CONF_WINDOWS
  if(w >= width - 2) {
    window->x = 0;
  } else {
    window->x = (width - w - 2) / 2;
  }
  if(h >= height - 2 - ctk_draw_windowtitle_height) {
    window->y = 0;
  } else {
    window->y = (height - h - 2 - ctk_draw_windowtitle_height) / 2;
  }
#endif /* CTK_CONF_WINDOWS */

  window->w = w;
  window->h = h;
  window->title = title;
  if(title != NULL) {
    window->titlelen = (unsigned char)strlen(title);
  } else {
    window->titlelen = 0;
  }
  window->next = window->prev = NULL;
  window->owner = PROCESS_CURRENT();
  window->active = window->inactive = window->focused = NULL;
}
/*---------------------------------------------------------------------------*/
/**
 * Create a new window.
 *
 * Creates a new window. The memory for the window structure must
 * already be allocated by the caller, and is usually done with a
 * static declaration.
 *
 * This function sets up the internal structure of the ctk_window
 * struct and creates the move and close buttons, but it does not open
 * the window. The window must be explicitly opened by calling the
 * ctk_window_open() function.
 *
 * \param window The window to be created.
 * \param w The width of the new window.
 * \param h The height of the new window.
 * \param title The title of the new window.
 */
/*---------------------------------------------------------------------------*/
void
ctk_window_new(struct ctk_window *window,
	       unsigned char w, unsigned char h, char *title)
{
  window_new(window, w, h, title);

#if CTK_CONF_WINDOWS
  make_windowbuttons(window);
#endif /* CTK_CONF_WINDOWS */
}
#if CTK_CONF_WINDOWS
/*---------------------------------------------------------------------------*/
/**
 * Creates a new dialog.
 *
 * This function only sets up the internal structure of the ctk_window
 * struct but does not open the dialog. The dialog must be explicitly
 * opened by calling the ctk_dialog_open() function.
 *
 * \param dialog The dialog to be created.
 * \param w The width of the dialog.
 * \param h The height of the dialog.
 */
/*---------------------------------------------------------------------------*/
void
ctk_dialog_new(CC_REGISTER_ARG struct ctk_window *dialog,
	       unsigned char w, unsigned char h)
{
  window_new(dialog, w, h, NULL);
}
#endif /* CTK_CONF_WINDOWS */
/*---------------------------------------------------------------------------*/
/**
 * Creates a new menu.
 *
 * This function sets up the internal structure of the menu, but does
 * not add it to the menubar. Use the function ctk_menu_add() for that
 * purpose.
 *
 * \param menu The menu to be created.
 * \param title The title of the menu.
 */
/*---------------------------------------------------------------------------*/
void
ctk_menu_new(CC_REGISTER_ARG struct ctk_menu *menu, char *title)
{
#if CTK_CONF_MENUS
  menu->next = NULL;
  menu->title = title;
  menu->titlelen = (unsigned char)strlen(title);
  menu->active = 0;
  menu->nitems = 0;
#endif /* CTK_CONF_MENUS */
}
/*---------------------------------------------------------------------------*/
/**
 * Adds a menu item to a menu.
 *
 * In CTK, each menu item is identified by a number which is unique
 * within each menu. When a menu item is selected, a
 * ctk_menuitem_activated signal is emitted and the menu item number
 * is passed as signal data with the signal.
 *
 * \param menu The menu to which the menu item should be added.
 * \param name The name of the menu item.
 * \return The number of the menu item.
 */
/*---------------------------------------------------------------------------*/
unsigned char
ctk_menuitem_add(CC_REGISTER_ARG struct ctk_menu *menu, char *name)
{
#if CTK_CONF_MENUS
  if(menu->nitems == CTK_MAXMENUITEMS) {
    return 0;
  }
  menu->items[menu->nitems].title = name;
  menu->items[menu->nitems].titlelen = (unsigned char)strlen(name);
  return menu->nitems++;
#else
  return 0;
#endif /* CTK_CONF_MENUS */
}
/*---------------------------------------------------------------------------*/
/**
 * \internal Adds a widget to the list of widgets that should be
 * redrawn.
 *
 * \param w The widget that should be redrawn.
 */
/*---------------------------------------------------------------------------*/
static void CC_FASTCALL
add_redrawwidget(struct ctk_widget *w)
{
  static unsigned char i;
  
  if(redraw_widgetptr == MAX_REDRAWWIDGETS) {
    redraw |= REDRAW_FOCUS;
  } else {
    redraw |= REDRAW_WIDGETS;
    /* Check if it is in the queue already. If so, we don't add it
       again. */
    for(i = 0; i < redraw_widgetptr; ++i) {
      if(redraw_widgets[i] == w) {
	return;
      }
    }
    redraw_widgets[redraw_widgetptr++] = w;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \internal Checks if a widget redrawn and adds it to the list of
 * widgets to be redrawn.
 *
 * A widget can be redrawn only if the current CTK mode is
 * CTK_MODE_NORMAL, if no menu is open, and the widget is in the
 * foremost window.
 *
 * \param widget The widget that should be redrawn.
 */
/*---------------------------------------------------------------------------*/
static void
widget_redraw(struct ctk_widget *widget)
{
  struct ctk_window *window;

  if(mode != CTK_MODE_NORMAL || widget == NULL) {
    return;
  }

  /* Only redraw widgets that are in the foremost window. If we would
     allow redrawing widgets in non-focused windows, we would have to
     redraw all the windows that cover the non-focused window as well,
     which would lead to flickering.

     Also, we avoid drawing any widgets when the menus are active.
    */
    
#if CTK_CONF_MENUS
  if(menus.open == NULL)
#endif /* CTK_CONF_MENUS */
  {
    window = widget->window;
#if CTK_CONF_WINDOWS
    if(window == dialog) {
      ctk_draw_widget(widget, CTK_FOCUS_DIALOG, 0, height);
    } else if(dialog == NULL &&
	      (window == windows ||
	       window == &desktop_window))
#endif /* CTK_CONF_WINDOWS */
    {
      ctk_draw_widget(widget, CTK_FOCUS_WINDOW, 0, height);
    }
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Redraws a widget.
 *
 * This function will set a flag which causes the widget to be redrawn
 * next time the CTK process is scheduled.
 *
 * \param widget The widget that is to be redrawn.
 *
 * \note This function should usually not be called directly since it
 * requires typecasting of the widget parameter. The wrapper macro
 * CTK_WIDGET_REDRAW() does the required typecast and should be used
 * instead.
 */
/*---------------------------------------------------------------------------*/
void
ctk_widget_redraw(struct ctk_widget *widget)
{
  if(mode != CTK_MODE_NORMAL || widget == NULL) {
    return;
  }

  /* Since this function isn't called by CTK itself, we only queue the
     redraw request. */
  add_redrawwidget(widget);
}
/*---------------------------------------------------------------------------*/
/**
 * Adds a widget to a window.
 *
 * This function adds a widget to a window. The order of which the
 * widgets are added is important, as it sets the order to which
 * widgets are cycled with the widget selection keys.
 *
 * \param window The window to which the widhet should be added.
 * \param widget The widget to be added.
 */
/*---------------------------------------------------------------------------*/
void CC_FASTCALL
ctk_widget_add(CC_REGISTER_ARG struct ctk_window *window,
	       CC_REGISTER_ARG struct ctk_widget *widget)
{
  if(widget->type == CTK_WIDGET_LABEL ||
     widget->type == CTK_WIDGET_SEPARATOR) {
    widget->next = window->inactive;
    window->inactive = widget;
    widget->window = window;
  } else {
    widget->next = window->active;
    window->active = widget;
    widget->window = window;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Gets the width of the desktop.
 *
 * \param d The desktop.
 * \return The width of the desktop, in characters.
 *
 * \note The d parameter is currently unused and must be set to NULL.
 */
/*---------------------------------------------------------------------------*/
unsigned char
ctk_desktop_width(struct ctk_desktop *d)
{
  return ctk_draw_width();
}
/*---------------------------------------------------------------------------*/
/**
 * Gets the height of the desktop.
 *
 * \param d The desktop.
 * \return The height of the desktop, in characters.
 *
 * \note The d parameter is currently unused and must be set to NULL.
 */
/*---------------------------------------------------------------------------*/
unsigned char
ctk_desktop_height(struct ctk_desktop *d)
{
  return ctk_draw_height();
}
/*---------------------------------------------------------------------------*/
/**
 * \internal Selects a widget in the window of the widget.
 *
 * \param focus The widget to be focused.
 */
/*---------------------------------------------------------------------------*/
static void CC_FASTCALL
select_widget(struct ctk_widget *focus)
{
  struct ctk_window *window;

  window = focus->window;
  
  if(focus != window->focused) {
    window->focused = focus;
    /* The operation changed the focus, so we emit a "hover" signal
       for those widgets that support it. */
    
    if(window->focused->type == CTK_WIDGET_HYPERLINK) {
      process_post(window->owner, ctk_signal_hyperlink_hover, window->focused);
    } else if(window->focused->type == CTK_WIDGET_BUTTON) {
      process_post(window->owner, ctk_signal_button_hover, window->focused);
    }
    
    add_redrawwidget(window->focused);

    process_post(focus->window->owner, ctk_signal_widget_select, focus);
  }
}
/*---------------------------------------------------------------------------*/
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
static void CC_FASTCALL
switch_focus_widget(unsigned char direction)
{
#if CTK_CONF_WINDOWS
  register struct ctk_window *window;
#endif /* CTK_CONF_WINDOWS */
  register struct ctk_widget *focus;
  struct ctk_widget *widget;
  
#if CTK_CONF_WINDOWS
  if(dialog != NULL) {
    window = dialog;
  } else {
    window = windows;
  }

  /* If there are no windows open, we move focus around between the
     icons on the root window instead. */
  if(window == NULL) {
    window = &desktop_window;
  }
#else /* CTK_CONF_WINDOWS */
  if(window == NULL) {
    return;
  }
#endif /* CTK_CONF_WINDOWS */
 
  focus = window->focused;
  if(focus == NULL) {
    focus = window->active;
    if(focus == NULL) {
      return;
    }
  }
  add_redrawwidget(focus);
  
  if((direction & 1) == 0) {
    /* Move focus "up" */
    focus = focus->next;
  } else {
    /* Move focus "down" */
    for(widget = window->active;
	widget != NULL; widget = widget->next) {
	if(widget->next == focus) {
	  break;
	}
    }
    focus = widget;
    if(focus == NULL) {
      if(window->active != NULL) {
	for(focus = window->active;
	    focus->next != NULL; focus = focus->next);
      }
    }
  }
  if(focus == NULL) {
    focus = window->active;
  }

  select_widget(focus);
}
/*---------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
static void
switch_open_menu(unsigned char rightleft)
{
  struct ctk_menu *menu;
  
  if(rightleft == 0) {
    /* Move right */
    for(menu = menus.menus; menu != NULL; menu = menu->next) {
      if(menu->next == menus.open) {
	break;
      }
    }
    lastmenu = menus.open;
    menus.open = menu;
    if(menus.open == NULL) {
      for(menu = menus.menus;
	  menu->next != NULL; menu = menu->next);
      menus.open = menu;
    }
  } else {
    /* Move to left */
    lastmenu = menus.open;
    menus.open = menus.open->next;
    if(menus.open == NULL) {
      menus.open = menus.menus;
    }
  }

  menus.open->active = 0;
}
/*---------------------------------------------------------------------------*/
static void
switch_menu_item(unsigned char updown)
{
  register struct ctk_menu *m;

  m = menus.open;
  
  if(updown == 0) {
    /* Move up */
    if(m->active == 0) {
      m->active = m->nitems - 1;
    } else {
      --m->active;
      if(m->items[m->active].title[0] == '-') {
	--m->active;
      }
    }
  } else {
    /* Move down */
    if(m->active >= m->nitems - 1) {
      m->active = 0;
    } else {
      ++m->active;
      if(m->items[m->active].title[0] == '-') {
	++m->active;
      }
    }
  }
}
#endif /* CTK_CONF_MENUS */
/*---------------------------------------------------------------------------*/
static unsigned char CC_FASTCALL
activate(CC_REGISTER_ARG struct ctk_widget *w)
{
  if(w->type == CTK_WIDGET_BUTTON) {
#if CTK_CONF_WINDOWCLOSE
    if(w == (struct ctk_widget *)&windows->closebutton) {
      process_post(w->window->owner, ctk_signal_window_close, windows);
      ctk_window_close(windows);
      return REDRAW_ALL;
    } else
#endif /* CTK_CONF_WINDOWCLOSE */
#if CTK_CONF_WINDOWMOVE
    if(w == (struct ctk_widget *)&windows->titlebutton) {
      mode = CTK_MODE_WINDOWMOVE;
      return REDRAW_ALL;
    } else
#endif /* CTK_CONF_WINDOWMOVE */
    {
      process_post(w->window->owner, ctk_signal_widget_activate, w);
    }
#if CTK_CONF_ICONS
  } else if(w->type == CTK_WIDGET_ICON) {
    if(w->widget.icon.owner != PROCESS_NONE) {
      process_post(w->widget.icon.owner, ctk_signal_widget_activate, w);
    } else {
      process_post(w->window->owner, ctk_signal_widget_activate, w);
    }
#endif /* CTK_CONF_ICONS */
  } else if(w->type == CTK_WIDGET_HYPERLINK) {
    process_post(PROCESS_BROADCAST, ctk_signal_hyperlink_activate, w);
  } else if(w->type == CTK_WIDGET_TEXTENTRY) {
    if(w->widget.textentry.state == CTK_TEXTENTRY_NORMAL) {
      w->widget.textentry.state = CTK_TEXTENTRY_EDIT;
      textentry_input(0, (struct ctk_textentry *)w);
    } else {
      w->widget.textentry.state = CTK_TEXTENTRY_NORMAL;
      process_post(w->window->owner, ctk_signal_widget_activate, w);
    }
    add_redrawwidget(w);
    return REDRAW_WIDGETS;
  } else {
    process_post(w->window->owner, ctk_signal_widget_activate, w);
  }
  return REDRAW_NONE;
}
/*---------------------------------------------------------------------------*/
#ifdef SDCC
/* Dummy function that we define to keep sdcc happy - with sdcc,
   function pointers cannot be NULL. ctk_textentry_input is typedef'd
   in ctk/ctk.h, hence the strange-looking function signature. */
unsigned char
ctk_textentry_input_null(ctk_arch_key_t c, struct ctk_textentry *t)
{
  return 0;
}
#endif /* SDCC */
/*---------------------------------------------------------------------------*/
static void CC_FASTCALL
textentry_input(ctk_arch_key_t c, CC_REGISTER_ARG struct ctk_textentry *t)
{
  register char *cptr, *cptr2;
  static unsigned char len, txpos, typos, tlen;

  if(t->input != NULL && t->input(c, t)) {
    return;
  }

  txpos = t->xpos;
  typos = t->ypos;
  tlen = t->len;

  cptr = &t->text[txpos + typos * (tlen + 1)];
      
  switch(c) {
  case CH_CURS_LEFT:
    if(txpos > 0) {
      --txpos;
    }
    break;
    
  case CH_CURS_RIGHT:
    if(txpos < tlen - 1 && *cptr != 0) {
      ++txpos;
    }
    break;

  case CH_CURS_UP:
    txpos = 0;
    break;
    
  case 0:
  case CH_CURS_DOWN:
    txpos = (unsigned char)strlen(t->text);
    if(txpos == tlen) {
      --txpos;
    }
    break;
    
  case CH_ENTER:
    activate((struct ctk_widget *)t);
    switch_focus_widget(DOWN);
    break;
    
  case CTK_CONF_WIDGETDOWN_KEY:
    t->state = CTK_TEXTENTRY_NORMAL;
    switch_focus_widget(DOWN);
    break;
  case CTK_CONF_WIDGETUP_KEY:
    t->state = CTK_TEXTENTRY_NORMAL;
    switch_focus_widget(UP);
    break;
    
  default:
    len = tlen - txpos;
    if(c == CH_DEL) {
      if(len == 1 && *cptr != 0) {
	*cptr = 0;
      } else {
        if(txpos > 0) {
	  --txpos;
	  strcpy(cptr - 1, cptr);
	}
      }
    } else {
      if(ctk_arch_isprint(c)) {
	if(len > 1) {
	  cptr2 = cptr + len - 1;
	  while(cptr2 > cptr) {
	    *cptr2 = *(cptr2 - 1);
	    --cptr2;
	  }
	  ++txpos;
	}
	*cptr = c;
      }
    }
    break;
  }

  t->xpos = txpos;
  t->ypos = typos;
}
/*---------------------------------------------------------------------------*/
#if CTK_CONF_MENUS
static unsigned char
activate_menu(void)
{
  struct ctk_window *w;
  
  lastmenu = menus.open;
  if(menus.open == &desktopmenu) {
    for(w = windows; w != NULL; w = w->next) {
      if(w->title == desktopmenu.items[desktopmenu.active].title) {
	ctk_window_open(w);
	menus.open = NULL;
	return REDRAW_ALL;
      }
    }
  } else {
    process_post(PROCESS_BROADCAST, ctk_signal_menu_activate, menus.open);
  }
  menus.open = NULL;
  return REDRAW_MENUPART;
}
/*---------------------------------------------------------------------------*/
static unsigned char
menus_input(ctk_arch_key_t c)
{
  if(menus.open->nitems > maxnitems) {
    maxnitems = menus.open->nitems;
  }
  
  switch(c) {
  case CH_CURS_RIGHT:
    switch_open_menu(1);
    return REDRAW_MENUPART;

  case CH_CURS_DOWN:
    switch_menu_item(1);
    return REDRAW_MENUS;

  case CH_CURS_LEFT:
    switch_open_menu(0);
    return REDRAW_MENUPART;

  case CH_CURS_UP:
    switch_menu_item(0);
    return REDRAW_MENUS;
    
  case CH_ENTER:
    return activate_menu();

  case CTK_CONF_MENU_KEY:
    lastmenu = menus.open;
    menus.open = NULL;
    return REDRAW_MENUPART;
  }

  return REDRAW_NONE;
}
#endif /* CTK_CONF_MENUS */
/*---------------------------------------------------------------------------*/
#if CTK_CONF_SCREENSAVER
static void
handle_timer(void)
{
  if(mode == CTK_MODE_NORMAL) {
    ++screensaver_timer;
    if(screensaver_timer >= ctk_screensaver_timeout) {
      process_post(PROCESS_BROADCAST, ctk_signal_screensaver_start, NULL);
#ifdef CTK_SCREENSAVER_INIT
      CTK_SCREENSAVER_INIT();
#endif /* CTK_SCREENSAVER_INIT */

      screensaver_timer = 0;
    }
  }
}
#endif /* CTK_CONF_SCREENSAVER */
/*---------------------------------------------------------------------------*/
static void
unfocus_widget(CC_REGISTER_ARG struct ctk_widget *w)
{
  if(w != NULL) {
    redraw |= REDRAW_WIDGETS;
    add_redrawwidget(w);
    if(CTK_WIDGET_TYPE(w) == CTK_WIDGET_TEXTENTRY) {
      ((struct ctk_textentry *)w)->state =
	CTK_TEXTENTRY_NORMAL;
    }
    w->window->focused = NULL;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ctk_process, ev, data)
{
  static ctk_arch_key_t c;
  static unsigned char i;
#if CTK_CONF_WINDOWS
  register struct ctk_window *window;
#endif /* CTK_CONF_WINDOWS */
  register struct ctk_widget *widget;
  register struct ctk_widget **widgetptr;
#if CTK_CONF_MOUSE_SUPPORT
  static unsigned char mxc, myc, mouse_button_changed, mouse_moved,
    mouse_clicked;
#if CTK_CONF_MENUS
  static unsigned char menux;
  register struct ctk_menu *menu;
#endif /* CTK_CONF_MENUS */
#endif /* CTK_CONF_MOUSE_SUPPORT */
  
  PROCESS_BEGIN();
  
#if CTK_CONF_MENUS
  ctk_menu_new(&desktopmenu, "Desktop");
  make_desktopmenu();
  menus.menus = menus.desktopmenu = &desktopmenu;
#endif /* CTK_CONF_MENUS */

#if CTK_CONF_MOUSE_SUPPORT
  ctk_mouse_init();
  ctk_mouse_show();
#endif /* CTK_CONF_MOUSE_SUPPORT */
  
  ctk_restore();

#if CTK_CONF_WINDOWS
  desktop_window.owner = &ctk_process;
#endif /* CTK_CONF_WINDOWS */

  ctk_signal_keypress = process_alloc_event();
  
  ctk_signal_button_activate =
    ctk_signal_widget_activate = process_alloc_event();
  
  ctk_signal_button_hover =
    ctk_signal_hyperlink_hover =
    ctk_signal_widget_select = process_alloc_event();
  
  ctk_signal_hyperlink_activate = process_alloc_event();

  ctk_signal_menu_activate = process_alloc_event();

  ctk_signal_window_close = process_alloc_event();

#if CTK_CONF_MOUSE_SUPPORT
  ctk_signal_pointer_move = process_alloc_event();
  ctk_signal_pointer_button = process_alloc_event();
#endif /* CTK_CONF_MOUSE_SUPPORT */

#if CTK_CONF_SCREENSAVER
  ctk_signal_screensaver_start = process_alloc_event();
  ctk_signal_screensaver_stop = process_alloc_event();
#endif /* CTK_CONF_SCREENSAVER */

  mode = CTK_MODE_NORMAL;

#if CTK_CONF_ICONS
  iconx = ICONX_START;
  icony = ICONY_START;
#endif /* CTK_CONF_ICONS */

#if CTK_CONF_SCREENSAVER
  timer_set(&timer, CLOCK_SECOND);
#endif /* CTK_CONF_SCREENSAVER */
  
  while(1) {
    process_poll(&ctk_process);
    PROCESS_WAIT_EVENT();
    
#if CTK_CONF_SCREENSAVER
    if(timer_expired(&timer)) {
      timer_reset(&timer);
      handle_timer();
    }
#endif /* CTK_CONF_SCREENSAVER */

#if CTK_CONF_MENUS
    if(menus.open != NULL) {
      maxnitems = menus.open->nitems;
    } else {
      maxnitems = 0;
    }
#endif /* CTK_CONF_MENUS */

#if CTK_CONF_MOUSE_SUPPORT
    mouse_button_changed = mouse_moved = mouse_clicked = 0;

    /* See if there is any change in the buttons. */
    if(ctk_mouse_button() != mouse_button) {
      mouse_button = ctk_mouse_button();
      mouse_button_changed = 1;
      if(mouse_button == 0) {
	mouse_clicked = 1;
      }
    }
  
    /* Check if the mouse pointer has moved. */
    if(ctk_mouse_x() != mouse_x ||
       ctk_mouse_y() != mouse_y) {
      mouse_x = ctk_mouse_x();
      mouse_y = ctk_mouse_y();
      mouse_moved = 1;
    }

    mxc = ctk_mouse_xtoc(mouse_x);
    myc = ctk_mouse_ytoc(mouse_y);
#endif /* CTK_CONF_MOUSE_SUPPORT */

#if CTK_CONF_SCREENSAVER
    if(mode == CTK_MODE_SCREENSAVER) {
      if(ctk_arch_keyavail()
#if CTK_CONF_MOUSE_SUPPORT
	 || mouse_moved || mouse_button_changed
#endif /* CTK_CONF_MOUSE_SUPPORT */
	 ) {
	process_post(PROCESS_BROADCAST, ctk_signal_screensaver_stop, NULL);
	mode = CTK_MODE_NORMAL;
      }
    } else
#endif /* CTK_CONF_SCREENSAVER */
      if(mode == CTK_MODE_NORMAL) {
#if CTK_CONF_MOUSE_SUPPORT
	/* If there is any change in the mouse conditions, find out in
	   which window the mouse pointer currently is in order to send
	   the correct signals, or bring a window to focus. */
	if(mouse_moved || mouse_button_changed) {
	  ctk_mouse_show();
#if CTK_CONF_SCREENSAVER
	  screensaver_timer = 0;
#endif /* CTK_CONF_SCREENSAVER */
      
#if CTK_CONF_MENUS
	  if(myc == 0) {
	    /* Here we should do whatever needs to be done when the mouse
	       moves around and clicks in the menubar. */
	    if(mouse_clicked) {
	      static unsigned char titlelen;
	  
	      /* Find out which menu that the mouse pointer is in. Start
		 with the ->next menu after the desktop menu. We assume
		 that the menus start one character from the left screen
		 side and that the desktop menu is farthest to the
		 right. */
	      menux = 1;
	      for(menu = menus.menus->next;
		  menu != NULL; menu = menu->next) {
		titlelen = menu->titlelen;
		if(mxc >= menux && mxc <= menux + titlelen) {
		  break;
		}
		menux += titlelen;
	      }
	  
	      /* Also check desktop menu. */
	      if(mxc >= width - 7 &&
		 mxc <= width - 1) {
		menu = &desktopmenu;
	      }
	  
	      menus.open = menu;
	      redraw |= REDRAW_MENUPART;
	    }
	  } else {
	    --myc;

	    if(menus.open != NULL) {
	      static unsigned char nitems;
	  
	      /* Do whatever needs to be done when a menu is open. */

	      /* First check if the mouse pointer is in the currently open
		 menu. */
	      if(menus.open == &desktopmenu) {
		menux = width - CTK_CONF_MENUWIDTH;
	      } else {
		menux = 1;
		for(menu = menus.menus->next; menu != menus.open;
		    menu = menu->next) {
		  menux += menu->titlelen;
		}
	      }

	      nitems = menus.open->nitems;
	      /* Find out which of the menu items the mouse is pointing
		 to. */
	      if(mxc >= menux && mxc <= menux + CTK_CONF_MENUWIDTH) {
		if(myc <= nitems) {
		  menus.open->active = myc;
		} else {
		  menus.open->active = nitems - 1;
		}
	      }
	  
	      if(mouse_clicked) {
		if(mxc >= menux && mxc <= menux + CTK_CONF_MENUWIDTH &&
		   myc <= nitems) {
		  redraw |= activate_menu();
		} else {
		  lastmenu = menus.open;
		  menus.open = NULL;
		  redraw |= REDRAW_MENUPART;
		}
	      } else {
		redraw |= REDRAW_MENUS;
	      }
	    } else {
#endif /* CTK_CONF_MENUS */

#if CTK_CONF_WINDOWS
	      /* Walk through the windows from top to bottom to see in
		 which window the mouse pointer is. */
	      if(dialog != NULL) {
		window = dialog;
	      } else {
		for(window = windows; window != NULL;
		    window = window->next) {
	      
		  /* Check if the mouse is within the window. */
		  if(mxc >= window->x &&
		     mxc <= window->x + window->w +
		     2 * ctk_draw_windowborder_width &&
		     myc >= window->y &&
		     myc <= window->y + window->h +
		     ctk_draw_windowtitle_height +
		     ctk_draw_windowborder_height) {
		    break;
		  }
		}
	      }

	      /* If we didn't find any window, and there are no windows
		 open, the mouse pointer will definately be within the
		 background desktop window. */
	      if(window == NULL) {
		window = &desktop_window;
	      }

	      /* If the mouse pointer moves around outside of the
		 currently focused window (or dialog), we should not have
		 any focused widgets in the focused window so we make sure
		 that there are none. */
	      if(windows != NULL &&
		 window != windows &&
		 windows->focused != NULL){
		unfocus_widget(windows->focused);
	      }
#endif /* CTK_CONF_WINDOWS */

	      if(window != NULL) {
#if CTK_CONF_WINDOWS
		/* If the mouse was clicked outside of the current window,
		   we bring the clicked window to front. */
		if(dialog == NULL &&
		   window != &desktop_window &&
		   window != windows &&
		   mouse_clicked) {
		  /* Bring window to front. */
		  ctk_window_open(window);
		  redraw |= REDRAW_ALL;
		} else {

		  /* Find out which widget currently is under the mouse
		     pointer and give it focus, unless it already has
		     focus. */
		  mxc = mxc - window->x - ctk_draw_windowborder_width;
		  myc = myc - window->y - ctk_draw_windowtitle_height;
#endif /* CTK_CONF_WINDOWS */
	    
		  /* See if the mouse pointer is on a widget. If so, it
		     should be selected and, if the button is clicked,
		     activated. */
		  for(widget = window->active; widget != NULL;
		      widget = widget->next) {
		
		    if(mxc >= widget->x &&
		       mxc <= widget->x + widget->w + 1 &&
		       myc >= widget->y &&
		       myc <= widget->y + widget->h - 1) {
		      break;
		    }
		  }
	    
		  /* if the mouse is moved in the focused window, we emit
		     a ctk_signal_pointer_move signal to the owner of the
		     window. */
		  if(mouse_moved
#if CTK_CONF_WINDOWS
		     && (window != &desktop_window || windows == NULL)
#endif /* CTK_CONF_WINDOWS */
		      ) {

		    process_post(window->owner, ctk_signal_pointer_move, NULL);

		    /* If there was a focused widget that is not below the
		       mouse pointer, we remove focus from the widget and
		       redraw it. */
		    if(window->focused != NULL &&
		       widget != window->focused) {
		      unfocus_widget(window->focused);
		    }
		    redraw |= REDRAW_WIDGETS;
		    if(widget != NULL) {
		      select_widget(widget);
		    }
		  }
	    
		  if(mouse_button_changed) {
		    process_post(window->owner, ctk_signal_pointer_button,
				 (process_data_t)(size_t)mouse_button);
		    if(mouse_clicked && widget != NULL) {
		      select_widget(widget);
		      redraw |= activate(widget);
		    }
		  }
#if CTK_CONF_WINDOWS
		}
#endif /* CTK_CONF_WINDOWS */
	      }
#if CTK_CONF_MENUS
	    }
	  }
#endif /* CTK_CONF_MENUS */
	}
#endif /* CTK_CONF_MOUSE_SUPPORT */
    
	while(ctk_arch_keyavail()) {

	  ctk_mouse_hide();
      
#if CTK_CONF_SCREENSAVER
	  screensaver_timer = 0;
#endif /* CTK_CONF_SCREENSAVER */
      
	  c = ctk_arch_getkey();
      
#if CTK_CONF_WINDOWS
	  if(dialog != NULL) {
	    window = dialog;
	  } else if(windows != NULL) {
	    window = windows;
	  } else {
	    window = &desktop_window;
	  }
#else /* CTK_CONF_WINDOWS */
	  if(window == NULL) {
	    continue;
	  }
#endif /* CTK_CONF_WINDOWS */

          /* Allow to exit the process owning the foreground window by
             pressing ctrl-c. This is especially useful if there's no
             closebutton on the window frames (or no windows at all).
          */
          if(c == 3) {
            process_post(window->owner, PROCESS_EVENT_EXIT, NULL);
          }

	  widget = window->focused;
	  
	  if(widget != NULL &&
	     widget->type == CTK_WIDGET_TEXTENTRY &&
	     widget->widget.textentry.state == CTK_TEXTENTRY_EDIT) {
	    textentry_input(c, (struct ctk_textentry *)widget);
	    add_redrawwidget(widget);
#if CTK_CONF_MENUS
	  } else if(menus.open != NULL) {
	    redraw |= menus_input(c);
#endif /* CTK_CONF_MENUS */
	  } else {
	    switch(c) {
	    case CTK_CONF_WIDGETDOWN_KEY:
	      switch_focus_widget(DOWN);
	      break;
	    case CTK_CONF_WIDGETUP_KEY:
	      switch_focus_widget(UP);
	      break;
#if CTK_CONF_MENUS
	    case CTK_CONF_MENU_KEY:
	      if(dialog == NULL) {
		if(lastmenu == NULL) {
		  menus.open = menus.menus;
		} else {
		  menus.open = lastmenu;
		}
		menus.open->active = 0;
		redraw |= REDRAW_MENUS;
	      }
	      break;
#endif /* CTK_CONF_MENUS */
#if CTK_CONF_WINDOWS
	    case CTK_CONF_WINDOWSWITCH_KEY:
	      if(windows != NULL) {
		for(window = windows; window->next != NULL;
		    window = window->next);
		ctk_window_open(window);
	      }
	      break;
#endif /* CTK_CONF_WINDOWS */
	    default:

	      if(c == CH_ENTER &&
		 widget != NULL) {
		redraw |= activate(widget);
	      } else {
		if(widget != NULL &&
		   widget->type == CTK_WIDGET_TEXTENTRY) {
		  if(widget->widget.textentry.state == CTK_TEXTENTRY_NORMAL) {
		    widget->widget.textentry.state = CTK_TEXTENTRY_EDIT;
		    textentry_input(0, (struct ctk_textentry *)widget);
		  }
		  textentry_input(c, (struct ctk_textentry *)widget);
		  add_redrawwidget(widget);
		} else {
		  unfocus_widget(window->focused);
		  process_post_synch(window->owner, ctk_signal_keypress,
				     (process_data_t)(size_t)c);
		}
	      }
	      break;
	    }
	  }

#if 0
	  if(redraw & REDRAW_WIDGETS) {
	    widgetptr = redraw_widgets;
	    for(i = 0; i < MAX_REDRAWWIDGETS; ++i) {
	      widget_redraw(*widgetptr);
	      *widgetptr = NULL;
	      ++widgetptr;
	    }
	    redraw &= ~REDRAW_WIDGETS;
	    redraw_widgetptr = 0;
	  }
#endif /* 0 */
	}
#if CTK_CONF_WINDOWMOVE
      } else if(mode == CTK_MODE_WINDOWMOVE) {

	redraw = 0;

	window = windows;

#if CTK_CONF_MOUSE_SUPPORT

	/* If the mouse has moved, we move the window as well. */
	if(mouse_moved) {

	  if(window->w + mxc + 2 >= width) {
	    window->x = width - 2 - window->w;
	  } else {
	    window->x = mxc;
	  }

	  if(window->h + myc + ctk_draw_windowtitle_height +
	     ctk_draw_windowborder_height >= height) {
	    window->y = height - window->h -
	      ctk_draw_windowtitle_height - ctk_draw_windowborder_height;
	  } else {
	    window->y = myc;
	  }
#if CTK_CONF_MENUS
	  if(window->y > 0) {
	    --window->y;
	  }
#endif /* CTK_CONF_MENUS */

	  redraw = REDRAW_ALL;
	}
    
	/* Check if the mouse has been clicked, and stop moving the window
	   if so. */
	if(mouse_button_changed &&
	   mouse_button == 0) {
	  mode = CTK_MODE_NORMAL;
	  redraw = REDRAW_ALL;
	}
#endif /* CTK_CONF_MOUSE_SUPPORT */
    
	while(mode == CTK_MODE_WINDOWMOVE && ctk_arch_keyavail()) {
    
#if CTK_CONF_SCREENSAVER
	  screensaver_timer = 0;
#endif /* CTK_CONF_SCREENSAVER */
      
	  c = ctk_arch_getkey();
      
	  switch(c) {
	  case CH_CURS_RIGHT:
	    ++window->x;
	    if(window->x + window->w + 1 >= width) {
	      --window->x;
	    }
	    redraw = REDRAW_ALL;
	    break;
	  case CH_CURS_LEFT:
	    if(window->x > 0) {
	      --window->x;
	    }
	    redraw = REDRAW_ALL;
	    break;
	  case CH_CURS_DOWN:
	    ++window->y;
	    if(window->y + window->h + 1 + CTK_CONF_MENUS >= height) {
	      --window->y;
	    }
	    redraw = REDRAW_ALL;
	    break;
	  case CH_CURS_UP:
	    if(window->y > 0) {
	      --window->y;
	    }
	    redraw = REDRAW_ALL;
	    break;
	  default:
	    mode = CTK_MODE_NORMAL;
	    redraw = REDRAW_ALL;
	    break;
	  }
	}
#endif /* CTK_CONF_WINDOWMOVE */
      }

    if(redraw & REDRAW_ALL) {
      do_redraw_all(CTK_CONF_MENUS, height);
#if CTK_CONF_MENUS
    } else if(redraw & REDRAW_MENUPART) {
      do_redraw_all(CTK_CONF_MENUS, maxnitems + 1);
    } else if(redraw & REDRAW_MENUS) {
      ctk_draw_menus(&menus);
#endif /* CTK_CONF_MENUS */
    } else if(redraw & REDRAW_FOCUS) {
#if CTK_CONF_WINDOWS
      if(dialog != NULL) {
	ctk_window_redraw(dialog);
      } else if(windows != NULL) {
	ctk_window_redraw(windows);
      } else {
	ctk_window_redraw(&desktop_window);
      }
#else /* CTK_CONF_WINDOWS */
      if(window != NULL) {
	ctk_window_redraw(window);
      }
#endif /* CTK_CONF_WINDOWS */
    } else if(redraw & REDRAW_WIDGETS) {
      widgetptr = redraw_widgets;
      for(i = 0; i < MAX_REDRAWWIDGETS; ++i) {
	widget_redraw(*widgetptr);
	*widgetptr = NULL;
	++widgetptr;
      }
    }
    redraw = 0;
    redraw_widgetptr = 0;
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
