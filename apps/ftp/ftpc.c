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
 */
#include "contiki.h"
#include "ftpc.h"
#include "lib/petsciiconv.h"

#include <string.h>
#include <stdio.h>

#define ISO_nl 0x0a
#define ISO_cr 0x0d

#define DATAPORT 6510

#define MAX_FILENAMELEN 32

struct ftp_dataconn {
  unsigned char type;
  unsigned char conntype;
#define CONNTYPE_LIST 0
#define CONNTYPE_FILE 1

  uint16_t port;
  
  unsigned char filenameptr;
  char filename[MAX_FILENAMELEN];

  
};

struct ftp_connection {
  unsigned char type;
#define TYPE_CONTROL 1
#define TYPE_DATA    2
#define TYPE_ABORT   3
#define TYPE_CLOSE   4

  unsigned char state;
#define STATE_NONE         0
#define STATE_INITIAL      1
#define STATE_SEND_USER    2
#define STATE_USER_SENT    3
#define STATE_SEND_PASS    4
#define STATE_PASS_SENT    5
#define STATE_SEND_PORT    6
#define STATE_PORT_SENT    7
#define STATE_SEND_OPTIONS 8
#define STATE_OPTION_SENT  9
#define STATE_CONNECTED   10
#define STATE_SEND_NLST   11
#define STATE_NLST_SENT   12
#define STATE_SEND_RETR   13
#define STATE_RETR_SENT   14
  
#define STATE_SEND_CWD    15
#define STATE_CWD_SENT    16

#define STATE_SEND_CDUP   17
#define STATE_CDUP_SENT   18

#define STATE_SEND_QUIT   19
#define STATE_QUIT_SENT   20
  
  unsigned char connected_confirmed;
  
  struct ftp_dataconn dataconn;
  
  char code[3];
  unsigned char codeptr;

  unsigned char optionsptr;

  char filename[MAX_FILENAMELEN];

};

#define NUM_OPTIONS 1
static const struct {
  unsigned char num;
  char *commands[NUM_OPTIONS];
} options = {
  NUM_OPTIONS,
  {"TYPE I\r\n"}
};

static struct ftp_connection *waiting_for_dataconn;

MEMB(connections, struct ftp_connection, 1);

