/*
 * Copyright (c) 2012, François Revol <mmu_man@users.sourceforge.net>
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
 */

#include <string.h>
#include <time.h>

#include "contiki.h"
#include "ctk/ctk.h"

/* TODO: Fix wrong calculation: the seconds dot disappears sometimes */
/* TODO: Add hours and minutes dots (and remove seconds?) */
/*#define ANALOG*/

static struct ctk_window window;

#ifdef ANALOG
static const char text_clock_tmpl[] = \
". . . . ."
".       ."
".   o   ."
".       ."
". . . . .";
static char text_clock[] = \
". . . . ."
".       ."
".   o   ."
".       ."
". . . . .";
#define MO(x,y) (y*9+x)
static const char offsets1[] = { MO(4, 2), };
static struct ctk_label clocklabel = { CTK_LABEL(1, 0, 9, 5, text_clock) };
static char text_time[] = "  :  :  ";
static struct ctk_label clocklabel2 = { CTK_LABEL(1, 6, 10, 1, text_time) };
#else
static char text_date[] = "  /  /    ";
static char text_time[] = "  :  :  ";
static struct ctk_label clocklabel1 = { CTK_LABEL(1, 0, 10, 1, text_date) };
static struct ctk_label clocklabel2 = { CTK_LABEL(2, 1, 10, 1, text_time) };
#endif

PROCESS(clock_process, "Clock");

AUTOSTART_PROCESSES(&clock_process);

/*-----------------------------------------------------------------------------------*/
static char
calc_off1(char a)
{
  char x, y;

  a += 7;
  a %= 60;
  if(a < 15) {
    x = a * 9 / 15;
    y = 0;
    return y * 9 + x;
  }
  if(a < 30) {
    x = 8;
    y = (a - 15) * 5 / 15;
    return y * 9 + x;
  }
  if(a < 45) {
    x = 9 - (a - 30) * 9 / 15;
    y = 4;
    return y * 9 + x;
  }
  x = 0;
  y = 5 - (a - 45) * 5 / 15;
  return y * 9 + x;
}
/*-----------------------------------------------------------------------------------*/
static void
clock_quit(void)
{
  process_exit(&clock_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
update_clock(void)
{
  time_t secs = clock_seconds();

  /*
   * TODO: implement it for other platforms
   * cf. http://git.uclibc.org/uClibc/tree/libc/misc/time/time.c _time_t2tm
   */
  struct tm *t = localtime(&secs);
  if(t == NULL) {
    return;
  }

#ifdef ANALOG
  memcpy(text_clock, text_clock_tmpl, sizeof(text_clock_tmpl));
  text_clock[calc_off1(t->tm_sec)] = 'o';
  snprintf(text_time, sizeof(text_time), "%02d:%02d:%02d", t->tm_hour,
           t->tm_min, t->tm_sec);
  ctk_widget_redraw(&clocklabel);
  ctk_widget_redraw(&clocklabel2);
#else
  snprintf(text_date, sizeof(text_date), "%04d-%02d-%02d", t->tm_year + 1900,
           t->tm_mon, t->tm_mday);
  snprintf(text_time, sizeof(text_time), "%02d:%02d:%02d", t->tm_hour,
           t->tm_min, t->tm_sec);
  ctk_widget_redraw(&clocklabel2);
#endif
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(clock_process, ev, data)
{
  static struct etimer periodic;
  unsigned char width;

  PROCESS_BEGIN();

  width = ctk_desktop_width(NULL);

#ifdef ANALOG
  ctk_window_new(&window, 11, 5, "Clock");
  ctk_window_move(&window, width - 11 - 2, 0);

  CTK_WIDGET_ADD(&window, &clocklabel);
  CTK_WIDGET_ADD(&window, &clocklabel2);
#else
  ctk_window_new(&window, 12, 2, "Clock");
  ctk_window_move(&window, width - 12 - 2, 0);

  CTK_WIDGET_ADD(&window, &clocklabel1);
  CTK_WIDGET_ADD(&window, &clocklabel2);
#endif

  update_clock();

  etimer_set(&periodic, CLOCK_SECOND * 1);

  ctk_window_open(&window);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER) {
      update_clock();
      etimer_set(&periodic, CLOCK_SECOND * 1);
    } else if(ev == PROCESS_EVENT_EXIT) {
      clock_quit();
      PROCESS_EXIT();
    } else if(ev == ctk_signal_window_close &&
              data == (process_data_t)&window) {
      clock_quit();
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
