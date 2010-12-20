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
 * $Id: httpd.c,v 1.6 2010/12/20 20:06:06 dak664 Exp $
 */

#include <string.h>
 
#include "contiki-net.h"

#include "webserver.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
#include "httpd.h"
#include "raven-lcd.h"

//#include "http-strings.h"
#if COFFEE_FILES
#include "cfs-coffee-arch.h"
#endif /* COFFEE_FILES */

/* DEBUGLOGIC is a convenient way to debug in a simulator without a tcp/ip connection.
 * Break the program in the process loop and step from the entry in httpd_appcall.
 * The input file is forced to /index.html and the output directed to TCPBUF.
 * If cgi's are invoked define it in httpd-cgi.c as well!
 * psock_generator_send in /core/net/psock.c must also be modified as follows:
 * ...
 * // Wait until all data is sent and acknowledged.
 * if (!s->sendlen) break;                            //<---add this line
 * PT_YIELD_UNTIL(&s->psockpt, uip_acked() || uip_rexmit());
 * ...
 */
#define DEBUGLOGIC 0
#define DEBUG 0
#if DEBUGLOGIC
struct httpd_state *sg;
#define uip_mss(...) 512
#define uip_appdata TCPBUF
char TCPBUF[512];
#endif
#if DEBUG
#include <stdio.h>
#if HTTPD_STRING_TYPE==PROGMEM_TYPE
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF printf
#endif
#else
#define PRINTF(...)
#endif

#ifndef WEBSERVER_CONF_CGI_CONNS
#define CONNS 4
#else
#define CONNS WEBSERVER_CONF_CGI_CONNS
#endif /* WEBSERVER_CONF_CGI_CONNS */

#define STATE_WAITING 0
#define STATE_OUTPUT  1

//#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, (unsigned int)strlen(str))
MEMB(conns, struct httpd_state, CONNS);

/* MAX_SCRIPT_NAME_LENGTH should be at least be the maximum file name length+2 for %!: includes */
#define MAX_SCRIPT_NAME_LENGTH 20
#define ISO_tab     0x09
#define ISO_nl      0x0a
#define ISO_cr      0x0d
#define ISO_space   0x20
#define ISO_bang    0x21
#define ISO_percent 0x25
#define ISO_period  0x2e
#define ISO_slash   0x2f
#define ISO_colon   0x3a

