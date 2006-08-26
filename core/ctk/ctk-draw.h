/**
 * \addtogroup ctk
 * @{
 */

/**
 * \file
 * CTK screen drawing module interface, ctk-draw.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This file contains the interface for the ctk-draw module.The
 * ctk-draw module takes care of the actual screen drawing for CTK by
 * implementing a handful of functions that are called by CTK.
 *
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: ctk-draw.h,v 1.2 2006/08/26 23:56:18 oliverschmidt Exp $
 *
 */

#ifndef __CTK_DRAW_H__
#define __CTK_DRAW_H__

#include "ctk/ctk.h"
#include "contiki-conf.h"

/**
 * \defgroup ctkdraw CTK device driver functions
 * @{
 *
 * The CTK device driver functions are divided into two modules, the
 * ctk-draw module and the ctk-arch module. The purpose of the
 * ctk-arch and the ctk-draw modules is to act as an interface between
 * the CTK and the actual hardware of the system on which Contiki is
 * run. The ctk-arch takes care of the keyboard input from the user,
 * and the ctk-draw is responsible for drawing the CTK desktop,
 * windows and user interface widgets onto the actual screen.
 *
 * More information about the ctk-draw and the ctk-arch modules can be
 * found in the sections \ref ctk-draw and \ref ctk-arch.
 */

/**
 * \page ctk-draw The ctk-draw module
 *
 * In order to work efficiently even on limited systems, CTK uses a
 * simple coordinate system, where the screen is addressed using
 * character coordinates instead of pixel coordinates. This makes it
 * trivial to implement the coordinate system on a text-based screen,
 * and significantly reduces complexity for pixel based screen
 * systems.
 *
 * The top left of the screen is (0,0) with x and y coordinates
 * growing downwards and to the right.
 *
 * It is the responsibility of the ctk-draw module to keep track of
 * the screen size and must implement the two functions
 * ctk_draw_width() and ctk_draw_height(), which are used by the CTK
 * for querying the screen size. The functions must return the width
 * and the height of the ctk-draw screen in character coordinates.
 *
 * The ctk-draw module is responsible for drawing CTK windows onto the
 * screen through the function ctk_draw_window().. A pseudo-code
 * implementation of this  function might look like this:
 * \code
   ctk_draw_window(window, focus, clipy1, clipy2, draw_borders) {
      if(draw_borders) {
         draw_window_borders(window, focus, clipy1, clipy2);
      }
      foreach(widget, window->inactive) {
         ctk_draw_widget(widget, focus, clipy1, clipy2);
      }
      foreach(widget, window->active) {
         if(widget == window->focused) {
	    ctk_draw_widget(widget, focus | CTK_FOCUS_WIDGET,
	                    clipy1, clipy2);
	 } else {
	    ctk_draw_widget(widget, focus, clipy1, clipy2);
	 }
      }
   }
   
   \endcode
 *
 * Where draw_window_borders() draws the window borders (also between
 * clipy1 and clipy2). The ctk_draw_widget() function is explained
 * below. Notice how the clipy1 and clipy2 parameters are passed to
 * all other functions; every function needs to know the boundaries
 * within which they are allowed to draw.
 *
 * In order to aid in implementing a ctk-draw module, a text-based
 * ctk-draw called ctk-conio has already been implemented. It conforms
 * to the Borland conio C library, and a skeleton implementation of
 * said library exists in lib/libconio.c. If a more machine specific
 * ctk-draw module is to be implemented, the instructions in this file
 * should be followed.
 * 
 */

/**
 * The initialization function.
 *
 * This function is supposed to get the screen ready for drawing, and
 * may be called at more than one time during the operation of the
 * system.
 */
void ctk_draw_init(void);

/**
 * Clear the screen between the clip bounds.
 *
 * This function should clear the screen between the y coordinates
 * "clipy1" and "clipy2", including the line at y coordinate "clipy1",
 * but not the line at y coordinate "clipy2".
 *
 * \note This function may be used to draw a background image
 * (wallpaper) on the desktop; it does not necessarily "clear" the
 * screen.
 *
 * \param clipy1 The lower y coordinate of the clip region.
 * \param clipy2 The upper y coordinate of the clip region. 
 */
void ctk_draw_clear(unsigned char clipy1, unsigned char clipy2);

/**
 * Draw the window background.
 *
 * This function will be called by the CTK before a window will be
 * completely redrawn.The function is supposed to draw the window
 * background, excluding window borders as these should be drawn by
 * the function that actually draws the window, between "clipy1" and
 * "clipy2".
 *
 * \note This function does not necessarily have to clear the window -
 * it can be used for drawing a background pattern in the window as
 * well.
 *
 * \param window The window for which the background should be drawn.
 *
 * \param focus The focus of the window, either CTK_FOCUS_NONE for a
 * background window, or CTK_FOCUS_WINDOW for the foreground window.
 * 
 * \param clipy1 The lower y coordinate of the clip region.
 * \param clipy2 The upper y coordinate of the clip region. 
*/
void ctk_draw_clear_window(struct ctk_window *window,
			   unsigned char focus,
			   unsigned char clipy1,
			   unsigned char clipy2);
