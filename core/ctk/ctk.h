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
 * This file is part of the Contiki desktop OS.
 *
 *
 */

/**
 * \file
 * CTK header file.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * The CTK header file contains functioin declarations and definitions
 * of CTK structures and macros.
 */

/**
 * \addtogroup ctk
 * @{
 */

#ifndef CTK_H_
#define CTK_H_


#include "contiki-conf.h"
#include "contiki.h"

/* Defintions for the CTK widget types. */

/**
 * \addtogroup ctkdraw
 * @{
 */

/** Widget number: The CTK separator widget. */
#define CTK_WIDGET_SEPARATOR 1
/** Widget number: The CTK label widget. */
#define CTK_WIDGET_LABEL     2
/** Widget number: The CTK button widget. */
#define CTK_WIDGET_BUTTON    3
/** Widget number: The CTK hyperlink widget. */
#define CTK_WIDGET_HYPERLINK 4
/** Widget number: The CTK textentry widget. */
#define CTK_WIDGET_TEXTENTRY 5
/** Widget number: The CTK bitmap widget. */
#define CTK_WIDGET_BITMAP    6
/** Widget number: The CTK icon widget. */
#define CTK_WIDGET_ICON      7

/** @} */

struct ctk_widget;

#if CTK_CONF_WIDGET_FLAGS
#define CTK_WIDGET_FLAG_INITIALIZER(x) x,
#else
#define CTK_WIDGET_FLAG_INITIALIZER(x)
#endif

/**
 * \defgroup ctkappfunc CTK application functions
 *
 * The CTK functions used by an application program.
 *
 * @{
 */

/**
 * Instantiating macro for the ctk_separator widget.
 *
 * This macro is used when instantiating a ctk_separator widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_separator sep =
         {CTK_SEPARATOR(0, 0, 23)};
 \endcode
 * \param x The x position of the widget, relative to the widget's
 * window.
 * \param y The y position of the widget, relative to the widget's
 * window.
 * \param w The widget's width.
 */
#define CTK_SEPARATOR(x, y, w) \
 NULL, NULL, x, y, CTK_WIDGET_SEPARATOR, w, 1, CTK_WIDGET_FLAG_INITIALIZER(0)
struct ctk_separator {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
};

/**
 * Instantiating macro for the ctk_button widget.
 *
 * This macro is used when instantiating a ctk_button widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_button but =
         {CTK_BUTTON(0, 0, 2, "Ok")};
 \endcode
 * \param x The x position of the widget, relative to the widget's
 * window.
 * \param y The y position of the widget, relative to the widget's
 * window.
 * \param w The widget's width.
 * \param text The button text.
 */
#define CTK_BUTTON(x, y, w, text) \
 NULL, NULL, x, y, CTK_WIDGET_BUTTON, w, 1, CTK_WIDGET_FLAG_INITIALIZER(0) text
struct ctk_button {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *text;
};

/**
 * Instantiating macro for the ctk_label widget.
 *
 * This macro is used when instantiating a ctk_label widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_label lab =
         {CTK_LABEL(0, 0, 5, 1, "Label")};
 \endcode
 * \param x The x position of the widget, relative to the widget's
 * window.
 * \param y The y position of the widget, relative to the widget's
 * window.
 * \param w The widget's width.
 * \param h The height of the label.
 * \param text The label text.
 */
#define CTK_LABEL(x, y, w, h, text) \
 NULL, NULL, x, y, CTK_WIDGET_LABEL, w, h, CTK_WIDGET_FLAG_INITIALIZER(0) text,
struct ctk_label {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *text;
};

/**
 * Instantiating macro for the ctk_hyperlink widget.
 *
 * This macro is used when instantiating a ctk_hyperlink widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_hyperlink hlink =
         {CTK_HYPERLINK(0, 0, 7, "Contiki", "http://dunkels.com/adam/contiki/")};
 \endcode
 * \param x The x position of the widget, relative to the widget's
 * window.
 * \param y The y position of the widget, relative to the widget's
 * window.
 * \param w The widget's width.
 * \param text The hyperlink text.
 * \param url The hyperlink URL.
 */
#define CTK_HYPERLINK(x, y, w, text, url) \
 NULL, NULL, x, y, CTK_WIDGET_HYPERLINK, w, 1, CTK_WIDGET_FLAG_INITIALIZER(0) text, url
struct ctk_hyperlink {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *text;
  char *url;
};

