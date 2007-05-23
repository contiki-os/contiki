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
 * $Id: wget.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */


#include "ctk/ctk.h"
#include "contiki-net.h"
#include "webclient.h"
#include "lib/petsciiconv.h"

#include "program-handler.h"

#include <c64.h>
#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "c64-dio.h"

#define USE_KERNAL 0

static struct ctk_window window;

static struct ctk_label urllabel =
  {CTK_LABEL(0, 1, 4, 1, "URL:")};
static char url[80];
static char urledit[80];
struct ctk_textentry urltextentry =
  {CTK_TEXTENTRY(5, 1, 29, 1, urledit, 78)};


static struct ctk_label savefilenamelabel =
  {CTK_LABEL(0, 3, 14, 1, "Save filename:")};
static char savefilename[40];
static struct ctk_textentry savefilenametextentry =
  {CTK_TEXTENTRY(15, 3, 19, 1, savefilename, 38)};

static struct ctk_button filebutton =
  {CTK_BUTTON(0, 5, 13, "Download file")};

static struct ctk_button d64button =
  {CTK_BUTTON(17, 5, 18, "Download D64 disk")};

static struct ctk_label statustext =
  {CTK_LABEL(0, 7, 36, 1, "")};
static char statusmsg[40];

static struct ctk_window d64dialog;
static struct ctk_label overwritelabel =
  {CTK_LABEL(0, 1, 36, 1, "This will overwrite the entire disk!")};
static struct ctk_label makesurelabel1 =
  {CTK_LABEL(7, 3, 22, 1, "Make sure you have the")};
static struct ctk_label makesurelabel2 =
  {CTK_LABEL(6, 4, 24, 1, "right disk in the drive!")};
static struct ctk_button overwritebutton =
  {CTK_BUTTON(2, 6, 14, "Overwrite disk")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(26, 6, 6, "Cancel")};

PROCESS(wget_process, "Web downloader");
/* State */

#define DLOAD_NONE 0
#define DLOAD_FILE 1
#define DLOAD_D64  2
static u8_t dload_state;
static unsigned long dload_bytes;



struct drv_state {
  u8_t track;
  u8_t sect;
};

static struct drv_state ds;

static char buffer[256];
static u16_t bufferptr;

/*-----------------------------------------------------------------------------------*/
static void
show_statustext(char *text)
{
  ctk_label_set_text(&statustext, text);
  CTK_WIDGET_REDRAW(&statustext);
}
/*-----------------------------------------------------------------------------------*/
/* open_url():
 *
 * Called when the URL present in the global "url" variable should be
 * opened. It will call the hostname resolver as well as the HTTP
 * client requester.
 */
