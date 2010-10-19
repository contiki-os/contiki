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
 * $Id: ircc.c,v 1.5 2010/10/19 18:29:03 adamdunkels Exp $
 */

#include "contiki.h"
#include "ircc.h"

#include "ircc-strings.h"

#include "lib/petsciiconv.h"

#include <string.h>

#ifdef IRC_CONF_SYSTEM_STRING
#define IRC_SYSTEM_STRING IRC_CONF_SYSTEM_STRING
#else
#define IRC_SYSTEM_STRING "Contiki"
#endif

#define PORT 6667

#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, (unsigned int)strlen(str))

#define ISO_space 0x20
#define ISO_bang  0x21
#define ISO_at    0x40
#define ISO_cr    0x0d
#define ISO_nl    0x0a
#define ISO_colon 0x3a
#define ISO_O     0x4f

enum {
  COMMAND_NONE,
  COMMAND_JOIN,
  COMMAND_PART,  
  COMMAND_MSG,
  COMMAND_ACTIONMSG,
  COMMAND_LIST,
  COMMAND_QUIT
};

/*---------------------------------------------------------------------------*/
void
ircc_init(void)
{

}
/*---------------------------------------------------------------------------*/
static char *
copystr(char *dest, const char *src, size_t n)
{
  size_t len;

  len = strlen(src);
  strncpy(dest, src, n);

  if(len > n) {
    return dest + n;
  } else {
    return dest + len;
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(setup_connection(struct ircc_state *s))
{
  char *ptr;

  
  PSOCK_BEGIN(&s->s);
  
  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_strings_nick, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->nick, (int)sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_crnl_user, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->nick, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_contiki, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->server, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_colon_contiki, sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(join_channel(struct ircc_state *s))
{
  PSOCK_BEGIN(&s->s);
  
  SEND_STRING(&s->s, ircc_strings_join);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_strings_crnl);

  ircc_sent(s);
  
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(part_channel(struct ircc_state *s))
{
  PSOCK_BEGIN(&s->s);

  SEND_STRING(&s->s, ircc_strings_part);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_strings_crnl);

  ircc_sent(s);
  
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(list_channel(struct ircc_state *s))
{
  PSOCK_BEGIN(&s->s);

  SEND_STRING(&s->s, ircc_strings_list);
  SEND_STRING(&s->s, s->channel);
  SEND_STRING(&s->s, ircc_strings_crnl);

  ircc_sent(s);
  
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_message(struct ircc_state *s))
{
  char *ptr;
  
  PSOCK_BEGIN(&s->s);

  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_strings_privmsg, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->channel, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_colon, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->msg, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_crnl, sizeof(s->outputbuf) - (ptr - s->outputbuf));

  SEND_STRING(&s->s, s->outputbuf);

  ircc_sent(s);
  
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_actionmessage(struct ircc_state *s))
{
  char *ptr;
  
  PSOCK_BEGIN(&s->s);

  ptr = s->outputbuf;
  ptr = copystr(ptr, ircc_strings_privmsg, sizeof(s->outputbuf));
  ptr = copystr(ptr, s->channel, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_colon, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, ircc_strings_action, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  ptr = copystr(ptr, s->msg, sizeof(s->outputbuf) - (ptr - s->outputbuf)); 
  ptr = copystr(ptr, ircc_strings_ctcpcrnl, sizeof(s->outputbuf) - (ptr - s->outputbuf));
  

  SEND_STRING(&s->s, s->outputbuf);

  ircc_sent(s);
  
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
struct parse_result {
  char *msg;
  
  char *user;
  char *host;
  char *name;
  char *command;
  char *middle;
  char *trailing;
};
static struct parse_result r;
static void
parse_whitespace(void)
{
  while(*r.msg == ISO_space) ++r.msg;
}
static void
parse_word(void)
{
  char *ptr;
  ptr = strchr(r.msg, ISO_space);
  if(ptr != NULL) {
    r.msg = ptr;
  }  
}
static void
parse_user(void)
{
  parse_whitespace();
  r.user = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}
static void
parse_host(void)
{
  parse_whitespace();
  r.host = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

static void
parse_name(void)
{
  parse_whitespace();
  r.name = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

static void
parse_prefix(void)
{
  parse_name();
  if(*r.msg == ISO_bang) {
    ++r.msg;
    parse_user();
  }
  if(*r.msg == ISO_at) {
    ++r.msg;
    parse_host();
  }
}

static void
parse_command(void)
{
  parse_whitespace();
  r.command = r.msg;
  parse_word();
  *r.msg = 0;
  ++r.msg;
}

/*static void
parse_trailing(void)
{
  r.trailing = r.msg;
  while(*r.msg != 0 && *r.msg != ISO_cr && *r.msg != ISO_nl) ++r.msg;
  *r.msg = 0;
  ++r.msg;
}*/

static void
parse_params(void)
{
  char *ptr;

  parse_whitespace();
  ptr = strchr(r.msg, ISO_colon);
  if(ptr != NULL) {
    r.trailing = ptr + 1;
    ptr = strchr(ptr, ISO_cr);
    if(ptr != NULL) {
      *ptr = 0;
    }
  }
}

static void
parse(char *msg, struct parse_result *dummy)
{
  r.msg = msg;
  if(*r.msg == ISO_cr || *r.msg == ISO_nl) {
    return;
  }
  if(*r.msg == ISO_colon) {
    ++r.msg;
    parse_prefix();
  }
  
  parse_command();
  parse_params();

  /*  printf("user %s host %s name %s command %s middle %s trailing %s\n",
      r.user, r.host, r.name, r.command, r.middle, r.trailing);*/
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct ircc_state *s))
{
  char *ptr;
  /*  struct parse_result r;*/
  
  PSOCK_BEGIN(&s->s);
  
  PSOCK_READTO(&s->s, ISO_nl);
  
  if(PSOCK_DATALEN(&s->s) > 0) {
    
    s->inputbuf[PSOCK_DATALEN(&s->s)] = 0;

    if(strncmp(s->inputbuf, ircc_strings_ping, 5) == 0) {
      strncpy(s->outputbuf, s->inputbuf, sizeof(s->outputbuf));
      
      /* Turn "PING" into "PONG" */
      s->outputbuf[1] = ISO_O;
      SEND_STRING(&s->s, s->outputbuf);
    } else {

      memset(&r, 0, sizeof(r));

      parse(s->inputbuf, &r);

      if(r.name != NULL) {
	ptr = strchr(r.name, ISO_bang);
	if(ptr != NULL) {
	  *ptr = 0;
	}
      }
      
      if(r.command != NULL && strncmp(r.command, ircc_strings_join, 4) == 0) {
	ircc_text_output(s, "Joined channel", r.name);
      } else if(r.command != NULL && strncmp(r.command, ircc_strings_part, 4) == 0) {
	ircc_text_output(s, "Left channel", r.name);
      } else if(r.trailing != NULL) {
	if(strncmp(r.trailing, ircc_strings_action,
		   strlen(ircc_strings_action)) == 0) {
	  ptr = strchr(&r.trailing[1], 1);
	  if(ptr != NULL) {
	    *ptr = 0;
	  }
	  ptr = &r.trailing[strlen(ircc_strings_action)];
	  petsciiconv_topetscii(r.name, strlen(r.name));
	  petsciiconv_topetscii(ptr, strlen(ptr));
	  ircc_text_output(s, r.name, ptr);
	} else if(strncmp(r.trailing, ircc_strings_version_query,
			  strlen(ircc_strings_version_query)) == 0) {
	  if(r.name != NULL) {
	    strncpy(s->outputbuf, r.name, sizeof(s->outputbuf));
	    SEND_STRING(&s->s, ircc_strings_notice);
	    /* user is temporarily stored in outputbuf. */
	    SEND_STRING(&s->s, s->outputbuf); 
	    SEND_STRING(&s->s, ircc_strings_colon);
	    SEND_STRING(&s->s, ircc_strings_version);
	    SEND_STRING(&s->s, ircc_strings_version_string);
	    SEND_STRING(&s->s, IRC_SYSTEM_STRING);
	    SEND_STRING(&s->s, ircc_strings_ctcpcrnl);
	  }
	} else {
	  petsciiconv_topetscii(r.name, strlen(r.name));
	  petsciiconv_topetscii(r.trailing, strlen(r.trailing));
	  ircc_text_output(s, r.name, r.trailing);
	}
      }
    }
  }

  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(data_or_command(struct ircc_state *s))
{
  PSOCK_BEGIN(&s->s);

  PSOCK_WAIT_UNTIL(&s->s, PSOCK_NEWDATA(&s->s) ||
		    (s->command != COMMAND_NONE));

  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_connection(struct ircc_state *s))
{
  PT_BEGIN(&s->pt);

  PSOCK_INIT(&s->s, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
  
  PT_WAIT_THREAD(&s->pt, setup_connection(s));

  while(1) {

    PT_WAIT_UNTIL(&s->pt, data_or_command(s));

    if(PSOCK_NEWDATA(&s->s)) {
      PT_WAIT_THREAD(&s->pt, handle_input(s));      
    } 
      
    if(s->command == COMMAND_JOIN) {
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, join_channel(s));
    } else if(s->command == COMMAND_PART) {
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, part_channel(s));
    } else if(s->command == COMMAND_MSG) {
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, send_message(s));
    } else if(s->command == COMMAND_ACTIONMSG) {
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, send_actionmessage(s));
    } else if(s->command == COMMAND_LIST) {
      s->command = COMMAND_NONE;
      PT_WAIT_THREAD(&s->pt, list_channel(s));
    } else if(s->command == COMMAND_QUIT) {
      s->command = COMMAND_NONE;
      tcp_markconn(uip_conn, NULL);
      PSOCK_CLOSE(&s->s);
      process_post(PROCESS_CURRENT(), PROCESS_EVENT_EXIT, NULL);
      PT_EXIT(&s->pt);
    }
  }
  
  PT_END(&s->pt);
}
/*---------------------------------------------------------------------------*/
void
ircc_appcall(void *s)
{
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    ircc_closed(s);
  } else if(uip_connected()) {
    ircc_connected(s);
    PT_INIT(&((struct ircc_state *)s)->pt);
    memset(((struct ircc_state *)s)->channel, 0,
	   sizeof(((struct ircc_state *)s)->channel));
    ((struct ircc_state *)s)->command = COMMAND_NONE;
    handle_connection(s);
  } else if(s != NULL) {
    handle_connection(s);
  }
}
/*---------------------------------------------------------------------------*/
struct ircc_state *
ircc_connect(struct ircc_state *s, char *servername, uip_ipaddr_t *ipaddr,
	     char *nick)
{
  s->conn = tcp_connect((uip_ipaddr_t *)ipaddr, UIP_HTONS(PORT), s);
  if(s->conn == NULL) {
    return NULL;
  }
  s->server = servername;  
  s->nick = nick;
  return s;
}
/*---------------------------------------------------------------------------*/
void
ircc_list(struct ircc_state *s)
{
  s->command = COMMAND_LIST;
}
/*---------------------------------------------------------------------------*/
void
ircc_join(struct ircc_state *s, char *channel)
{
  strncpy(s->channel, channel, sizeof(s->channel));
  s->command = COMMAND_JOIN;
}
/*---------------------------------------------------------------------------*/
void
ircc_part(struct ircc_state *s)
{
  s->command = COMMAND_PART;
}
/*---------------------------------------------------------------------------*/
void
ircc_quit(struct ircc_state *s)
{
  s->command = COMMAND_QUIT;
}
/*---------------------------------------------------------------------------*/
void
ircc_msg(struct ircc_state *s, char *msg)
{
  s->msg = msg;
  s->command = COMMAND_MSG;
}
/*---------------------------------------------------------------------------*/
void
ircc_actionmsg(struct ircc_state *s, char *msg)
{
  s->msg = msg;
  s->command = COMMAND_ACTIONMSG;
}
/*---------------------------------------------------------------------------*/