/* Editing modes of the CTK textentry widget. */
#define CTK_TEXTENTRY_NORMAL 0 /**< \internal Textentry state: not
				  edited. */
#define CTK_TEXTENTRY_EDIT   1 /**< \internal Textentry state:
				  currenly being edited. */

/**
 * Clears a text entry widget and sets the cursor to the start of the
 * text line.
 *
 * \param e The text entry widget to be cleared.
 */
#define CTK_TEXTENTRY_CLEAR(e) \
	do { memset((e)->text, 0, (e)->h * ((e)->len + 1)); \
	     (e)->xpos = 0; (e)->ypos = 0; } while(0)

#ifdef CTK_ARCH_KEY_T
typedef CTK_ARCH_KEY_T ctk_arch_key_t;
#else /* CTK_ARCH_KEY_T */
typedef char ctk_arch_key_t;
#endif /* CTK_ARCH_KEY_T */

#ifndef CH_ENTER
#define CH_ENTER '\n'
#endif /* CH_ENTER */

struct ctk_textentry;
typedef unsigned char (* ctk_textentry_input)(ctk_arch_key_t c,
					      struct ctk_textentry *t);

/**
 * Instantiating macro for the ctk_textentry widget.
 *
 * This macro is used when instantiating a ctk_textentry widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_textentry tentry =
         {CTK_TEXTENTRY(0, 0, 30, 1, textbuffer, 50)};
 \endcode
 * \note The height of the text entry widget is obsolete and not
 * intended to be used.
 *
 * \param x The x position of the widget, relative to the widget's
 * window.
 * \param y The y position of the widget, relative to the widget's
 * window.
 * \param w The widget's width.
 * \param h The text entry height (obsolete).
 * \param text A pointer to the buffer that should be edited.
 * \param len The length of the text buffer
 */
#ifdef SDCC
#define CTK_TEXTENTRY(x, y, w, h, text, len) \
  NULL, NULL, x, y, CTK_WIDGET_TEXTENTRY, w, 1, CTK_WIDGET_FLAG_INITIALIZER(0) text, len, \
  CTK_TEXTENTRY_NORMAL, 0, 0, ctk_textentry_input_null
#else /* SDCC */
#define CTK_TEXTENTRY(x, y, w, h, text, len) \
  NULL, NULL, x, y, CTK_WIDGET_TEXTENTRY, w, 1, CTK_WIDGET_FLAG_INITIALIZER(0) text, len, \
  CTK_TEXTENTRY_NORMAL, 0, 0, NULL
#endif /* SDCC */
#define CTK_TEXTENTRY_INPUT(x, y, w, h, text, len, input) \
  NULL, NULL, x, y, CTK_WIDGET_TEXTENTRY, w, h, CTK_WIDGET_FLAG_INITIALIZER(0) text, len, \
  CTK_TEXTENTRY_NORMAL, 0, 0, input
struct ctk_textentry {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *text;
  unsigned char len;
  unsigned char state;
  unsigned char xpos, ypos;
  ctk_textentry_input input;
};

#ifdef SDCC
/* Dummy function that we define to keep sdcc happy - with sdcc,
   function pointers cannot be NULL.*/
unsigned char ctk_textentry_input_null(ctk_arch_key_t c, struct ctk_textentry *t);
#endif /* SDCC */

#if CTK_CONF_ICON_BITMAPS
#define CTK_ICON_BITMAP(bitmap)	  bitmap
#else
#define CTK_ICON_BITMAP(bitmap)	  NULL
#endif

#if CTK_CONF_ICON_TEXTMAPS
#define CTK_ICON_TEXTMAP(textmap) textmap
#else
#define CTK_ICON_TEXTMAP(textmap) NULL
#endif

/**
 * Instantiating macro for the ctk_icon widget.
 *
 * This macro is used when instantiating a ctk_icon widget and is
 * intended to be used together with a struct assignment like this:
 \code
  struct ctk_icon icon =
         {CTK_ICON("An icon", bitmapptr, textmapptr)};
 \endcode
 * \param title The icon's text.
 * \param bitmap A pointer to the icon's bitmap image.
 * \param textmap A pointer to the icon's text version of the bitmap.
 */
