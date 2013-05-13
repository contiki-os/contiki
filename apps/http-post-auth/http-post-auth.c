/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Contiki interface to post to http basic auth services
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"

#include "http-post-auth.h"

#include <stdio.h>
#include <string.h>

#define MAX_USERNAME_PASSWORD  32
#define MAX_MESSAGE           160
#define MAX_LENGTH             10

#define HOST_NAME	"api.example.org"

struct http_post_auth_state {
  unsigned char timer;
  struct psock sin, sout;
  char lengthstr[MAX_LENGTH];
  char base64_username_password[MAX_USERNAME_PASSWORD];
  char message[MAX_MESSAGE];
  uint8_t inputbuf[UIP_TCP_MSS];
  uip_ipaddr_t addr;
};

struct http_post_auth_state conn;

#ifndef DEBUG
#define DEBUG 0
#endif
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(http_post_auth_process, "HTTP POST auth client");
/*---------------------------------------------------------------------------*/
static uint8_t
base64_encode_6bits(uint8_t c)
{
  if(c <= 25) {
    return c + 'A';
  } else if(c <= 51) {
    return c - 26 + 'a';
  } else if(c <= 61) {
    return c - 52 + '0';
  } else if(c == 62) {
    return '+';
  } else if(c == 63) {
    return '/';
  }
  /* This shouldn't happen because only 6 bits of data should be
     passed to this function. */
  return '=';
}
/*---------------------------------------------------------------------------*/
static void
base64_encode_24bits(const uint8_t inputdata[], char outputdata[], int len)
{
  switch(len) {
  case 0:
    outputdata[0] = outputdata[1] = outputdata[2] = outputdata[3] = '=';
    break;
  case 1:
    outputdata[0] = base64_encode_6bits((inputdata[0] >> 2) & 0x3f);
    outputdata[1] = base64_encode_6bits((((inputdata[0] << 4) & 0x30)));
    outputdata[2] = outputdata[3] = '=';
    break;
  case 2:
    outputdata[0] = base64_encode_6bits((inputdata[0] >> 2) & 0x3f);
    outputdata[1] = base64_encode_6bits((((inputdata[0] << 4) & 0x30) |
					 (inputdata[1] >> 4)) & 0x3f);
    outputdata[2] = base64_encode_6bits((((inputdata[1] << 2) & 0x3f)));
    outputdata[3] = '=';
    break;
  case 3:
  default:
    outputdata[0] = base64_encode_6bits((inputdata[0] >> 2) & 0x3f);
    outputdata[1] = base64_encode_6bits((((inputdata[0] << 4) & 0x30) |
					 (inputdata[1] >> 4)) & 0x3f);
    outputdata[2] = base64_encode_6bits((((inputdata[1] << 2) & 0x3c) |
					 (inputdata[2] >> 6)) & 0x3f);
    outputdata[3] = base64_encode_6bits((inputdata[2]) & 0x3f);
    break;
  }
}
/*---------------------------------------------------------------------------*/
int
http_post_auth(const uint8_t *username_password, const char *msg)
{
  int len;
  int i, j;
  struct http_post_auth_state *s;

  process_exit(&http_post_auth_process);
  
  /*  s = (struct http_post_auth_state *)memb_alloc(&conns);*/
  s = &conn;
  if(s == NULL) {
    PRINTF("Could not allocate memory for the tweet\n");
    return 0;
  }
  PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
  PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
  s->timer = 0;
  
  /* dec64 encode username:password pair */
  len = strlen((char *)username_password);
  j = 0;
  for(i = 0; i < len; i += 3) {
    base64_encode_24bits(&username_password[i], &s->base64_username_password[j],
			 len - i);
    j += 4;
  }
  s->base64_username_password[j] = 0;

  /* Copy the status message, and avoid the leading whitespace. */
  strcpy(s->message, "status=");
  strcpy(&s->message[7], msg);

  /*  PRINTF("username_password '%s'\n", s->base64_username_password);
      PRINTF("message '%s'\n", s->message);*/

  /* Spawn process to deal with TCP connection */
  process_start(&http_post_auth_process, (char *)s);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
handle_output(struct http_post_auth_state *s)
{
  PSOCK_BEGIN(&s->sout);
  /* Send POST header */
  
  PSOCK_SEND_STR(&s->sout, "POST /statuses/update.json HTTP/1.1\r\n");
  
  /* Send Authorization header */
  PSOCK_SEND_STR(&s->sout, "Authorization: Basic ");
  PSOCK_SEND_STR(&s->sout, s->base64_username_password);
  PSOCK_SEND_STR(&s->sout, "\r\n");
  
  /* Send Agent header */
  PSOCK_SEND_STR(&s->sout, "User-Agent: Contiki 2.x\r\n");
  PSOCK_SEND_STR(&s->sout, "Host: " HOST_NAME "\r\n");
  PSOCK_SEND_STR(&s->sout, "Accept: */*\r\n");
  
  /* Send Content length header */
  PSOCK_SEND_STR(&s->sout, "Content-Length: ");
  snprintf(s->lengthstr, sizeof(s->lengthstr), "%d", strlen(s->message));
  PSOCK_SEND_STR(&s->sout, s->lengthstr);
  PSOCK_SEND_STR(&s->sout, "\r\n");

  /* Send Content type header */
  PSOCK_SEND_STR(&s->sout,
		 "Content-Type: application/x-www-form-urlencoded\r\n\r\n");


  /* Send status message */
  PSOCK_SEND_STR(&s->sout, s->message);

  /* Close connection */
  PSOCK_CLOSE(&s->sout);
  PSOCK_EXIT(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static int
handle_input(struct http_post_auth_state *s)
{
  PSOCK_BEGIN(&s->sin);

  /* We don't care about input data for now */
  
  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct http_post_auth_state *s)
{
  handle_input(s);
  handle_output(s);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(http_post_auth_process, ev, data)
{
  struct http_post_auth_state *s = data;
  struct uip_conn *conn;
  
  PROCESS_BEGIN();

  /* Lookup host */
  
  /* XXX for now, just use 128.121.146.228 */
  uip_ipaddr(&s->addr, 128,121,146,228);

  
  /* Open a TCP connection to port 80 */
  conn = tcp_connect(&s->addr, uip_htons(80), s);
  if(conn == NULL) {
    PRINTF("Could not open TCP connection\n");
    /*    memb_free(&conns, s);*/
    PROCESS_EXIT();
  }

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == tcpip_event) {
      struct http_post_auth_state *s = (struct http_post_auth_state *)data;
      
      if(uip_closed() || uip_aborted() || uip_timedout()) {
	if(uip_closed()) {
	  PRINTF("Connection closed\n");
	} else {
	  PRINTF("Connection aborted/timedout\n");
	}
	/*	if(s != NULL) {
	  memb_free(&conns, s);
	  }*/
	PROCESS_EXIT();
      } else if(uip_connected()) {
	handle_connection(s);
	s->timer = 0;
      } else if(s != NULL) {
	if(uip_poll()) {
	  ++s->timer;
	  if(s->timer >= 20) {
	    PRINTF("Timed out due to inactivity\n");
	    uip_abort();
	    PROCESS_EXIT();
	    /*	    memb_free(&conns, s);*/
	  }
	} else {
	  s->timer = 0;
	}
	handle_connection(s);
      } else {
	PRINTF("Abort because s == NULL\n");
	uip_abort();
	PROCESS_EXIT();
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
