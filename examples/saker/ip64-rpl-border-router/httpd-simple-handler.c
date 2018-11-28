/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "httpd-simple.h"
#include "httpd-simple-handler.h"
#include "project-conf.h"
#include "contiki.h"
#include "project-conf.h"

#include <stdio.h>

/* Declarations to make the buffer handling more readable. */
#if BUF_USES_STACK
#define BUF_STACK_DECLARATION       char buf[BUF_SIZE]
#define BUF_GLOBAL_DECLARATION      static char *bufptr, *bufend
#define BUF_INDEX                   bufptr
#define BUF_INDEX_T                 char *
#define BUF_END                     bufend
#define BUF_RESET() do { \
    bufptr = buf; bufend = bufptr + sizeof(buf); \
} while(0)
#define ADD(...) do { \
    bufptr += snprintf(bufptr, bufend - bufptr, __VA_ARGS__); \
} while(0)

#else /* BUF_USES_STACK */
#define BUF_STACK_DECLARATION
#define BUF_GLOBAL_DECLARATION      static char buf[BUF_SIZE]; \
  static uint8_t blen;
#define BUF_INDEX                   blen
#define BUF_INDEX_T                 uint8_t
#define BUF_END                     sizeof(buf)
#define BUF_RESET()                 blen = 0;
#define ADD(...) do { \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__); \
} while(0)
#endif /* BUF_USES_STACK */

PROCESS(httpd_simple_handler, "Web server");
PROCESS_THREAD(httpd_simple_handler, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}

static const char *TOP = "<html><head><title>Weptech Gateway</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";

BUF_GLOBAL_DECLARATION;

/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        ADD("::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static uip_ds6_route_t *r;
  static uip_ds6_nbr_t *nbr;
  BUF_STACK_DECLARATION;

#if WEBSERVER_CONF_LOADTIME
  static clock_time_t numticks;
  numticks = clock_time();
#endif

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  BUF_RESET();

  ADD("Neighbors<pre>");

  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {

#if WEBSERVER_CONF_NEIGHBOR_STATUS
    BUF_INDEX_T j = BUF_INDEX + 25;
    ipaddr_add(&nbr->ipaddr);
    while(BUF_INDEX < j) ADD(" ");
    switch(nbr->state) {
    case NBR_INCOMPLETE: ADD(" INCOMPLETE");
      break;
    case NBR_REACHABLE: ADD(" REACHABLE");
      break;
    case NBR_STALE: ADD(" STALE");
      break;
    case NBR_DELAY: ADD(" DELAY");
      break;
    case NBR_PROBE: ADD(" NBR_PROBE");
      break;
    }
#else
    ipaddr_add(&nbr->ipaddr);
#endif

    ADD("\n");
    if(BUF_INDEX > BUF_END - 45) {
      SEND_STRING(&s->sout, buf);
      BUF_RESET();
    }
  }

  ADD("</pre>Routes<pre>");
  SEND_STRING(&s->sout, buf);
  BUF_RESET();

  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
#if WEBSERVER_CONF_ROUTE_LINKS
    ADD("<a href=http://[");
    ipaddr_add(&r->ipaddr);
    ADD("]/status.shtml>");
    SEND_STRING(&s->sout, buf); /* TODO: why tunslip6 needs an output here, wpcapslip does not */
    BUF_RESET();
    ipaddr_add(&r->ipaddr);
    ADD("</a>");
#else
    ipaddr_add(&r->ipaddr);
#endif
    ADD("/%u (via ", r->length);
    ipaddr_add(uip_ds6_route_nexthop(r));
    if(1 || (r->state.lifetime < 600)) {
      ADD(") %lus\n", (unsigned long)r->state.lifetime);
    } else {
      ADD(")\n");
    }
    SEND_STRING(&s->sout, buf);
    BUF_RESET();
  }
  ADD("</pre>");

#if WEBSERVER_CONF_FILESTATS
  static uint16_t numtimes;
  ADD("<br><i>This page was sent %u times</i>", ++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
  numticks = clock_time() - numticks + 1;
  ADD(" <i>(%lu.%02lu sec)</i>", numticks / CLOCK_SECOND, (100 * (numticks % CLOCK_SECOND)) / CLOCK_SECOND);
#endif

  SEND_STRING(&s->sout, buf);
  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{

  return generate_routes;
}
