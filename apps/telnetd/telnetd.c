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
 *
 */

#include <string.h>

#include "contiki-lib.h"
#include "contiki-net.h"
#include "lib/petsciiconv.h"
#include "shell.h"

#include "telnetd.h"

#define ISO_nl       0x0a
#define ISO_cr       0x0d

PROCESS(telnetd_process, "Telnet server");

AUTOSTART_PROCESSES(&telnetd_process);

#ifndef TELNETD_CONF_LINELEN
#define TELNETD_CONF_LINELEN 80
#endif
#ifndef TELNETD_CONF_NUMLINES
#define TELNETD_CONF_NUMLINES 25
#endif

#ifdef TELNETD_CONF_REJECT
extern char telnetd_reject_text[];
#else
static char telnetd_reject_text[] =
            "Too many connections, please try again later.";
#endif

struct telnetd_state {
  char buf[TELNETD_CONF_LINELEN + 1];
  char bufptr;
  uint16_t numsent;
  uint8_t state;
#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4
#define STATE_DONT   5
#define STATE_CLOSE  6
  struct timer silence_timer;
};
static struct telnetd_state s;

#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

struct telnetd_buf {
  char bufmem[TELNETD_CONF_NUMLINES * TELNETD_CONF_LINELEN];
  int ptr;
  int size;
};

static struct telnetd_buf buf;

static uint8_t connected;

#define MAX_SILENCE_TIME (CLOCK_SECOND * 30)

