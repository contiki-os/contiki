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
 *
 */

#include "ctk/ctk.h"
#include "contiki.h"

#include <string.h>

#ifdef PROCESSLIST_CONF_HEIGHT
#define PROCESSLIST_HEIGHT PROCESSLIST_CONF_HEIGHT
#else /* PROCESSLIST_CONF_HEIGHT */
#define PROCESSLIST_HEIGHT 16
#endif /* PROCESSLIST_CONF_HEIGHT */

#define MAX_PROCESSLABELS 13

static struct ctk_window processwindow;
static struct {struct process *p; char id[2];} processes[MAX_PROCESSLABELS];
static struct ctk_label processidlabels[MAX_PROCESSLABELS];
static struct ctk_label processnamelabels[MAX_PROCESSLABELS];

static struct ctk_label killlabel =
  {CTK_LABEL(0, PROCESSLIST_HEIGHT - 2, 12, 1, "Kill process")};
static char killprocnum[3];
static struct ctk_textentry killtextentry =
  {CTK_TEXTENTRY(13, PROCESSLIST_HEIGHT - 2, 2, 1, killprocnum, 2)};
static struct ctk_button killbutton =
  {CTK_BUTTON(19, PROCESSLIST_HEIGHT - 2, 2, "Ok")};
static struct ctk_button processupdatebutton =
  {CTK_BUTTON(0, PROCESSLIST_HEIGHT - 1, 6, "Update")};
static struct ctk_button processclosebutton =
  {CTK_BUTTON(19, PROCESSLIST_HEIGHT - 1, 5, "Close")};

PROCESS(processes_process, "Process listing");

AUTOSTART_PROCESSES(&processes_process);

enum {
  EVENT_UPDATE
};

/*-----------------------------------------------------------------------------------*/
static void
update_processwindow(void)
{
  unsigned char i;
  struct process *p;
  char *idptr;

  i = 0;
  for(p = PROCESS_LIST(); p != NULL && i < MAX_PROCESSLABELS; p = p->next) {
    processes[i].p = p;
    idptr = processes[i].id;
    idptr[0] = '0' + (i / 10) % 10;
    idptr[1] = '0' + i % 10;
    CTK_LABEL_NEW(&processidlabels[i],
		  1, i + 1, 2, 1, idptr);
    CTK_WIDGET_ADD(&processwindow, &processidlabels[i]);
    
    CTK_LABEL_NEW(&processnamelabels[i],
		  4, i + 1, 22, 1, PROCESS_NAME_STRING(p));
    CTK_WIDGET_ADD(&processwindow, &processnamelabels[i]);

    ++i;
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
  unsigned char procnum, valid, i;
  struct process *p;
  
  procnum = 0;
  valid = 0;
  for(i = 0; i < 2; ++i) {
    if(killprocnum[i] >= '0' && killprocnum[i] <= '9') {
      procnum = procnum * 10 + (killprocnum[i] - '0');
      valid = 1;
    }
  }

  if(valid == 0) {
    return;
  }

  /* Make sure the process ID exists. */
  for(p = PROCESS_LIST(); p != NULL; p = p->next) {
    if(p == processes[procnum].p) {
      break;
    }
  }

  if(p != NULL) {
    process_post(p, PROCESS_EVENT_EXIT, NULL);
    CTK_WIDGET_FOCUS(&processwindow, &processupdatebutton);
    CTK_WIDGET_REDRAW(&killbutton);
    CTK_WIDGET_REDRAW(&processupdatebutton);
  }

  CTK_TEXTENTRY_CLEAR(&killtextentry);
  CTK_WIDGET_REDRAW(&killtextentry);
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(processes_process, ev, data)     
{

  PROCESS_BEGIN();
  
  ctk_window_new(&processwindow, 26, PROCESSLIST_HEIGHT, "Processes");
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