#if CTK_CONF_ICONS
#define CTK_ICON(title, bitmap, textmap) \
 NULL, NULL, 0, 0, CTK_WIDGET_ICON, 2, 4, CTK_WIDGET_FLAG_INITIALIZER(0) \
 title, PROCESS_NONE, \
 CTK_ICON_BITMAP(bitmap), CTK_ICON_TEXTMAP(textmap)
struct ctk_icon {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *title;
  struct process *owner;
  unsigned char *bitmap;
  char *textmap;
};

#define CTK_BITMAP(x, y, w, h, bitmap, bitmap_width, bitmap_height) \
  NULL, NULL, x, y, CTK_WIDGET_BITMAP, w, h, \
  CTK_WIDGET_FLAG_INITIALIZER(0) bitmap, bitmap_width, bitmap_height
struct ctk_bitmap {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  unsigned char *bitmap;
  unsigned short bw, bh;
};

#define CTK_TEXTMAP_NORMAL 0
#define CTK_TEXTMAP_ACTIVE 1

#define CTK_TEXTMAP(x, y, w, h, textmap) \
 NULL, NULL, x, y, CTK_WIDGET_LABEL, w, h, CTK_WIDGET_FLAG_INITIALIZER(0) text, CTK_TEXTMAP_NORMAL
struct ctk_textmap {
  struct ctk_widget *next;
  struct ctk_window *window;
  unsigned char x, y;
  unsigned char type;
  unsigned char w, h;
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  char *textmap;
  unsigned char state;
};
#endif /* CTK_CONF_ICONS */

/**
 * \internal The CTK button widget structure.
 */
struct ctk_widget_button {
  char *text;  /**< The button text. */
};

/**
 * \internal The CTK label widget structure.
 */
struct ctk_widget_label {
  char *text; /**< The label text. */
};

/**
 * \internal The CTK hyperlink widget structure.
 */
struct ctk_widget_hyperlink {
  char *text; /**< The text of the hyperlink. */
  char *url;  /**< The hyperlink's URL. */
};

struct ctk_widget_textentry {
  char *text;
  unsigned char len;
  unsigned char state;
  unsigned char xpos, ypos;
  ctk_textentry_input input;
};

struct ctk_widget_icon {
  char *title;
  struct process *owner;
  unsigned char *bitmap;
  char *textmap;
};

struct ctk_widget_bitmap {
  unsigned char *bitmap;
  unsigned short bw, bh;
};
/** @} */

/**
 * \addtogroup ctkdraw
 * @{
 */

/**
 * The generic CTK widget structure that contains all other widget
 * structures.
 *
 * Since the widgets of a window are arranged on a linked list, the
 * widget structure contains a next pointer which is used for this
 * purpose. The widget structure also contains the placement and the
 * size of the widget.
 *
 * Finally, the actual per-widget structure is contained in this
 * top-level widget structure.
 */
struct ctk_widget {
  struct ctk_widget *next;   /**< The next widget in the linked list
				of widgets that is contained in the
				ctk_window structure. */
  struct ctk_window *window; /**< The window in which the widget is
				contained. */
  unsigned char x,           /**< The x position of the widget within
				the containing window, in character
				coordinates. */
    y;                       /**< The y position of the widget within
				the containing window, in character
				coordinates. */
  unsigned char type;        /**< The type of the widget:
				CTK_WIDGET_SEPARATOR,
				CTK_WIDGET_LABEL, CTK_WIDGET_BUTTON,
				CTK_WIDGET_HYPERLINK,
				CTK_WIDGET_TEXTENTRY,
				CTK_WIDGET_BITMAP or
				CTK_WIDGET_ICON. */
  unsigned char w,           /**< The width of the widget in character
				coordinates. */
    h;                       /**< The height of the widget in
				character coordinates. */
#if CTK_CONF_WIDGET_FLAGS
  unsigned char flags;
#endif /* CTK_CONF_WIDGET_FLAGS */
  
  union {
    struct ctk_widget_label label;
    struct ctk_widget_button button;
    struct ctk_widget_hyperlink hyperlink;
    struct ctk_widget_textentry textentry;
    struct ctk_widget_icon icon;
    struct ctk_widget_bitmap bitmap;
  } widget;                  /**< The union which contains the actual
				widget structure, as determined by the
				type field. */
};


struct ctk_desktop;

#define CTK_WIDGET_FLAG_NONE      0
#define CTK_WIDGET_FLAG_MONOSPACE 1
#define CTK_WIDGET_FLAG_CENTER    2

