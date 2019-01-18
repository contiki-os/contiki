/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: editor.c,v 1.1 2007/05/23 23:34:33 oliverschmidt Exp $
 */
#define EDITOR_CONF_WIDTH 32
#define EDITOR_CONF_HEIGHT 16
#define EDITOR_CONF_MAX_FILENAME_LEN 16

#include "contiki.h"
#include "contiki-lib.h"
#include "ctk/ctk.h"
#include "cfs/cfs.h"

#include "lib/ctk-filedialog.h"

#define ISO_nl 0x0a

PROCESS(editor_process, "Editor");

static struct ctk_window window;

#define LINE_LEN 60
#define NUM_LINES EDITOR_CONF_HEIGHT

struct line {
  struct line *next, *prev;
  char text[LINE_LEN];
};
MEMB(linesmem, struct line, NUM_LINES);

static struct line *lines;

static struct {
  unsigned char x, y;
  struct ctk_label labels[NUM_LINES];

} editor_state;

static struct ctk_button openbutton =
  {CTK_BUTTON(0, 0, 4, "Open")};
static char statustext[EDITOR_CONF_WIDTH + 1];
static struct ctk_label statuslabel =
  {CTK_LABEL(0, EDITOR_CONF_HEIGHT + 2, EDITOR_CONF_WIDTH, 1, statustext)};

static struct ctk_menu menu;
static unsigned char menuitem_new, menuitem_open, menuitem_save;

static char filename[EDITOR_CONF_MAX_FILENAME_LEN];


static struct ctk_filedialog_state filedialog;

enum {
  OPEN_EVENT
};

