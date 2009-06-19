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
 * $Id: httpd.c,v 1.2 2009/06/19 17:11:28 dak664 Exp $
 */

#include <string.h>
 
#include "contiki-net.h"

#include "webserver.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
//#include "lib/petsciiconv.h"
//#include "http-strings.h"

#include "httpd.h"

#ifndef WEBSERVER_CONF_CGI_CONNS
#define CONNS 4
#else /* WEBSERVER_CONF_CGI_CONNS */
#define CONNS WEBSERVER_CONF_CGI_CONNS
#endif /* WEBSERVER_CONF_CGI_CONNS */

#define STATE_WAITING 0
#define STATE_OUTPUT  1

//#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, (unsigned int)strlen(str))
MEMB(conns, struct httpd_state, CONNS);

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
  
  return s->len;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  
  do { 
    PSOCK_GENERATOR_SEND(&s->sout, generate, s);
    s->file.len -= s->len;
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

  if((p = (char *)httpd_fs_strchr(s->scriptptr, ISO_nl)) != NULL) {
    p += 1;
    s->scriptlen -= (unsigned short)(p - s->scriptptr);
    s->scriptptr = p;
  } else {
    s->scriptlen = 0;
  }

  /*  char *p;
  p = strchr(s->scriptptr, ISO_nl) + 1;
  s->scriptlen -= (unsigned short)(p - s->scriptptr);
  s->scriptptr = p;*/
}

/*---------------------------------------------------------------------------*/
void
memcpy_P_trim(char *toram, char *fromflash)
{
  uint8_t i;
  for (i=0;i<19;) {
    toram[i]=pgm_read_byte_near(fromflash++);
    if (toram[i]==ISO_tab) {if (i) break; else continue;}    //skip leading tabs
    if (toram[i]==ISO_space) {if (i) break; else continue;}  //skip leading spaces
    if (toram[i]==ISO_nl) break;            //nl is preferred delimiter
    if (toram[i]==ISO_cr) break;            //some editors insert cr
    if (toram[i]==0) break;                 //files are terminated with null
    i++;
  }
  toram[i]=0;

  }
