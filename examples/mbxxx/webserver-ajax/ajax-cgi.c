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
#include "net/rime/collect-neighbor.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "lib/petsciiconv.h"

#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"

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
  static char buf[24];
  PSOCK_BEGIN(&s->sout);
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
	   linkaddr_node_addr.u8[0],
       linkaddr_node_addr.u8[1],
       linkaddr_node_addr.u8[2],
       linkaddr_node_addr.u8[3],
       linkaddr_node_addr.u8[4],
       linkaddr_node_addr.u8[5],
       linkaddr_node_addr.u8[6],
       linkaddr_node_addr.u8[7]);
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
static
PT_THREAD(sensorscall(struct httpd_state *s, char *ptr))
{
  static struct timer t;
  static int i;
  static char buf[100];
  static unsigned long last_cpu, last_lpm, last_listen, last_transmit;
  
  PSOCK_BEGIN(&s->sout);

  timer_set(&t, CLOCK_SECOND);
 
  SENSORS_ACTIVATE(acc_sensor);
      
  snprintf(buf, sizeof(buf),
            "t(%d);ax(%d);ay(%d);az(%d);",
	     temperature_sensor.value(0),
	     acc_sensor.value(ACC_X_AXIS),
	     acc_sensor.value(ACC_Y_AXIS),
	     acc_sensor.value(ACC_Z_AXIS));
    
  SENSORS_DEACTIVATE(acc_sensor);

  PSOCK_SEND_STR(&s->sout, buf);


  snprintf(buf, sizeof(buf),
	     "p(%lu,%lu,%lu,%lu);v(%d);",
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

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
#if 0
static unsigned short
make_neighbor(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  struct collect_neighbor *n = collect_neighbor_get(s->u.count);

  if(n == NULL) {
    return 0;
  }

#if !NETSTACK_CONF_WITH_IPV6
  return snprintf((char *)uip_appdata, uip_mss(),
		  "<li><a href=\"http://172.16.%d.%d/\">%d.%d</a>\r\n",
		  n->addr.u8[0], n->addr.u8[1],
		  n->addr.u8[0], n->addr.u8[1]);
#else
#if 0
  uip_ipaddr_t ipaddr;
  char ipaddr_str[41];
  
  uip_ip6addr(&ipaddr, NET_ADDR_A, NET_ADDR_B, NET_ADDR_C, NET_ADDR_D,
              (uint16_t)(((uint16_t)(n->addr.u8[0]^0x02))<<8 | (uint16_t)n->addr.u8[1]),
              ((uint16_t)(n->addr.u8[2]))<<8 | (uint16_t)n->addr.u8[3],
              (uint16_t)(n->addr.u8[4])<<8 | n->addr.u8[5],
              (uint16_t)(n->addr.u8[6])<<8 | n->addr.u8[7]);
  httpd_sprint_ip6(ipaddr, ipaddr_str);
  
  return snprintf((char *)uip_appdata, uip_mss(),
		  "<li><a href=\"http://%s/\">%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X</a>\r\n",
		  ipaddr_str,
          n->addr.u8[0],
          n->addr.u8[1],
          n->addr.u8[2],
          n->addr.u8[3],
          n->addr.u8[4],
          n->addr.u8[5],
          n->addr.u8[6],
          n->addr.u8[7]);
#endif
  /* Automatic generation of node address. Javascript funcion required.
   * Client-side generation is simpler than server-side, as parsing http header
   * would be requied.
   */
  return snprintf((char *)uip_appdata, uip_mss(),
                  "<li><a id=node name='%x:%x:%x:%x'>%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X</a>\r\n",
                  (uint16_t)(((uint16_t)(n->addr.u8[0]^0x02))<<8 | (uint16_t)n->addr.u8[1]),
                  ((uint16_t)(n->addr.u8[2]))<<8 | (uint16_t)n->addr.u8[3],
                  (uint16_t)(n->addr.u8[4])<<8 | n->addr.u8[5],
                  (uint16_t)(n->addr.u8[6])<<8 | n->addr.u8[7],
                  n->addr.u8[0],
                  n->addr.u8[1],
                  n->addr.u8[2],
                  n->addr.u8[3],
                  n->addr.u8[4],
                  n->addr.u8[5],
                  n->addr.u8[6],
                  n->addr.u8[7]);
  
#endif /* NETSTACK_CONF_WITH_IPV6 */
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(neighborscall(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  announcement_listen(1);
  
  /*  printf("neighbor_num %d\n", neighbor_num());*/
  
  for(s->u.count = 0; s->u.count < collect_neighbor_num(); s->u.count++) {
    /*    printf("count %d\n", s->u.count);*/
    if(collect_neighbor_get(s->u.count) != NULL) {
      /*      printf("!= NULL\n");*/
      PSOCK_GENERATOR_SEND(&s->sout, make_neighbor, s);
    }
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static void
received_announcement(struct announcement *a, const linkaddr_t *from,
	     uint16_t id, uint16_t value)
{
  struct collect_neighbor *n;

  /*  printf("adv_received %d.%d\n", from->u8[0], from->u8[1]);*/
  
  n = collect_neighbor_find(from);
  
  if(n == NULL) {
    collect_neighbor_add(from, value, 1);
  } else {
    collect_neighbor_update(n, value);
  }
}
#endif

/*static const struct neighbor_discovery_callbacks neighbor_discovery_callbacks =
  { adv_received, NULL};*/


HTTPD_CGI_CALL(sensors, "sensors", sensorscall);
HTTPD_CGI_CALL(nodeid, "nodeid", nodeidcall);
//HTTPD_CGI_CALL(neighbors, "neighbors", neighborscall);

/*static struct neighbor_discovery_conn conn;*/
//static struct announcement announcement;

void
httpd_cgi_init(void)
{
  
  httpd_cgi_add(&sensors);
  httpd_cgi_add(&nodeid);
/*  httpd_cgi_add(&neighbors);

  announcement_register(&announcement, 31,
			received_announcement);
  announcement_listen(2);*/
  
  /*  neighbor_discovery_open(&conn, 31,
			  CLOCK_SECOND * 4,
			  CLOCK_SECOND * 20,
			  CLOCK_SECOND * 60,
			  &neighbor_discovery_callbacks);
			  neighbor_discovery_start(&conn, 0);*/
}
/*---------------------------------------------------------------------------*/