/**
 * Draw a window onto the screen.
 *
 * This function is called by the CTK when a window should be drawn on
 * the screen. The ctk-draw layer is free to choose how the window
 * will appear on screen; with or without window borders and the style
 * of the borders, with or without transparent window background and
 * how the background shall look, etc.
 *
 * \param window The window which is to be drawn.
 *
 * \param focus Specifies if the window should be drawn in foreground
 * or background colors and can be either CTK_FOCUS_NONE or
 * CTK_FOCUS_WINDOW. Windows with a focus of CTK_FOCUS_WINDOW is
 * usually drawn in a brighter color than those with CTK_FOCUS_NONE.
 *
 * \param clipy1 Specifies the first lines on screen that actually
 * should be drawn, in screen coordinates (line 1 is the first line
 * below the menus).
 *
 * \param clipy2 Specifies the last + 1 line on screen that should be
 * drawn, in screen coordinates (line 1 is the first line below the
 * menus)
 *
 */
void ctk_draw_window(struct ctk_window *window,
		     unsigned char focus,
		     unsigned char clipy1,
		     unsigned char clipy2,
		     unsigned char draw_borders);


/**
 * Draw a dialog onto the screen.
 *
 * In CTK, a dialog is similar to a window, with the only exception
 * being that they are drawn in a different style. Also, since dialogs
 * always are drawn on top of everything else, they do not need to be
 * drawn within any special boundaries.
 *
 * \note This function can usually be implemented so that it uses the
 * same widget drawing code as the ctk_draw_window() function.
 *
 * \param dialog The dialog that is to be drawn.
 */
void ctk_draw_dialog(struct ctk_window *dialog);

/**
 * Draw a widget on a window.
 *
 * This function is used for drawing a CTK widgets onto the screem is
 * likely to be the most complex function in the ctk-draw
 * module. Still, it is straightforward to implement as it can be
 * written in an incremental fashion, starting with a single widget
 * type and adding more widget types, one at a time.

 * The ctk-draw module may exploit how the CTK focus constants are
 * defined in order to use a look-up table for the colors. The CTK
 * focus constants are defined in the file ctk/ctk.h as follows:
 \code
   #define CTK_FOCUS_NONE     0
   #define CTK_FOCUS_WIDGET   1
   #define CTK_FOCUS_WINDOW   2
   #define CTK_FOCUS_DIALOG   4
 \endcode

 * This gives the following table:
 \code
   0: CTK_FOCUS_NONE      (Background window, non-focused widget)
   1: CTK_FOCUS_WIDGET    (Background window, focused widget)
   2: CTK_FOCUS_WINDOW    (Foreground window, non-focused widget)
   3: CTK_FOCUS_WINDOW | CTK_FOCUS_WIDGET
                          (Foreground window, focused widget)
   4: CTK_FOCUS_DIALOG    (Dialog, non-focused widget)
   5: CTK_FOCUS_DIALOG | CTK_FOCUS_WIDGET
                          (Dialog, focused widget)
 \endcode 


 * \param w The widget to be drawn.
 * \param focus The focus of the widget.
 * \param clipy1 The lower y coordinate of the clip region.
 * \param clipy2 The upper y coordinate of the clip region. 
 */

void ctk_draw_widget(struct ctk_widget *w,
		     unsigned char focus,
		     unsigned char clipy1,
		     unsigned char clipy2);

void ctk_draw_menus(struct ctk_menus *menus);



/* Returns width and height of screen. */
CCIF unsigned char ctk_draw_width(void);
CCIF unsigned char ctk_draw_height(void);


extern unsigned char ctk_draw_windowborder_width,
  ctk_draw_windowborder_height,
  ctk_draw_windowtitle_height;


#endif /* __CTK_DRAW_H__ */


/**
 * The keyboard character type of the system
 *
 * The ctk_arch_key_t is usually typedef'd to the char type, but some
 * systems (such as VNC) have a 16-bit key type.
 *
 * \var typedef char ctk_arch_key_t;
 */

/**
 * Get a keypress from the keyboard input queue.
 *
 * This function will remove the first keypress in the keyboard input
 * queue and return it. If the keyboard queue is empty, the return
 * value is undefined. This function is intended to be used only after
 * the ctk_arch_keyavail() function has returned non-zero.
 *
 * \return The first keypress from the keyboard input queue.
 *
 * \fn ctk_arch_key_t ctk_arch_getkey(void);
 */

/**
 * Check if there is a keypress in the keyboard input queue.
 *
 * \return Zero if the keyboard input queue is empty, non-zero
 * otherwise.
 *
 * \fn unsigned char ctk_arch_keyavail(void);
 */

/**
 * The character used for the Return/Enter key.
 *
 * \define #define CH_ENTER '\n'
 */

/**
 * \page ctk-arch The ctk-arch module
 *
 * The ctk-arch module deals with keyboard input from the underlying
 * target system on which Contiki is running. The ctk-arch manages a
 * keyboard input queue that is queried using the two functions
 * ctk_arch_keyavail() and ctk_arch_getkey().
 */

/** @} */
/** @} */
