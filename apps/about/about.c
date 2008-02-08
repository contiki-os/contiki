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
 * $Id: about.c,v 1.4 2008/02/08 22:48:28 oliverschmidt Exp $
 *
 */

#include <string.h>

#include "contiki.h"
#include "ctk/ctk.h"
#include "lib/petsciiconv.h"

static struct ctk_window aboutdialog;
static struct ctk_label aboutlabel1 =
  {CTK_LABEL(2, 0, 28, 1, "The Contiki Operating System")};
static struct ctk_label aboutlabel2 =
  {CTK_LABEL(3, 2, 28, 1, "A modern, Internet-enabled")};
static struct ctk_label aboutlabel3 =
  {CTK_LABEL(6, 3, 20, 1, "operating system and")};
static struct ctk_label aboutlabel4 =
  {CTK_LABEL(6, 4, 20, 1, "desktop environment.")};

static char abouturl_petscii[] = "http://www.sics.se/~adam/contiki/";
static char abouturl_ascii[40];
static struct ctk_hyperlink abouturl = 
  {CTK_HYPERLINK(0, 6, 32, "http://www.sics.se/~adam/contiki/",
		 abouturl_ascii)};
static struct ctk_button aboutclose =
  {CTK_BUTTON(12, 8, 5, "Close")};


PROCESS(about_process, "About Contiki");

AUTOSTART_PROCESSES(&about_process);

/*-----------------------------------------------------------------------------------*/
static void
about_quit(void)
{
  ctk_dialog_close();
  process_exit(&about_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(about_process, ev, data)     
{
  unsigned char width;

  PROCESS_BEGIN();
  
  width = ctk_desktop_width(NULL);
  
  strcpy(abouturl_ascii, abouturl_petscii);
  petsciiconv_toascii(abouturl_ascii, sizeof(abouturl_ascii));
  
  if(width > 34) {
    ctk_dialog_new(&aboutdialog, 32, 9);
  } else {
    ctk_dialog_new(&aboutdialog, width - 2, 9);
  }
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel1);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel2);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel3);
  CTK_WIDGET_ADD(&aboutdialog, &aboutlabel4);
  if(width > 34) {
    CTK_WIDGET_ADD(&aboutdialog, &abouturl);
    CTK_WIDGET_SET_FLAG(&abouturl, CTK_WIDGET_FLAG_MONOSPACE);
  } else {
    CTK_WIDGET_SET_XPOS(&aboutlabel1, 0);
    CTK_WIDGET_SET_XPOS(&aboutlabel2, 0);
    CTK_WIDGET_SET_XPOS(&aboutlabel3, 0);
    CTK_WIDGET_SET_XPOS(&aboutlabel4, 0);
    
    CTK_WIDGET_SET_XPOS(&aboutclose, 0);
  }
  CTK_WIDGET_ADD(&aboutdialog, &aboutclose);
  CTK_WIDGET_FOCUS(&aboutdialog, &aboutclose);
  
  ctk_dialog_open(&aboutdialog);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_EXIT) {
      about_quit();
      PROCESS_EXIT();
    } else if(ev == ctk_signal_button_activate) {
      if(data == (process_data_t)&aboutclose) {
	about_quit();
	PROCESS_EXIT();
      }
    } else if(ev == ctk_signal_hyperlink_activate) {
      if((struct ctk_widget *)data == (struct ctk_widget *)&abouturl) {
	about_quit();
	PROCESS_EXIT();
      }
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
