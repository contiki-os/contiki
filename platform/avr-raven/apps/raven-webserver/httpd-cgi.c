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
 * $Id: httpd-cgi.c,v 1.3 2009/07/23 16:16:07 dak664 Exp $
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
#include "httpd-fsdata.h"
//#include "lib/petsciiconv.h"
#define petsciiconv_toascii(...)

#include "sensors.h"

#define DEBUGLOGIC 0        //see httpd.c
#if DEBUGLOGIC
#define uip_mss(...) 512
#define uip_appdata TCPBUF
extern char TCPBUF[512];
#endif

static struct httpd_cgi_call *calls = NULL;

/*cgi function names*/
#if HTTPD_FS_STATISTICS
static const char   file_name[] PROGMEM = "file-stats";
#endif
static const char    tcp_name[] PROGMEM = "tcp-connections";
static const char   proc_name[] PROGMEM = "processes";
static const char sensor_name[] PROGMEM = "sensors";

/*Process states for processes cgi*/
static const char      closed[] PROGMEM = "CLOSED";
static const char    syn_rcvd[] PROGMEM = "SYN-RCVD";
static const char    syn_sent[] PROGMEM = "SYN-SENT";
static const char established[] PROGMEM = "ESTABLISHED";
static const char  fin_wait_1[] PROGMEM = "FIN-WAIT-1";
static const char  fin_wait_2[] PROGMEM = "FIN-WAIT-2";
static const char     closing[] PROGMEM = "CLOSING";
static const char   time_wait[] PROGMEM = "TIME-WAIT";
static const char    last_ack[] PROGMEM = "LAST-ACK";
static const char        none[] PROGMEM = "NONE";
static const char     running[] PROGMEM = "RUNNING";
static const char      called[] PROGMEM = "CALLED";
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

  char sensor_temperature[12];

  uint8_t sprint_ip6(uip_ip6addr_t addr, char * result);


void
web_set_temp(char *s)
{
  strcpy(sensor_temperature, s);
}

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
    if(strncmp_P(name, f->name, strlen_P(f->name)) == 0) {
      return f->function;
    }
  }
  return nullfunction;
}

#if HTTPD_FS_STATISTICS
static char *thisfilename;
/*---------------------------------------------------------------------------*/
static unsigned short
generate_file_stats(void *arg)
{
  char tmp[20];
  struct httpd_fsdata_file_noconst *f,fram;
  u16_t i;
  unsigned short numprinted;

  /* Transfer arg from whichever flash that contains the html file to RAM */
  httpd_fs_cpy(&tmp, (char *)arg, 20);

  /* Count for this page, with common page footer */
  if (tmp[0]=='.') {
    numprinted=snprintf_P((char *)uip_appdata, uip_mss(),
      PSTR("<p class=right><br><br><i>This page has been sent %u times</i></div></body></html>"), httpd_fs_open(thisfilename, 0));

  /* Count for all files */
  /* Note buffer will overflow if there are too many files! */
  } else if (tmp[0]=='*') {
    i=0;numprinted=0;
    for(f = (struct httpd_fsdata_file_noconst *)httpd_fs_get_root();
        f != NULL;
        f = (struct httpd_fsdata_file_noconst *)fram.next) {

      /* Get the linked list file entry into RAM from program flash memory*/
      memcpy_P(&fram,f,sizeof(fram));
 
      /* Get the file name from whatever flash memory it is in */
      httpd_fs_cpy(&tmp, fram.name, sizeof(tmp));
      numprinted+=snprintf_P((char *)uip_appdata + numprinted, uip_mss() - numprinted,
#if HTTPD_FS_STATISTICS==1
          PSTR("<tr><td><a href=\"%s\">%s</a></td><td>%d</td>"),tmp,tmp,f->count);
#elif HTTPD_FS_STATISTICS==2
          PSTR("<tr><td><a href=\"%s\">%s</a></td><td>%d</td>"),tmp,tmp,httpd_filecount[i++]);
#endif
    }

  /* Count for specified file */
  } else {
    numprinted=snprintf_P((char *)uip_appdata, uip_mss(), PSTR("%5u"), httpd_fs_open(tmp, 0));
  }
#if DEBUGLOGIC
  return 0;
