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

#include <string.h>

#include "ctk/ctk.h"
#include "lib/ctk-textentry-cmdline.h"
#include "contiki-net.h"
#include "lib/petsciiconv.h"
#include "sys/arg.h"
#if WWW_CONF_WITH_WGET
#include "program-handler.h"
#endif /* WWW_CONF_WITH_WGET */

#include "webclient.h"
#include "htmlparser.h"
#include "http-strings.h"

#include "www.h"

#if 1
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif


/* The array that holds the current URL. */
static char url[WWW_CONF_MAX_URLLEN + 1];
static char tmpurl[WWW_CONF_MAX_URLLEN + 1];

/* The array that holds the web page text. */
static char webpage[WWW_CONF_WEBPAGE_WIDTH *
		    WWW_CONF_WEBPAGE_HEIGHT + 1];


/* The CTK widgets for the main window. */
static struct ctk_window mainwindow;

#if WWW_CONF_HISTORY_SIZE > 0
static struct ctk_button backbutton =
  {CTK_BUTTON(0, 0, 4, "Back")};
static struct ctk_button downbutton =
  {CTK_BUTTON(10, 0, 4, "Down")};
#else /* WWW_CONF_HISTORY_SIZE > 0 */
static struct ctk_button downbutton =
  {CTK_BUTTON(0, 0, 4, "Down")};
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
static struct ctk_button stopbutton =
  {CTK_BUTTON(WWW_CONF_WEBPAGE_WIDTH - 16, 0, 4, "Stop")};
static struct ctk_button gobutton =
  {CTK_BUTTON(WWW_CONF_WEBPAGE_WIDTH - 4, 0, 2, "Go")};

static struct ctk_separator sep1 =
  {CTK_SEPARATOR(0, 2, WWW_CONF_WEBPAGE_WIDTH)};

static char editurl[WWW_CONF_MAX_URLLEN + 1];
static struct ctk_textentry urlentry =
  {CTK_TEXTENTRY(0, 1, WWW_CONF_WEBPAGE_WIDTH - 2,
		 1, editurl, WWW_CONF_MAX_URLLEN)};
static struct ctk_label webpagelabel =
  {CTK_LABEL(0, 3, WWW_CONF_WEBPAGE_WIDTH,
	     WWW_CONF_WEBPAGE_HEIGHT, webpage)};

static char statustexturl[WWW_CONF_WEBPAGE_WIDTH];
static struct ctk_label statustext =
  {CTK_LABEL(0, WWW_CONF_WEBPAGE_HEIGHT + 4,
	     WWW_CONF_WEBPAGE_WIDTH, 1, "")};
static struct ctk_separator sep2 =
  {CTK_SEPARATOR(0, WWW_CONF_WEBPAGE_HEIGHT + 3,
		 WWW_CONF_WEBPAGE_WIDTH)};

#if WWW_CONF_WITH_WGET
static struct ctk_window wgetdialog;
static struct ctk_label wgetlabel1 =
  {CTK_LABEL(1, 1, 34, 1, "This web page cannot be displayed.")};
static struct ctk_label wgetlabel2 =
  {CTK_LABEL(1, 3, 35, 1, "Would you like to download instead?")};
static struct ctk_button wgetnobutton =
  {CTK_BUTTON(1, 5, 6, "Cancel")};
static struct ctk_button wgetyesbutton =
  {CTK_BUTTON(11, 5, 24, "Close browser & download")};
#endif /* WWW_CONF_WITH_WGET */

#if WWW_CONF_HISTORY_SIZE > 0
/* The char arrays that hold the history of visited URLs. */
static char history[WWW_CONF_HISTORY_SIZE][WWW_CONF_MAX_URLLEN];
static char history_last;
#endif /* WWW_CONF_HISTORY_SIZE > 0 */

/* The CTK widget definitions for the hyperlinks and the char arrays
   that hold the link URLs. */
struct formattribs {
  char formaction[WWW_CONF_MAX_FORMACTIONLEN];
  char formname[WWW_CONF_MAX_FORMNAMELEN];
#define FORMINPUTTYPE_SUBMITBUTTON 1
#define FORMINPUTTYPE_INPUTFIELD   2
  unsigned char inputtype;
  char inputname[WWW_CONF_MAX_INPUTNAMELEN];
  char *inputvalue;
};