/*---------------------------------------------------------------------------*/
static unsigned short
generate(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(s->file.len > uip_mss()) {
    s->len = uip_mss();
  } else {
    s->len = s->file.len;
  }
  httpd_fs_cpy(uip_appdata, s->file.data, s->len);
#if DEBUGLOGIC
  return 0;
#else
  return s->len;
#endif
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  
  do {
    PSOCK_GENERATOR_SEND(&s->sout, generate, s);
    s->file.len  -= s->len;
    s->file.data += s->len;
  } while(s->file.len > 0);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_part_of_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  static int oldfilelen, oldlen;
  static char * olddata;

  //Store stuff that gets clobbered...
  oldfilelen = s->file.len;
  oldlen = s->len;
  olddata = s->file.data;

  //How much to send  
  s->file.len = s->len;
  
  do { 
    PSOCK_GENERATOR_SEND(&s->sout, generate, s);
    s->file.len -= s->len;
    s->file.data += s->len;
  } while(s->file.len > 0);

  s->len = oldlen;
  s->file.len = oldfilelen;
  s->file.data = olddata;
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static void
next_scriptstate(struct httpd_state *s)
{
  char *p;
/* Skip over any script parameters to the beginning of the next line */
  if((p = (char *)httpd_fs_strchr(s->scriptptr, ISO_nl)) != NULL) {
    p += 1;
    s->scriptlen -= (unsigned short)(p - s->scriptptr);
    s->scriptptr  = p;
  } else {
    s->scriptlen = 0;
  }

  /*  char *p;
  p = strchr(s->scriptptr, ISO_nl) + 1;
  s->scriptlen -= (unsigned short)(p - s->scriptptr);
  s->scriptptr = p;*/
}

/*---------------------------------------------------------------------------*/
char *
get_scriptname(char *dest, char *fromfile)
{
  uint8_t i=0,skip=1;
  /* Extract a file or cgi name, trim leading spaces and replace termination with zero */
  /* Returns number of characters processed up to the next non-tab or space */
  do {
    dest[i]=httpd_fs_getchar(fromfile++);
    if (dest[i]==ISO_colon) {if (!skip) break;}                 //allow leading colons
    else if (dest[i]==ISO_tab  ) {if (skip) continue;else break;}//skip leading tabs
    else if (dest[i]==ISO_space) {if (skip) continue;else break;}//skip leading spaces
    else if (dest[i]==ISO_nl   ) break;                         //nl is preferred delimiter
    else if (dest[i]==ISO_cr   ) break;                         //some editors insert cr
    else if (dest[i]==0        ) break;                         //files are terminated with null
    else skip=0;
    i++;
  } while (i<(MAX_SCRIPT_NAME_LENGTH+1));
  fromfile--;
  while ((dest[i]==ISO_space) || (dest[i]==ISO_tab)) dest[i]=httpd_fs_getchar(++fromfile);
  dest[i]=0;
  return (fromfile);
}
/*---------------------------------------------------------------------------*/

static
PT_THREAD(handle_script(struct httpd_state *s))
{
  /* Note script includes will attach a leading : to the filename and a trailing zero */
  static char scriptname[MAX_SCRIPT_NAME_LENGTH+1],*pptr;
  static uint16_t filelength;

  PT_BEGIN(&s->scriptpt);

  filelength=s->file.len;
  while(s->file.len > 0) {
    /* Sanity check */
    if (s->file.len > filelength) break;

    /* Check if we should start executing a script, flagged by %! */
    if(httpd_fs_getchar(s->file.data) == ISO_percent &&
       httpd_fs_getchar(s->file.data + 1) == ISO_bang) {

       /* Extract name, if starts with colon include file else call cgi */
       s->scriptptr=get_scriptname(scriptname,s->file.data+2);
       s->scriptlen=s->file.len-(s->scriptptr-s->file.data);
       PRINTF("httpd: Handle script named %s\n",scriptname);
       if(scriptname[0] == ISO_colon) {
         if (httpd_fs_open(&scriptname[1], &s->file)) {
           PT_WAIT_THREAD(&s->scriptpt, send_file(s));
         }
       } else {
         PT_WAIT_THREAD(&s->scriptpt,httpd_cgi(scriptname)(s, s->scriptptr));
       }
       next_scriptstate(s);
      
      /* Reset the pointers and continue sending the current file. */
      s->file.data = s->scriptptr;
      s->file.len  = s->scriptlen;
    } else {

     /* Send file up to the next potential script */
      if(s->file.len > uip_mss()) {
        s->len = uip_mss();
      } else {
        s->len = s->file.len;
      }

      if(httpd_fs_getchar(s->file.data) == ISO_percent) {
        pptr = (char *) httpd_fs_strchr(s->file.data + 1, ISO_percent);
      } else {
        pptr = (char *) httpd_fs_strchr(s->file.data, ISO_percent);
      }

      if(pptr != NULL && pptr != s->file.data) {
        s->len = (int)(pptr - s->file.data);
        if(s->len >= uip_mss()) {
          s->len = uip_mss();
        }
      }
      PRINTF("httpd: Sending %u bytes from 0x%04x\n",s->file.len,(unsigned int)s->file.data);
      PT_WAIT_THREAD(&s->scriptpt, send_part_of_file(s));
      s->file.data += s->len;
      s->file.len  -= s->len;
    }
  }

  PT_END(&s->scriptpt);
}
/*---------------------------------------------------------------------------*/
const char httpd_http[]     HTTPD_STRING_ATTR = "HTTP/1.0 ";
const char httpd_server[]   HTTPD_STRING_ATTR = "\r\nServer: Contiki/2.0 http://www.sics.se/contiki/\r\nConnection: close\r\n";
static unsigned short
generate_status(void *sstr)
{
  uint8_t slen=httpd_strlen((char *)sstr);
  httpd_memcpy(uip_appdata, httpd_http, sizeof(httpd_http)-1);
  httpd_memcpy(uip_appdata+sizeof(httpd_http)-1, (char *)sstr, slen);
  slen+=sizeof(httpd_http)-1;
  httpd_memcpy(uip_appdata+slen, httpd_server, sizeof(httpd_server)-1);
#if DEBUGLOGIC
  return 0;
#else
  return slen+sizeof(httpd_server)-1;
#endif
}
/*---------------------------------------------------------------------------*/
const char httpd_content[]  HTTPD_STRING_ATTR = "Content-type: ";
const char httpd_crlf[]     HTTPD_STRING_ATTR = "\r\n\r\n";
static unsigned short
generate_header(void *hstr)
{
  uint8_t slen=httpd_strlen((char *)hstr);
  httpd_memcpy(uip_appdata,httpd_content,sizeof(httpd_content)-1); 
  httpd_memcpy(uip_appdata+sizeof(httpd_content)-1, (char *)hstr, slen);
  slen+=sizeof(httpd_content)-1;
  httpd_memcpy(uip_appdata+slen,httpd_crlf,sizeof(httpd_crlf)-1);
#if DEBUGLOGIC
  return 0;
#else
  return slen+4;
#endif
}
/*---------------------------------------------------------------------------*/
char http_htm[10]   PROGMEM ="text/html";
char http_css[ 9]   PROGMEM ="text/css";
const char httpd_mime_htm[] HTTPD_STRING_ATTR = "text/html";
const char httpd_mime_css[] HTTPD_STRING_ATTR = "text/css";
const char httpd_mime_png[] HTTPD_STRING_ATTR = "image/png";
const char httpd_mime_gif[] HTTPD_STRING_ATTR = "image/gif";
const char httpd_mime_jpg[] HTTPD_STRING_ATTR = "image/jpeg";
const char httpd_mime_txt[] HTTPD_STRING_ATTR = "text/plain";
const char httpd_mime_bin[] HTTPD_STRING_ATTR = "application/octet-stream";
const char httpd_jpg     [] HTTPD_STRING_ATTR = "jpg";
const char httpd_shtml   [] HTTPD_STRING_ATTR = ".shtml";

static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  char *ptr;
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_status, (char *)statushdr);

  ptr = strrchr(s->filename, ISO_period);
  if (httpd_strncmp("4", statushdr, 1)==0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_htm  );
  } else if(ptr == NULL) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_bin );
  } else {
    ptr++;
    if(httpd_strncmp(ptr, &httpd_mime_htm[5],3)== 0 ||httpd_strncmp(ptr, &httpd_shtml[1], 4) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_htm );
    } else if(httpd_strcmp(ptr, &httpd_mime_css[5]) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_css );
    } else if(httpd_strcmp(ptr, &httpd_mime_png[6]) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_png );
    } else if(httpd_strcmp(ptr, &httpd_mime_gif[6])== 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_gif );
    } else if(httpd_strcmp(ptr, httpd_mime_jpg) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_jpg );
    } else {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header, &httpd_mime_txt);
    }
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
const char httpd_indexfn [] HTTPD_STRING_ATTR = "/index.html";
const char httpd_404fn   [] HTTPD_STRING_ATTR = "/404.html";
const char httpd_404notf [] HTTPD_STRING_ATTR = "404 Not found";
const char httpd_200ok   [] HTTPD_STRING_ATTR = "200 OK";
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->outputpt);
#if DEBUGLOGIC
   httpd_strcpy(s->filename,httpd_indexfn);