/*---------------------------------------------------------------------------*/
static char filenamebuf[25],*pptr;//See below!
static
PT_THREAD(handle_script(struct httpd_state *s))
{
//  char *ptr; //one of these gets whomped unless in globals
//  char filenamebuf[25];
  
  PT_BEGIN(&s->scriptpt);

  while(s->file.len > 0) {

    /* Check if we should start executing a script. */
    if(httpd_fs_getchar(s->file.data) == ISO_percent &&
       httpd_fs_getchar(s->file.data + 1) == ISO_bang) {
      s->scriptptr = s->file.data + 3;
      s->scriptlen = s->file.len - 3; 
      memcpy_P_trim(filenamebuf,s->scriptptr);

      if(httpd_fs_getchar(s->scriptptr - 1) == ISO_colon) {
        httpd_fs_open(filenamebuf, &s->file);
        PT_WAIT_THREAD(&s->scriptpt, send_file(s));
      } else {
        PT_WAIT_THREAD(&s->scriptpt,
                       httpd_cgi(filenamebuf)(s, s->scriptptr));
      }
      next_scriptstate(s);
      
      /* The script is over, so we reset the pointers and continue
	 sending the rest of the file. */
      s->file.data = s->scriptptr;
      s->file.len = s->scriptlen;
    } else {
      /* See if we find the start of script marker in the block of HTML
	 to be sent. */

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
      PT_WAIT_THREAD(&s->scriptpt, send_part_of_file(s));
      s->file.data += s->len;
      s->file.len -= s->len;
    }
  }

  PT_END(&s->scriptpt);
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_status_P(void *pstr)
{
  uint8_t slen=strlen_P(pstr);
  memcpy_P(uip_appdata, PSTR("HTTP/1.0 "), 9);
  memcpy_P(uip_appdata+9, pstr, slen);
  slen+=9;
  memcpy_P(uip_appdata+slen, PSTR("\r\nServer: Contiki/2.0 http://www.sics.se/contiki/\r\nConnection: close\r\n"), 70);
  return slen+70;
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_header_P(void *pstr)
{
  uint8_t slen=strlen_P(pstr);
  memcpy_P(uip_appdata,PSTR("Content-type: "),14); 
  memcpy_P(uip_appdata+14, pstr, slen);
  slen+=14;
  memcpy_P(uip_appdata+slen,PSTR("\r\n\r\n"),4);
  return slen+4;
}
/*---------------------------------------------------------------------------*/

char http_htm[10]   PROGMEM ="text/html";
char http_css[ 9]   PROGMEM ="text/css";
char http_png[10]   PROGMEM ="image/png";
char http_gif[10]   PROGMEM ="image/gif";
char http_jpg[11]   PROGMEM ="image/jpeg";
char http_txt[11]   PROGMEM ="text/plain";
char http_shtml[ 6] PROGMEM =".shtml";
char index_html[12] PROGMEM ="/index.html";

static
PT_THREAD(send_headers(struct httpd_state *s, char *statushdr))
{
  char *ptr;
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout,generate_status_P,statushdr);

  ptr = strrchr(s->filename, ISO_period);

  if (pgm_read_byte_near(statushdr)=='4') {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_htm);
  } else if(ptr == NULL) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P,PSTR("application/octet-stream"));
  } else {
    ptr++;
    if(strncmp_P(ptr, &http_htm[5],3)== 0 ||strncmp_P(ptr, &http_shtml[1], 4) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_htm );
    } else if(strcmp_P(ptr, &http_css[5]) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_css );
    } else if(strcmp_P(ptr, &http_png[6]) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_png );
    } else if(strcmp_P(ptr, &http_gif[6])== 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_gif );
    } else if(strcmp_P(ptr, PSTR("jpg")) == 0) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_jpg );
    } else {
      PSOCK_GENERATOR_SEND(&s->sout, generate_header_P, http_txt);
    }
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->outputpt);
//   strcpy(s->filename,"/index.html"); //for debugging
  if(!httpd_fs_open(s->filename, &s->file)) {
    strcpy_P(s->filename, PSTR("/404.html"));
    httpd_fs_open(s->filename, &s->file);
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, PSTR("404 Not found")));
    PT_WAIT_THREAD(&s->outputpt, send_file(s));
  } else {
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, PSTR("200 OK")));
    ptr = strchr(s->filename, ISO_period);
    if((ptr != NULL && strncmp_P(ptr, http_shtml, 6) == 0) || strcmp_P(s->filename,index_html)==0) {
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
char http_get[4] PROGMEM ="GET ";
char http_ref[8] PROGMEM ="Referer:";
static
PT_THREAD(handle_input(struct httpd_state *s))
{ 
  PSOCK_BEGIN(&s->sin); 

  PSOCK_READTO(&s->sin, ISO_space);

  if(strncmp_P(s->inputbuf, http_get, 4) != 0) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ISO_space);

  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

  if(s->inputbuf[1] == ISO_space) {
    strcpy_P(s->filename, index_html);
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, &s->inputbuf[0], sizeof(s->filename));
  }

  webserver_log_file(&uip_conn->ripaddr, s->filename);

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);

    if(strncmp_P(s->inputbuf, http_ref, 8) == 0) {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
//    petsciiconv_topetscii(s->inputbuf, PSOCK_DATALEN(&s->sin) - 2);
      webserver_log(s->inputbuf);
    }
  }
  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}
/*---------------------------------------------------------------------------*/
void
httpd_appcall(void *state)
{
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
  tcp_listen(HTONS(80));
  memb_init(&conns);
  httpd_cgi_init();
}
/*---------------------------------------------------------------------------*/
