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

#include "contiki-net.h"
#include "lib/petsciiconv.h"
#include "ctk/ctk.h"

#include "telnet.h"
#include "simpletelnet.h"

/* Telnet window */
static struct ctk_window telnetwindow;

static struct ctk_label telnethostlabel =
  {CTK_LABEL(0, 0, 4, 1, "Host")};
static char telnethost[25];
static struct ctk_textentry telnethosttextentry =
  {CTK_TEXTENTRY(4, 0, 24, 1, telnethost, 24)};

static struct ctk_label telnetportlabel =
  {CTK_LABEL(0, 1, 4, 1, "Port")};
static char telnetport[6];
static struct ctk_textentry telnetporttextentry =
  {CTK_TEXTENTRY(4, 1, 5, 1, telnetport, 5)};

static struct ctk_button telnetconnectbutton =
  {CTK_BUTTON(2, 2, 7, "Connect")};
static struct ctk_button telnetdisconnectbutton =
  {CTK_BUTTON(15, 2, 10, "Disconnect")};

static char telnetline[31];
static struct ctk_textentry telnetlinetextentry =
  {CTK_TEXTENTRY(0, 3, TELNET_ENTRY_WIDTH, 1, telnetline, TELNET_ENTRY_WIDTH)};

static struct ctk_button telnetsendbutton =
  {CTK_BUTTON(TELNET_ENTRY_WIDTH + 2, 3, 4, "Send")};

static struct ctk_label telnetstatus =
  {CTK_LABEL(0, TELNET_WINDOW_HEIGHT - 1, TELNET_WINDOW_WIDTH, 1, "")};

static struct ctk_separator telnetsep1 =
  {CTK_SEPARATOR(0, 4, TELNET_WINDOW_WIDTH)};

static struct ctk_separator telnetsep2 =
  {CTK_SEPARATOR(0, TELNET_WINDOW_HEIGHT - 2, TELNET_WINDOW_WIDTH)};

static char telnettext[TELNET_WINDOW_WIDTH*TELNET_TEXTAREA_HEIGHT];
static struct ctk_label telnettextarea =
  {CTK_LABEL(0, 5, TELNET_WINDOW_WIDTH, TELNET_TEXTAREA_HEIGHT, telnettext)};

static struct telnet_state ts_appstate;

#define ISO_NL       0x0a
#define ISO_CR       0x0d

static char sendline[31+2];

PROCESS(simpletelnet_process, "Telnet client");

AUTOSTART_PROCESSES(&simpletelnet_process);

