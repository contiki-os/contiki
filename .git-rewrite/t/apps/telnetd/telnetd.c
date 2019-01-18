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
 * $Id: telnetd.c,v 1.7 2007/11/17 20:13:54 oliverschmidt Exp $
 *
 */

#include "contiki-net.h"
#include "lib/petsciiconv.h"
#include "contiki-lib.h"

#include "shell.h"
#include "telnetd.h"


#include <string.h>

#define ISO_nl       0x0a
#define ISO_cr       0x0d

#define XSIZE 36
#define YSIZE 12

PROCESS(telnetd_process, "Shell server");

#ifndef TELNETD_CONF_LINELEN
#define TELNETD_CONF_LINELEN 40
#endif
#ifndef TELNETD_CONF_NUMLINES
#define TELNETD_CONF_NUMLINES 16
#endif

struct telnetd_line {
  char line[TELNETD_CONF_LINELEN];
};
MEMB(linemem, struct telnetd_line, TELNETD_CONF_NUMLINES);

struct telnetd_state {
  char *lines[TELNETD_CONF_NUMLINES];
  char buf[TELNETD_CONF_LINELEN];
  char bufptr;
  u8_t numsent;
  u8_t state;
#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4
#define STATE_DONT   5
  
#define STATE_CLOSE  6
};
static struct telnetd_state s;