/*---------------------------------------------------------------------------*/
void
ftpc_init(void)
{
  memb_init(&connections);
  /*  tcp_listen(UIP_HTONS(DATAPORT));*/
}
/*---------------------------------------------------------------------------*/
void *
ftpc_connect(uip_ipaddr_t *ipaddr, uint16_t port)
{
  struct ftp_connection *c;

  c = (struct ftp_connection *)memb_alloc(&connections);
  if(c == NULL) {
    return NULL;
  }
  c->type = TYPE_CONTROL;
  c->state = STATE_INITIAL;
  c->connected_confirmed = 0;
  c->codeptr = 0;
  c->dataconn.type = TYPE_DATA;
  c->dataconn.port = DATAPORT;
  tcp_listen(UIP_HTONS(DATAPORT));

  if(tcp_connect(ipaddr, port, c) == NULL) {
    memb_free(&connections, c);
    return NULL;
  }

  return c;
}
/*---------------------------------------------------------------------------*/
static void
handle_input(struct ftp_connection *c)
{
  int code;

  code = (c->code[0] - '0') * 100 +
    (c->code[1] - '0') * 10 +
    (c->code[2] - '0');
  /*  printf("Handle input code %d state %d\n", code, c->state);*/

  if(c->state == STATE_INITIAL) {
    if(code == 220) {
      c->state = STATE_SEND_USER;
    }
  } else if(c->state == STATE_USER_SENT) {
    if(code == 331) {
      c->state = STATE_SEND_PASS;
    }
  } else if(c->state == STATE_PASS_SENT) {
    if(code == 230) {
      c->state = STATE_SEND_OPTIONS;
      c->optionsptr = 0;
    }
  } else if(c->state == STATE_PORT_SENT) {
    c->state = STATE_CONNECTED;
    if(c->connected_confirmed == 0) {
      ftpc_connected(c);
      c->connected_confirmed = 1;
    }
  } else if(c->state == STATE_OPTION_SENT) {
    if(c->optionsptr >= options.num) {
      c->state = STATE_SEND_PORT;
    } else {
      c->state = STATE_SEND_OPTIONS;
    }
  } else if((c->state == STATE_NLST_SENT ||
	     c->state == STATE_RETR_SENT ||
	     c->state == STATE_CONNECTED)) {
    if(code == 226 || code == 550) {
      tcp_unlisten(uip_htons(c->dataconn.port));
      ++c->dataconn.port;
      tcp_listen(uip_htons(c->dataconn.port));
      c->state = STATE_SEND_PORT;
    }

    if(code == 550) {
      ftpc_list_file(NULL);
    }
  } else if(c->state == STATE_CWD_SENT ||
	    c->state == STATE_CDUP_SENT) {
    c->state = STATE_CONNECTED;
    ftpc_cwd_done(code);
    /*  } else if(c->state == STATE_) {
	c->state = STATE_CONNECTED;*/
  }
}
/*---------------------------------------------------------------------------*/
static void
newdata(struct ftp_connection *c)
{
  uint16_t i;
  uint8_t d;
  
  for(i = 0; i < uip_datalen(); ++i) {
    d = ((char *)uip_appdata)[i];
    if(c->codeptr < sizeof(c->code)) {
      c->code[c->codeptr] = d;
      ++c->codeptr;
    }

    if(d == ISO_nl) {
      handle_input(c);
      c->codeptr = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
acked(struct ftp_connection *c)
{
  switch(c->state) {
  case STATE_SEND_USER:
    c->state = STATE_USER_SENT;
    break;
  case STATE_SEND_PASS:
    c->state = STATE_PASS_SENT;
    break;
  case STATE_SEND_PORT:
    c->state = STATE_PORT_SENT;
    break;
  case STATE_SEND_OPTIONS:
    ++c->optionsptr;
    c->state = STATE_OPTION_SENT;
    break;
  case STATE_SEND_NLST:
    c->state = STATE_NLST_SENT;
    break;
  case STATE_SEND_RETR:
    c->state = STATE_RETR_SENT;
    break;
  case STATE_SEND_CWD:
    c->state = STATE_CWD_SENT;
    break;
  case STATE_SEND_CDUP:
    c->state = STATE_CDUP_SENT;
    break;
  case STATE_SEND_QUIT:
    c->state = STATE_QUIT_SENT;
    uip_close();
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
senddata(struct ftp_connection *c)
{
  uint16_t len;
  
  switch(c->state) {
  case STATE_SEND_USER:
    len = 5 + (uint16_t)strlen(ftpc_username()) + 2;
    strcpy(uip_appdata, "USER ");
    strncpy((char *)uip_appdata + 5, ftpc_username(), uip_mss() - 5 - 2);
    strcpy((char *)uip_appdata + len - 2, "\r\n");
    break;
  case STATE_SEND_PASS:
    len = 5 + (uint16_t)strlen(ftpc_password()) + 2;
    strcpy(uip_appdata, "PASS ");
    strncpy((char *)uip_appdata + 5, ftpc_password(), uip_mss() - 5 - 2);
    strcpy((char *)uip_appdata + len - 2, "\r\n");
    break;
  case STATE_SEND_PORT:
    len = sprintf(uip_appdata, "PORT %d,%d,%d,%d,%d,%d\n",
		  uip_ipaddr_to_quad(&uip_hostaddr),
		  (c->dataconn.port) >> 8,
		  (c->dataconn.port) & 0xff);
    break;
  case STATE_SEND_OPTIONS:
    len = (uint16_t)strlen(options.commands[c->optionsptr]);
    strcpy(uip_appdata, options.commands[c->optionsptr]);
    break;
  case STATE_SEND_NLST:
    len = 6;
    strcpy(uip_appdata, "NLST\r\n");
    break;
  case STATE_SEND_RETR:
    len = sprintf(uip_appdata, "RETR %s\r\n", c->filename);
    break;
  case STATE_SEND_CWD:
    len = sprintf(uip_appdata, "CWD %s\r\n", c->filename);
    break;
  case STATE_SEND_CDUP:
    len = 6;
    strcpy(uip_appdata, "CDUP\r\n");
    break;
  case STATE_SEND_QUIT:
    len = 6;
    strcpy(uip_appdata, "QUIT\r\n");
    break;
  default:
    return;
  }

  petsciiconv_toascii(uip_appdata, len);
  uip_send(uip_appdata, len);
}
/*---------------------------------------------------------------------------*/
void
ftpc_appcall(void *state)
{
  int i, t;
  struct ftp_connection *c = (struct ftp_connection *)state;
  struct ftp_dataconn *d = (struct ftp_dataconn *)state;
    
  if(uip_connected()) {
    if(state == NULL) {
      if(waiting_for_dataconn != NULL) {
	d = &waiting_for_dataconn->dataconn;
	waiting_for_dataconn = NULL;
	tcp_markconn(uip_conn, d);
	d->filenameptr = 0;
	
      } else {
	uip_abort();
      }
    } else {
      /*      tcp_listen(uip_conn->lport);*/
      senddata(c);
    }
    return;
  }

  if(c->type == TYPE_ABORT) {
    uip_abort();
    return;
  }

  if(c->type == TYPE_CLOSE) {
    uip_close();
    c->type = TYPE_CONTROL;
    return;
  }

  if(c->type == TYPE_CONTROL) {
    if(uip_closed()) {
      c->dataconn.type = TYPE_ABORT;
      ftpc_closed();
      memb_free(&connections, c);
    }
    if(uip_aborted()) {
      c->dataconn.type = TYPE_ABORT;
      ftpc_aborted();
      memb_free(&connections, c);
    }
    if(uip_timedout()) {
      c->dataconn.type = TYPE_ABORT;
      ftpc_timedout();
      memb_free(&connections, c);
    }


    if(uip_acked()) {
      acked(c);
    }
    if(uip_newdata()) {
      newdata(c);
    }
    if(uip_rexmit() ||
       uip_newdata() ||
     uip_acked()) {
      senddata(c);
    } else if(uip_poll()) {
      senddata(c);
    }
  } else {
    if(d->conntype == CONNTYPE_LIST) {
      if(uip_newdata()) {
	for(i = 0; i < uip_datalen(); ++i) {
	  t = ((char *)uip_appdata)[i];
	  
	  if(d->filenameptr < sizeof(d->filename) - 1 &&
	     t != ISO_cr &&
	     t != ISO_nl) {
	    d->filename[d->filenameptr] = t;
	    ++d->filenameptr;
	  }
	  
	  if(t == ISO_nl) {
	    d->filename[d->filenameptr] = 0;
	    petsciiconv_topetscii(d->filename, d->filenameptr);
            ftpc_list_file(d->filename);
	    d->filenameptr = 0;
	  }

	}
      }
      if(uip_closed()) {
	ftpc_list_file(NULL);
      }
    } else {
      if(uip_newdata()) {
	ftpc_data(uip_appdata, uip_datalen());
	/*	printf("Received %d data bytes: '%s'\n",
		uip_datalen(), uip_appdata);*/
      } else if(uip_closed() || uip_timedout() || uip_aborted()) {
	ftpc_data(NULL, 0);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
char
ftpc_list(void *conn)
{
  struct ftp_connection *c;

  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }

  c->state = STATE_SEND_NLST;
  c->dataconn.conntype = CONNTYPE_LIST;
  waiting_for_dataconn = c;
  return 1;
}
/*---------------------------------------------------------------------------*/
char
ftpc_get(void *conn, char *filename)
{
  struct ftp_connection *c;

  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }

  strncpy(c->filename, filename, sizeof(c->filename));
  petsciiconv_toascii(c->filename, sizeof(c->filename));

  c->state = STATE_SEND_RETR;
  c->dataconn.conntype = CONNTYPE_FILE;
  waiting_for_dataconn = c;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
ftpc_close(void *conn)
{
  struct ftp_connection *c;
  
  c = conn;
  
  if(c == NULL) {
    return;
  }

  c->type = TYPE_CLOSE;
}
/*---------------------------------------------------------------------------*/
void
ftpc_cwd(void *conn, char *dirname)
{
  struct ftp_connection *c;
  
  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return;
  }

  strncpy(c->filename, dirname, sizeof(c->filename));
  c->state = STATE_SEND_CWD;
}
/*---------------------------------------------------------------------------*/
void
ftpc_cdup(void *conn)
{
  struct ftp_connection *c;
  
  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return;
  }
  
  c->state = STATE_SEND_CDUP;
}
/*---------------------------------------------------------------------------*/
