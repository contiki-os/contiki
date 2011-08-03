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
 * $Id: httpd-cgi.c,v 1.2 2010/10/19 18:29:05 adamdunkels Exp $
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


#include "contiki-net.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "lib/petsciiconv.h"

#include <stdio.h>
#include <string.h>


static struct httpd_cgi_call *calls = NULL;

/*struct cgifunction {
  char *name;
  httpd_cgifunction function;
};

static struct cgifunction cgitab[] = {
  {"file-stats", file_stats},
  {"tcp-connections", tcp_stats},
  {"processes", processes},
  {NULL, NULL}
  };*/


static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56, 
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e, 
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48, 
 0x45, 0x44, 0};
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
static const char none[] = "NONE";
static const char init[] = "INIT";
static const char running[] = "RUNNING";
static const char needs_poll[] = "NEEDS POLL";

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
  init,
  running,
  needs_poll};
  

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
  return sprintf((char *)uip_appdata, "%5u", httpd_fs_count(f));
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
  return sprintf((char *)uip_appdata,
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

  return sprintf((char *)uip_appdata,
		 "<tr align=\"center\"><td>%p</td><td>%s</td><td>%p</td><td>%s</td></tr>\r\n",
		 p, name,
		 (char *)((struct process *)p)->thread,
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

HTTPD_CGI_CALL(file, "file-stats", file_stats);
HTTPD_CGI_CALL(tcp, "tcp-connections", tcp_stats);
HTTPD_CGI_CALL(proc, "processes", processes);

void
httpd_cgi_init(void)
{
  httpd_cgi_add(&file);
  httpd_cgi_add(&tcp);
  httpd_cgi_add(&proc);
}
/*---------------------------------------------------------------------------*/