#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254
/*-----------------------------------------------------------------------------------*/
static char *
alloc_line(void)
{
  return memb_alloc(&linemem);
}
/*-----------------------------------------------------------------------------------*/
static void
dealloc_line(char *line)
{
  memb_free(&linemem, line);
}
/*-----------------------------------------------------------------------------------*/
void
shell_quit(char *str)
{
  s.state = STATE_CLOSE;
}
/*-----------------------------------------------------------------------------------*/
void
telnetd_quit(void)
{
#if TELNETD_CONF_GUI
  telnetd_gui_quit();
#endif /* TELNETD_CONF_GUI */
  process_exit(&telnetd_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
sendline(char *line)
{
  static unsigned int i;
  
  for(i = 0; i < TELNETD_CONF_NUMLINES; ++i) {
    if(s.lines[i] == NULL) {
      s.lines[i] = line;
      break;
    }
  }
  if(i == TELNETD_CONF_NUMLINES) {
    dealloc_line(line);
  }
}
/*-----------------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  char *line;
  line = alloc_line();
  if(line != NULL) {
    strncpy(line, str, TELNETD_CONF_LINELEN);
    petsciiconv_toascii(line, TELNETD_CONF_LINELEN);
    sendline(line);
  }
}
/*-----------------------------------------------------------------------------------*/
void
shell_output(char *str1, char *str2)
{
  static unsigned len;
  char *line;

#if TELNETD_CONF_GUI
  telnetd_gui_output(str1, str2);
#endif /* TELNETD_CONF_GUI */
  line = alloc_line();
  if(line != NULL) {
    len = (unsigned int)strlen(str1);
    strncpy(line, str1, TELNETD_CONF_LINELEN);
    if(len < TELNETD_CONF_LINELEN) {
      strncpy(line + len, str2, TELNETD_CONF_LINELEN - len);
    }
    len = (unsigned int)strlen(line);
    if(len < TELNETD_CONF_LINELEN - 2) {
      line[len] = ISO_cr;
      line[len+1] = ISO_nl;
      line[len+2] = 0;
    }
    petsciiconv_toascii(line, TELNETD_CONF_LINELEN);
    sendline(line);
  }
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(telnetd_process, ev, data)
{
  PROCESS_BEGIN();
  
  tcp_listen(HTONS(23));
  memb_init(&linemem);
  shell_init();
#if TELNETD_CONF_GUI
  telnetd_gui_init();
#endif /* TELNETD_CONF_GUI */

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      telnetd_appcall(data);
    } else if(ev == PROCESS_EVENT_EXIT) {
      telnetd_quit();
    } else {
      shell_eventhandler(ev, data);
#if TELNETD_CONF_GUI
      telnetd_gui_eventhandler(ev, data);
#endif /* TELNETD_CONF_GUI */
    }
  }
  
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
static void
acked(void)
{
  static unsigned int i;
  
  while(s.numsent > 0) {
    dealloc_line(s.lines[0]);
    for(i = 1; i < TELNETD_CONF_NUMLINES; ++i) {
      s.lines[i - 1] = s.lines[i];
    }
    s.lines[TELNETD_CONF_NUMLINES - 1] = NULL;
    --s.numsent;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
senddata(void)
{
  static char *bufptr, *lineptr;
  static int buflen, linelen;
  
  bufptr = uip_appdata;
  buflen = 0;
  for(s.numsent = 0; s.numsent < TELNETD_CONF_NUMLINES &&
	s.lines[s.numsent] != NULL ; ++s.numsent) {
    lineptr = s.lines[s.numsent];
    linelen = (int)strlen(lineptr);
    if(linelen > TELNETD_CONF_LINELEN) {
      linelen = TELNETD_CONF_LINELEN;
    }
    if(buflen + linelen < uip_mss()) {
      memcpy(bufptr, lineptr, linelen);
      bufptr += linelen;
      buflen += linelen;
    } else {
      break;
    }
  }
  uip_send(uip_appdata, buflen);
}
/*-----------------------------------------------------------------------------------*/
static void
closed(void)
{
  static unsigned int i;
  
  for(i = 0; i < TELNETD_CONF_NUMLINES; ++i) {
    if(s.lines[i] != NULL) {
      dealloc_line(s.lines[i]);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
get_char(u8_t c)
{
  if(c == ISO_cr) {
    return;
  }
  
  s.buf[(int)s.bufptr] = c;
  if(s.buf[(int)s.bufptr] == ISO_nl ||
     s.bufptr == sizeof(s.buf) - 1) {
    if(s.bufptr > 0) {
      s.buf[(int)s.bufptr] = 0;
      petsciiconv_topetscii(s.buf, TELNETD_CONF_LINELEN);
    }
    shell_input(s.buf);
    s.bufptr = 0;
  } else {
    ++s.bufptr;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
sendopt(u8_t option, u8_t value)
{
  char *line;
  line = alloc_line();
  if(line != NULL) {
    line[0] = (char)TELNET_IAC;
    line[1] = option;
    line[2] = value;
    line[3] = 0;
    sendline(line);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
newdata(void)
{
  u16_t len;
  u8_t c;
    
  len = uip_datalen();
  
  while(len > 0 && s.bufptr < sizeof(s.buf)) {
    c = *(char *)uip_appdata;
    uip_appdata = (char *)uip_appdata + 1;
    --len;
    switch(s.state) {
    case STATE_IAC:
      if(c == TELNET_IAC) {
	get_char(c);
	s.state = STATE_NORMAL;
      } else {
	switch(c) {
	case TELNET_WILL:
	  s.state = STATE_WILL;
	  break;
	case TELNET_WONT:
	  s.state = STATE_WONT;
	  break;
	case TELNET_DO:
	  s.state = STATE_DO;
	  break;
	case TELNET_DONT:
	  s.state = STATE_DONT;
	  break;
	default:
	  s.state = STATE_NORMAL;
	  break;
	}
      }
      break;
    case STATE_WILL:
      /* Reply with a DONT */
      sendopt(TELNET_DONT, c);
      s.state = STATE_NORMAL;
      break;
      
    case STATE_WONT:
      /* Reply with a DONT */
      sendopt(TELNET_DONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_DO:
      /* Reply with a WONT */
      sendopt(TELNET_WONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_DONT:
      /* Reply with a WONT */
      sendopt(TELNET_WONT, c);
      s.state = STATE_NORMAL;
      break;
    case STATE_NORMAL:
      if(c == TELNET_IAC) {
	s.state = STATE_IAC;
      } else {
	get_char(c);
      }
      break;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
telnetd_appcall(void *ts)
{
  static unsigned int i;
  if(uip_connected()) {
    tcp_markconn(uip_conn, &s);
    for(i = 0; i < TELNETD_CONF_NUMLINES; ++i) {
      s.lines[i] = NULL;
    }
    s.bufptr = 0;
    s.state = STATE_NORMAL;

    shell_start();
  }

  if(s.state == STATE_CLOSE) {
    s.state = STATE_NORMAL;
    uip_close();
    return;
  }
  
  if(uip_closed() ||
     uip_aborted() ||
     uip_timedout()) {
    closed();
  }
  
  if(uip_acked()) {
    acked();
  }
  
  if(uip_newdata()) {
    newdata();
  }
  
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked() ||
     uip_connected() ||
     uip_poll()) {
    senddata();
  }
}
/*-----------------------------------------------------------------------------------*/
