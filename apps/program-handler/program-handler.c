/**
 * \file
 * The program handler, used for loading programs and starting the
 * screensaver. 
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * The Contiki program handler is responsible for the Contiki menu and
 * the desktop icons, as well as for loading programs and displaying a
 * dialog with a message telling which program that is loading.
 *
 * The program handler also is responsible for starting the
 * screensaver when the CTK detects that it should be started.
 */
 
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
 * This file is part of the Contiki desktop OS
 *
 *
 */

#include <string.h>
#include <stdlib.h>

#include "contiki.h"
#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"

#include "program-handler.h"


/* Menus */
static struct ctk_menu contikimenu = {NULL, "Contiki", 7, 0, 0};

#ifndef PROGRAM_HANDLER_CONF_MAX_NUMDSCS
#define MAX_NUMDSCS 10
#else /* PROGRAM_HANDLER_CONF_MAX_NUMDSCS */
#define MAX_NUMDSCS PROGRAM_HANDLER_CONF_MAX_NUMDSCS
#endif /* PROGRAM_HANDLER_CONF_MAX_NUMDSCS */

static struct dsc *contikidsc[MAX_NUMDSCS];
static unsigned char contikidsclast = 0;

#ifndef PROGRAM_HANDLER_CONF_QUIT_MENU
#define QUIT_MENU 0
#else /* PROGRAM_HANDLER_CONF_QUIT_MENU */
#define QUIT_MENU PROGRAM_HANDLER_CONF_QUIT_MENU
#endif /* PROGRAM_HANDLER_CONF_QUIT_MENU */

#if QUIT_MENU

static unsigned char quitmenuitem;

/* "Quit" dialog */
static struct ctk_window quitdialog;
static struct ctk_label quitdialoglabel =
  {CTK_LABEL(2, 1, 20, 1, "Really quit Contiki?")};
static struct ctk_button quityesbutton =
  {CTK_BUTTON(4, 3, 3, "Yes")};
static struct ctk_button quitnobutton =
  {CTK_BUTTON(16, 3, 2, "No")};

#endif /* QUIT_MENU */

#if WITH_LOADER_ARCH

/* "Run..." window */
static struct ctk_window runwindow;
static unsigned char runmenuitem;
static struct ctk_label namelabel =
  {CTK_LABEL(0, 0, 13, 1, "Program name:")};
static char name[31];
static struct ctk_textentry nameentry =
  {CTK_TEXTENTRY(0, 1, 14, 1, name, 30)};
static struct ctk_button loadbutton =
  {CTK_BUTTON(10, 2, 4, "Load")};

static struct ctk_window loadingdialog;
static struct ctk_label loadingmsg =
  {CTK_LABEL(0, 0, 8, 1, "Starting")};
static struct ctk_label loadingname =
  {CTK_LABEL(9, 0, 16, 1, name)};

static struct ctk_window errordialog;
static struct ctk_label errormsg =
  {CTK_LABEL(0, 1, 22, 1, "Error loading program:")};
static char errorfilename[22];
static struct ctk_label errorfilelabel =
  {CTK_LABEL(0, 3, 22, 1, errorfilename)};
static struct ctk_label errortype =
  {CTK_LABEL(4, 5, 16, 1, "")};
static struct ctk_button errorokbutton =
  {CTK_BUTTON(9, 7, 2, "Ok")};

static const char * const errormsgs[] = {
  "Ok",
  "Read error",
  "Header error",
  "OS error",
  "Data format error",
  "Out of memory",
  "File not found",
  "No loader"
};

#endif /* WITH_LOADER_ARCH */

#define LOADER_EVENT_LOAD 1
#define LOADER_EVENT_DISPLAY_NAME 2

PROCESS(program_handler_process, "Program handler");

AUTOSTART_PROCESSES(&program_handler_process);

static char *displayname;

#if CTK_CONF_SCREENSAVER
char program_handler_screensaver[20];
#endif /* CTK_CONF_SCREENSAVER */

/*-----------------------------------------------------------------------------------*/
/**
 * Add a program to the program handler.
 *
 * \param dsc The DSC description structure for the program to be added.
 *
 * \param menuname The name that the program should have in the
 * Contiki menu.
 *
 * \param desktop Flag which specifies if the program should show up
 * as an icon on the desktop or not.
 */