union pagewidgetattrib {
  char url[WWW_CONF_MAX_URLLEN];
#if WWW_CONF_FORMS
  struct formattribs form;
#endif /* WWW_CONF_FORMS */
};
static struct ctk_widget pagewidgets[WWW_CONF_MAX_NUMPAGEWIDGETS];
static union pagewidgetattrib pagewidgetattribs[WWW_CONF_MAX_NUMPAGEWIDGETS];
static unsigned char pagewidgetptr;

#if WWW_CONF_RENDERSTATE
static unsigned char renderstate;
#endif /* WWW_CONF_RENDERSTATE */

#define ISO_nl    0x0a
#define ISO_space 0x20
#define ISO_ampersand 0x26
#define ISO_plus 0x2b
#define ISO_slash 0x2f
#define ISO_eq    0x3d
#define ISO_questionmark  0x3f

/* The state of the rendering code. */
static char *webpageptr;
static unsigned char x, y;
static unsigned char loading;
static unsigned short firsty, pagey;

static unsigned char count;
static char receivingmsgs[4][23] = {
  "Receiving web page ...",
  "Receiving web page. ..",
  "Receiving web page.. .",
  "Receiving web page... "
};

PROCESS(www_process, "Web browser");

AUTOSTART_PROCESSES(&www_process);

static void formsubmit(struct formattribs *attribs);

/*-----------------------------------------------------------------------------------*/
/* make_window()
 *
 * Creates the web browser's window.
 */
static void
make_window(void)
{
#if WWW_CONF_HISTORY_SIZE > 0
  CTK_WIDGET_ADD(&mainwindow, &backbutton);
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
  CTK_WIDGET_ADD(&mainwindow, &downbutton);
  CTK_WIDGET_ADD(&mainwindow, &stopbutton);
  CTK_WIDGET_ADD(&mainwindow, &gobutton);
  CTK_WIDGET_ADD(&mainwindow, &urlentry);
  CTK_WIDGET_ADD(&mainwindow, &sep1);
  CTK_WIDGET_ADD(&mainwindow, &webpagelabel);
  CTK_WIDGET_SET_FLAG(&webpagelabel, CTK_WIDGET_FLAG_MONOSPACE);
  CTK_WIDGET_ADD(&mainwindow, &sep2);
  CTK_WIDGET_ADD(&mainwindow, &statustext);

  pagewidgetptr = 0;
}
/*-----------------------------------------------------------------------------------*/
/* redraw_window():
 *
 * Convenience function that calls upon CTK to redraw the browser
 * window. */
static void
redraw_window(void)
{
  ctk_window_redraw(&mainwindow);
}
/*-----------------------------------------------------------------------------------*/
static void
clear_page(void)
{
  ctk_window_clear(&mainwindow);
  make_window();
  redraw_window();
  memset(webpage, 0, WWW_CONF_WEBPAGE_WIDTH * WWW_CONF_WEBPAGE_HEIGHT);
}
/*-----------------------------------------------------------------------------------*/
static void
show_url(void)
{
  memcpy(editurl, url, WWW_CONF_MAX_URLLEN);
  strncpy(editurl, "http://", 7);
  petsciiconv_topetscii(editurl + 7, WWW_CONF_MAX_URLLEN - 7);
  CTK_WIDGET_REDRAW(&urlentry);
}
/*-----------------------------------------------------------------------------------*/
static void
start_loading(void)
{
  loading = 1;
  x = y = 0;
  pagey = 0;
  webpageptr = webpage;

  clear_page();
}
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
open_url(void)
{
  unsigned char i;
  static char host[32];
  char *file;
  register char *urlptr;
  static uip_ipaddr_t addr;

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
  
#if UIP_UDP
  /* Try to lookup the hostname. If it fails, we initiate a hostname
     lookup and print out an informative message on the statusbar. */
  if(uiplib_ipaddrconv(host, &addr) == 0) {
    if(resolv_lookup(host) == NULL) {
      resolv_query(host);
      show_statustext("Resolving host...");
      return;
    }
  }
#else /* UIP_UDP */
  uiplib_ipaddrconv(host, &addr);
#endif /* UIP_UDP */

  /* The hostname we present in the hostname table, so we send out the
     initial GET request. */
  if(webclient_get(host, 80, file) == 0) {
    show_statustext("Out of memory error.");
  } else {
    show_statustext("Connecting...");
  }
  redraw_window();
}
/*-----------------------------------------------------------------------------------*/
/* open_link(link):
 *
 * Will format a link from the current web pages so that it suits the
 * open_url() function and finally call it to open the requested URL.
 */
