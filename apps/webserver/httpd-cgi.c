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
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "lib/petsciiconv.h"

static struct httpd_cgi_call *calls = NULL;

static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49,
 0x53, 0x48, 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
 0x4b, 0};
static const char none[] = /*  "NONE"*/
{0x4e, 0x4f, 0x4e, 0x45, 0};
static const char running[] = /*  "RUNNING"*/
{0x52, 0x55, 0x4e, 0x4e, 0x49, 0x4e, 0x47,
 0};
static const char called[] = /*  "CALLED"*/
{0x43, 0x41, 0x4c, 0x4c, 0x45, 0x44, 0};
static const char file_name[] = /*  "file-stats"*/
{0x66, 0x69, 0x6c, 0x65, 0x2d, 0x73, 0x74,
 0x61, 0x74, 0x73, 0};
static const char tcp_name[] = /*  "tcp-connections"*/
{0x74, 0x63, 0x70, 0x2d, 0x63, 0x6f, 0x6e,
 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e,
 0x73, 0};
static const char proc_name[] = /*  "processes"*/
{0x70, 0x72, 0x6f, 0x63, 0x65, 0x73, 0x73,
 0x65, 0x73, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack,
  none,
  running,
  called};

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
static unsigned short
generate_file_stats(void *arg)
{
  char *f = (char *)arg;
  return snprintf((char *)uip_appdata, uip_mss(), "%5u", httpd_fs_count(f));
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(file_stats(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_file_stats, (void *) (strchr(ptr, ' ') + 1));
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
make_tcp_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  conn = &uip_conns[s->u.count];

#if UIP_CONF_IPV6
  char buf[48];
  httpd_sprint_ip6(conn->ripaddr, buf);
  return snprintf((char *)uip_appdata, uip_mss(),
         "<tr align=\"center\"><td>%d</td><td>%s:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
         uip_htons(conn->lport),
         buf,
         uip_htons(conn->rport),
         states[conn->tcpstateflags & UIP_TS_MASK],
         conn->nrtx,
         conn->timer,
         (uip_outstanding(conn))? '*':' ',
         (uip_stopped(conn))? '!':' ');
#else
  return snprintf((char *)uip_appdata, uip_mss(),
         "<tr align=\"center\"><td>%d</td><td>%u.%u.%u.%u:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
         uip_htons(conn->lport),
         conn->ripaddr.u8[0],
         conn->ripaddr.u8[1],
         conn->ripaddr.u8[2],
         conn->ripaddr.u8[3],
         uip_htons(conn->rport),
         states[conn->tcpstateflags & UIP_TS_MASK],
         conn->nrtx,
         conn->timer,
        (uip_outstanding(conn))? '*':' ',
        (uip_stopped(conn))? '!':' ');
#endif /* UIP_CONF_IPV6 */
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(tcp_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  for(s->u.count = 0; s->u.count < UIP_CONNS; ++s->u.count) {
    if((uip_conns[s->u.count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, make_tcp_stats, s);
    }
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
make_processes(void *p)
{
  char name[40];
 
  strncpy(name, PROCESS_NAME_STRING((struct process *)p), 40);
  petsciiconv_toascii(name, 40);

  return snprintf((char *)uip_appdata, uip_mss(),
		 "<tr align=\"center\"><td>%p</td><td>%s</td><td>%p</td><td>%s</td></tr>\r\n",
		 p, name,
		 *((char **)&(((struct process *)p)->thread)),
		 states[9 + ((struct process *)p)->state]);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(processes(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  for(s->u.ptr = PROCESS_LIST(); s->u.ptr != NULL; s->u.ptr = ((struct process *)s->u.ptr)->next) {
    PSOCK_GENERATOR_SEND(&s->sout, make_processes, s->u.ptr);
  }
  PSOCK_END(&s->sout);
}
#if WEBSERVER_CONF_STATUSPAGE && UIP_CONF_IPV6
/* These cgi's are invoked by the status.shtml page in /apps/webserver/httpd-fs.
 * To keep the webserver build small that 160 byte page is not present in the
 * default httpd-fsdata.c file. Run the PERL script /../../tools/makefsdata from the
 * /apps/webserver/ directory to include it. Delete status.shtml before running
 * the script if you want to exclude it again.
 * NB: Webserver builds on all platforms will use the current httpd-fsdata.c file. The added 160 bytes
 * could overflow memory on the smaller platforms.
 */
/*---------------------------------------------------------------------------*/
#define HTTPD_STRING_ATTR
#define httpd_snprintf snprintf
#define httpd_cgi_sprint_ip6 httpd_sprint_ip6

static const char httpd_cgi_addrh[] HTTPD_STRING_ATTR = "<code>";
static const char httpd_cgi_addrf[] HTTPD_STRING_ATTR = "</code>[Room for %u more]";
static const char httpd_cgi_addrb[] HTTPD_STRING_ATTR = "<br>";
static const char httpd_cgi_addrn[] HTTPD_STRING_ATTR = "(none)<br>";
extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];
extern uip_ds6_netif_t uip_ds6_if;

static unsigned short
make_addresses(void *p)
{
uint8_t i,j=0;
uint16_t numprinted;
  numprinted = httpd_snprintf((char *)uip_appdata, uip_mss(),httpd_cgi_addrh);
  for (i=0; i<UIP_DS6_ADDR_NB;i++) {
    if (uip_ds6_if.addr_list[i].isused) {
      j++;
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, uip_appdata + numprinted);
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrb); 
    }
  }
//if (j==0) numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrn);
  numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf, UIP_DS6_ADDR_NB-j); 
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(addresses(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, make_addresses, s->u.ptr);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/	
static unsigned short
make_neighbors(void *p)
{
uint8_t i,j=0;
uint16_t numprinted;
  numprinted = httpd_snprintf((char *)uip_appdata, uip_mss(),httpd_cgi_addrh);
  for (i=0; i<UIP_DS6_NBR_NB;i++) {
    if (uip_ds6_nbr_cache[i].isused) {
      j++;
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_nbr_cache[i].ipaddr, uip_appdata + numprinted);
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrb); 
    }
  }
//if (j==0) numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrn);
  numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf,UIP_DS6_NBR_NB-j);
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(neighbors(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, make_neighbors, s->u.ptr);  
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/			
static unsigned short
make_routes(void *p)
{
  static const char httpd_cgi_rtes1[] HTTPD_STRING_ATTR = "(%u (via ";
  static const char httpd_cgi_rtes2[] HTTPD_STRING_ATTR = ") %lus<br>";
  static const char httpd_cgi_rtes3[] HTTPD_STRING_ATTR = ")<br>";
  uint8_t i,j=0;
  uint16_t numprinted;
  uip_ds6_route_t *r;

  numprinted = httpd_snprintf((char *)uip_appdata, uip_mss(),httpd_cgi_addrh);
  for(r = uip_ds6_route_list_head();
      r != NULL;
      r = list_item_next(r)) {
    j++;
    numprinted += httpd_cgi_sprint_ip6(r->ipaddr, uip_appdata + numprinted);
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes1, r->length);
    numprinted += httpd_cgi_sprint_ip6(r->nexthop, uip_appdata + numprinted);
    if(r->state.lifetime < 3600) {
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes2, r->state.lifetime);
    } else {
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes3);
    }
  }
  if (j==0) numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrn);
  numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf,UIP_DS6_ROUTE_NB-j);
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(routes(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
 
  PSOCK_GENERATOR_SEND(&s->sout, make_routes, s->u.ptr); 
 
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_STATUSPAGE */
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
#if WEBSERVER_CONF_STATUSPAGE && UIP_CONF_IPV6
static const char   adrs_name[] HTTPD_STRING_ATTR = "addresses";
static const char   nbrs_name[] HTTPD_STRING_ATTR = "neighbors";
static const char   rtes_name[] HTTPD_STRING_ATTR = "routes";
#endif
HTTPD_CGI_CALL(file, file_name, file_stats);
HTTPD_CGI_CALL(tcp, tcp_name, tcp_stats);
HTTPD_CGI_CALL(proc, proc_name, processes);
#if WEBSERVER_CONF_STATUSPAGE && UIP_CONF_IPV6
HTTPD_CGI_CALL(adrs, adrs_name, addresses);
HTTPD_CGI_CALL(nbrs, nbrs_name, neighbors);
HTTPD_CGI_CALL(rtes, rtes_name, routes);
#endif

void
httpd_cgi_init(void)
{
  httpd_cgi_add(&file);
  httpd_cgi_add(&tcp);
  httpd_cgi_add(&proc);
#if WEBSERVER_CONF_STATUSPAGE && UIP_CONF_IPV6
  httpd_cgi_add(&adrs);
  httpd_cgi_add(&nbrs);
  httpd_cgi_add(&rtes);
#endif
}
/*---------------------------------------------------------------------------*/
