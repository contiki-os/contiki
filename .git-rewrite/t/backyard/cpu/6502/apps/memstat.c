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
 * $Id: memstat.c,v 1.1 2007/05/23 23:47:44 oliverschmidt Exp $
 *
 */

#include <stdlib.h>

#include "contiki.h"
#include "ctk/ctk.h"

static struct ctk_window window;
static struct ctk_label freemsg =
  {CTK_LABEL(2, 0, 12, 1, "Free memory:")};
static char freemem[6];
static struct ctk_label freenum =
  {CTK_LABEL(18, 0, 5, 1, freemem)};

static struct ctk_label lblockmsg =
  {CTK_LABEL(2, 2, 14, 1, "Largest block:")};
static char lblock[6];
static struct ctk_label lblocknum =
  {CTK_LABEL(18, 2, 5, 1, lblock)};

static struct ctk_button updatebutton =
  {CTK_BUTTON(0, 4, 6, "Update")};
static struct ctk_button closebutton =
  {CTK_BUTTON(17, 4, 5, "Close")};

PROCESS(memstat_process, "Memory statistics");

/*-----------------------------------------------------------------------------------*/
static void
update(void)
{
  int mem;

  mem = _heapmemavail();
  freemem[0] = (mem/10000) % 10 + '0';
  freemem[1] = (mem/1000) % 10 + '0';
  freemem[2] = (mem/100) % 10 + '0';
  freemem[3] = (mem/10) % 10 + '0';
  freemem[4] = (mem) % 10 + '0';

  mem = _heapmaxavail();
  lblock[0] = (mem/10000) % 10 + '0';
  lblock[1] = (mem/1000) % 10 + '0';
  lblock[2] = (mem/100) % 10 + '0';
  lblock[3] = (mem/10) % 10 + '0';
  lblock[4] = (mem) % 10 + '0';

}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(memstat_process, ev, data)
{
  PROCESS_BEGIN();
  
  ctk_window_new(&window, 24, 5, "Memory stats");
  /*    ctk_window_move(&window, 0, 1);*/
  
  CTK_WIDGET_ADD(&window, &freemsg);
  CTK_WIDGET_ADD(&window, &freenum);
  
  CTK_WIDGET_ADD(&window, &lblockmsg);
  CTK_WIDGET_ADD(&window, &lblocknum);
  
  CTK_WIDGET_ADD(&window, &updatebutton);
  CTK_WIDGET_ADD(&window, &closebutton);
  
  CTK_WIDGET_FOCUS(&window, &updatebutton);
  
  update();
  
  ctk_window_open(&window);

  while(1) {

    PROCESS_WAIT_EVENT();
    
    if(ev == ctk_signal_button_activate) {
      if(data == &updatebutton) {
	update();
	ctk_window_redraw(&window);
      } else if(data == &closebutton) {
	ctk_window_close(&window);
	break;
      }
    } else if((ev == ctk_signal_window_close &&
	       data == &window) ||
	      ev == PROCESS_EVENT_EXIT) {
      break;
    }
  }
  
  PROCESS_END();
  
}
/*-----------------------------------------------------------------------------------*/