/*-----------------------------------------------------------------------------------*/
void
program_handler_add(struct dsc *dsc, char *menuname,
		    unsigned char desktop)
{
  contikidsc[contikidsclast++] = dsc;
  ctk_menuitem_add(&contikimenu, menuname);
  if(desktop) {
#if CTK_CONF_ICONS
    CTK_ICON_ADD(dsc->icon, &program_handler_process);
#endif /* CTK_CONF_ICONS */
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initializes the program handler.
 *
 * Is called by the initialization before any programs have been added
 * with program_handler_add().
 *
 */
/*-----------------------------------------------------------------------------------*/
#ifdef WITH_LOADER_ARCH
#define NUM_PNARGS 6
#define NAMELEN 32
struct pnarg {
  char name[NAMELEN];
  char *arg;
};
static struct pnarg pnargs[NUM_PNARGS];
static struct pnarg *
pnarg_copy(char *name, char *arg)
{
  char i;
  struct pnarg *pnargsptr;

  pnargsptr = pnargs;
  /* Allocate a place in the loadernames table. */
  for(i = 0; i < NUM_PNARGS; ++i) {
    if(*(pnargsptr->name) == 0) {
      strncpy(pnargsptr->name, name, NAMELEN);
      pnargsptr->arg = arg;
      return pnargsptr;
    }
    ++pnargsptr;
  }
  return NULL;
}

static void
pnarg_free(struct pnarg *pn)
{
  *(pn->name) = 0;
}
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
/**
 * Loads a program and displays a dialog telling the user about it.
 *
 * \param name The name of the program to be loaded.
 *
 * \param arg An argument which is passed to the new process when it
 * is loaded.
 */
/*-----------------------------------------------------------------------------------*/
void
program_handler_load(char *name, char *arg)
{
#ifdef WITH_LOADER_ARCH
  struct pnarg *pnarg;
  
  pnarg = pnarg_copy(name, arg);
  if(pnarg != NULL) {
    process_post(&program_handler_process, LOADER_EVENT_DISPLAY_NAME, pnarg);
  } else {
    ctk_label_set_text(&errortype, "Out of memory");
    ctk_dialog_open(&errordialog);
  }
  /*  ctk_redraw(); */
  /*  ctk_window_redraw(&loadingdialog);*/
#endif /* WITH_LOADER_ARCH */
}

#ifdef WITH_LOADER_ARCH
#define RUN(prg, name, arg) program_handler_load(prg, arg)
#else /* WITH_LOADER_ARCH */
#define RUN(prg, process, arg) process_start(process, arg)
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
/**
 * Configures the name of the screensaver to be loaded when
 * appropriate.
 *
 * \param name The name of the screensaver or NULL if no screensaver
 * should be used.
 */
/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_SCREENSAVER
void
program_handler_setscreensaver(char *name)
{
  if(name == NULL) {
    program_handler_screensaver[0] = 0;
  } else {
    strncpy(program_handler_screensaver, name, sizeof(program_handler_screensaver));
  }
}
#endif /* CTK_CONF_SCREENSAVER */
/*-----------------------------------------------------------------------------------*/
#ifdef WITH_LOADER_ARCH
static void
make_windows(void)
{
  ctk_window_new(&runwindow, 16, 3, "Run");
  
  CTK_WIDGET_ADD(&runwindow, &namelabel);
  CTK_WIDGET_ADD(&runwindow, &nameentry);
  CTK_WIDGET_ADD(&runwindow, &loadbutton);
  
  CTK_WIDGET_FOCUS(&runwindow, &nameentry);
  
  ctk_dialog_new(&loadingdialog, 25, 1);
  CTK_WIDGET_ADD(&loadingdialog, &loadingmsg);
  CTK_WIDGET_ADD(&loadingdialog, &loadingname);
  
  ctk_dialog_new(&errordialog, 22, 8);
  CTK_WIDGET_ADD(&errordialog, &errormsg);
  CTK_WIDGET_ADD(&errordialog, &errorfilelabel);
  CTK_WIDGET_ADD(&errordialog, &errortype);
  CTK_WIDGET_ADD(&errordialog, &errorokbutton);
  CTK_WIDGET_FOCUS(&errordialog, &errorokbutton);
}
#endif /* WITH_LOADER_ARCH */
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(program_handler_process, ev, data)
{
#ifdef WITH_LOADER_ARCH
  unsigned char err;
  struct dsc *dsc;
#endif /* WITH_LOADER_ARCH */
  unsigned char i;
  struct dsc **dscp;

  PROCESS_BEGIN();
  
  /* Create the menus */
  ctk_menu_add(&contikimenu);
#if WITH_LOADER_ARCH
  runmenuitem = ctk_menuitem_add(&contikimenu, "Run program...");
  
  make_windows();
#endif /* WITH_LOADER_ARCH */
#if QUIT_MENU
  quitmenuitem = ctk_menuitem_add(&contikimenu, "Quit");
#endif /* QUIT_MENU */
  
  displayname = NULL;
  
#if CTK_CONF_SCREENSAVER
  program_handler_screensaver[0] = 0;
#endif /* CTK_CONF_SCREENSAVER */

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == ctk_signal_button_activate) {
#ifdef WITH_LOADER_ARCH
      if(data == (process_data_t)&loadbutton) {
	ctk_window_close(&runwindow);
	program_handler_load(name, NULL);
      } else if(data == (process_data_t)&errorokbutton) {
	ctk_dialog_close();
      }
#endif /* WITH_LOADER_ARCH */
#if QUIT_MENU
      if(data == (process_data_t)&quityesbutton) {
	ctk_draw_init();
	exit(EXIT_SUCCESS);
      } else if(data == (process_data_t)&quitnobutton) {
	ctk_dialog_close();
      }
#endif /* QUIT_MENU */
      dscp = &contikidsc[0];
      for(i = 0; i < CTK_MAXMENUITEMS; ++i) {    
	if(*dscp != NULL
#if CTK_CONF_ICONS
	  && data == (process_data_t)(*dscp)->icon
#endif /* CTK_CONF_ICONS */
	  ) {
	  RUN((*dscp)->prgname, (*dscp)->process, NULL);
	  break;
	}
	++dscp;
      }
    } else if(ev == ctk_signal_menu_activate) {
      if((struct ctk_menu *)data == &contikimenu) {
#if WITH_LOADER_ARCH
	dsc = contikidsc[contikimenu.active];
	if(dsc != NULL) {
	  RUN(dsc->prgname, dsc->process, NULL);
	} else if(contikimenu.active == runmenuitem) {
	  make_windows();
	  ctk_window_close(&runwindow);
	  ctk_window_open(&runwindow);
	  CTK_WIDGET_FOCUS(&runwindow, &nameentry);
	}
#else /* WITH_LOADER_ARCH */
	if(contikidsc[contikimenu.active] != NULL) {
	  RUN(contikidsc[contikimenu.active]->prgname,
	      contikidsc[contikimenu.active]->process,
	      NULL);
	}
#endif /* WITH_LOADER_ARCH */
#if QUIT_MENU
	if(contikimenu.active == quitmenuitem) {
	  ctk_dialog_new(&quitdialog, 24, 5);
	  CTK_WIDGET_ADD(&quitdialog, &quitdialoglabel);
	  CTK_WIDGET_ADD(&quitdialog, &quityesbutton);
	  CTK_WIDGET_ADD(&quitdialog, &quitnobutton);
	  CTK_WIDGET_FOCUS(&quitdialog, &quitnobutton);
	  ctk_dialog_open(&quitdialog);      
	}
#endif /* QUIT_MENU */
      }
#if CTK_CONF_SCREENSAVER
    } else if(ev == ctk_signal_screensaver_start) {
#if WITH_LOADER_ARCH
      if(program_handler_screensaver[0] != 0) {
	program_handler_load(program_handler_screensaver, NULL);
      }
#endif /* WITH_LOADER_ARCH */
#endif /* CTK_CONF_SCREENSAVER */
    } else if(ev == LOADER_EVENT_DISPLAY_NAME) {
#if WITH_LOADER_ARCH
      if(displayname == NULL) {
	make_windows();
	
	ctk_label_set_text(&loadingname, ((struct pnarg *)data)->name);
	ctk_dialog_open(&loadingdialog);
	process_post(&program_handler_process, LOADER_EVENT_LOAD, data);
	displayname = data;
      } else {
	/* Try again. */
	process_post(&program_handler_process, LOADER_EVENT_DISPLAY_NAME, data);
      }
#endif /* WITH_LOADER_ARCH */
    } else if(ev == LOADER_EVENT_LOAD) {
#if WITH_LOADER_ARCH
      if(displayname == data) {
	ctk_dialog_close();
	displayname = NULL;
	log_message("Loading ", ((struct pnarg *)data)->name);
	err = LOADER_LOAD(((struct pnarg *)data)->name,
			  ((struct pnarg *)data)->arg);
	if(err != LOADER_OK) {
	  make_windows();
	  errorfilename[0] = '"';
	  strncpy(errorfilename + 1, ((struct pnarg *)data)->name,
		  sizeof(errorfilename) - 2);
	  errorfilename[1 + strlen(((struct pnarg *)data)->name)] = '"';
	  ctk_label_set_text(&errortype, (char *)errormsgs[err]);
	  ctk_dialog_open(&errordialog);
	  log_message((char *)errormsgs[err], errorfilename);
	}
	pnarg_free(data);
      } else {
	/* Try again. */
	process_post(&program_handler_process, LOADER_EVENT_DISPLAY_NAME, data);
      }
#endif /* WITH_LOADEER_ARCH */
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
