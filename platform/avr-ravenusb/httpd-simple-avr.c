/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         A simple web server forwarding page generation to a protothread
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         David Kopf <dak664@embarqmail.com> (AVR adaptation)
 */

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "contiki-net.h"

#ifndef WEBSERVER_CONF_CFS_PATHLEN
#define HTTPD_PATHLEN 2
#else
#define HTTPD_PATHLEN WEBSERVER_CONF_CFS_PATHLEN
#endif

struct httpd_state;
typedef char (* httpd_simple_script_t)(struct httpd_state *s);

struct httpd_state {
  struct timer timer;
  struct psock sin, sout;
  struct pt outputpt;
  char inputbuf[HTTPD_PATHLEN + 30];
  char outputbuf[UIP_TCP_MSS];
  char filename[HTTPD_PATHLEN];
  httpd_simple_script_t script;
  char state;
};

/* DEBUGLOGIC is a convenient way to debug in a simulator without a tcp/ip connection.
 * Break the program in the process loop and step from the entry in httpd_appcall.
 * The input file is forced to /index.html and the output directed to uip_aligned_buf.
 * If cgi's are invoked define it in httpd-cgi.c as well!
 * psock_generator_send in /core/net/psock.c must also be modified as follows:
 * ...
 * // Wait until all data is sent and acknowledged.
 * if (!s->sendlen) break;                            //<---add this line
 * PT_YIELD_UNTIL(&s->psockpt, uip_acked() || uip_rexmit());
 * ...
 */
#define DEBUGLOGIC 0
#if DEBUGLOGIC
struct httpd_state *sg;
#define uip_mss(...) sizeof(uip_aligned_buf)
#define uip_appdata (char *) &uip_aligned_buf
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define CONNS UIP_CONNS
#else /* WEBSERVER_CONF_CFS_CONNS */
#define CONNS WEBSERVER_CONF_CFS_CONNS
#endif /* WEBSERVER_CONF_CFS_CONNS */

#ifndef WEBSERVER_CONF_CFS_URLCONV
#define URLCONV 0
#else /* WEBSERVER_CONF_CFS_URLCONV */
#define URLCONV WEBSERVER_CONF_CFS_URLCONV
#endif /* WEBSERVER_CONF_CFS_URLCONV */

#define STATE_WAITING 0
#define STATE_OUTPUT  1

MEMB(conns, struct httpd_state, CONNS);

#define webserver_log_file(...)

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_period  0x2e
#define ISO_slash   0x2f

/*---------------------------------------------------------------------------*/
static unsigned short
generate_string(void *sstr)
{
  uint8_t slen=strlen((char *)sstr);
  memcpy(uip_appdata, (char *)sstr, slen);

#if DEBUGLOGIC
  return 0;
#else
  return slen;
#endif
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_string_P(void *sstr)
{
  uint8_t slen=strlen_P((char *)sstr);
  memcpy_P(uip_appdata, (char *)sstr, slen);

#if DEBUGLOGIC
  return 0;
#else
  return slen;
#endif
}
/*---------------------------------------------------------------------------*/
#if FIND_THE_SCRIPT
/* Needed if more than one script is implemented.
 * The generate_routes RPL page is hard coded at present
 */
static
PT_THREAD(send_string_P(struct httpd_state *s, char *str))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, str);
  PSOCK_END(&s->sout);
}
#endif
/*---------------------------------------------------------------------------*/
const char http_content_type_html[] PROGMEM = "Content-type: text/html\r\n\r\n";
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, (char *) statushdr);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, (char *) http_content_type_html);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
const char http_index_html[] PROGMEM = "/index.html";
const char http_referer[] PROGMEM = "Referer:";
const char http_get[] PROGMEM = "GET ";
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

#if URLCONV
  s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
  urlconv_tofilename(s->filename, s->inputbuf, sizeof(s->filename));
#else /* URLCONV */
  if(s->inputbuf[1] == ISO_space) {
    strncpy_P(s->filename, http_index_html, sizeof(s->filename));
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, s->inputbuf, sizeof(s->filename));
  }
#endif /* URLCONV */

  webserver_log_file(&uip_conn->ripaddr, s->filename);

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);

 //   if(strncmp_P(s->inputbuf, http_referer, 8) == 0) {
 //     s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
 //     webserver_log(s->inputbuf);
 //   }
  }

  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
void
httpd_init(void)
{
  tcp_listen(UIP_HTONS(80));
  memb_init(&conns);
}