#if CTK_CONF_WIDGET_FLAGS
#define CTK_WIDGET_SET_FLAG(w, f) ((struct ctk_widget *)(w))->flags = (f)
#else /* CTK_CONF_WIDGET_FLAGS */
#define CTK_WIDGET_SET_FLAG(w, f)
#endif /* CTK_CONF_WIDGET_FLAGS */

/**
 * Representation of a CTK window.
 *
 * For the CTK, each window is repessented by a ctk_window
 * structure. All open windows are kept on a doubly linked list,
 * linked by the next and prev fields in the ctk_window struct. The
 * window structure holds all widgets that is contained in the window
 * as well as a pointer to the currently selected widget.
 *
 */
struct ctk_window {
  struct ctk_window *next,  /**< The next window in the doubly linked
			       list of open windows. */

    *prev;                  /**< The previous window in the doubly
			       linked list of open windows. */
  struct ctk_desktop *desktop;/**< The desktop on which this window is
				 open. */
  
  struct process *owner;            /**< The process that owns the
			       window. This process will be the
			       receiver of all CTK signals that
			       pertain to this window. */
  
  char *title;              /**< The title of the window. Used for
			       constructing the "Dekstop" menu. */
  unsigned char titlelen;   /**< The length of the title, cached for
			       speed reasons. */

#if CTK_CONF_WINDOWCLOSE
  struct ctk_button closebutton; /**< The closebutton. This is also
				    present in the list of active
				    widgets. */
#else /* CTK_CONF_WINDOWCLOSE */
  struct ctk_label closebutton;
#endif /* CTK_CONF_WINDOWCLOSE */
  
#if CTK_CONF_WINDOWMOVE
  struct ctk_button titlebutton;/**< The titlebutton which is used for
				     moving the window. This is also
				     present in the list of active
				     widgets. */
#else /* CTK_CONF_WINDOWMOVE */
  struct ctk_label titlebutton;
#endif /* CTK_CONF_WINDOWMOVE */

#if CTK_CONF_WINDOWS
  unsigned char x,             /**< The x coordinate of the window, in
				  characters. */
    y;                         /**< The y coordinate of the window, in
				  characters. */
#endif /* CTK_CONF_WINDOWS */
  unsigned char w,             /**< The width of the window, excluding
				  window borders. */
    h;                         /**< The height of the window,
				  excluding window borders. */


  struct ctk_widget *inactive; /**< The list if widgets that cannot be
				  selected by the user. Labels and
				  separator widgets are placed on this
				  list. */
  struct ctk_widget *active;   /**< The list of widgets that can be
				  selected by the user. Buttons,
				  hyperlinks, text entry fields, etc.,
				  are placed on this list. */
  struct ctk_widget *focused;  /**< A pointer to the widget on the
				  active list that is currently
				  selected, or NULL if no widget is
				  selected. */
};

/**
 * Representation of an individual menu item.
 */
struct ctk_menuitem {
  char *title;           /**< The menu items text. */
  unsigned char titlelen;/**< The length of the item text, cached for
			    speed. */
};

#ifdef CTK_CONF_MAXMENUITEMS
#define CTK_MAXMENUITEMS CTK_CONF_MAXMENUITEMS
#else
#define CTK_MAXMENUITEMS 8
#endif

/**
 * Representation of an individual menu.
 */
struct ctk_menu {
  struct ctk_menu *next; /**< Apointer to the next menu, or is NULL if
			    this is the last menu, and should be used
			    by the ctk-draw module when stepping
			    through the menus when drawing them on
			    screen. */
  char *title;           /**< The menu title. */
  unsigned char titlelen;/**< The length of the title in
			    characters. Cached for speed reasons. */
#if CC_UNSIGNED_CHAR_BUGS
  unsigned int nitems;
  unsigned int active;
#else /* CC_UNSIGNED_CHAR_BUGS */
  unsigned char nitems;  /**< The total number of menu items in the
			    menu. */
  unsigned char active;  /**< The currently active menu item. */
#endif /* CC_UNSIGNED_CHAR_BUGS */
  struct ctk_menuitem items[CTK_MAXMENUITEMS];
                         /**< The array which contains all the menu
			    items. */
};

/**
 * Representation of the menu bar.
 */
