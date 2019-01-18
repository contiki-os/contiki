/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
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
 * $Id: shell-gui.c,v 1.3 2006/12/29 23:05:19 oliverschmidt Exp $
 *
 */


#include "program-handler.h"
#include "contiki.h"

#include "shell.h"

#include "lib/ctk-textentry-cmdline.h"

#include <string.h>

#ifdef SHELL_GUI_CONF_XSIZE
#define SHELL_GUI_XSIZE SHELL_GUI_CONF_XSIZE
#else
#define SHELL_GUI_XSIZE 10
#endif

#ifdef SHELL_GUI_CONF_YSIZE
#define SHELL_GUI_YSIZE SHELL_GUI_CONF_YSIZE
#else
#define SHELL_GUI_YSIZE 10
#endif


static struct ctk_window window;
static char log[SHELL_GUI_XSIZE * SHELL_GUI_YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, SHELL_GUI_XSIZE, SHELL_GUI_YSIZE, log)};
static char command[SHELL_GUI_XSIZE - 1];
static struct ctk_textentry commandentry =
  {CTK_TEXTENTRY_INPUT(0, SHELL_GUI_YSIZE, SHELL_GUI_XSIZE - 2, 1, command,
		       SHELL_GUI_XSIZE - 2, ctk_textentry_cmdline_input)};

PROCESS(shell_gui_process, "Command shell");

/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  ctk_window_close(&window);
  process_exit(&shell_gui_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned char i, len;
  
  for(i = 1; i < SHELL_GUI_YSIZE; ++i) {
    memcpy(&log[(i - 1) * SHELL_GUI_XSIZE],
	   &log[i * SHELL_GUI_XSIZE], SHELL_GUI_XSIZE);
  }
  memset(&log[(SHELL_GUI_YSIZE - 1) * SHELL_GUI_XSIZE],
	 0, SHELL_GUI_XSIZE);

  len = (unsigned char)strlen(str1);

  strncpy(&log[(SHELL_GUI_YSIZE - 1) * SHELL_GUI_XSIZE],
	  str1, SHELL_GUI_XSIZE);
  if(len < SHELL_GUI_XSIZE) {
    strncpy(&log[(SHELL_GUI_YSIZE - 1) * SHELL_GUI_XSIZE] + len,
	    str2, SHELL_GUI_XSIZE - len);
  }

  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(shell_gui_process, ev, data)
{

  PROCESS_BEGIN();
    
  ctk_window_new(&window, SHELL_GUI_XSIZE,
		 SHELL_GUI_YSIZE + 1, "Command shell");
  CTK_WIDGET_ADD(&window, &loglabel);
  /*    CTK_WIDGET_SET_FLAG(&loglabel, CTK_WIDGET_FLAG_MONOSPACE);*/
  CTK_WIDGET_ADD(&window, &commandentry);
  /*    CTK_WIDGET_SET_FLAG(&commandentry, CTK_WIDGET_FLAG_MONOSPACE);*/
  CTK_WIDGET_FOCUS(&window, &commandentry);
  memset(log, 0, sizeof(log));
  
  shell_init();
  ctk_window_open(&window);
  shell_start();

  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == ctk_signal_widget_activate &&
       data == (process_data_t)&commandentry) {
      shell_output("> ", command);
      shell_input(command);
      if(shell_gui_process.state) {
        CTK_TEXTENTRY_CLEAR(&commandentry);
        CTK_WIDGET_REDRAW(&commandentry);
      }
    } else if(ev == ctk_signal_window_close ||
	      ev == PROCESS_EVENT_EXIT) {
      shell_quit(NULL);
    } else {
      shell_eventhandler(ev, data);
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
