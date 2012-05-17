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
 * This file is part of the Contiki VNC client
 *
 *
 */

#include <string.h>

#include "contiki.h"

/*#include "petsciiconv.h"*/
#include "net/uiplib.h"
#include "net/uip.h"
#include "ctk/ctk.h"
#include "ctk/ctk-mouse.h"
#include "net/resolv.h"
/*#include "telnet.h"*/
#include "vnc.h"
#include "vnc-draw.h"
#include "vnc-viewer.h"
#include "contiki-conf.h"

#include "sys/loader.h"

#if 1
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif


#ifdef VNC_CONF_VIEWPORT_WIDTH
#define VNC_VIEWPORT_WIDTH VNC_CONF_VIEWPORT_WIDTH
#else
#define VNC_VIEWPORT_WIDTH 10
#endif

#ifdef VNC_CONF_VIEWPORT_HEIGHT
#define VNC_VIEWPORT_HEIGHT VNC_CONF_VIEWPORT_HEIGHT
#else
#define VNC_VIEWPORT_HEIGHT 10
#endif

#define HEIGHT (4 + VNC_VIEWPORT_HEIGHT/8)

/* Main window */
static struct ctk_window mainwindow;

static char host[20];
static struct ctk_textentry hosttextentry =
  {CTK_TEXTENTRY(0, 0, 18, 1, host, 18)};

static char portentry[4];
static struct ctk_textentry porttextentry =
  {CTK_TEXTENTRY(21, 0, 3, 1, portentry, 3)};

static struct ctk_button connectbutton =
  {CTK_BUTTON(27, 0, 7, "Connect")};
/*static struct ctk_button disconnectbutton =
  {CTK_BUTTON(25, 3, 10, "Disconnect")};*/

static struct ctk_separator sep1 =
  {CTK_SEPARATOR(0, 1, 36)};

static struct ctk_bitmap vncbitmap =
  {CTK_BITMAP(2, 2,
	      VNC_VIEWPORT_WIDTH / 8,
	      VNC_VIEWPORT_HEIGHT / 8,
	      vnc_draw_bitmap,
	      VNC_VIEWPORT_WIDTH,
	      VNC_VIEWPORT_HEIGHT)};

static struct ctk_button leftbutton =
  {CTK_BUTTON(6, HEIGHT - 1, 4, "Left")};

static struct ctk_button upbutton =
  {CTK_BUTTON(13, HEIGHT - 1, 2, "Up")};

static struct ctk_button downbutton =
  {CTK_BUTTON(18, HEIGHT - 1, 4, "Down")};

static struct ctk_button rightbutton =
  {CTK_BUTTON(25, HEIGHT - 1, 5, "Right")};

PROCESS(vnc_process, "VNC viewer");

/*-----------------------------------------------------------------------------------*/
static void
show(char *text)
{

}
/*-----------------------------------------------------------------------------------*/
static void
connect(void)
{
  uip_ipaddr_t addr, *addrptr;
  uint16_t port;
  char *cptr;

  /* Find the first space character in host and put a zero there
     to end the string. */
  for(cptr = host; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;

  addrptr = &addr;
  if(uiplib_ipaddrconv(host, &addr) == 0) {
    if(resolv_lookup(host, &addrptr) == RESOLV_STATUS_UNCACHED) {
      resolv_query(host);
      show("Resolving host...");
      return;
    }
  }

  port = 0;
  for(cptr = portentry; *cptr != ' ' && *cptr != 0; ++cptr) {
    if(*cptr < '0' || *cptr > '9') {
      show("Port number error");
      return;
    }
    port = 10 * port + *cptr - '0';
  }


  vnc_viewer_connect(addrptr, port);

  show("Connecting...");

}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(vnc_process, ev, data)
{
  unsigned short x, y;
  unsigned char xc, yc;

  PROCESS_BEGIN();
  
  ctk_window_new(&mainwindow, 36, HEIGHT, "VNC client");
  ctk_window_move(&mainwindow, 0, 0);
  
  CTK_WIDGET_ADD(&mainwindow, &hosttextentry);
  CTK_WIDGET_FOCUS(&mainwindow, &hosttextentry);
  CTK_WIDGET_ADD(&mainwindow, &porttextentry);
  CTK_WIDGET_ADD(&mainwindow, &connectbutton);
  
  CTK_WIDGET_ADD(&mainwindow, &sep1);
  
  CTK_WIDGET_ADD(&mainwindow, &vncbitmap);
  
  CTK_WIDGET_ADD(&mainwindow, &leftbutton);
  CTK_WIDGET_ADD(&mainwindow, &upbutton);
  CTK_WIDGET_ADD(&mainwindow, &downbutton);
  CTK_WIDGET_ADD(&mainwindow, &rightbutton);
  
  vnc_draw_init();
  
  ctk_window_open(&mainwindow);

  while(1) {
    PROCESS_WAIT_EVENT();
  
    if(ev == ctk_signal_button_activate) {
      if(data == (process_data_t)&connectbutton) {
	connect();
      }
    } else if(ev == ctk_signal_window_close) {
      process_exit(&vnc_process);
      LOADER_UNLOAD();
    } else if(ev == resolv_event_found) {
      if(strcmp(data, host) == 0) {
	if(resolv_lookup(host, NULL) == RESOLV_STATUS_CACHED) {
	  connect();
	} else {
	  show("Host not found");
	}
      }
    } else if(ev == ctk_signal_pointer_move) {
      /* Check if pointer is within the VNC viewer area */
      x = ctk_mouse_x();
      y = ctk_mouse_y();
      
      xc = ctk_mouse_xtoc(x);
      yc = ctk_mouse_ytoc(y);
      
      if(xc >= 2 && yc >= 2 &&
	 xc < 2 + VNC_VIEWPORT_WIDTH / 8 &&
	 yc < 2 + VNC_VIEWPORT_HEIGHT / 8) {
	
	VNC_VIEWER_POST_POINTER_EVENT(x, y, 0);
      }
      
    } else if(ev == tcpip_event) {
      vnc_viewer_appcall(data);
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_refresh(void)
{
  CTK_WIDGET_REDRAW(&vncbitmap);
}
/*-----------------------------------------------------------------------------------*/
