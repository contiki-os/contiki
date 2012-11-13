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
 * This file is part of the Contiki desktop environment
 *
 * $Id: blanker.c,v 1.4 2008/02/08 22:48:28 oliverschmidt Exp $
 *
 */

#include <string.h>

#include "contiki.h"
#include "ctk/ctk.h"

#define SAVER_TIMER CLOCK_SECOND / 2

/*
static struct ctk_window blankerdialog;
static struct ctk_label blankerlabel1 =
  {CTK_LABEL(2, 0, 28, 1, "Save the planet!")};

static struct ctk_button blankerclose =
  {CTK_BUTTON(12, 8, 5, "Close")};
*/

static char menu_names[][6] = { " Text", "", " 30s", " 1m", " 2m", " 5m" };
static struct ctk_menu menu;
unsigned char menuitem_setup, menuitem_quit;


static struct etimer periodic;

static unsigned char width, height;
static unsigned char x, y;
static const char text[] = "The Contiki Operating System";

PROCESS(blanker_process, "Blanker");

AUTOSTART_PROCESSES(&blanker_process);

/*-----------------------------------------------------------------------------------*/
static void
blanker_stop(void)
{
  etimer_stop(&periodic);
  ctk_restore();
  //ctk_desktop_redraw(NULL);
  //ctk_dialog_close();
}
/*-----------------------------------------------------------------------------------*/
static void
blanker_quit(void)
{
  blanker_stop();
  ctk_menu_remove(&menu);
  process_exit(&blanker_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
blanker_update(void)
{
  static char dx = 1, dy = 1;

  bgcolor(COLOR_BLACK);
  textcolor(COLOR_WHITE);
  clrscr();
  cputsxy(x, y, text);
  if (x == 0 && dx < 0)
    dx = -dx;
  if (x == width && dx > 0)
    dx = -dx;
  x += dx;
  if (y == 0 && dy < 0)
    dy = -dy;
  if (y == height && dy > 0)
    dy = -dy;
  y += dy;
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(blanker_process, ev, data)     
{
  int i;
  PROCESS_BEGIN();
  
  width = ctk_desktop_width(NULL);
  height = ctk_desktop_height(NULL);
  width -= sizeof(text) - 1;
  height -= 1;
  x = width / 2;
  y = height / 2;
  ctk_screensaver_timeout = 10; /* XXX: for testing */

  ctk_menu_new(&menu, "Blanker");
  for (i = 0; i < 6; i++)
    ctk_menuitem_add(&menu, menu_names[i]);
  ctk_menu_add(&menu);
  menu_names[0][0] = '>';

  while(1) {
    PROCESS_WAIT_EVENT();
fprintf(stderr, "mode: %d\n", ctk_mode_get());
    if(ev == PROCESS_EVENT_TIMER) {
      if(ctk_mode_get() == CTK_MODE_SCREENSAVER) {
        fprintf(stderr, "saver timer\n");
        blanker_update();
        etimer_reset(&periodic);
      }
    } else if(ev == ctk_signal_screensaver_start) {
	  fprintf(stderr, "saver start\n");
      /* XXX: shouldn't CTK set the mode itself? */
      ctk_mode_set(CTK_MODE_SCREENSAVER);
      etimer_set(&periodic, SAVER_TIMER);
    } else if(ev == ctk_signal_screensaver_stop) {
	  fprintf(stderr, "saver stop\n");
      blanker_stop();
    } else if(ev == PROCESS_EVENT_EXIT) {
	  fprintf(stderr, "saver quit\n");
      blanker_quit();
      PROCESS_EXIT();
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
