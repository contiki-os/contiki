/*
 * Copyright (c) 2003, Adam Dunkels.
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

#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "cfs/cfs.h"
#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"

#include "program-handler.h"

#define FILENAMELEN 24
#define MAX_NUMFILES 40
#define WIDTH 36
#define HEIGHT 22

static char (filenames[FILENAMELEN + 1])[MAX_NUMFILES];
static struct dsc *dscs[MAX_NUMFILES];
static unsigned char numfiles, morestart, filenameptr;

static struct ctk_window window;

static struct ctk_label description =
  {CTK_LABEL(0, HEIGHT - 1, WIDTH, 1, "")};

static char autoexit = 1;
static struct ctk_button autoexitbutton =
  {CTK_BUTTON(WIDTH/2 - 9, 20, 9, "Auto-exit")};
static char autoexiton[] = "is On ";
static char autoexitoff[] = "is Off";
static struct ctk_label autoexitlabel =
  {CTK_LABEL(WIDTH/2 - 9 + 12, 20, 6, 1, autoexiton)};

static struct ctk_button morebutton =
  {CTK_BUTTON(0, 20, 4, "More")};

static struct ctk_button backbutton =
  {CTK_BUTTON(0, 20, 4, "Back")};

static struct ctk_button reloadbutton =
  {CTK_BUTTON(30, 20, 6, "Reload")};

PROCESS(directory_process, "Directory browser");

AUTOSTART_PROCESSES(&directory_process);

static unsigned char width, height;

#define LOADING_DIR 1
#define LOADING_DSC 2
static char loading = 0;
static struct cfs_dir dir;
/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&description, text);
  CTK_WIDGET_REDRAW(&description);
}
/*-----------------------------------------------------------------------------------*/
static void
startloading(void)
{
  if(cfs_opendir(&dir, "/") != 0) {
    show_statustext("Cannot open directory");
    loading = 0;
  } else {
    loading = LOADING_DIR;
    process_post(&directory_process, PROCESS_EVENT_CONTINUE, NULL);
    numfiles = 0;
  }
}    
/*-----------------------------------------------------------------------------------*/
static void
makewindow(unsigned char i)
{
  unsigned char x, y;

  ctk_window_clear(&window);
  CTK_WIDGET_SET_YPOS(&description, height - 3);
  CTK_WIDGET_SET_WIDTH(&description, width);
  CTK_WIDGET_ADD(&window, &description);

  morestart = i;
  
  x = 0; y = 1;
  for(; dscs[i] != NULL; ++i) {

    if(x + strlen(dscs[i]->icon->title) >= width) {
      y += 5;
      x = 0;
      if(y >= height - 2 - 4) {
	morestart = i;
	break;
      }
    }
    CTK_WIDGET_SET_XPOS(dscs[i]->icon, x);
    CTK_WIDGET_SET_YPOS(dscs[i]->icon, y);
    CTK_WIDGET_ADD(&window, dscs[i]->icon);

    x += (unsigned char)strlen(dscs[i]->icon->title) + 2;
  }
  CTK_WIDGET_SET_YPOS(&autoexitbutton, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitbutton);
  CTK_WIDGET_SET_YPOS(&autoexitlabel, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitlabel);
  CTK_WIDGET_FOCUS(&window, &autoexitbutton);

  if(i != morestart) {
    CTK_WIDGET_SET_YPOS(&backbutton, height - 1);
    CTK_WIDGET_ADD(&window, &backbutton);
  } else {
    CTK_WIDGET_SET_YPOS(&morebutton, height - 1);
    CTK_WIDGET_ADD(&window, &morebutton);
  }
  CTK_WIDGET_SET_XPOS(&reloadbutton, width - 8);
  CTK_WIDGET_SET_YPOS(&reloadbutton, height - 1);
  CTK_WIDGET_ADD(&window, &reloadbutton);    
}
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  unsigned char i;

  if(loading == LOADING_DIR) {
    cfs_closedir(&dir);
  }
  ctk_window_close(&window);
  for(i = 0; dscs[i] != NULL; ++i) {
    LOADER_UNLOAD_DSC(dscs[i]);
  }
  process_exit(&directory_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
read_dirent(void)
{
  static struct cfs_dirent dirent;
  static char message[40];
    
  if(loading == LOADING_DIR) {
    if(cfs_readdir(&dir, &dirent)) {
      cfs_closedir(&dir);
      loading = LOADING_DSC;
      filenameptr = 0;
    } else if(strcasecmp(&dirent.name[strlen(dirent.name) - 4], ".dsc") == 0) {
      strncpy(filenames[numfiles], dirent.name, FILENAMELEN);
      ++numfiles;
      if(numfiles == MAX_NUMFILES) {
	cfs_closedir(&dir);	
	loading = LOADING_DSC;
	filenameptr = 0;
	return;
      }      
      strcpy(message, "Found \"");
      strcpy(message + 7, dirent.name);
      strcpy(message + 7 + strlen(dirent.name), "\"...");
      show_statustext(message);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
load_dirent(void)
{
  static char message[40];
  char *name;
    
  if(loading == LOADING_DSC) {

    name = filenames[filenameptr];
    dscs[filenameptr] = LOADER_LOAD_DSC(name);
    if(dscs[filenameptr] == NULL || filenameptr + 1 >= numfiles) {
      loading = 0;
      makewindow(0);
      show_statustext("Directory loaded");
      ctk_window_redraw(&window);
      return;
    }
    ++filenameptr;
    strcpy(message, "Loading \"");
    strcpy(message + 9, name);
    strcpy(message + 9 + strlen(name), "\"...");
    show_statustext(message);
  }
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(directory_process, ev, data)
{
  unsigned char i;

  PROCESS_BEGIN();
  
  width = ctk_draw_width() - 2;
  height = ctk_draw_height() - 2 - CTK_CONF_MENUS;
  
  ctk_window_new(&window, width, height, "Directory");
  
  /*    loaddirectory();*/
  makewindow(0);
  show_statustext("Loading directory...");
  startloading();
  
  ctk_window_open(&window);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_CONTINUE) {
      read_dirent();
      load_dirent();
      if(loading != 0) {
	process_post(&directory_process, PROCESS_EVENT_CONTINUE, NULL);
      }
    } else if(ev == ctk_signal_widget_activate) {
      if(data == (process_data_t)&reloadbutton) {
	for(i = 0; dscs[i] != NULL; ++i) {
	  LOADER_UNLOAD_DSC(dscs[i]);
	  dscs[i] = NULL;
	}     
	/*      loaddirectory();*/
	startloading();
	makewindow(0);
	ctk_window_open(&window);
      } else if(data == (process_data_t)&morebutton) {
	makewindow(morestart);
	ctk_window_open(&window);
      } else if(data == (process_data_t)&backbutton) {
	makewindow(0);
	ctk_window_open(&window);
      } else if(data == (process_data_t)&autoexitbutton) {
	autoexit = 1 - autoexit;
	if(autoexit == 1) {
	  ctk_label_set_text(&autoexitlabel, autoexiton);
	} else {
	  ctk_label_set_text(&autoexitlabel, autoexitoff);
	}
	CTK_WIDGET_REDRAW(&autoexitlabel);
      } else {
	for(i = 0; dscs[i] != NULL; ++i) {
	  if(data == (process_data_t)(dscs[i]->icon)) {
	    program_handler_load(dscs[i]->prgname, NULL);
	    if(autoexit) {
	      ctk_window_close(&window);
	      quit();
	    }
	    break;
	  }
	}
      }
    } else if(ev == ctk_signal_widget_select) {
      if(data == (process_data_t)&reloadbutton) {
	show_statustext("Reload directory");
      } else if(data == (process_data_t)&morebutton) {
	show_statustext("Show more files");
      } else if(data == (process_data_t)&backbutton) {
	show_statustext("Show first files");
      } else if(data == (process_data_t)&autoexitbutton) {
	show_statustext("Exit when loading program");
      } else {
	for(i = 0; dscs[i] != NULL; ++i) {
	  if(data == (process_data_t)(dscs[i]->icon)) {
	    show_statustext(dscs[i]->description);
	    break;
	  }
	}
      }
    } else if(ev == ctk_signal_window_close &&
	      data == (process_data_t)&window) {
      quit();
    } else if(ev == PROCESS_EVENT_EXIT) {
      ctk_window_close(&window);
      quit();
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
