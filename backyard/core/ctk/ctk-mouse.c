/*
 * Copyright (c) 2002-2004, Adam Dunkels.
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
 * $Id: ctk-mouse.c,v 1.1 2007/05/26 21:38:53 oliverschmidt Exp $
 *
 */

#include "process.h"
#include "cc.h"

#include "ctk/ctk.h"
#include "ctk-draw.h"
#include "contiki-conf.h"
#include "ctk-mouse.h"

unsigned short mouse_x, mouse_y, mouse_button;

/*-----------------------------------------------------------------------------------*/
EK_POLLHANDLER(ctk_poll)
{
  static ctk_arch_key_t c;
  static unsigned char i;
  register struct ctk_window *window;
  register struct ctk_widget *widget;
  register struct ctk_widget **widgetptr;

  static unsigned char mxc, myc, mouse_button_changed, mouse_moved,
    mouse_clicked;
  static unsigned char menux;
  register struct ctk_menu *menu;

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

  if(mode == CTK_MODE_SCREENSAVER) {
    if(mouse_moved || mouse_button_changed) {      
      ek_post(EK_BROADCAST, ctk_signal_screensaver_stop, NULL);
      mode = CTK_MODE_NORMAL;
    }
  } else {
    
    /* If there is any change in the mouse conditions, find out in
       which window the mouse pointer currently is in order to send
       the correct signals, or bring a window to focus. */
    if(mouse_moved || mouse_button_changed) {
      ctk_mouse_show();
      screensaver_timer = 0;
      
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

	  if(window != NULL) {
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
	    
	      /* See if the mouse pointer is on a widget. If so, it
		 should be selected and, if the button is clicked,
		 activated. */
	      for(widget = window->active; widget != NULL;
		  widget = widget->next) {
		
		if(mxc >= widget->x &&
		   mxc <= widget->x + widget->w &&
		   (myc == widget->y ||
		    ((widget->type == CTK_WIDGET_BITMAP ||
		      widget->type == CTK_WIDGET_ICON) &&
		     (myc >= widget->y &&
		      myc <= widget->y +
		      ((struct ctk_bitmap *)widget)->h)))) {
		  break;
		}
	      }
	    

	      /* if the mouse is moved in the focused window, we emit
		 a ctk_signal_pointer_move signal to the owner of the
		 window. */
	      if(mouse_moved &&
		 (window != &desktop_window ||
		  windows == NULL)) {

		ek_post(window->owner, ctk_signal_pointer_move, NULL);

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
		ek_post(window->owner, ctk_signal_pointer_button,
			(ek_data_t)mouse_button);
		if(mouse_clicked && widget != NULL) {
		  select_widget(widget);
		  redraw |= activate(widget);
		}
	      }
	    }
	  }    
	}
      }
    }

    if(ctk_arch_keyavail()) {
      ctk_mouse_hide();
    }
      
#if CTK_CONF_WINDOWMOVE
  } else if(mode == CTK_MODE_WINDOWMOVE) {

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
      if(window->y > 0) {
	--window->y;
      }

      redraw = REDRAW_ALL;
    }		  
    
    /* Check if the mouse has been clicked, and stop moving the window
       if so. */
    if(mouse_button_changed &&
       mouse_button == 0) {
      mode = CTK_MODE_NORMAL;
      redraw = REDRAW_ALL;      
    }
#endif /* CTK_CONF_WINDOWMOVE */
  }  
}
/*-----------------------------------------------------------------------------------*/
