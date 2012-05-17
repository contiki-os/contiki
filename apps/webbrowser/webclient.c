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
 * This file is part of the "contiki" web browser.
 *
 *
 */

#include <string.h>

#include "contiki-net.h"
#include "www.h"

#include "webclient.h"

#define WEBCLIENT_TIMEOUT 100

#define WEBCLIENT_STATE_STATUSLINE 0
#define WEBCLIENT_STATE_HEADERS    1
#define WEBCLIENT_STATE_DATA       2
#define WEBCLIENT_STATE_CLOSE      3

#define HTTPFLAG_NONE   0
#define HTTPFLAG_OK     1
#define HTTPFLAG_MOVED  2
#define HTTPFLAG_ERROR  3


#define ISO_nl       0x0a
#define ISO_cr       0x0d
#define ISO_space    0x20

struct webclient_state {
  uint8_t timer;
  uint8_t state;
  uint8_t httpflag;

  uint16_t port;
  char host[40];
  char file[WWW_CONF_MAX_URLLEN];
  uint16_t getrequestptr;
  uint16_t getrequestleft;
  
  char httpheaderline[200];
  uint16_t httpheaderlineptr;

  char mimetype[32];
};

static struct webclient_state s;

/*-----------------------------------------------------------------------------------*/
char *
webclient_mimetype(void)
{
  return s.mimetype;
}
/*-----------------------------------------------------------------------------------*/
char *
webclient_filename(void)
{
  return s.file;
}
/*-----------------------------------------------------------------------------------*/
char *
webclient_hostname(void)
{
  return s.host;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
webclient_port(void)
{
  return s.port;
}
/*-----------------------------------------------------------------------------------*/
void
webclient_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
static void
init_connection(void)
{
  s.state = WEBCLIENT_STATE_STATUSLINE;

  s.getrequestleft = sizeof(http_get) - 1 + 1 +
    sizeof(http_10) - 1 +
    sizeof(http_crnl) - 1 +
    sizeof(http_host) - 1 +
    sizeof(http_crnl) - 1 +
    (uint16_t)strlen(http_user_agent_fields) +
    (uint16_t)strlen(s.file) + (uint16_t)strlen(s.host);
  s.getrequestptr = 0;

  s.httpheaderlineptr = 0;
}
/*-----------------------------------------------------------------------------------*/
void
webclient_close(void)
{
  s.state = WEBCLIENT_STATE_CLOSE;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
webclient_get(const char *host, uint16_t port, const char *file)
{
  uip_ipaddr_t addr;
  struct uip_conn *conn;
  uip_ipaddr_t *ipaddr;
  
  /* First check if the host is an IP address. */
  ipaddr = &addr;
  if(uiplib_ipaddrconv(host, &addr) == 0) {
#if UIP_UDP
    if(resolv_lookup(host,&ipaddr) != RESOLV_STATUS_CACHED) {
      return 0;
    }
#else /* UIP_UDP */
    return 0;
#endif /* UIP_UDP */
  }
  
  conn = tcp_connect(ipaddr, uip_htons(port), NULL);
  
  if(conn == NULL) {
    return 0;
  }
  
  s.port = port;
  strncpy(s.file, file, sizeof(s.file));
  strncpy(s.host, host, sizeof(s.host));
  
  init_connection();
  return 1;
}
/*-----------------------------------------------------------------------------------*/
/* Copy data into a "window", specified by the windowstart and
   windowend variables. Only data that fits within the window is
   copied. This function is used to copy data into the uIP buffer, which
   typically is smaller than the data that is to be copied.
*/
static unsigned char *windowptr;
static int windowstart, windowend;
static int
window_copy(int curptr, const char *data, unsigned char datalen)
{
  int len;

  if(windowstart == windowend) {
    return curptr + datalen;
  }
  
  if(curptr + datalen < windowstart) {
    /* If all the data is before the window, we do not copy the
       data. */
    return curptr + datalen;
  }

  if(curptr > windowend) {
    /* If all the data is after the window, we do not copy the data. */
    return curptr + datalen;
  }

  len = datalen;
  
  /* Trim off data before the window. */
  data += windowstart - curptr;
  len  -= windowstart - curptr;

  /* Trim off data after the window. */
  if(len > windowend - windowstart) {
    len = windowend - windowstart;
  }

  strncpy(windowptr + windowstart, data, len);
  windowstart += len;

  return curptr + datalen;
}
/*-----------------------------------------------------------------------------------*/
static void
senddata(void)
{
  uint16_t len;
  int curptr;
  
  if(s.getrequestleft > 0) {

    windowstart = s.getrequestptr;
    curptr = 0;
    windowend = windowstart + uip_mss();
    windowptr = (char *)uip_appdata - windowstart;

    curptr = window_copy(curptr, http_get, sizeof(http_get) - 1);
    curptr = window_copy(curptr, s.file, (unsigned char)strlen(s.file));
    curptr = window_copy(curptr, " ", 1);
    curptr = window_copy(curptr, http_10, sizeof(http_10) - 1);

    curptr = window_copy(curptr, http_crnl, sizeof(http_crnl) - 1);
    
    curptr = window_copy(curptr, http_host, sizeof(http_host) - 1);
    curptr = window_copy(curptr, s.host, (unsigned char)strlen(s.host));
    curptr = window_copy(curptr, http_crnl, sizeof(http_crnl) - 1);

    curptr = window_copy(curptr, http_user_agent_fields,
		       (unsigned char)strlen(http_user_agent_fields));
    
    len = s.getrequestleft > uip_mss()?
      uip_mss():
      s.getrequestleft;
    uip_send(uip_appdata, len);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
acked(void)
{
  uint16_t len;
  
  if(s.getrequestleft > 0) {
    len = s.getrequestleft > uip_mss()?
      uip_mss():
      s.getrequestleft;
    s.getrequestleft -= len;
    s.getrequestptr += len;
  }
}
/*-----------------------------------------------------------------------------------*/
static uint16_t
parse_statusline(uint16_t len)
{
  char *cptr;
  
  while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) {
    s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
    uip_appdata = (char *)uip_appdata + 1;
    --len;
    if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) {

      if((strncmp(s.httpheaderline, http_10,
		  sizeof(http_10) - 1) == 0) ||
	 (strncmp(s.httpheaderline, http_11,
		  sizeof(http_11) - 1) == 0)) {
	cptr = &(s.httpheaderline[9]);
	s.httpflag = HTTPFLAG_NONE;
	if(strncmp(cptr, http_200, sizeof(http_200) - 1) == 0) {
	  /* 200 OK */
	  s.httpflag = HTTPFLAG_OK;
	} else if(strncmp(cptr, http_301, sizeof(http_301) - 1) == 0 ||
		  strncmp(cptr, http_302, sizeof(http_302) - 1) == 0) {
	  /* 301 Moved permanently or 302 Found. Location: header line
	     will contain thw new location. */
	  s.httpflag = HTTPFLAG_MOVED;
	} else {
	  s.httpheaderline[s.httpheaderlineptr - 1] = 0;
	}
      } else {
	uip_abort();
	webclient_aborted();
	return 0;
      }
      
      /* We're done parsing the status line, so we reset the pointer
	 and start parsing the HTTP headers.*/
      s.httpheaderlineptr = 0;
      s.state = WEBCLIENT_STATE_HEADERS;
      break;
    } else {
      ++s.httpheaderlineptr;
    }
  }
  return len;
}
/*-----------------------------------------------------------------------------------*/
static char
casecmp(char *str1, const char *str2, char len)
{
  static char c;
  
  while(len > 0) {
    c = *str1;
    /* Force lower-case characters. */
    if(c & 0x40) {
      c |= 0x20;
    }
    if(*str2 != c) {
      return 1;
    }
    ++str1;
    ++str2;
    --len;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static uint16_t
parse_headers(uint16_t len)
{
  char *cptr;
  static unsigned char i;
  
  while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) {
    s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
    uip_appdata = (char *)uip_appdata + 1;
    --len;
    if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) {
      /* We have an entire HTTP header line in s.httpheaderline, so
	 we parse it. */
      if(s.httpheaderline[0] == ISO_cr) {
	/* This was the last header line (i.e., and empty "\r\n"), so
	   we are done with the headers and proceed with the actual
	   data. */
	s.state = WEBCLIENT_STATE_DATA;
	return len;
      }

      s.httpheaderline[s.httpheaderlineptr - 1] = 0;
      /* Check for specific HTTP header fields. */
      if(casecmp(s.httpheaderline, http_content_type,
		     sizeof(http_content_type) - 1) == 0) {
	/* Found Content-type field. */
	cptr = strchr(s.httpheaderline, ';');
	if(cptr != NULL) {
	  *cptr = 0;
	}
	strncpy(s.mimetype, s.httpheaderline +
		sizeof(http_content_type) - 1, sizeof(s.mimetype));
      } else if(casecmp(s.httpheaderline, http_location,
			    sizeof(http_location) - 1) == 0) {
	cptr = s.httpheaderline +
	  sizeof(http_location) - 1;
	
	if(strncmp(cptr, http_http, 7) == 0) {
	  cptr += 7;
	  for(i = 0; i < s.httpheaderlineptr - 7; ++i) {
	    if(*cptr == 0 ||
	       *cptr == '/' ||
	       *cptr == ' ' ||
	       *cptr == ':') {
	      s.host[i] = 0;
	      break;
	    }
	    s.host[i] = *cptr;
	    ++cptr;
	  }
	}
	strncpy(s.file, cptr, sizeof(s.file));
	/*	s.file[s.httpheaderlineptr - i] = 0;*/
      }


      /* We're done parsing, so we reset the pointer and start the
	 next line. */
      s.httpheaderlineptr = 0;
    } else {
      ++s.httpheaderlineptr;
    }
  }
  return len;
}
/*-----------------------------------------------------------------------------------*/
static void
newdata(void)
{
  uint16_t len;

  len = uip_datalen();

  if(s.state == WEBCLIENT_STATE_STATUSLINE) {
    len = parse_statusline(len);
  }
  
  if(s.state == WEBCLIENT_STATE_HEADERS && len > 0) {
    len = parse_headers(len);
  }

  if(len > 0 && s.state == WEBCLIENT_STATE_DATA &&
     s.httpflag != HTTPFLAG_MOVED) {
    webclient_datahandler((char *)uip_appdata, len);
  }
}
/*-----------------------------------------------------------------------------------*/
void
webclient_appcall(void *state)
{
  char *dataptr;
  
  if(uip_connected()) {
    s.timer = 0;
    s.state = WEBCLIENT_STATE_STATUSLINE;
    senddata();
    webclient_connected();
    tcp_markconn(uip_conn, &s);
    return;
  }

  if(uip_timedout()) {
    webclient_timedout();
  }

  if(uip_aborted()) {
    webclient_aborted();
  }

  if(state == NULL) {
    uip_abort();
    return;
  }

  if(s.state == WEBCLIENT_STATE_CLOSE) {
    webclient_closed();
    uip_abort();
    return;
  }


  /* The acked() and newdata() functions may alter the uip_appdata
     ptr, so we need to store it in the "dataptr" variable so that we
     can restore it before the senddata() function is called. */  
  dataptr = uip_appdata;
  
  if(uip_acked()) {
    s.timer = 0;
    acked();
  }
  if(uip_newdata()) {
    s.timer = 0;
    newdata();
  }

  uip_appdata = dataptr;
  
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata();
  } else if(uip_poll()) {
    ++s.timer;
    if(s.timer == WEBCLIENT_TIMEOUT) {
      webclient_timedout();
      uip_abort();
      return;
    }
        /*    senddata();*/
  }

  if(uip_closed()) {
    tcp_markconn(uip_conn, NULL);
    if(s.httpflag != HTTPFLAG_MOVED) {
      /* Send NULL data to signal EOF. */
      webclient_datahandler(NULL, 0);
    } else {
      /*      conn = uip_connect(uip_conn->ripaddr, s.port);
      if(conn != NULL) {
	dispatcher_markconn(conn, NULL);
	init_connection();
	}*/
#if UIP_UDP
      if(resolv_lookup(s.host, NULL) != RESOLV_STATUS_CACHED) {
	resolv_query(s.host);
      }
#endif /* UIP_UDP */
      webclient_get(s.host, s.port, s.file);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