#define MIN(a, b) ((a) < (b)? (a): (b))
/*---------------------------------------------------------------------------*/
static void
buf_init(struct telnetd_buf *buf)
{
  buf->ptr = 0;
  buf->size = TELNETD_CONF_NUMLINES * TELNETD_CONF_LINELEN;
}
/*---------------------------------------------------------------------------*/
static int
buf_append(struct telnetd_buf *buf, const char *data, int len)
{
  int copylen;

  PRINTF("buf_append len %d (%d) '%.*s'\n", len, buf->ptr, len, data);
  copylen = MIN(len, buf->size - buf->ptr);
  memcpy(&buf->bufmem[buf->ptr], data, copylen);
  petsciiconv_toascii(&buf->bufmem[buf->ptr], copylen);
  buf->ptr += copylen;

  return copylen;
}
/*---------------------------------------------------------------------------*/
static void
buf_copyto(struct telnetd_buf *buf, char *to, int len)
{
  memcpy(to, &buf->bufmem[0], len);
}
/*---------------------------------------------------------------------------*/
static void
buf_pop(struct telnetd_buf *buf, int len)
{
  int poplen;

  PRINTF("buf_pop len %d (%d)\n", len, buf->ptr);
  poplen = MIN(len, buf->ptr);
  memcpy(&buf->bufmem[0], &buf->bufmem[poplen], buf->ptr - poplen);
  buf->ptr -= poplen;
}
/*---------------------------------------------------------------------------*/
static int
buf_len(struct telnetd_buf *buf)
{
  return buf->ptr;
}
/*---------------------------------------------------------------------------*/
void
telnetd_quit(void)
{
  shell_quit();
#if TELNETD_CONF_GUI
  telnetd_gui_quit();
#endif /* TELNETD_CONF_GUI */
  process_exit(&telnetd_process);
  LOADER_UNLOAD();
}
/*---------------------------------------------------------------------------*/
void
shell_prompt(char *str)
{
  buf_append(&buf, str, (int)strlen(str));
}
/*---------------------------------------------------------------------------*/
void
shell_default_output(const char *str1, int len1, const char *str2, int len2)
{
  if(len1 > 0 && str1[len1 - 1] == '\n') {
    --len1;
  }
  if(len2 > 0 && str2[len2 - 1] == '\n') {
    --len2;
  }

  /*  PRINTF("shell_default_output: %.*s %.*s\n", len1, str1, len2, str2);*/
  
#if TELNETD_CONF_GUI
  telnetd_gui_output(str1, len1, str2, len2);
#endif /* TELNETD_CONF_GUI */
  buf_append(&buf, str1, len1);
  buf_append(&buf, str2, len2);
  buf_append(&buf, "\r\n", 2);
}
/*---------------------------------------------------------------------------*/
void
shell_exit(void)
{
  s.state = STATE_CLOSE;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(telnetd_process, ev, data)
{
  PROCESS_BEGIN();
  
  shell_init();

#if TELNETD_CONF_GUI
  telnetd_gui_init();
#endif /* TELNETD_CONF_GUI */

  petsciiconv_toascii(telnetd_reject_text, strlen(telnetd_reject_text));

  tcp_listen(UIP_HTONS(23));

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      telnetd_appcall(data);
    } else if(ev == PROCESS_EVENT_EXIT) {
      telnetd_quit();
    } else {
#if TELNETD_CONF_GUI
      telnetd_gui_eventhandler(ev, data);
#endif /* TELNETD_CONF_GUI */
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
acked(void)
{
  buf_pop(&buf, s.numsent);
}
/*---------------------------------------------------------------------------*/
static void
senddata(void)
{
  int len;
  len = MIN(buf_len(&buf), uip_mss());
  PRINTF("senddata len %d\n", len);
  buf_copyto(&buf, uip_appdata, len);
  uip_send(uip_appdata, len);
  s.numsent = len;
}
/*---------------------------------------------------------------------------*/
static void
get_char(uint8_t c)
{
  PRINTF("telnetd: get_char '%c' %d %d\n", c, c, s.bufptr);

  if(c == 0) {
    return;
  }

  if(c != ISO_nl && c != ISO_cr) {
    s.buf[(int)s.bufptr] = c;
    ++s.bufptr;
  }
  if(((c == ISO_nl || c == ISO_cr) && s.bufptr > 0) ||
     s.bufptr == sizeof(s.buf)) {
    if(s.bufptr < sizeof(s.buf)) {
      s.buf[(int)s.bufptr] = 0;
    }
    petsciiconv_topetscii(s.buf, TELNETD_CONF_LINELEN);
    PRINTF("telnetd: get_char '%.*s'\n", s.bufptr, s.buf);
    shell_input(s.buf, s.bufptr);
    s.bufptr = 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
sendopt(uint8_t option, uint8_t value)
{
  char line[4];
  line[0] = (char)TELNET_IAC;
  line[1] = option;
  line[2] = value;
  line[3] = 0;
  petsciiconv_topetscii(line, 4);
  buf_append(&buf, line, 4);
}
/*---------------------------------------------------------------------------*/
static void
newdata(void)
{
  uint16_t len;
  uint8_t c;
  uint8_t *ptr;
    
  len = uip_datalen();
  PRINTF("newdata len %d '%.*s'\n", len, len, (char *)uip_appdata);

  ptr = uip_appdata;
  while(len > 0 && s.bufptr < sizeof(s.buf)) {
    c = *ptr;
    PRINTF("newdata char '%c' %d %d state %d\n", c, c, len, s.state);
    ++ptr;
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
/*---------------------------------------------------------------------------*/
void
telnetd_appcall(void *ts)
{
  if(uip_connected()) {
    if(!connected) {
      buf_init(&buf);
      s.bufptr = 0;
      s.state = STATE_NORMAL;
      connected = 1;
      shell_start();
      timer_set(&s.silence_timer, MAX_SILENCE_TIME);
      ts = (char *)0;
    } else {
      uip_send(telnetd_reject_text, strlen(telnetd_reject_text));
      ts = (char *)1;
    }
    tcp_markconn(uip_conn, ts);
  }

  if(!ts) {
    if(s.state == STATE_CLOSE) {
      s.state = STATE_NORMAL;
      uip_close();
      return;
    }
    if(uip_closed() ||
       uip_aborted() ||
       uip_timedout()) {
      shell_stop();
      connected = 0;
    }
    if(uip_acked()) {
      timer_set(&s.silence_timer, MAX_SILENCE_TIME);
      acked();
    }
    if(uip_newdata()) {
      timer_set(&s.silence_timer, MAX_SILENCE_TIME);
      newdata();
    }
    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {
      senddata();
      if(s.numsent > 0) {
	timer_set(&s.silence_timer, MAX_SILENCE_TIME);
      }
    }
    if(uip_poll()) {
      if(timer_expired(&s.silence_timer)) {
        uip_close();
        tcp_markconn(uip_conn, NULL);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
telnetd_init(void)
{
  process_start(&telnetd_process, NULL);
}
/*---------------------------------------------------------------------------*/