/*---------------------------------------------------------------------------*/
/* Only one single web request at time, MSS is 48 to save RAM */
static char buf[48];
static uint8_t blen;
#define ADD(FORMAT,args...) do {                                                 \
    blen += snprintf_P(&buf[blen], sizeof(buf) - blen, PSTR(FORMAT),##args);      \
  } while(0)
/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0 && sizeof(buf) - blen >= 2) {
        buf[blen++] = ':';
        buf[blen++] = ':';
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0 && blen < sizeof(buf)) {
        buf[blen++] = ':';
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
const char TOP1[] PROGMEM = "<html><head><title>ContikiRPL(Jackdaw)";
const char TOP2[] PROGMEM = "</title></head><body>";
const char BOTTOM[] PROGMEM = "</body></html>";

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  uint8_t i=0;
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, (char *) TOP1);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, (char *) TOP2);

#if UIP_CONF_IPV6     //allow ip4 builds
  blen = 0;
  ADD("<h2>Neighbors [%u max]</h2>",NBR_TABLE_CONF_MAX_NEIGHBORS);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);  
  blen = 0;
  uip_ds6_nbr_t *nbr;
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
      ipaddr_add(&nbr->ipaddr);
      ADD("<br>");
//    if(blen > sizeof(buf) - 45) {
        PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);  
        blen = 0;
//    }
  }

  ADD("<h2>Routes [%u max]</h2>",UIP_DS6_ROUTE_NB);
  PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);  
  blen = 0;
  uip_ds6_route_t *route;
  for(route = uip_ds6_route_head();
      route != NULL;
      route = uip_ds6_route_next(route)) {
    ipaddr_add(&route->ipaddr);
    ADD("/%u (via ", route->length);
    PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);
    blen=0;
    ipaddr_add(uip_ds6_route_nexthop(route));
    if(route->state.lifetime < 600) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);
      blen=0;
      ADD(") %lus<br>", route->state.lifetime);
    } else {
      ADD(")<br>");
    }
    PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);
    blen = 0;
  }
  if(blen > 0) {
	PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);  
    blen = 0;
  }
#else /* UIP_CONF_IPV6 */
  blen = 0;i++;
  ADD("<h2>Hey, you got ip4 working!</h2>");
  PSOCK_GENERATOR_SEND(&s->sout, generate_string, buf);  
#endif /* UIP_CONF_IPV6 */

  PSOCK_GENERATOR_SEND(&s->sout, generate_string_P, (char *) BOTTOM);  

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
const char http_header_200[] PROGMEM = "HTTP/1.0 200 OK\r\nServer: Jackdaw\r\nConnection: close\r\n";
const char http_header_404[] PROGMEM = "HTTP/1.0 404 Not found\r\nServer: Jackdaw\r\nConnection: close\r\n";
const char NOT_FOUND[] PROGMEM = "<html><body bgcolor=\"white\"><center><h1>404 - file not found</h1></center></body></html>";
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

#if DEBUGLOGIC
   strcpy_P(s->filename,PSTR("/x"));
#endif
#if FIND_THE_SCRIPT
  s->script = httpd_simple_get_script(&s->filename[1]);
  if(s->script == NULL) {
    printf_P(PSTR("not found!"));
    strcpy_P(s->filename, PSTR("/notfound.html"));

    PT_WAIT_THREAD(&s->outputpt,
                   send_headers(s, http_header_404));
    PT_WAIT_THREAD(&s->outputpt,
                   send_string_P(s, NOT_FOUND));
    uip_close();

    PT_EXIT(&s->outputpt);
  } else {
#else
  s->script = generate_routes;
  if (1) {
#endif

    PT_WAIT_THREAD(&s->outputpt,
                   send_headers(s, http_header_200));
    PT_WAIT_THREAD(&s->outputpt, s->script(s));
  }
  s->script = NULL;
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
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
  s = sg = (struct httpd_state *)memb_alloc(&conns);  //put ram watch on sg
  if (1) {
#else
  struct httpd_state *s = (struct httpd_state *)state;

  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      s->script = NULL;
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      webserver_log_file(&uip_conn->ripaddr, "reset (no memory block)");
      return;
    }
#endif
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->script = NULL;
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
        uip_abort();
        s->script = NULL;
        memb_free(&conns, s);
        webserver_log_file(&uip_conn->ripaddr, "reset (timeout)");
      }
    } else {
      timer_restart(&s->timer);
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(httpd_process, "httpd");
PROCESS_THREAD(httpd_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}