static void
start_get(void)
{
  u16_t addr[2];
  unsigned char i;
  static char host[32];
  char *file;
  register char *urlptr;
  unsigned short port;

  /* Trim off any spaces in the end of the url. */
  urlptr = url + strlen(url) - 1;
  while(*urlptr == ' ' && urlptr > url) {
    *urlptr = 0;
    --urlptr;
  }

  /* Don't even try to go further if the URL is empty. */
  if(urlptr == url) {
    return;
  }

  /* See if the URL starts with http://, otherwise prepend it. */
  if(strncmp(url, http_http, 7) != 0) {
    while(urlptr >= url) {
      *(urlptr + 7) = *urlptr;
      --urlptr;
    }
    strncpy(url, http_http, 7);
  } 

  /* Find host part of the URL. */
  urlptr = &url[7];  
  for(i = 0; i < sizeof(host); ++i) {
    if(*urlptr == 0 ||
       *urlptr == '/' ||
       *urlptr == ' ' ||
       *urlptr == ':') {
      host[i] = 0;
      break;
    }
    host[i] = *urlptr;
    ++urlptr;
  }

  /* XXX: Here we should find the port part of the URL, but this isn't
     currently done because of laziness from the programmer's side
     :-) */
  
  /* Find file part of the URL. */
  while(*urlptr != '/' && *urlptr != 0) {
    ++urlptr;
  }
  if(*urlptr == '/') {
    file = urlptr;
  } else {
    file = "/";
  }
      
  
  /* First check if the host is an IP address. */
  if(uiplib_ipaddrconv(host, (unsigned char *)addr) == 0) {    
    
    /* Try to lookup the hostname. If it fails, we initiate a hostname
       lookup and print out an informative message on the
       statusbar. */
    if(resolv_lookup(host) == NULL) {
      resolv_query(host);
      show_statustext("Resolving host...");
      return;
    }
  }

  /* The hostname we present in the hostname table, so we send out the
     initial GET request. */
  if(webclient_get(host, 80, file) == 0) {
    show_statustext("Out of memory error.");
  } else {
    show_statustext("Connecting...");
  }
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(wget_process, ev, data)
{
  int ret;
  static unsigned char i;
  
  
  
  
  PROCESS_BEGIN();

  if(data != NULL) {
    strncpy(url, data, sizeof(url));
    strncpy(urledit, data, sizeof(urledit));
    petsciiconv_topetscii(urledit, sizeof(urledit));
    arg_free(data);
  } else {
#ifdef WGET_CONF_URL
    strncpy(url, WGET_CONF_URL, sizeof(url));
    strncpy(urledit, WGET_CONF_URL, sizeof(urledit));
#endif /* WGET_CONF_URL  */
  }
  
  
  /* Create the main window. */
  ctk_window_new(&window, 36, 8, "Web downloader");
  
  
  CTK_WIDGET_ADD(&window, &urllabel);
  CTK_WIDGET_ADD(&window, &urltextentry);
  
  CTK_WIDGET_ADD(&window, &savefilenamelabel);
  CTK_WIDGET_ADD(&window, &savefilenametextentry);
  
  /*    CTK_WIDGET_ADD(&window, &filebutton);*/
  
  CTK_WIDGET_ADD(&window, &d64button);
  
  CTK_WIDGET_ADD(&window, &statustext);
  
  dload_state = DLOAD_NONE;
  
  memset(savefilename, 0, sizeof(savefilename));
  memset(url, 0, sizeof(url));
  
  ctk_dialog_new(&d64dialog, 36, 8);
  CTK_WIDGET_ADD(&d64dialog, &overwritelabel);
  CTK_WIDGET_ADD(&d64dialog, &makesurelabel1);
  CTK_WIDGET_ADD(&d64dialog, &makesurelabel2);
  CTK_WIDGET_ADD(&d64dialog, &overwritebutton);
  CTK_WIDGET_ADD(&d64dialog, &cancelbutton);
  
  
  ctk_window_open(&window);

  while(1) {

    PROCESS_WAIT_EVENT();
  
    if(ev == tcpip_event) {
      webclient_appcall(data);
    } else if(ev == ctk_signal_button_activate) {
      if(data == (void *)&filebutton) {
	/*      ret = cbm_open(2, 8, 2, savefilename);
		if(ret == -1) {*/
	sprintf(statusmsg, "Open error with '%s'", savefilename);
	show_statustext(statusmsg);
	/*      } else {
		strncpy(url, urledit, sizeof(url));
		petsciiconv_toascii(url, sizeof(url));
		start_get();
		dload_bytes = 0;
		dload_state = DLOAD_FILE;
		}*/
      } else if(data == (void *)&d64button) {
	ctk_dialog_open(&d64dialog);
      } else if(data == (void *)&cancelbutton) {
	ctk_dialog_close();
      } else if(data == (void *)&overwritebutton) {
	ctk_dialog_close();
	
	/* Turn of screensaver. */
	program_handler_setscreensaver(NULL);
	
	strncpy(url, urledit, sizeof(url));
	petsciiconv_toascii(url, sizeof(url));
	start_get();
	dload_bytes = 0;
	dload_state = DLOAD_D64;
	ds.track = 1;
	ds.sect = 0;
	bufferptr = 0;
#if USE_KERNAL
#else
	c64_dio_init(_curunit);
#endif
	
	/*      c64_dio_init(8);*/
      }
    } else if(ev == ctk_signal_hyperlink_activate) {
      if(dload_state == DLOAD_NONE) {
	/*      open_link(w->widget.hyperlink.url);*/
	strncpy(urledit,
		((struct ctk_widget *)data)->widget.hyperlink.url, sizeof(urledit));
	petsciiconv_topetscii(urledit, sizeof(urledit));
	CTK_WIDGET_REDRAW(&urltextentry);
	CTK_WIDGET_FOCUS(&window, &urltextentry);
      }
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data) != NULL) {
	start_get();
      } else {
	show_statustext("Host not found.");
      }
    } else if(ev == ctk_signal_window_close) {
      break;
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
/* webclient_aborted():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection was abruptly aborted.
 */
void
webclient_aborted(void)
{
  show_statustext("Connection reset by peer");
}
/*-----------------------------------------------------------------------------------*/
/* webclient_timedout():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection timed out.
 */
void
webclient_timedout(void)
{
  show_statustext("Connection timed out");
  if(dload_state == DLOAD_FILE) {
    cbm_close(2);
  }

}
/*-----------------------------------------------------------------------------------*/
/* webclient_closed():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection was closed after a request from the "webclient_close()"
 * function. .
 */
void
webclient_closed(void)
{  
  show_statustext("Done.");
}
/*-----------------------------------------------------------------------------------*/
/* webclient_closed():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection is connected.
 */
void
webclient_connected(void)
{    
  show_statustext("Request sent...");
}
/*-----------------------------------------------------------------------------------*/
static u8_t
next_sector(void)
{
  ++ds.sect;
  if(ds.track < 18) {
    if(ds.sect == 21) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 25) {
    if(ds.sect == 19) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 31) {
    if(ds.sect == 18) {
      ++ds.track;
      ds.sect = 0;
    }
  } else if(ds.track < 36) {
    if(ds.sect == 17) {
      ++ds.track;
      ds.sect = 0;
    }
  }

  if(ds.track == 36) {
    return 1;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
x_open(u8_t f, u8_t d, u8_t cmd, u8_t *fname)
{
  u8_t ret;
  
  ret = cbm_open(f, d, cmd, fname);
  if(ret != 0) {
    /*    printf("open: error %d\n", ret);*/
    /*    ctk_label_set_text(&statuslabel, "Open err");
	  CTK_WIDGET_REDRAW(&statuslabel);*/
    show_statustext("Open error");
  }
  
}
static void
write_sector(u8_t device, u8_t track, u8_t sect, void *mem)
{
  u16_t ret;
  static u8_t cmd[32];
  
  x_open(15, device, 15, NULL);
  x_open(2, device, 2, "#");

  ret = cbm_write(2, mem, 256);
  
  sprintf(cmd, "u2: 2 0 %d %d", track, sect);  
  cbm_write(15, cmd, strlen(cmd));
  /*  printf("%s\n", cmd);*/
    

  /*  ret = 0;*/
  if(ret == -1) {
    sprintf(statusmsg, "Write error at %d:%d", track, sect);
    show_statustext(statusmsg);
  } else {
    sprintf(statusmsg, "Wrote %d bytes to %d:%d", ret, track, sect);
    show_statustext(statusmsg);
  }
  /*  printf("write: wrote %d bytes\n", ret);*/

  cbm_close(2);
  cbm_close(15);
}

static void
write_buffer(void)
{
#if USE_KERNAL
  write_sector(8, ds.track, ds.sect, buffer);
#else
  c64_dio_write_block(ds.track, ds.sect, buffer);
#endif
  if(next_sector() != 0) {
    dload_state = DLOAD_NONE;
  }
}
static void
handle_d64_data(char *data, u16_t len)
{
  u16_t bufferlen;

  while(dload_state == DLOAD_D64 &&
	len > 0) {
    bufferlen = sizeof(buffer) - bufferptr;
    if(len < bufferlen) {
      bufferlen = len;
    }
    
    memcpy(&buffer[bufferptr], data, bufferlen);

    data += bufferlen;
    bufferptr += bufferlen;
    len -= bufferlen;
    
    if(bufferptr == sizeof(buffer)) {
      write_buffer();
      bufferptr = 0;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/* webclient_datahandler():   
 *
 * Callback function. Called from the webclient module when HTTP data
 * has arrived.
 */
void
webclient_datahandler(char *data, u16_t len)
{
  int ret;
  
  if(len > 0) {
    dload_bytes += len;    
    sprintf(statusmsg, "Downloading (%lu bytes)", dload_bytes);
    show_statustext(statusmsg);
    if(dload_state == DLOAD_D64) {
      handle_d64_data(data, len);
    } else if(dload_state == DLOAD_FILE) {      
      ret = cbm_write(2, data, len);       
      if(ret != len) {
	sprintf(statusmsg, "Wrote only %d bytes", ret);
	  show_statustext(statusmsg);	  
      }
    }
  }
  
  if(data == NULL) {
    if(dload_state == DLOAD_FILE) {
      cbm_close(2);
    }
    dload_state = DLOAD_NONE;
    sprintf(statusmsg, "Finished downloading %lu bytes", dload_bytes);
    show_statustext(statusmsg);
  }
}
/*-----------------------------------------------------------------------------------*/