struct ctk_menus {
  struct ctk_menu *menus;       /**< A pointer to a linked list of all
				   menus, including the open menu and
				   the desktop menu.*/
  struct ctk_menu *open;        /**< The currently open menu, if
				   any. If all menus are closed, this
				   item is NULL: */
  struct ctk_menu *desktopmenu; /**< A pointer to the "Desktop" menu
				   that can be used for drawing the
				   desktop menu in a special way (such
				   as drawing it at the rightmost
				   position). */
};

/** @} */


/**
 * \internal The structure describing a Contiki desktop.
 */
struct ctk_desktop {
  char *name; /**< The name of the desktop. */
   
  struct ctk_window desktop_window; /**< The background window which
				       contains tha desktop icons. */
  struct ctk_window *windows; /**< The list of open windows. */
  struct ctk_window *dialog;  /**< A pointer to the open dialog, or
				 NULL if no dialog is open. */
  
#if CTK_CONF_MENUS
  struct ctk_menus menus;     /**< The list of desktop menus. */
  struct ctk_menu *lastmenu;  /**< Pointer to the menu that was last open. */
  struct ctk_menu desktopmenu;/**< The desktop menu. */
#endif /* CTK_CONF_MENUS */

  unsigned char height, /**< The height of the desktop, in characters. */
    width; /**< The width of the desktop, in characters. */

  
#define CTK_REDRAW_NONE         0 /**< \internal Redraw flag: nothing
				     to be redrawn. */
#define CTK_REDRAW_ALL          1 /**< \internal Redraw flag:
				     everything should be redrawn. */
#define CTK_REDRAW_WINDOWS      2 /**< \internal Redraw flag: redraw
				     windows in queue.*/
#define CTK_REDRAW_WIDGETS      4 /**< \internal Redraw flag: redraw
				     widgets in queue. */
#define CTK_REDRAW_MENUS        8 /**< \internal Redraw flag: redraw
				     menus. */
#define CTK_REDRAW_PART        16 /**< \internal Redraw flag: redraw
				     parts of the desktop. */

#ifndef CTK_CONF_MAX_REDRAWWIDGETS
#define CTK_CONF_MAX_REDRAWWIDGETS 8
#endif /* CTK_CONF_MAX_REDRAWWIDGETS */
#ifndef CTK_CONF_MAX_REDRAWWINDOWS
#define CTK_CONF_MAX_REDRAWWINDOWS 8
#endif /* CTK_CONF_MAX_REDRAWWINDOWS */
  
  unsigned char redraw; /**< The redraw flag. */
  
  struct ctk_widget *redraw_widgets[CTK_CONF_MAX_REDRAWWIDGETS]; /**< The list of widgets to be redrawn. */
  unsigned char redraw_widgetptr; /**< Pointer to the last widget on the redraw_widgets list. */

  struct ctk_window *redraw_windows[CTK_CONF_MAX_REDRAWWINDOWS]; /**< The list of windows to be redrawn. */
  unsigned char redraw_windowptr; /**< Pointer to the last window on the redraw_windows list. */

   unsigned char redraw_y1, /**< The lower y bound of the area to be redrawn if CTK_REDRAW_PART is flagged. */
    redraw_y2; /**< The upper y bound of the area to be redrawn if CTK_REDRAW_PART is flagged. */
};


/* Global CTK modes. */
#define CTK_MODE_NORMAL      0
#define CTK_MODE_WINDOWMOVE  1
#define CTK_MODE_SCREENSAVER 2
#define CTK_MODE_EXTERNAL    3

/* General ctk functions. */
PROCESS_NAME(ctk_process);
void ctk_init(void);
void ctk_restore(void);

void ctk_mode_set(unsigned char mode);
unsigned char ctk_mode_get(void);
/*void ctk_redraw(void);*/

/* Functions for manipulating windows. */
CCIF void ctk_window_new(struct ctk_window *window,
			 unsigned char w, unsigned char h,
			 char *title);
CCIF void ctk_window_clear(struct ctk_window *w);
CCIF void ctk_window_open(struct ctk_window *w);
#define ctk_window_move(w,xpos,ypos) do { (w)->x=xpos; (w)->y=ypos; } while(0)
CCIF void ctk_window_close(struct ctk_window *w);
CCIF void ctk_window_redraw(struct ctk_window *w);
#define ctk_window_isopen(w) ((w)->next != NULL)


/* Functions for manipulating dialogs. */
CCIF void ctk_dialog_new(struct ctk_window *window,
			unsigned char w, unsigned char h);
CCIF void ctk_dialog_open(struct ctk_window *d);
CCIF void ctk_dialog_close(void);