/*-----------------------------------------------------------------------------------*/
static void
scrollup(void)
{
  unsigned char i;
  for(i = 1; i < TELNET_TEXTAREA_HEIGHT; ++i) {
    memcpy(&telnettext[(i - 1) * TELNET_WINDOW_WIDTH], &telnettext[i * TELNET_WINDOW_WIDTH], TELNET_WINDOW_WIDTH);
  }
  memset(&telnettext[(TELNET_TEXTAREA_HEIGHT - 1) * TELNET_WINDOW_WIDTH], 0, TELNET_WINDOW_WIDTH);
}
/*-----------------------------------------------------------------------------------*/
static void
add_text(char *text)
{
  unsigned char i;
  unsigned int len;
  
  len = (unsigned int)strlen(text);

  i = 0;
  while(len > 0) {
    if(*text == '\n') {
      scrollup();
      i = 0;
    } else if(*text == '\r') {
      i = 0;
    } else if(*text >= ' ') {
      telnettext[(TELNET_TEXTAREA_HEIGHT - 1) * TELNET_WINDOW_WIDTH + i] = *text;
      ++i;
      if(i == TELNET_WINDOW_WIDTH) {
	scrollup();
	i = 0;
      }
    }
    ++text;
    --len;
  }

  /*  if(strlen(text) > 37) {
      memcpy(&telnettext[9 * 38], text, 37);
      } else {
      memcpy(&telnettext[9 * 38], text, strlen(text));
      }
  */
}
/*-----------------------------------------------------------------------------------*/
static void
show(char *text)
{
  add_text(text);
  add_text("\n");
  ctk_label_set_text(&telnetstatus, text);
  ctk_window_redraw(&telnetwindow);
}
/*-----------------------------------------------------------------------------------*/
static void
connect(void)
{
  uip_ipaddr_t addr, *addrptr;
  uint16_t port;
  char *cptr;
  struct uip_conn *conn;

  /* Find the first space character in host and put a zero there
     to end the string. */
  for(cptr = telnethost; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;

  addrptr = &addr;
#if UIP_UDP
  if(uiplib_ipaddrconv(telnethost, &addr) == 0) {
    addrptr = resolv_lookup(telnethost);
    if(addrptr == NULL) {
      resolv_query(telnethost);
      show("Resolving host...");
      return;
    }
  }
#else /* UIP_UDP */
  uiplib_ipaddrconv(telnethost, &addr);
#endif /* UIP_UDP */

  port = 0;
  for(cptr = telnetport; *cptr != ' ' && *cptr != 0; ++cptr) {
    if(*cptr < '0' || *cptr > '9') {
      show("Port number error");
      return;
    }
    port = 10 * port + *cptr - '0';
  }


  conn = tcp_connect(addrptr, uip_htons(port), &ts_appstate);
  if(conn == NULL) {
    show("Out of memory error");
    return;
  }

  show("Connecting...");

}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(simpletelnet_process, ev, data)
{
  struct ctk_widget *w;
  int sendlen;

  PROCESS_BEGIN();
  
  ctk_window_new(&telnetwindow, TELNET_WINDOW_WIDTH, TELNET_WINDOW_HEIGHT, "Simple telnet");
  
  strcpy(telnetport, "23");
  
  CTK_WIDGET_ADD(&telnetwindow, &telnethostlabel);
  CTK_WIDGET_ADD(&telnetwindow, &telnetportlabel);
  CTK_WIDGET_ADD(&telnetwindow, &telnethosttextentry);
  CTK_WIDGET_ADD(&telnetwindow, &telnetporttextentry);
  CTK_WIDGET_ADD(&telnetwindow, &telnetconnectbutton);
  CTK_WIDGET_ADD(&telnetwindow, &telnetdisconnectbutton);
  CTK_WIDGET_ADD(&telnetwindow, &telnetlinetextentry);
  CTK_WIDGET_ADD(&telnetwindow, &telnetsendbutton);
  
  CTK_WIDGET_ADD(&telnetwindow, &telnetsep1);
  CTK_WIDGET_ADD(&telnetwindow, &telnettextarea);
  
  CTK_WIDGET_ADD(&telnetwindow, &telnetsep2);
  CTK_WIDGET_ADD(&telnetwindow, &telnetstatus);
  
  CTK_WIDGET_FOCUS(&telnetwindow, &telnethosttextentry);
  
  ctk_window_open(&telnetwindow);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == ctk_signal_button_activate) {
      
      w = (struct ctk_widget *)data;
      if(w == (struct ctk_widget *)&telnetsendbutton) {
	strcpy(sendline, telnetline);
	sendlen = (int)strlen(sendline);
	petsciiconv_toascii(sendline, sendlen);
	sendline[sendlen++] = ISO_CR;
	sendline[sendlen++] = ISO_NL;
	if(telnet_send(&ts_appstate, sendline, sendlen)) {
	  /* Could not send. */
	  ctk_label_set_text(&telnetstatus, "Could not send");
	  ctk_window_redraw(&telnetwindow);
	  /*      } else {*/
	  /* Could send */
	}
      } else if(w == (struct ctk_widget *)&telnetdisconnectbutton) {
	telnet_close(&ts_appstate);
	show("Closing...");
      } else if(w == (struct ctk_widget *)&telnetconnectbutton) {
	connect();
	ctk_window_redraw(&telnetwindow);
      }
#if UIP_UDP
    } else if(ev == resolv_event_found) {
      if(strcmp(data, telnethost) == 0) {
	if(resolv_lookup(telnethost) != NULL) {
	  connect();
	} else {
	  show("Host not found");
	}
      }
#endif /* UIP_UDP */
    } else if(
#if CTK_CONF_WINDOWCLOSE
	      ev == ctk_signal_window_close ||
#endif /* CTK_CONF_WINDOWCLOSE */
	      ev == PROCESS_EVENT_EXIT) {
      process_exit(&simpletelnet_process);
      ctk_window_close(&telnetwindow);
      LOADER_UNLOAD();
    } else if(ev == tcpip_event) {
      telnet_app(data);
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
void
telnet_connected(struct telnet_state *s)
{  
  show("Connected");
}
/*-----------------------------------------------------------------------------------*/
void
telnet_closed(struct telnet_state *s)
{
  show("Connection closed");
}
/*-----------------------------------------------------------------------------------*/
void
telnet_sent(struct telnet_state *s)
{
  petsciiconv_topetscii(sendline, sizeof(sendline));
  scrollup();
  add_text(sendline);
  CTK_TEXTENTRY_CLEAR(&telnetlinetextentry);
  ctk_window_redraw(&telnetwindow);
}
/*-----------------------------------------------------------------------------------*/
void
telnet_aborted(struct telnet_state *s)
{
  show("Connection reset by peer");
}
/*-----------------------------------------------------------------------------------*/
void
telnet_timedout(struct telnet_state *s)
{
  show("Connection timed out");
}
/*-----------------------------------------------------------------------------------*/
void
telnet_newdata(struct telnet_state *s, char *data, uint16_t len)
{
  petsciiconv_topetscii(data, len);
  data[len] = 0;
  add_text(data);
  ctk_window_redraw(&telnetwindow);
}
/*-----------------------------------------------------------------------------------*/
