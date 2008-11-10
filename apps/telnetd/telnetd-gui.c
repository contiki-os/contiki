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
 * $Id: telnetd-gui.c,v 1.6 2008/11/10 22:10:33 oliverschmidt Exp $
 *
 */

#include "program-handler.h"
#include "contiki-net.h"
#include "lib/petsciiconv.h"

#include "shell.h"
#include "telnetd.h"

#include <string.h>

#define ISO_nl       0x0a
#define ISO_cr       0x0d

#define XSIZE 78
#define YSIZE 30

static struct ctk_window window;
static char log[XSIZE * YSIZE];
static struct ctk_label loglabel =
  {CTK_LABEL(0, 0, XSIZE, YSIZE, log)};

/*-----------------------------------------------------------------------------------*/
void
telnetd_gui_output(const char *str1, int len1, const char *str2, int len2)
{
  static unsigned int i;
  
  for(i = 1; i < YSIZE; ++i) {
    memcpy(&log[(i - 1) * XSIZE], &log[i * XSIZE], XSIZE);
  }

  strncpy(&log[(YSIZE - 1) * XSIZE], str1, XSIZE);
  if(len1 < XSIZE) {
    strncpy(&log[(YSIZE - 1) * XSIZE] + len1, str2, XSIZE - len1);
    if(len1 + len2 < XSIZE) {
      log[(YSIZE - 1) * XSIZE + len1 + len2] = 0;
    }
  }
  
  CTK_WIDGET_REDRAW(&loglabel);
}
/*-----------------------------------------------------------------------------------*/
void
telnetd_gui_quit(void)
{
  ctk_window_close(&window);
}
/*-----------------------------------------------------------------------------------*/
void
telnetd_gui_init(void)
{
  shell_file_init();
  shell_ps_init();
  shell_run_init();
  shell_text_init();
  shell_time_init();
  shell_wget_init();

  ctk_window_new(&window, XSIZE, YSIZE, "Shell server");
  CTK_WIDGET_ADD(&window, &loglabel);
  memset(log, 0, sizeof(log));
  ctk_window_open(&window);
}
/*-----------------------------------------------------------------------------------*/
void
telnetd_gui_eventhandler(process_event_t ev, process_data_t data)
{
  if(ev == ctk_signal_window_close) {
    telnetd_quit();
  }
}
/*-----------------------------------------------------------------------------------*/