static void
open_link(char *link)
{
  char *urlptr;
    
  if(strncmp(link, http_http, 7) == 0) {
    /* The link starts with http://. We just copy the contents of the
       link into the url string and jump away. */
    strncpy(url, link, WWW_CONF_MAX_URLLEN);
  } else if(*link == ISO_slash &&
	    *(link + 1) == ISO_slash) {
    /* The link starts with //, so we'll copy it into the url
       variable, starting after the http (which already is present in
       the url variable since we were able to open the web page on
       which this link was found in the first place). */
    strncpy(&url[5], link, WWW_CONF_MAX_URLLEN);
  } else if(*link == ISO_slash) {
    /* The link starts with a slash, so it is a non-relative link
       within the same web site. We find the start of the filename of
       the current URL and paste the contents of this link there, and
       head off to the new URL. */
    for(urlptr = &url[7];
	*urlptr != 0 && *urlptr != ISO_slash;
	++urlptr);
    strncpy(urlptr, link, WWW_CONF_MAX_URLLEN - (urlptr - url));
  } else {
    /* A fully relative link is found. We find the last slash in the
       current URL and paste the link there. */
    
    /* XXX: we should really parse any ../ in the link as well. */
    for(urlptr = url + strlen(url);
	urlptr != url && *urlptr != ISO_slash;
	--urlptr);
    ++urlptr;
    strncpy(urlptr, link, WWW_CONF_MAX_URLLEN - (urlptr - url));
  }

  /* Open the URL. */
  show_url();
  open_url();

  start_loading();
}
/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_HISTORY_SIZE > 0
/* log_back():
 *
 * Copies the current URL from the url variable and into the log for
 * the back button.
 */