/* Functions for manipulating menus. */
CCIF void ctk_menu_new(struct ctk_menu *menu, char *title);
CCIF void ctk_menu_add(struct ctk_menu *menu);
CCIF void ctk_menu_remove(struct ctk_menu *menu);
CCIF unsigned char ctk_menuitem_add(struct ctk_menu *menu, char *name);

/* Functions for icons. */

/**
 * \addtogroup ctkappfunc
 * @{
 */
/**
 * Add an icon to the desktop.
 *
 * \param icon The icon to be added.
 *
 * \param p The process ID of the process that owns the icon.
 */
#define CTK_ICON_ADD(icon, p) ctk_icon_add((struct ctk_widget *)icon, p)
void ctk_icon_add(struct ctk_widget *icon, struct process *p);

/* Functions for manipulating widgets. */

/**
 * Add a widget to a window.
 *
 * \param win The window to which the widget should be added.
 * \param widg The widget to be added.
 */
#define CTK_WIDGET_ADD(win, widg) \
 ctk_widget_add(win, (struct ctk_widget *)widg)
CCIF void CC_FASTCALL ctk_widget_add(struct ctk_window *window,
				     struct ctk_widget *widget);

/**
 * Set focus to a widget.
 *
 * \param win The widget's window.
 * \param widg The widget
 */
#define CTK_WIDGET_FOCUS(win, widg) \
  (win)->focused = (struct ctk_widget *)(widg)

/**
 * Add a widget to the redraw queue.
 *
 * \param widg The widget to be redrawn.
 */
#define CTK_WIDGET_REDRAW(widg) \
 ctk_widget_redraw((struct ctk_widget *)widg)
CCIF void ctk_widget_redraw(struct ctk_widget *w);

/**
 * Obtain the type of a widget.
 *
 * \param w The widget.
 */
#define CTK_WIDGET_TYPE(w) ((w)->type)


/**
 * Sets the width of a widget.
 *
 * \param widget The widget.
 * \param width The width of the widget, in characters.
 */
#define CTK_WIDGET_SET_WIDTH(widget, width) do { \
    ((struct ctk_widget *)(widget))->w = (width); } while(0)

/**
 * Retrieves the x position of a widget, relative to the window in
 * which the widget is contained.
 *
 * \param w The widget.
 * \return The x position of the widget.
 */
#define CTK_WIDGET_XPOS(w) (((struct ctk_widget *)(w))->x)

/**
 * Sets the x position of a widget, relative to the window in
 * which the widget is contained.
 *
 * \param w The widget.
 * \param xpos The x position of the widget.
 */
#define CTK_WIDGET_SET_XPOS(w, xpos) \
        ((struct ctk_widget *)(w))->x = (xpos)
/**
 * Retrieves the y position of a widget, relative to the window in
 * which the widget is contained.
 *
 * \param w The widget.
 * \return The y position of the widget.
 */
#define CTK_WIDGET_YPOS(w) (((struct ctk_widget *)(w))->y)

/**
 * Sets the y position of a widget, relative to the window in
 * which the widget is contained.
 *
 * \param w The widget.
 * \param ypos The y position of the widget.
 */
#define CTK_WIDGET_SET_YPOS(w, ypos) \
        ((struct ctk_widget *)(w))->y = (ypos)

/* XXX: should be removed.
#define ctk_textentry_set_height(w, height) \
                           (w)->widget.textentry.h = (height)
*/

/** \def ctk_label_set_height(w, height)
 * \brief Set the height of a label.
 *
 * \param w The CTK label widget.
 * \param height The new height of the label.
 */
#define ctk_label_set_height(w, height) \
                           (w)->widget.label.h = (height)

/**
 * Set the text of a label.
 *
 * \param l The CTK label widget.
 * \param t The new text of the label.
 */
#define ctk_label_set_text(l, t) (l)->text = (t)

/**
 * Set the text of a button.
 *
 * \param b The CTK button widget.
 * \param t The new text of the button.
 */
#define ctk_button_set_text(b, t) (b)->text = (t)

#define ctk_bitmap_set_bitmap(b, m) (b)->bitmap = (m)

#define CTK_BUTTON_NEW(widg, xpos, ypos, width, buttontext) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_BUTTON; \
 (widg)->x = (xpos); \
 (widg)->y = (ypos); \
 (widg)->w = (width); \
 (widg)->h = 1; \
 (widg)->text = (buttontext); \
 } while(0)