/*---------------------------------------------------------------------------*/
static void
show_statustext(char *text1, char *text2)
{
  int len;

  len = strlen(text1);
  if(len < sizeof(statustext)) {
    strncpy(statustext, text1, sizeof(statustext));
    strncpy(statustext + len, text2, sizeof(statustext) - len);
    CTK_WIDGET_REDRAW(&statuslabel);
  }
  
}
/*---------------------------------------------------------------------------*/
static void
editor_start(void)
{
  unsigned char i;
  register struct ctk_label *label;
  struct line *l, *m;

  m = NULL;
  
  for(i = 0; i < NUM_LINES; ++i) {
    label = &editor_state.labels[i];
    l = (struct line *)memb_alloc(&linesmem);
    
    if(l != NULL) {
      l->next = NULL;
      l->prev = m;
      if(m == NULL) {
	/* First line */
	lines = l;
      } else {
	m->next = l;
      }
      CTK_LABEL_NEW(label, 0, i + 1, EDITOR_CONF_WIDTH, 1, l->text);
      CTK_WIDGET_SET_FLAG(label, CTK_WIDGET_FLAG_MONOSPACE);
      CTK_WIDGET_ADD(&window, label);
    }
    m = l;
  }
}
/*---------------------------------------------------------------------------*/
static void
editor_eventhandler(process_event_t ev, process_data_t data)
{
  char *textptr, *textptr2;
  unsigned char len;
  
  if(ev == ctk_signal_keypress) {
    /*    CTK_WIDGET_FOCUS(t->label.window, &t->label);*/
    textptr = &(editor_state.labels[editor_state.y].text[editor_state.x]);
    *textptr &= 0x7f;
    CTK_WIDGET_REDRAW(&(editor_state.labels[editor_state.y]));
    
    switch((ctk_arch_key_t)data) {
    case CH_CURS_DOWN:
      ++editor_state.y;
      if(editor_state.y >= EDITOR_CONF_HEIGHT) {
	editor_state.y = EDITOR_CONF_HEIGHT - 1;
      }
      break;
    case CH_CURS_UP:
      if(editor_state.y > 0) {
	--editor_state.y;
      }
      break;
    case CH_CURS_RIGHT:
      if(editor_state.x < strlen(editor_state.labels[editor_state.y].text)) {
	++editor_state.x;
      }
      break;
    case CH_CURS_LEFT:
      if(editor_state.x > 0) {
	--editor_state.x;
      } else {
	if(editor_state.y > 0) {
	  --editor_state.y;
	  editor_state.x = strlen(editor_state.labels[editor_state.y].text);
	}
      }
      break;
    case CH_ENTER:
      editor_state.x = 0;
      ++editor_state.y;
      if(editor_state.y >= EDITOR_CONF_HEIGHT) {
	editor_state.y = EDITOR_CONF_HEIGHT - 1;
      }
      break;
    case CH_DEL:
      /*      len = t->label.w - t->xpos;
      if(t->xpos > 0 && len > 0) {
	strncpy(textptr - 1, textptr,
		len);
	*(textptr + len - 1) = 0;
	--t->xpos;
	}*/
      break;
    default:
      len = EDITOR_CONF_WIDTH - editor_state.x;
      if(len > 0) {
	textptr2 = textptr + len - 1;
	while(textptr2 + 1 > textptr) {
	  *(textptr2 + 1) = *textptr2;
	  --textptr2;
	}
	
	*textptr = (char)data;
	++editor_state.x;
	if(editor_state.x == EDITOR_CONF_WIDTH) {
	  editor_state.x = 0;
	  if(editor_state.y < EDITOR_CONF_HEIGHT - 1) {
	    ++editor_state.y;
	  }
	}
      }
      break;
    }
    textptr = &(editor_state.labels[editor_state.y].text[editor_state.x]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&(editor_state.labels[editor_state.y]));
    /*  } else if(s == ctk_signal_widget_activate &&
	    data == (process_data_t)t) {
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr &= 0x7f;
    t->xpos = 0;
    if(t->ypos < t->label.h - 1) {
      ++t->ypos;
    }
    textptr = &(t->label.text[t->ypos * t->label.w + t->xpos]);
    *textptr |= 0x80;
    CTK_WIDGET_REDRAW(&t->label);*/
  

  }
}
/*---------------------------------------------------------------------------*/
static void
open_file(char *name)
{
  int fd;
  struct line *l;
  char line[LINE_LEN];
  char *cptr;
  int i, len, clen;
  
  fd = cfs_open(name, 0);
  if(fd < 0) {
    show_statustext("Could not open file ", name);
    return;
  }

  l = lines;
  cptr = l->text;
  clen = LINE_LEN;
  
  do {
    /* Read a portion of the input file */
    len = cfs_read(fd, line, LINE_LEN);
    
    /* Split the input into lines. */
    for(i = 0; i < len; ++i) {
      if(line[i] == ISO_nl ||
	 clen == 0) {
	*cptr = 0;
	l = l->next;
	if(l != NULL) {
	  cptr = l->text;
	  clen = LINE_LEN;
	} else {
	  len = -1;
	  break;
	}
      } else {
	*cptr++ = line[i];
	--clen;
      }
    }
  } while(len > 0);


  cfs_close(fd);
}
/*---------------------------------------------------------------------------*/
static void
quit(void)
{
  ctk_window_close(&window);
  process_exit(&editor_process);
  LOADER_UNLOAD();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(editor_process, ev, data)
{

  PROCESS_BEGIN();
  
  memb_init(&linesmem);
  
  ctk_window_new(&window,
		 EDITOR_CONF_WIDTH + 2,
		 EDITOR_CONF_HEIGHT + 3,
		 "Editor");
  
  CTK_WIDGET_ADD(&window, &openbutton);
  CTK_WIDGET_ADD(&window, &statuslabel);
  
  CTK_WIDGET_FOCUS(&window, &openbutton);
  
  editor_start();
  
  ctk_window_open(&window);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ctk_filedialog_eventhandler(&filedialog, ev, data)) {
      
    } else {
      if(ev == PROCESS_EVENT_EXIT) {
	quit();
      } else if(ev == OPEN_EVENT) {
	/*    printf("Open file '%s'\n", (char *)data);*/
	open_file((char *)data);
	ctk_window_redraw(&window);
      } else {
	
	if(ev == ctk_signal_window_close &&
	   data == (process_data_t)&window) {
	  quit();
	} else if(ev == ctk_signal_widget_activate) {
	  if(data == (process_data_t)&openbutton) {
	    ctk_filedialog_open(&filedialog, "Open", OPEN_EVENT);
	  }
	} else {
	  editor_eventhandler(ev, data);
	}
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*LOADER_INIT_FUNC(editor_init, arg)
{
  arg_free(arg);
  id = ek_start(&p);
}*/
/*---------------------------------------------------------------------------*/
