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
 * $Id: process-list.c,v 1.1 2006/06/17 22:41:12 adamdunkels Exp $
 *
 */

#include "ctk/ctk.h"
#include "contiki.h"

#include <string.h>

#define MAX_PROCESSLABELS 13
static struct ctk_window processwindow;
static unsigned char ids[MAX_PROCESSLABELS][4];
static struct ctk_label processidlabels[MAX_PROCESSLABELS];
static struct ctk_label processnamelabels[MAX_PROCESSLABELS];

static struct ctk_label killlabel =
  {CTK_LABEL(0, 14, 12, 1, "Kill process")};
static char killprocnum[4];
static struct ctk_textentry killtextentry =
  {CTK_TEXTENTRY(13, 14, 3, 1, killprocnum, 3)};
static struct ctk_button killbutton =
  {CTK_BUTTON(19, 14, 2, "Ok")};
static struct ctk_button processupdatebutton =
  {CTK_BUTTON(0, 15, 6, "Update")};
static struct ctk_button processclosebutton =
  {CTK_BUTTON(19, 15, 5, "Close")};

PROCESS(processes_process, "Process listing");

enum {
  EVENT_UPDATE
};

/*-----------------------------------------------------------------------------------*/
static void
update_processwindow(void)
{
  unsigned char i, j, *idsptr;
  struct process *p;

  /* Step through each possible process ID and see if there is a
     matching process. */
  j = 0;
  for(p = PROCESS_LIST(); p != NULL && j < MAX_PROCESSLABELS; p = p->next) {
    idsptr = ids[j];
    i = (int)&p;
    idsptr[0] = '0' + i / 100;
    if(idsptr[0] == '0') {
      idsptr[0] = ' ';
    }
    idsptr[1] = '0' + (i / 10) % 10;
    idsptr[2] = '0' + i % 10;
    idsptr[3] = 0;
    CTK_LABEL_NEW(&processidlabels[j],
		  0, j + 1, 3, 1, idsptr);
    CTK_WIDGET_ADD(&processwindow, &processidlabels[j]);
    
    CTK_LABEL_NEW(&processnamelabels[j],
		  4, j + 1, 22, 1, (char *)p->name);
    CTK_WIDGET_ADD(&processwindow, &processnamelabels[j]);

    ++j;
  }

  CTK_WIDGET_ADD(&processwindow, &killlabel);

  CTK_WIDGET_ADD(&processwindow, &killtextentry);
  CTK_WIDGET_ADD(&processwindow, &killbutton);  
  
  CTK_WIDGET_ADD(&processwindow, &processupdatebutton);
  CTK_WIDGET_ADD(&processwindow, &processclosebutton);
  CTK_WIDGET_FOCUS(&processwindow, &processupdatebutton);

}
/*-----------------------------------------------------------------------------------*/
static void
processes_quit(void)
{
  process_exit(&processes_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
killproc(void)
{
#if 0
  int procnum;
  unsigned char i, j;
  struct ek_proc *p;
  
  /* Find first zero char in killprocnum string. */
  for(i = 0; killprocnum[i] != 0 &&
	i < sizeof(killprocnum); ++i);

  if(i == 0) {
    return;
  }
  
  procnum = 0;
  
  for(j = 0; j < i; ++j) {
    procnum = procnum * 10 + (killprocnum[j] - '0');
    killprocnum[j] = 0;
  }

  /* Make sure the process ID exists. */
  for(p = EK_PROCS(); p != NULL; p = p->next) {
    if(EK_PROC_ID(p) == procnum) {
      break;
    }
  }

  if(p != NULL) {
    /*    ek_post(procnum, EK_EVENT_REQUEST_EXIT, NULL);
	  ek_post(id, EVENT_UPDATE, NULL);*/
    CTK_TEXTENTRY_CLEAR(&killtextentry);
    CTK_WIDGET_REDRAW(&killtextentry);
    CTK_WIDGET_FOCUS(&processwindow, &processupdatebutton);
    CTK_WIDGET_REDRAW(&killbutton);
    CTK_WIDGET_REDRAW(&processupdatebutton);
  }
#endif
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(processes_process, ev, data)     
{

  PROCESS_BEGIN();
  
  ctk_window_new(&processwindow, 26, 16, "Processes");
  update_processwindow();
  
  ctk_window_open(&processwindow);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == EVENT_UPDATE) {
      ctk_window_clear(&processwindow);
      update_processwindow();
      ctk_window_open(&processwindow);
    } else if(ev == ctk_signal_button_activate) {
      if(data == (process_data_t)&processupdatebutton) {
	ctk_window_clear(&processwindow);
	update_processwindow();
	ctk_window_open(&processwindow);
      } else if(data == (process_data_t)&processclosebutton) {
	ctk_window_close(&processwindow);
	processes_quit();
	/*      ctk_desktop_redraw(processwindow.desktop);      */
      } else if(data == (process_data_t)&killbutton) {
	killproc();
      }
    } else if(ev == PROCESS_EVENT_EXIT ||
	      (ev == ctk_signal_window_close &&
	       data == (process_data_t)&processwindow)) {
      ctk_window_close(&processwindow);
      processes_quit();
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