#endif
  if(!httpd_fs_open(s->filename, &s->file)) {
    httpd_strcpy(s->filename, httpd_404fn);
    httpd_fs_open(s->filename, &s->file);
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, httpd_404notf));
    PT_WAIT_THREAD(&s->outputpt, send_file(s));
  } else {
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, httpd_200ok));
    ptr = strchr(s->filename, ISO_period);
    if((ptr != NULL && httpd_strncmp(ptr, httpd_shtml, 6) == 0) || httpd_strcmp(s->filename,httpd_indexfn)==0) {
      PT_INIT(&s->scriptpt);
      PT_WAIT_THREAD(&s->outputpt, handle_script(s));
    } else {
      PT_WAIT_THREAD(&s->outputpt, send_file(s));
    }
  }
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
const char httpd_get[] HTTPD_STRING_ATTR = "GET ";
const char httpd_ref[] HTTPD_STRING_ATTR = "Referer:";
static
PT_THREAD(handle_input(struct httpd_state *s))
{

  PSOCK_BEGIN(&s->sin); 

  PSOCK_READTO(&s->sin, ISO_space);

  if(httpd_strncmp(s->inputbuf, httpd_get, 4) != 0) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ISO_space);

  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

  if(s->inputbuf[1] == ISO_space) {
    httpd_strcpy(s->filename, httpd_indexfn);
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, &s->inputbuf[0], sizeof(s->filename));
{
    /* Look for ?, if found strip file name and send any following text to the LCD */
    uint8_t i;
    for (i=0;i<sizeof(s->inputbuf);i++) {
      if (s->inputbuf[i]=='?') {
        raven_lcd_show_text(&s->inputbuf[i]);
        if (i<sizeof(s->filename)) s->filename[i]=0;
 //     s->inputbuf[i]=0; //allow multiple beeps with multiple ?'s
      }
      if (s->inputbuf[i]==0) break;
    }
}
  }

  webserver_log_file(&uip_conn->ripaddr, s->filename);

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);

    if(httpd_strncmp(s->inputbuf, httpd_ref, 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
      petsciiconv_topetscii(s->inputbuf, PSOCK_DATALEN(&s->sin) - 2);
      webserver_log(s->inputbuf);
    }
  }
  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
#if DEBUGLOGIC
  handle_output(s);
#else
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
httpd_appcall(void *state)
{
#if DEBUGLOGIC
  struct httpd_state *s;   //Enter here for debugging with output directed to TCPBUF
  s = sg = (struct httpd_state *)memb_alloc(&conns);
  if (1) {
#else
  struct httpd_state *s = (struct httpd_state *)state;
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      return;
    }
#endif
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->state = STATE_WAITING;
    /*    timer_set(&s->timer, CLOCK_SECOND * 100);*/
    s->timer = 0;
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      ++s->timer;
      if(s->timer >= 20) {
        uip_abort();
        memb_free(&conns, s);
      }
    } else {
      s->timer = 0;
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
void
httpd_init(void)
{
  tcp_listen(UIP_HTONS(80));
  memb_init(&conns);
  httpd_cgi_init();
}
/*---------------------------------------------------------------------------*/