static void
log_back(void)
{
  if(strncmp(url, history[(int)history_last], WWW_CONF_MAX_URLLEN) != 0) {
    memcpy(history[(int)history_last], url, WWW_CONF_MAX_URLLEN);
    ++history_last;
    if(history_last >= WWW_CONF_HISTORY_SIZE) {
      history_last = 0;
    }
  }
}
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
/*-----------------------------------------------------------------------------------*/
static void
quit(void)
{
  ctk_window_close(&mainwindow);
  process_exit(&www_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
/* www_process():
 *
 * The program's signal dispatcher function. Is called whenever a signal arrives.
 */
PROCESS_THREAD(www_process, ev, data)
{
  static struct ctk_widget *w;
  static unsigned char i;
#if WWW_CONF_WITH_WGET
  static char *argptr;
#endif /* WWW_CONF_WITH_WGET */

  w = (struct ctk_widget *)data;

  PROCESS_BEGIN();
  
  /* Create the main window. */
  memset(webpage, 0, sizeof(webpage));
  ctk_window_new(&mainwindow, WWW_CONF_WEBPAGE_WIDTH,
		 WWW_CONF_WEBPAGE_HEIGHT+5, "Web browser");
  make_window();
#ifdef WWW_CONF_HOMEPAGE
  strncpy(editurl, WWW_CONF_HOMEPAGE, sizeof(editurl));
#endif /* WWW_CONF_HOMEPAGE */
  CTK_WIDGET_FOCUS(&mainwindow, &urlentry);
  
#if WWW_CONF_WITH_WGET
  /* Create download dialog.*/
  ctk_dialog_new(&wgetdialog, 38, 7);
  CTK_WIDGET_ADD(&wgetdialog, &wgetlabel1);
  CTK_WIDGET_ADD(&wgetdialog, &wgetlabel2);
  CTK_WIDGET_ADD(&wgetdialog, &wgetnobutton);
  CTK_WIDGET_ADD(&wgetdialog, &wgetyesbutton);
#endif /* WWW_CONF_WITH_WGET */

  ctk_window_open(&mainwindow);
  
  while(1) {

    PROCESS_WAIT_EVENT();
    
    if(ev == tcpip_event) {
      webclient_appcall(data);
    } else if(ev == ctk_signal_widget_activate) {
      if(w == (struct ctk_widget *)&gobutton ||
	 w == (struct ctk_widget *)&urlentry) {
	start_loading();
	firsty = 0;
#if WWW_CONF_HISTORY_SIZE > 0	
	log_back();
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
	memcpy(url, editurl, WWW_CONF_MAX_URLLEN);
	petsciiconv_toascii(url, WWW_CONF_MAX_URLLEN);
	open_url();
	CTK_WIDGET_FOCUS(&mainwindow, &gobutton);
#if WWW_CONF_HISTORY_SIZE > 0
      } else if(w == (struct ctk_widget *)&backbutton) {
	firsty = 0;
	start_loading();
	--history_last;
	if(history_last > WWW_CONF_HISTORY_SIZE) {
	  history_last = WWW_CONF_HISTORY_SIZE - 1;
	}
	memcpy(url, history[(int)history_last], WWW_CONF_MAX_URLLEN);
	open_url();
	CTK_WIDGET_FOCUS(&mainwindow, &backbutton);
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
      } else if(w == (struct ctk_widget *)&downbutton) {
	firsty = pagey + WWW_CONF_WEBPAGE_HEIGHT - 4;
	start_loading();
	open_url();
	CTK_WIDGET_FOCUS(&mainwindow, &downbutton);
      } else if(w == (struct ctk_widget *)&stopbutton) {
	loading = 0;
	webclient_close();
#if WWW_CONF_WITH_WGET
      } else if(w == (struct ctk_widget *)&wgetnobutton) {
	ctk_dialog_close();
      } else if(w == (struct ctk_widget *)&wgetyesbutton) {
	ctk_dialog_close();
	quit();
	argptr = arg_alloc((char)WWW_CONF_MAX_URLLEN);
	if(argptr != NULL) {
	  strncpy(argptr, url, WWW_CONF_MAX_URLLEN);
	}
	program_handler_load("wget.prg", argptr);
#endif /* WWW_CONF_WITH_WGET */
#if WWW_CONF_FORMS
      } else {
	/* Check form buttons */
	for(i = 0; i < pagewidgetptr; ++i) {
	  if(&pagewidgets[i] == w) {
	    formsubmit(&pagewidgetattribs[i].form);
	    /*	  show_statustext(pagewidgetattribs[i].form.formaction);*/
	    /*	  PRINTF(("Formaction %s formname %s inputname %s\n",
		  pagewidgetattribs[i].form.formaction,
		  pagewidgetattribs[i].form.formname,
		  pagewidgetattribs[i].form.inputname));*/
	    break;
	  }
	}
#endif /* WWW_CONF_FORMS */
      }
    } else if(ev == ctk_signal_hyperlink_activate) {
      firsty = 0;
#if WWW_CONF_HISTORY_SIZE > 0
      log_back();
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
      open_link(w->widget.hyperlink.url);
      CTK_WIDGET_FOCUS(&mainwindow, &stopbutton);
      /*    ctk_window_open(&mainwindow);*/
    } else if(ev == ctk_signal_hyperlink_hover) {
      if(CTK_WIDGET_TYPE((struct ctk_widget *)data) ==
	 CTK_WIDGET_HYPERLINK) {
	strncpy(statustexturl, w->widget.hyperlink.url,
		sizeof(statustexturl));
	petsciiconv_topetscii(statustexturl, sizeof(statustexturl));
	show_statustext(statustexturl);
      }
#if UIP_UDP
    } else if(ev == resolv_event_found) {
      /* Either found a hostname, or not. */
      if((char *)data != NULL &&
	 resolv_lookup((char *)data) != NULL) {
	open_url();
      } else {
	show_statustext("Host not found.");
      }
#endif /* UIP_UDP */
    } else if(ev == ctk_signal_window_close ||
	      ev == PROCESS_EVENT_EXIT) {
      quit();
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
/* set_url():
 *
 * Constructs an URL from the arguments and puts it into the global
 * "url" variable and the visible "editurl" (which is shown in the URL
 * text entry widget in the browser window).
 */
static void
set_url(char *host, uint16_t port, char *file)
{
  char *urlptr;

  memset(url, 0, WWW_CONF_MAX_URLLEN);
  
  if(strncmp(file, http_http, 7) == 0) {
    strncpy(url, file, sizeof(url));
  } else {
    strncpy(url, http_http, 7);
    urlptr = url + 7;
    strcpy(urlptr, host);
    urlptr += strlen(host);
    strcpy(urlptr, file);
  }

  show_url();
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
}
/*-----------------------------------------------------------------------------------*/
/* webclient_closed():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection was closed after a request from the "webclient_close()"
 * function.
 */
void
webclient_closed(void)
{
  show_statustext("Stopped.");
  petsciiconv_topetscii(webpageptr - x, x);
  CTK_WIDGET_FOCUS(&mainwindow, &downbutton);
  redraw_window();
}
/*-----------------------------------------------------------------------------------*/
/* webclient_connected():
 *
 * Callback function. Called from the webclient when the HTTP
 * connection is connected.
 */
void
webclient_connected(void)
{
  start_loading();
    
  clear_page();
  
  show_statustext("Request sent...");
  set_url(webclient_hostname(), webclient_port(), webclient_filename());

#if WWW_CONF_RENDERSTATE
  renderstate = HTMLPARSER_RENDERSTATE_NONE;
#endif /* WWW_CONF_RENDERSTATE */
  htmlparser_init();
}
/*-----------------------------------------------------------------------------------*/
/* webclient_datahandler():
 *
 * Callback function. Called from the webclient module when HTTP data
 * has arrived.
 */
void
webclient_datahandler(char *data, uint16_t len)
{
  if(len > 0) {
    if(strcmp(webclient_mimetype(), http_texthtml) == 0) {
      count = (count + 1) & 3;
      show_statustext(receivingmsgs[count]);
      htmlparser_parse(data, len);
      redraw_window();
    } else {
      uip_abort();
#if WWW_CONF_WITH_WGET
      ctk_dialog_open(&wgetdialog);
#endif /* WWW_CONF_WITH_WGET */
    }
  } else {
    /* Clear remaining parts of page. */
    loading = 0;
  }
  
  if(data == NULL) {
    loading = 0;
    show_statustext("Done.");
    petsciiconv_topetscii(webpageptr - x, x);
    CTK_WIDGET_FOCUS(&mainwindow, &urlentry);
    redraw_window();
  }
}
/*-----------------------------------------------------------------------------------*/
static void *
add_pagewidget(char *text, unsigned char len, unsigned char type,
		unsigned char border)
{
  register struct ctk_widget *lptr;
  register char *wptr;
  static unsigned char maxwidth;
  static void *dataptr;

  if(!loading) {
    return NULL;
  }
  
  if(len + border == 0) {
    return NULL;
  }
  
  maxwidth = WWW_CONF_WEBPAGE_WIDTH - (1 + 2 * border);
  
  /* If the text of the link is too long so that it does not fit into
     the width of the current window, counting from the current x
     coordinate, we first try to jump to the next line. */
  if(len + x > maxwidth) {
    htmlparser_newline();
    if(!loading) {
      return NULL;
    }
  }

  /* If the text of the link still is too long, we just chop it off!
     XXX: this is not really the right thing to do, we should probably
     either make a link into a multiline link, or add multiple
     buttons. But this will do for now. */
  if(len > maxwidth) {
    text[maxwidth] = 0;
    len = maxwidth;
  }

  dataptr = NULL;
  
  if(firsty == pagey) {
    wptr = webpageptr;
    /* To save memory, we'll copy the widget text to the web page
       drawing area and reference it from there. */
    wptr[0] = 0;
    wptr += border;
    memcpy(wptr, text, len);
    wptr[len] = 0;
    wptr[len + border] = ' ';
    if(pagewidgetptr < WWW_CONF_MAX_NUMPAGEWIDGETS) {
      dataptr = &pagewidgetattribs[pagewidgetptr];
      lptr = &pagewidgets[pagewidgetptr];
      
      switch(type) {
      case CTK_WIDGET_HYPERLINK:
	CTK_HYPERLINK_NEW((struct ctk_hyperlink *)lptr, x,
			  y + 3, len,
			  wptr, dataptr);
	break;
      case CTK_WIDGET_BUTTON:
	CTK_BUTTON_NEW((struct ctk_button *)lptr, x,
		       y + 3, len,
		       wptr);
	((struct formattribs *)dataptr)->inputvalue = wptr;
	break;
      case CTK_WIDGET_TEXTENTRY:
	CTK_TEXTENTRY_NEW((struct ctk_textentry *)lptr, x,
			  y + 3, len, 1,
			  wptr, len);
	((struct formattribs *)dataptr)->inputvalue = wptr;
	break;
      }
      CTK_WIDGET_SET_FLAG(lptr, CTK_WIDGET_FLAG_MONOSPACE);
      CTK_WIDGET_ADD(&mainwindow, lptr);

      ++pagewidgetptr;
    }
  }
  /* Increase the x coordinate with the length of the link text plus
     the extra space behind it and the CTK button markers. */
  len = len + 1 + 2 * border;
  x += len;

  if(firsty == pagey) {
    webpageptr += len;
  }
  
  if(x == WWW_CONF_WEBPAGE_WIDTH) {
    htmlparser_newline();
  }

  return dataptr;
}
/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_RENDERSTATE
static void
centerline(char *wptr)
{
  unsigned char spaces, i;
  char *cptr;
  register struct ctk_widget *linksptr;
  
  cptr = wptr + WWW_CONF_WEBPAGE_WIDTH;
  for(spaces = 0; spaces < WWW_CONF_WEBPAGE_WIDTH; ++spaces) {
    if(*--cptr != 0) {
      break;
    }
  }
  
  spaces /= 2;

  while(cptr >= wptr) {
    *(cptr + spaces) = *cptr;
    --cptr;
  }

  memset(wptr, ' ', spaces);
  
  linksptr = pagewidgets;
  
  for(i = 0; i < pagewidgetptr; ++i) {
    if(CTK_WIDGET_YPOS(linksptr) == y + 2) {
      linksptr->x += spaces;
      linksptr->widget.hyperlink.text += spaces;
    }
    ++linksptr;
  }
}
#endif /* WWW_CONF_RENDERSTATE */
/*-----------------------------------------------------------------------------------*/
void
htmlparser_newline(void)
{
  char *wptr;
  
  if(pagey < firsty) {
    ++pagey;
    x = 0;
    return;
  }
  
  if(!loading) {
    return;
  }

  webpageptr += (WWW_CONF_WEBPAGE_WIDTH - x);
  ++y;
  x = 0;
  
  wptr = webpageptr - WWW_CONF_WEBPAGE_WIDTH;
  petsciiconv_topetscii(wptr,
			WWW_CONF_WEBPAGE_WIDTH);
#if WWW_CONF_RENDERSTATE
  if(renderstate & HTMLPARSER_RENDERSTATE_CENTER) {
    centerline(wptr);
  }
#endif /* WWW_CONF_RENDERSTATE */
  
  if(y == WWW_CONF_WEBPAGE_HEIGHT) {
    loading = 0;
    webclient_close();
  }
}
/*-----------------------------------------------------------------------------------*/
void
htmlparser_word(char *word, unsigned char wordlen)
{

  if(loading) {
    if(wordlen + 1 > WWW_CONF_WEBPAGE_WIDTH - x) {
      htmlparser_newline();
    }

    if(loading) {
      if(pagey == firsty) {
	memcpy(webpageptr, word, wordlen);
	webpageptr += wordlen;
	*webpageptr = ' ';
	++webpageptr;
      }
      x += wordlen + 1;
      if(x == WWW_CONF_WEBPAGE_WIDTH) {
	htmlparser_newline();
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
htmlparser_link(char *text, unsigned char textlen, char *url)
{
  static char *linkurlptr;

  linkurlptr = add_pagewidget(text, textlen, CTK_WIDGET_HYPERLINK, 0);
  if(linkurlptr != NULL &&
     strlen(url) < WWW_CONF_MAX_URLLEN) {
    strcpy(linkurlptr, url);
  }
}
/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_RENDERSTATE
void
htmlparser_renderstate(unsigned char s)
{
  if((s & HTMLPARSER_RENDERSTATE_STATUSMASK) ==
     HTMLPARSER_RENDERSTATE_BEGIN) {
    renderstate |= s & ~HTMLPARSER_RENDERSTATE_STATUSMASK;
  } else {
    renderstate &= ~(s & ~HTMLPARSER_RENDERSTATE_STATUSMASK);
  }
}
#endif /* WWW_CONF_RENDERSTATE */

/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_FORMS
void
htmlparser_submitbutton(char *text, char *name,
			char *formname, char *formaction)
{
  register struct formattribs *form;
  
  form = add_pagewidget(text, (unsigned char)strlen(text), CTK_WIDGET_BUTTON, 1);
  if(form != NULL) {
    strncpy(form->formaction, formaction, WWW_CONF_MAX_FORMACTIONLEN);
    strncpy(form->formname, formname, WWW_CONF_MAX_FORMNAMELEN);
    strncpy(form->inputname, name, WWW_CONF_MAX_INPUTNAMELEN);
    form->inputtype = FORMINPUTTYPE_SUBMITBUTTON;
  }
}
/*-----------------------------------------------------------------------------------*/
void
htmlparser_inputfield(unsigned char size, char *text, char *name,
		      char *formname, char *formaction)
{
  register struct formattribs *form;

  form = add_pagewidget(text, size, CTK_WIDGET_TEXTENTRY, 1);
  if(form != NULL) {
    strncpy(form->formaction, formaction, WWW_CONF_MAX_FORMACTIONLEN);
    strncpy(form->formname, formname, WWW_CONF_MAX_FORMNAMELEN);
    strncpy(form->inputname, name, WWW_CONF_MAX_INPUTNAMELEN);
    form->inputtype = FORMINPUTTYPE_INPUTFIELD;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
formsubmit(struct formattribs *attribs)
{
  unsigned char i, j;
  register char *urlptr, *valueptr;
  register struct formattribs *faptr;

  urlptr = &tmpurl[0];

  strncpy(urlptr, attribs->formaction, WWW_CONF_MAX_URLLEN);
  tmpurl[WWW_CONF_MAX_URLLEN] = 0;
  urlptr += strlen(urlptr);
  *urlptr = ISO_questionmark;
  ++urlptr;
  
  /* Construct an URL by finding all input field forms with the same
     formname as the current submit button, and add the submit button
     URL stuff as well. */
  for(i = 0; i < pagewidgetptr; ++i) {
    if(urlptr - &tmpurl[0] >= WWW_CONF_MAX_URLLEN) {
      break;
    }

    faptr = &pagewidgetattribs[i].form;
    
    if(strcmp(attribs->formaction, faptr->formaction) == 0 &&
       strcmp(attribs->formname, faptr->formname) == 0 &&
       (faptr->inputtype == FORMINPUTTYPE_INPUTFIELD ||
	faptr == attribs)) {

      /* Copy the name of the input field into the URL and append a
	 questionmark. */
      strncpy(urlptr, faptr->inputname, WWW_CONF_MAX_URLLEN - strlen(tmpurl));
      tmpurl[WWW_CONF_MAX_URLLEN] = 0;
      urlptr += strlen(urlptr);
      *urlptr = ISO_eq;
      ++urlptr;

      /* Convert and copy the contents of the input field to the URL
	 and append an ampersand. */
      valueptr = pagewidgets[i].widget.textentry.text;
      petsciiconv_toascii(valueptr, WWW_CONF_MAX_INPUTVALUELEN);
      for(j = 0; j < WWW_CONF_MAX_INPUTVALUELEN; ++j) {
	if(urlptr - &tmpurl[0] >= WWW_CONF_MAX_URLLEN) {
	  break;
	}
	*urlptr = *valueptr;
	if(*urlptr == ISO_space) {
	  *urlptr = ISO_plus;
	}
	if(*urlptr == 0) {
	  break;
	}
	++urlptr;
	++valueptr;
      }
      
      *urlptr = ISO_ampersand;
      ++urlptr;
    }
  }
  --urlptr;
  *urlptr = 0;
#if WWW_CONF_HISTORY_SIZE > 0
  log_back();
#endif /* WWW_CONF_HISTORY_SIZE > 0 */
  open_link(tmpurl);
}
#endif /* WWW_CONF_FORMS */
/*-----------------------------------------------------------------------------------*/