#define CTK_LABEL_NEW(widg, xpos, ypos, width, height, labeltext) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_LABEL; \
 (widg)->x = (xpos); \
 (widg)->y = (ypos); \
 (widg)->w = (width); \
 (widg)->h = (height); \
 (widg)->text = (labeltext); \
 } while(0)

#define CTK_BITMAP_NEW(widg, xpos, ypos, width, height, bmap) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_BITMAP; \
 (widg)->x = (xpos); \
 (widg)->y = (ypos); \
 (widg)->w = (width); \
 (widg)->h = (height); \
 (widg)->bitmap = (bmap); \
 } while(0)

#define CTK_TEXTENTRY_NEW(widg, xxpos, yypos, width, height, textptr, textlen) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_TEXTENTRY; \
 (widg)->x = (xxpos); \
 (widg)->y = (yypos); \
 (widg)->w = (width); \
 (widg)->h = 1; \
 (widg)->text = (textptr); \
 (widg)->len = (textlen); \
 (widg)->state = CTK_TEXTENTRY_NORMAL; \
 (widg)->xpos = 0; \
 (widg)->ypos = 0; \
 (widg)->input = NULL; \
 } while(0)

#define CTK_TEXTENTRY_INPUT_NEW(widg, xxpos, yypos, width, height, textptr, textlen, iinput) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_TEXTENTRY; \
 (widg)->x = (xxpos); \
 (widg)->y = (yypos); \
 (widg)->w = (width); \
 (widg)->h = (height); \
 (widg)->text = (textptr); \
 (widg)->len = (textlen); \
 (widg)->state = CTK_TEXTENTRY_NORMAL; \
 (widg)->xpos = 0; \
 (widg)->ypos = 0; \
 (widg)->input = (ctk_textentry_input)(iinput); \
 } while(0)

#define CTK_HYPERLINK_NEW(widg, xpos, ypos, width, linktext, linkurl) \
 do { (widg)->window = NULL; \
 (widg)->next = NULL; \
 (widg)->type = CTK_WIDGET_HYPERLINK; \
 (widg)->x = (xpos); \
 (widg)->y = (ypos); \
 (widg)->w = (width); \
 (widg)->h = 1; \
 (widg)->text = (linktext); \
 (widg)->url = (linkurl); \
 } while(0)

/* Desktop interface. */
void ctk_desktop_redraw(struct ctk_desktop *d);
CCIF unsigned char ctk_desktop_width(struct ctk_desktop *d);
unsigned char ctk_desktop_height(struct ctk_desktop *d);

/* Signals. */
CCIF extern process_event_t ctk_signal_keypress,
  ctk_signal_widget_activate,
  ctk_signal_widget_select,
  ctk_signal_timer,
  ctk_signal_menu_activate,
  ctk_signal_window_close,
  ctk_signal_pointer_move,
  ctk_signal_pointer_button;

#if CTK_CONF_SCREENSAVER
extern process_event_t ctk_signal_screensaver_stop,
  ctk_signal_screensaver_start;

extern unsigned short ctk_screensaver_timeout;
/**
 * Set the screensaver timeout, in seconds.
 *
 * \param t The timeout in seconds.
 */
#define CTK_SCREENSAVER_SET_TIMEOUT(t) ctk_screensaver_timeout = (t)
/**
 * Obtain the screensaver timeout, in seconds.
 *
 * \raturn The timeout in seconds.
 */
#define CTK_SCREENSAVER_TIMEOUT() ctk_screensaver_timeout
#endif /* CTK_CONF_SCREENSAVER */

/* These should no longer be used: */
CCIF extern process_event_t ctk_signal_button_activate,
  ctk_signal_button_hover,
  ctk_signal_hyperlink_activate,
  ctk_signal_hyperlink_hover;
/** @} */

/**
 * \addtogroup ctkdraw
 * @{
 */

/* Focus flags */
/** Widget focus flag: no focus. */
#define CTK_FOCUS_NONE     0
/** Widget focus flag: widget has focus. */
#define CTK_FOCUS_WIDGET   1
/** Widget focus flag: widget's window is the foremost one. */
#define CTK_FOCUS_WINDOW   2
/** Widget focus flag: widget is in a dialog. */
#define CTK_FOCUS_DIALOG   4

/** @} */
/** @} */
/** @} */
#endif /* CTK_H_ */
