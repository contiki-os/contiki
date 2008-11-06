/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: ajax-cgi.c,v 1.2 2008/11/06 08:24:20 adamdunkels Exp $
 *
 */

/*
 * This file includes functions that are called by the web server
 * scripts. The functions takes no argument, and the return value is
 * interpreted as follows. A zero means that the function did not
 * complete and should be invoked for the next packet as well. A
 * non-zero value indicates that the function has completed and that
 * the web server should move along to the next script line.
 *
 */

#include <stdio.h>
#include <string.h>

#include "contiki-net.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "lib/petsciiconv.h"

static struct httpd_cgi_call *calls = NULL;

/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
  struct httpd_cgi_call *f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; f != NULL; f = f->next) {
    if(strncmp(f->name, name, strlen(f->name)) == 0) {
      return f->function;
    }
  }
  return nullfunction;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(nodeidcall(struct httpd_state *s, char *ptr))
{
  static char buf[10];
  PSOCK_BEGIN(&s->sout);
  snprintf(buf, sizeof(buf), "%d.%d",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  PSOCK_SEND_STR(&s->sout, buf);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
void
httpd_cgi_add(struct httpd_cgi_call *c)
{
  struct httpd_cgi_call *l;

  c->next = NULL;
  if(calls == NULL) {
    calls = c;
  } else {
    for(l = calls; l->next != NULL; l = l->next);
    l->next = c;
  }
}
/*---------------------------------------------------------------------------*/
#include "dev/sht11.h"
#include "dev/light.h"

static
PT_THREAD(sensorscall(struct httpd_state *s, char *ptr))
{
  static struct timer t;
  static int i;
  static char buf[100];
  static unsigned long last_cpu, last_lpm, last_listen, last_transmit;
  
  PSOCK_BEGIN(&s->sout);

  timer_set(&t, CLOCK_SECOND);
  i = 0;
  while(1) {
    timer_restart(&t);
    PSOCK_WAIT_UNTIL(&s->sout, timer_expired(&t));

    snprintf(buf, sizeof(buf),
	     "<script type='text/javascript'>t(%d);h(%d);l1(%d);l2(%d);</script>",
	     sht11_temp(),
	     sht11_humidity(),
	     sensors_light1(),
	     sensors_light2());
    PSOCK_SEND_STR(&s->sout, buf);


    timer_restart(&t);
    PSOCK_WAIT_UNTIL(&s->sout, timer_expired(&t));
    snprintf(buf, sizeof(buf),
	     "<script type='text/javascript'>p(%lu,%lu,%lu,%lu);i(%d);</script>",
	     energest_type_time(ENERGEST_TYPE_CPU) - last_cpu,
	     energest_type_time(ENERGEST_TYPE_LPM) - last_lpm,
	     energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit,
	     energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen,
	     i++);
    last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
    last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
    last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
    PSOCK_SEND_STR(&s->sout, buf);

}
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
make_neighbor(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  struct neighbor *n = neighbor_get(s->u.count);

  if(n == NULL) {
    return 0;
  }

  return snprintf((char *)uip_appdata, uip_mss(),
		  "<li><a href=\"http://172.16.%d.%d/\">%d.%d</a>\r\n",

		  n->addr.u8[0], n->addr.u8[1],
		  n->addr.u8[0], n->addr.u8[1]);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(neighborscall(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  /*  printf("neighbor_num %d\n", neighbor_num());*/
  
  for(s->u.count = 0; s->u.count < neighbor_num(); s->u.count++) {
    /*    printf("count %d\n", s->u.count);*/
    if(neighbor_get(s->u.count) != NULL) {
      /*      printf("!= NULL\n");*/
      PSOCK_GENERATOR_SEND(&s->sout, make_neighbor, s);
    }
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static void
adv_received(struct neighbor_discovery_conn *c, rimeaddr_t *from,
	     uint16_t rtmetric)
{
  struct neighbor *n;

  /*  printf("adv_received %d.%d\n", from->u8[0], from->u8[1]);*/
  
  n = neighbor_find(from);
  
  if(n == NULL) {
    neighbor_add(from, rtmetric, 1);
  } else {
    neighbor_update(n, rtmetric);
  }
}


static const struct neighbor_discovery_callbacks neighbor_discovery_callbacks =
  { adv_received, NULL};


HTTPD_CGI_CALL(sensors, "sensors", sensorscall);
HTTPD_CGI_CALL(nodeid, "nodeid", nodeidcall);
HTTPD_CGI_CALL(neighbors, "neighbors", neighborscall);

static struct neighbor_discovery_conn conn;

void
httpd_cgi_init(void)
{
  
  httpd_cgi_add(&sensors);
  httpd_cgi_add(&nodeid);
  httpd_cgi_add(&neighbors);

  neighbor_discovery_open(&conn, 31,
			  CLOCK_SECOND * 4,
			  CLOCK_SECOND * 20,
			  CLOCK_SECOND * 60,
			  &neighbor_discovery_callbacks);
  neighbor_discovery_start(&conn, 0);
}
/*---------------------------------------------------------------------------*/