#endif
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(file_stats(struct httpd_state *s, char *ptr))
{

  PSOCK_BEGIN(&s->sout);

  thisfilename=&s->filename[0]; //temporary way to pass filename to generate_file_stats
  
  PSOCK_GENERATOR_SEND(&s->sout, generate_file_stats, (void *) ptr);
  
  PSOCK_END(&s->sout);
}
#endif /*HTTPD_FS_STATISTICS*/
/*---------------------------------------------------------------------------*/
static unsigned short
make_tcp_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  char tstate[20];
  uint16_t numprinted;

  conn = &uip_conns[s->u.count];

  numprinted = snprintf_P((char *)uip_appdata, uip_mss(),
                 PSTR("<tr align=\"center\"><td>%d</td><td>"), 
                 htons(conn->lport));
                 
  numprinted += sprint_ip6(conn->ripaddr, uip_appdata + numprinted);
  strcpy_P(tstate,states[conn->tcpstateflags & UIP_TS_MASK]);
  numprinted +=  snprintf_P((char *)uip_appdata + numprinted, uip_mss() - numprinted,
                 PSTR("-%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n"),
                 htons(conn->rport),
                 tstate,
                 conn->nrtx,
                 conn->timer,
                 (uip_outstanding(conn))? '*':' ',
                 (uip_stopped(conn))? '!':' ');

  return numprinted;
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
  char name[40],tstate[20];

  strncpy(name, ((struct process *)p)->name, 40);
  petsciiconv_toascii(name, 40);
  strcpy_P(tstate,states[9 + ((struct process *)p)->state]);
  return snprintf_P((char *)uip_appdata, uip_mss(),
    PSTR("<tr align=\"center\"><td>%p</td><td>%s</td><td>%p</td><td>%s</td></tr>\r\n"),
    p, name,
//  *((char **)&(((struct process *)p)->thread)),
    *(char *)(&(((struct process *)p)->thread)),

    tstate);
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
static unsigned short
generate_sensor_readings(void *arg)
{
  if (!sensor_temperature[0]) return snprintf_P((char *)uip_appdata,uip_mss(),PSTR("<em>Temperature:</em> Not enabled\n"));
  return snprintf_P((char *)uip_appdata, uip_mss(), PSTR("<em>Temperature:</em> %s\n"), sensor_temperature);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(sensor_readings(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_sensor_readings, s);
  
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

#if HTTPD_FS_STATISTICS
HTTPD_CGI_CALL(file, file_name, file_stats);
#endif
HTTPD_CGI_CALL(tcp, tcp_name, tcp_stats);
HTTPD_CGI_CALL(proc, proc_name, processes);
HTTPD_CGI_CALL(sensors, sensor_name, sensor_readings);

void
httpd_cgi_init(void)
{
#if HTTPD_FS_STATISTICS
  httpd_cgi_add(&file);
#endif
  httpd_cgi_add(&tcp);
  httpd_cgi_add(&proc);
  httpd_cgi_add(&sensors);
}
/*---------------------------------------------------------------------------*/



uint8_t sprint_ip6(uip_ip6addr_t addr, char * result)
        {
        unsigned char zerocnt = 0;
        unsigned char numprinted = 0;
        char * starting = result;

    unsigned char i = 0;

        while (numprinted < 8)
                {
                //Address is zero, have we used our ability to
                //replace a bunch with : yet?
                if ((addr.u16[i] == 0) && (zerocnt == 0))
                        {
                        //How mant zeros?
                        zerocnt = 0;
                        while(addr.u16[zerocnt + i] == 0)
                                zerocnt++;

                        //just one, don't waste our zeros...
                        if (zerocnt == 1)
                                {
                                *result++ = '0';
                                numprinted++;
                                break;
                                }

                        //Cool - can replace a bunch of zeros
                        i += zerocnt;
                        numprinted += zerocnt;
                        }
                //Normal address, just print it
                else
                        {
                        result += sprintf(result, "%x", (unsigned int)(ntohs(addr.u16[i])));
                        i++;
                        numprinted++;
                        }

                //Don't print : on last one
                if (numprinted != 8)
                        *result++ = ':';
                }

    return (result - starting);
        }

