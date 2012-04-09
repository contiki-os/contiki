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
#include <stdlib.h>
#include <string.h>

#include "contiki-net.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"
#include "lib/petsciiconv.h"

#include "sensors.h"

/* RADIOSTATS must also be set in clock.c and the radio driver */
#if RF230BB
#define RADIOSTATS 1
#endif
#if CONTIKI_TARGET_REDBEE_ECONOTAG
#include "adc.h"
#endif
short sleepcount;
#if WEBSERVER_CONF_CGI
static struct httpd_cgi_call *calls = NULL;
/*cgi function names*/
#if WEBSERVER_CONF_HEADER
static const char    hdr_name[] HTTPD_STRING_ATTR = "header";
#endif
#if WEBSERVER_CONF_FILESTATS
static const char   file_name[] HTTPD_STRING_ATTR = "file-stats";
#endif
#if WEBSERVER_CONF_TCPSTATS
static const char    tcp_name[] HTTPD_STRING_ATTR = "tcp-connections";
#endif
#if WEBSERVER_CONF_PROCESSES
static const char   proc_name[] HTTPD_STRING_ATTR = "processes";
#endif
#if WEBSERVER_CONF_SENSORS
static const char sensor_name[] HTTPD_STRING_ATTR = "sensors";
#endif
#if WEBSERVER_CONF_ADDRESSES
static const char   adrs_name[] HTTPD_STRING_ATTR = "addresses";
#endif
#if WEBSERVER_CONF_NEIGHBORS
static const char   nbrs_name[] HTTPD_STRING_ATTR = "neighbors";
#endif
#if WEBSERVER_CONF_ROUTES
static const char   rtes_name[] HTTPD_STRING_ATTR = "routes";
#endif
#if WEBSERVER_CONF_TICTACTOE
static const char tictac_name[] HTTPD_STRING_ATTR = "tictac";
#endif
#if WEBSERVER_CONF_AJAX
static const char   ajax_name[] HTTPD_STRING_ATTR = "ajaxdata";
#endif
#endif

/*Process states for processes cgi*/

#if WEBSERVER_CONF_PROCESSES || WEBSERVER_CONF_TCPSTATS
static const char      closed[] HTTPD_STRING_ATTR = "CLOSED";
static const char    syn_rcvd[] HTTPD_STRING_ATTR = "SYN-RCVD";
static const char    syn_sent[] HTTPD_STRING_ATTR = "SYN-SENT";
static const char established[] HTTPD_STRING_ATTR = "ESTABLISHED";
static const char  fin_wait_1[] HTTPD_STRING_ATTR = "FIN-WAIT-1";
static const char  fin_wait_2[] HTTPD_STRING_ATTR = "FIN-WAIT-2";
static const char     closing[] HTTPD_STRING_ATTR = "CLOSING";
static const char   time_wait[] HTTPD_STRING_ATTR = "TIME-WAIT";
static const char    last_ack[] HTTPD_STRING_ATTR = "LAST-ACK";
static const char        none[] HTTPD_STRING_ATTR = "NONE";
static const char     running[] HTTPD_STRING_ATTR = "RUNNING";
static const char      called[] HTTPD_STRING_ATTR = "CALLED";
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
#endif

#if WEBSERVER_CONF_SENSORS
  static char sensor_temperature[14]="Not Available";
  static char sensor_extvoltage[14]="Not Available";
  static unsigned long last_tempupdate;//,last_extvoltageupdate;
//  extern unsigned long seconds, sleepseconds;
#if RADIOSTATS
  extern unsigned long radioontime;
  static unsigned long savedradioontime;
  extern uint8_t RF230_radio_on, rf230_last_rssi;
  extern uint16_t RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail;
#endif
#endif
 
#if 0
void
web_set_temp(char *s)
{
  strcpy(sensor_temperature, s);
//  printf_P(PSTR("got temp"));
  last_tempupdate=seconds;
}
void
web_set_voltage(char *s)
{
  strcpy(sensor_extvoltage, s);
//    printf_P(PSTR("got volts"));
  last_extvoltageupdate=seconds;
}
#endif

#if WEBSERVER_CONF_CGI
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
    if(httpd_strncmp(name, f->name, httpd_strlen(f->name)) == 0) {
      return f->function;
    }
  }
  return nullfunction;
}
#if WEBSERVER_CONF_HEADER
/*---------------------------------------------------------------------------*/
static unsigned short
generate_header(void *arg)
{
  unsigned short numprinted=0;

#if WEBSERVER_CONF_HEADER_W3C
#define _MSS1 100
  static const char httpd_cgi_headerw[] HTTPD_STRING_ATTR = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerw);
#endif

#if WEBSERVER_CONF_HEADER_ICON
#define _MSS2 105
#ifdef WEBSERVER_CONF_PAGETITLE
  static const char httpd_cgi_header1[] HTTPD_STRING_ATTR = "<html><head><title>%s</title><link rel=\"icon\" href=\"favicon.gif\" type=\"image/gif\"></head><body>";
#else
  static const char httpd_cgi_header1[] HTTPD_STRING_ATTR = "<html><head><title>Contiki-Nano</title><link rel=\"icon\" href=\"favicon.gif\" type=\"image/gif\"></head><body>";
#endif
#else
#define _MSS2 52
#ifdef WEBSERVER_CONF_PAGETITLE
  static const char httpd_cgi_header1[] HTTPD_STRING_ATTR = "<html><head><title>%s</title></head><body>";
#else
  static const char httpd_cgi_header1[] HTTPD_STRING_ATTR = "<html><head><title>Contiki-Nano</title></head><body>";
#endif
#endif

#ifdef WEBSERVER_CONF_PAGETITLE
#define WAD ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr.u8
{  char buf[40];
    WEBSERVER_CONF_PAGETITLE;
    numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_header1,buf);
}
#else
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_header1);
#endif

#if WEBSERVER_CONF_HEADER_MENU
#define _MSS3 32
  static const char httpd_cgi_headerm1[] HTTPD_STRING_ATTR = "<pre><a href=\"/\">Front page</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm1);
#if WEBSERVER_CONF_SENSORS
#define _MSS4 34
  static const char httpd_cgi_headerm2[] HTTPD_STRING_ATTR = "|<a href=\"status.shtml\">Status</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm2);
#endif
#if WEBSERVER_CONF_TCPSTATS
#define _MSS5 44
  static const char httpd_cgi_headerm3[] HTTPD_STRING_ATTR = "|<a href=\"tcp.shtml\">Network connections</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm3);
#endif
#if WEBSERVER_CONF_PROCESSES
#define _MSS6 46
  static const char httpd_cgi_headerm4[] HTTPD_STRING_ATTR = "|<a href=\"processes.shtml\">System processes</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm4);
#endif
#if WEBSERVER_CONF_FILESTATS
#define _MSS7 45
  static const char httpd_cgi_headerm5[] HTTPD_STRING_ATTR = "|<a href=\"files.shtml\">File statistics</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm5);
#endif
#if WEBSERVER_CONF_TICTACTOE
#define _MSS8 44
  static const char httpd_cgi_headerm6[] HTTPD_STRING_ATTR = "|<a href=\"/ttt/ttt.shtml\">TicTacToe</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm6);
#endif
#if WEBSERVER_CONF_AJAX
#define _MSS9 30
  static const char httpd_cgi_headerm7[] HTTPD_STRING_ATTR = "|<a href=\"ajax.shtml\">Ajax</a>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerm7);
#endif
  static const char httpd_cgi_headerme[] HTTPD_STRING_ATTR = "</pre>";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_headerme);
#endif /* WEBSERVER_CONF_MENU */

#if UIP_RECEIVE_WINDOW < _MSS1+_MSS2+_MSS3_+MSS4_+MSS5_MSS6+_MSS7+_MSS8+_MSS9
#warning ************************************************************
#warning UIP_RECEIVE_WINDOW not large enough for header cgi output.
#warning Web pages will not render properly!
#warning ************************************************************
#endif
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(header(struct httpd_state *s, char *ptr))
{

  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_header, (void *) ptr);
  
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_HEADER */

#if WEBSERVER_CONF_FILESTATS
/*---------------------------------------------------------------------------*/
static unsigned short
generate_file_stats(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
#if WEBSERVER_CONF_LOADTIME
  static const char httpd_cgi_filestat1[] HTTPD_STRING_ATTR = "<p align=\"right\"><br><br><i>This page has been sent %u times (%1u.%02u sec)</i></body></html>";
#else
  static const char httpd_cgi_filestat1[] HTTPD_STRING_ATTR = "<p align=\"right\"><br><br><i>This page has been sent %u times</i></body></html>";
#endif
  static const char httpd_cgi_filestat2[] HTTPD_STRING_ATTR = "<tr><td><a href=\"%s\">%s</a></td><td>%d</td>";
  static const char httpd_cgi_filestat3[] HTTPD_STRING_ATTR = "%5u";
  char tmp[20];
  struct httpd_fsdata_file_noconst *f,fram;
  uint16_t i;
  unsigned short numprinted;
  /* Transfer arg from whichever flash that contains the html file to RAM */
  httpd_fs_cpy(&tmp, s->u.ptr, 20);

  /* Count for this page, with common page footer */
  if (tmp[0]=='.') { 
#if WEBSERVER_CONF_LOADTIME
    s->pagetime = clock_time() - s->pagetime;
    numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_filestat1, httpd_fs_open(s->filename, 0), 
            (unsigned int)s->pagetime/CLOCK_SECOND,(100*((unsigned int)s->pagetime%CLOCK_SECOND))/CLOCK_SECOND);
#else
    numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_filestat1, httpd_fs_open(s->filename, 0));
#endif

  /* Count for all files */
  /* Note buffer will overflow if there are too many files! */
  } else if (tmp[0]=='*') {
    i=0;numprinted=0;
    for(f = (struct httpd_fsdata_file_noconst *)httpd_fs_get_root();
        f != NULL;
        f = (struct httpd_fsdata_file_noconst *)fram.next) {

      /* Get the linked list file entry into RAM from from wherever it is*/
      httpd_memcpy(&fram,f,sizeof(fram));
      /* Get the file name from whatever memory it is in */
      httpd_fs_cpy(&tmp, fram.name, sizeof(tmp));
#if WEBSERVER_CONF_FILESTATS==2
      numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_filestat2, tmp, tmp, f->count);
#else
      numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_filestat2, tmp, tmp, httpd_filecount[i]);
#endif
      i++;
    }

  /* Count for specified file */
  } else {
    numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_filestat3, httpd_fs_open(tmp, 0));
  }
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(file_stats(struct httpd_state *s, char *ptr))
{

  PSOCK_BEGIN(&s->sout);

  /* Pass string after cgi invocation to the generator */
  s->u.ptr = ptr;
  PSOCK_GENERATOR_SEND(&s->sout, generate_file_stats, s);
  
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_FILESTATS*/

#if WEBSERVER_CONF_TCPSTATS
/*---------------------------------------------------------------------------*/
static unsigned short
make_tcp_stats(void *arg)
{
  static const char httpd_cgi_tcpstat1[] HTTPD_STRING_ATTR = "<tr align=\"center\"><td>%d</td><td>";
  static const char httpd_cgi_tcpstat2[] HTTPD_STRING_ATTR = "-%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n";
  static const char httpd_cgi_tcpstat3[] HTTPD_STRING_ATTR = "[Room for %d more]";

  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  char tstate[20];
  uint16_t numprinted;
  
  if (s->u.count==UIP_CONNS){
    for(numprinted = 0; numprinted < UIP_CONNS; numprinted++ ) {
	    if((uip_conns[numprinted].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) s->u.count--;
	}
    return(httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_tcpstat3, s->u.count));
  }

  conn = &uip_conns[s->u.count];

  numprinted = httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_tcpstat1, uip_htons(conn->lport));
  numprinted += httpd_cgi_sprint_ip6(conn->ripaddr, uip_appdata + numprinted);
  httpd_strcpy(tstate,states[conn->tcpstateflags & UIP_TS_MASK]);
  numprinted +=  httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted,
                 httpd_cgi_tcpstat2,
                 uip_htons(conn->rport),
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

  s->u.count=UIP_CONNS;
  PSOCK_GENERATOR_SEND(&s->sout, make_tcp_stats, s);
  
  for(s->u.count = 0; s->u.count < UIP_CONNS; ++s->u.count) {
    if((uip_conns[s->u.count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, make_tcp_stats, s);
    }
  }

  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_TCPSTATS */

#if WEBSERVER_CONF_PROCESSES
/*---------------------------------------------------------------------------*/
static unsigned short
make_processes(void *p)
{
  static const char httpd_cgi_proc[] HTTPD_STRING_ATTR = "<tr align=\"center\"><td>%p</td><td>%s</td><td>%p</td><td>%s</td></tr>\r\n";
  char name[40],tstate[20];

  strncpy(name, PROCESS_NAME_STRING((struct process *)p), 40);
  petsciiconv_toascii(name, 40);
  httpd_strcpy(tstate,states[9 + ((struct process *)p)->state]);
  return httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_proc, p, name,
//  *((char **) &(((struct process *)p)->thread)),
    * (char **)(&(((struct process *)p)->thread)), //minimal net
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
#endif /* WEBSERVER_CONF_PROCESSES */

#if WEBSERVER_CONF_ADDRESSES || WEBSERVER_CONF_NEIGHBORS || WEBSERVER_CONF_ROUTES
#if WEBSERVER_CONF_SHOW_ROOM
static const char httpd_cgi_addrf[] HTTPD_STRING_ATTR = "[Room for %u more]\n";
#else
static const char httpd_cgi_addrf[] HTTPD_STRING_ATTR = "[Table is full]\n";
#endif
static const char httpd_cgi_addrn[] HTTPD_STRING_ATTR = "[None]\n";
#endif

#if WEBSERVER_CONF_ADDRESSES
/*---------------------------------------------------------------------------*/
extern uip_ds6_netif_t uip_ds6_if;

static unsigned short
make_addresses(void *p)
{
uint8_t i,j=0;
uint16_t numprinted = 0;
  for (i=0; i<UIP_DS6_ADDR_NB;i++) {
    if (uip_ds6_if.addr_list[i].isused) {
      j++;
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, uip_appdata + numprinted);
      *((char *)uip_appdata+numprinted++) = '\n';
    }
  }
#if WEBSERVER_CONF_SHOW_ROOM
  numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf, UIP_DS6_ADDR_NB-j);
#else
  if(UIP_DS6_ADDR_NB == j) {
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf);
  }
#endif
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
#endif /* WEBSERVER_CONF_ADDRESSES */

#if WEBSERVER_CONF_NEIGHBORS
extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
/*---------------------------------------------------------------------------*/	
static unsigned short
make_neighbors(void *p)
{
uint8_t i,j;
uint16_t numprinted=0;
struct httpd_state *s=p;
  /* Span generator calls over tcp segments */
  /* Note retransmissions will execute thise code multiple times for a segment */
  i=s->starti;j=s->startj;
  for (;i<UIP_DS6_NBR_NB;i++) {
    if (uip_ds6_nbr_cache[i].isused) {
      j++;

#if WEBSERVER_CONF_NEIGHBOR_STATUS
static const char httpd_cgi_nbrs1[] HTTPD_STRING_ATTR = " INCOMPLETE";
static const char httpd_cgi_nbrs2[] HTTPD_STRING_ATTR = " REACHABLE";
static const char httpd_cgi_nbrs3[] HTTPD_STRING_ATTR = " STALE";
static const char httpd_cgi_nbrs4[] HTTPD_STRING_ATTR = " DELAY";
static const char httpd_cgi_nbrs5[] HTTPD_STRING_ATTR = " NBR_PROBE";
{uint16_t k=numprinted+25;
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_nbr_cache[i].ipaddr, uip_appdata + numprinted);
      while (numprinted < k) {*((char *)uip_appdata+numprinted++) = ' ';}
      switch (uip_ds6_nbr_cache[i].state) {
      case NBR_INCOMPLETE: numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_nbrs1);break;
      case NBR_REACHABLE:  numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_nbrs2);break;
      case NBR_STALE:      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_nbrs3);break;  
      case NBR_DELAY:      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_nbrs4);break;
      case NBR_PROBE:      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_nbrs5);break;
      }
}
#else
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_nbr_cache[i].ipaddr, uip_appdata + numprinted);
#endif
	  *((char *)uip_appdata+numprinted++) = '\n';

	  /* If buffer near full, send it and wait for the next call. Could be a retransmission, or the next segment */
	  if(numprinted > (uip_mss() - 50)) {
		s->savei=i;s->savej=j;
	    return numprinted;
	  }
    }
  }
#if WEBSERVER_CONF_SHOW_ROOM
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf,UIP_DS6_NBR_NB-j);
#else
  if(UIP_DS6_NBR_NB == j) {
  	numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf);
  }
#endif

  /* Signal that this was the last segment */
  s->savei = 0;  
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(neighbors(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  /* Send as many TCP segments as needed for the neighbor table */
  /* Move to next seqment after each successful transmission */
  s->starti=s->startj=0;
  do {
	PSOCK_GENERATOR_SEND(&s->sout, make_neighbors, (void *)s);
	s->starti=s->savei+1;s->startj=s->savej;
  } while(s->savei);  
  
  PSOCK_END(&s->sout);
}
#endif

#if WEBSERVER_CONF_ROUTES
extern uip_ds6_route_t uip_ds6_routing_table[];
#if WEBSERVER_CONF_ROUTE_LINKS
static const char httpd_cgi_rtesl1[] HTTPD_STRING_ATTR = "<a href=http://[";
static const char httpd_cgi_rtesl2[] HTTPD_STRING_ATTR = "]/status.shtml>";
static const char httpd_cgi_rtesl3[] HTTPD_STRING_ATTR = "</a>";
#endif
/*---------------------------------------------------------------------------*/			
static unsigned short
make_routes(void *p)
{
static const char httpd_cgi_rtes1[] HTTPD_STRING_ATTR = "/%u (via ";
static const char httpd_cgi_rtes2[] HTTPD_STRING_ATTR = ") %lus\n";
static const char httpd_cgi_rtes3[] HTTPD_STRING_ATTR = ")\n";
uint8_t i,j;
uint16_t numprinted=0;
struct httpd_state *s=p;
  /* Span generator calls over tcp segments */
  /* Note retransmissions will execute thise code multiple times for a segment */
  i=s->starti;j=s->startj;
  for (;i<UIP_DS6_ROUTE_NB;i++) {
    if (uip_ds6_routing_table[i].isused) {
      j++;

#if WEBSERVER_CONF_ROUTE_LINKS
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl1);
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_routing_table[i].ipaddr, uip_appdata + numprinted);
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl2);
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_routing_table[i].ipaddr, uip_appdata + numprinted);
      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl3);
#else
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_routing_table[i].ipaddr, uip_appdata + numprinted);
#endif

      numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes1, uip_ds6_routing_table[i].length);
      numprinted += httpd_cgi_sprint_ip6(uip_ds6_routing_table[i].nexthop, uip_appdata + numprinted);
      if(1 || uip_ds6_routing_table[i].state.lifetime < 3600) {
         numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes2, (long unsigned int)uip_ds6_routing_table[i].state.lifetime);
      } else {
         numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtes3);
      }
      /* If buffer near full, send it and wait for the next call. Could be a retransmission, or the next segment */
      if(numprinted > (uip_mss() - 200)) {
        s->savei=i;s->savej=j;
        return numprinted;
      }
    }
  }
  if (j==0) numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrn);
#if WEBSERVER_CONF_SHOW_ROOM
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf,UIP_DS6_ROUTE_NB-j);
#else
  if(UIP_DS6_ROUTE_NB == j) {
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_addrf);
  }
#endif
{
 static const char httpd_cgi_defr1[] HTTPD_STRING_ATTR = "\n<big><b>RPL Parent</b></big>\n";
 static const char httpd_cgi_defr2[] HTTPD_STRING_ATTR = " (%u sec)\n";
#if 0
  uip_ip6addr_t *nexthop = uip_ds6_defrt_choose();
  if (nexthop) {
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_defr1);   
    numprinted += httpd_cgi_sprint_ip6(*nexthop, uip_appdata + numprinted);
    numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_defr2,nexthop->lifetime.start+nexthop->lifetime.interval-clock_seconds()); 
  }
#else
uip_ds6_defrt_t *locdefrt;
extern uip_ds6_defrt_t uip_ds6_defrt_list[UIP_DS6_DEFRT_NB];
    for(locdefrt = uip_ds6_defrt_list;
      locdefrt < uip_ds6_defrt_list + UIP_DS6_DEFRT_NB; locdefrt++) {
    if(locdefrt->isused) {    
        numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_defr1);

#if WEBSERVER_CONF_ROUTE_LINKS && 0
        numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl1);
        numprinted += httpd_cgi_sprint_ip6(locdefrt->ipaddr, uip_appdata + numprinted); 
        numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl2);
        numprinted += httpd_cgi_sprint_ip6(locdefrt->ipaddr, uip_appdata + numprinted); 
        numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_rtesl3);
#else
        numprinted += httpd_cgi_sprint_ip6(locdefrt->ipaddr, uip_appdata + numprinted); 
#endif   
        numprinted += httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_defr2,locdefrt->lifetime.start+locdefrt->lifetime.interval-clock_seconds());
  //      break;
        }
   }
 
#endif
}
  /* Signal that this was the last segment */
  s->savei = 0;
  return numprinted;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(routes(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  /* Send as many TCP segments as needed for the route table */
  /* Move to next seqment after each successful transmission */
  s->starti=s->startj=0;
  do {
    PSOCK_GENERATOR_SEND(&s->sout, make_routes, s);
    s->starti=s->savei+1;s->startj=s->savej;
  } while(s->savei);
 
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_ROUTES */

#if WEBSERVER_CONF_SENSORS
/*---------------------------------------------------------------------------*/
static unsigned short
generate_sensor_readings(void *arg)
{
  uint16_t numprinted=0;
  uint16_t days,h,m,s;
  unsigned long seconds=clock_seconds();
  static const char httpd_cgi_sensor0[] HTTPD_STRING_ATTR = "[Updated %d seconds ago]\n";
  static const char httpd_cgi_sensor1[] HTTPD_STRING_ATTR = "<em>Temperature:</em> %s\n";
  static const char httpd_cgi_sensor2[] HTTPD_STRING_ATTR = "<em>Battery    :</em> %s\n";
  static const char httpd_cgi_sensor3[] HTTPD_STRING_ATTR = "<em>Uptime     :</em> %02d:%02d:%02d\n";
  static const char httpd_cgi_sensor3d[] HTTPD_STRING_ATTR = "<em>Uptime    :</em> %u days %02u:%02u:%02u\n";

  /* Generate temperature and voltage strings for each platform */
#if CONTIKI_TARGET_AVR_ATMEGA128RFA1  
{uint8_t i;
  BATMON = 16; //give BATMON time to stabilize at highest range and lowest voltage

/* Measure internal temperature sensor, see atmega128rfa1 datasheet */
/* This code disabled by default for safety.
   Selecting an internal reference will short it to anything connected to the AREF pin
 */
#if 0
  ADCSRB|=1<<MUX5;          //this bit buffered till ADMUX written to!
  ADMUX =0xc9;              // Select internal 1.6 volt ref, temperature sensor ADC channel
  ADCSRA=0x85;              //Enable ADC, not free running, interrupt disabled, clock divider 32 (250 KHz@ 8 MHz)
//  while ((ADCSRB&(1<<AVDDOK))==0);  //wait for AVDD ok
//  while ((ADCSRB&(1<<REFOK))==0);  //wait for ref ok 
  ADCSRA|=1<<ADSC;          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  h=ADC;                    //Read adc
  h=11*h-2728+(h>>2);       //Convert to celcius*10 (should be 11.3*h, approximate with 11.25*h)
  ADCSRA=0;                 //disable ADC
  ADMUX=0;                  //turn off internal vref      
  m=h/10;s=h-10*m;
  static const char httpd_cgi_sensor1_printf[] HTTPD_STRING_ATTR = "%d.%d C";
  httpd_snprintf(sensor_temperature,sizeof(sensor_temperature),httpd_cgi_sensor1_printf,m,s);
#endif

/* Bandgap can't be measured against supply voltage in this chip. */
/* Use BATMON register instead */
  for ( i=16; i<31; i++) {
    BATMON = i;
    if ((BATMON&(1<<BATMON_OK))==0) break;
  }
  h=2550-75*16-75+75*i; //-75 to take the floor of the 75 mv transition window
  static const char httpd_cgi_sensor2_printf[] HTTPD_STRING_ATTR = "%u mv";
  httpd_snprintf(sensor_extvoltage,sizeof(sensor_extvoltage),httpd_cgi_sensor2_printf,h);
}
#elif CONTIKI_TARGET_AVR_RAVEN
{
#if 1
/* Usual way to get AVR supply voltage, measure 1.1v bandgap using Vcc as reference.
 * This connects the bandgap to the AREF pin, so enable only if there is no external AREF!
 * A capacitor may be connected to this pin to reduce reference noise.
 */
  ADMUX =0x5E;              //Select AVCC as reference, measure 1.1 volt bandgap reference.
  ADCSRA=0x87;              //Enable ADC, not free running, interrupt disabled, clock divider  128 (62 KHz@ 8 MHz)
  ADCSRA|=1<<ADSC;          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
//h=1126400UL/ADC;          //Get supply voltage (factor nominally 1100*1024)
  h=1198070UL/ADC;          //My Raven
  ADCSRA=0;                 //disable ADC
  ADMUX=0;                  //turn off internal vref    

  static const char httpd_cgi_sensor2_printf[] HTTPD_STRING_ATTR = "%u mv";
  httpd_snprintf(sensor_extvoltage,sizeof(sensor_extvoltage),httpd_cgi_sensor2_printf,h);
#endif
}
#elif CONTIKI_TARGET_REDBEE_ECONOTAG
//#include "adc.h"
{
#if 0
/* Scan ADC channels if not already being done elsewhere */
uint8_t c;
  adc_reading[8]=0;
  adc_init();
  while (adc_reading[8]==0) adc_service();
//for (c=0; c<NUM_ADC_CHAN; c++) printf("%u %04u\r\n", c, adc_reading[c]);
  adc_disable();
#endif

  static const char httpd_cgi_sensorv[] HTTPD_STRING_ATTR = "<em>ADC chans  :</em> %u %u %u %u %u %u %u %u \n";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensorv,
  adc_reading[0],adc_reading[1],adc_reading[2],adc_reading[3],adc_reading[4],adc_reading[5],adc_reading[6],adc_reading[7]);

}
#endif

  if (last_tempupdate) {
    numprinted =httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_sensor0,(unsigned int) (seconds-last_tempupdate));
  }
  if (sensor_temperature[0]!='N') {
    numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor1, sensor_temperature);
  }

#if CONTIKI_TARGET_REDBEE_ECONOTAG
/* Econotag at 3v55 with 10 ohms to LiFePO4 battery:  3680mv usb 3573 2 Fresh alkaline AAs. Take 3590 as threshold for USB connected */
    static const char httpd_cgi_sensor2u[] HTTPD_STRING_ATTR = "<em>Vcc (USB)  :</em> %s\n";
    if(adc_reading[8]<1368) {
        numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor2u, sensor_extvoltage);
    } else {
        numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor2, sensor_extvoltage);
    }
#else
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor2, sensor_extvoltage);
#endif

#if RADIOSTATS
  /* Remember radioontime for display below - slow connection might make it report longer than cpu ontime! */
  savedradioontime = radioontime;
#endif
  h=seconds/3600;s=seconds-h*3600;m=s/60;s=s-m*60;
  days=h/24;
  if (days == 0) {
    numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor3, h,m,s);
  } else {
  	h=h-days*24;	
	numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor3d, days,h,m,s);
  }
  return numprinted;
}
#if WEBSERVER_CONF_STATISTICS
/*---------------------------------------------------------------------------*/
static unsigned short
generate_stats(void *arg)
{
  uint16_t numprinted;
  uint16_t h,m,s;
  uint8_t p1,p2;
  uint32_t seconds=clock_seconds();
  
  static const char httpd_cgi_stats[] HTTPD_STRING_ATTR = "\n<big><b>Statistics</b></big>\n";
  numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_stats);

#if ENERGEST_CONF_ON
{uint8_t p1,p2;
 uint32_t sl;
#if 0
/* Update all the timers to get current values */
  for (p1=1;p1<ENERGEST_TYPE_MAX;p1++) {
    if (energest_current_mode[p1]) {
      ENERGEST_OFF(p1);
      ENERGEST_ON(p1);
    }
  }
#else
  energest_flush();
#endif
  static const char httpd_cgi_sensor4[] HTTPD_STRING_ATTR =  "<em>CPU time   (ENERGEST):</em> %02u:%02u:%02u (%u.%02u%%)\n";
  static const char httpd_cgi_sensor10[] HTTPD_STRING_ATTR = "<em>Radio      (ENERGEST):</em> Tx %02u:%02u:%02u (%u.%02u%%)  ";
  static const char httpd_cgi_sensor11[] HTTPD_STRING_ATTR = "Rx %02u:%02u:%02u (%u.%02u%%)\n";
  sl=energest_total_time[ENERGEST_TYPE_CPU].current/RTIMER_ARCH_SECOND;
  h=(10000UL*sl)/seconds;p1=h/100;p2=h-p1*100;h=sl/3600;s=sl-h*3600;m=s/60;s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor4, h,m,s,p1,p2);

  sl=energest_total_time[ENERGEST_TYPE_TRANSMIT].current/RTIMER_ARCH_SECOND;
  h=(10000UL*sl)/seconds;p1=h/100;p2=h-p1*100;h=sl/3600;s=sl-h*3600;m=s/60;s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor10, h,m,s,p1,p2);

  sl=energest_total_time[ENERGEST_TYPE_LISTEN].current/RTIMER_ARCH_SECOND;
  h=(10000UL*sl)/seconds;p1=h/100;p2=h-p1*100;h=sl/3600;s=sl-h*3600;m=s/60;s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor11, h,m,s,p1,p2);
}
#endif /* ENERGEST_CONF_ON */

#if CONTIKIMAC_CONF_COMPOWER
#include "sys/compower.h"
{uint8_t p1,p2;
 // extern struct compower_activity current_packet;
  static const char httpd_cgi_sensor31[] HTTPD_STRING_ATTR = "<em>ContikiMAC (COMPOWER):</em> Tx %02u:%02u:%02u (%u.%02u%%)  ";
  static const char httpd_cgi_sensor32[] HTTPD_STRING_ATTR = "Rx %02u:%02u:%02u (%u.%02u%%)\n";

  s=compower_idle_activity.transmit/RTIMER_ARCH_SECOND;
  h=((10000UL*compower_idle_activity.transmit)/RTIMER_ARCH_SECOND)/seconds;
  p1=h/100;p2=h-p1*100;h=s/3600;s=s-h*3600;m=s/60;s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor31, h,m,s,p1,p2);

  s=compower_idle_activity.listen/RTIMER_ARCH_SECOND;
  h=((10000UL*compower_idle_activity.listen)/RTIMER_ARCH_SECOND)/seconds;
  p1=h/100;p2=h-p1*100;h=s/3600;s=s-h*3600;m=s/60;s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor32, h,m,s,p1,p2);

}
#endif

#if RIMESTATS_CONF_ON
#include "net/rime/rimestats.h"
  static const char httpd_cgi_sensor21[] HTTPD_STRING_ATTR = "<em>Packets   (RIMESTATS):</em> Tx=%5lu  Rx=%5lu   TxL=%4lu  RxL=%4lu\n";
  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor21,
		rimestats.tx,rimestats.rx,rimestats.lltx-rimestats.tx,rimestats.llrx-rimestats.rx);
#endif

#if RADIOSTATS
  /* From RF230 statistics */
  static const char httpd_cgi_sensor10[] HTTPD_STRING_ATTR = "<em>Radio on  (RF230BB)  :</em> %02d:%02d:%02d (%d.%02d%%)\n";
  static const char httpd_cgi_sensor11[] HTTPD_STRING_ATTR = "<em>Packets:  (RF230BB)  :</em> Tx=%5d Rx=%5d  TxL=%5d RxL=%5d RSSI=%2ddBm\n";

  s=(10000UL*savedradioontime)/seconds;
  p1=s/100;
  p2=s-p1*100;
  h=savedradioontime/3600;
  s=savedradioontime-h*3600;
  m=s/60;
  s=s-m*60;

  numprinted =httpd_snprintf((char *)uip_appdata             , uip_mss()             , httpd_cgi_sensor10,\
    h,m,s,p1,p2);

#if RF230BB
  numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor11,\
    RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail,-92+rf230_last_rssi);
#else
  p1=0;
  radio_get_rssi_value(&p1);
  p1 = -91*3(p1-1);
  numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor11,\
    RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail,p1);
#endif
#endif /* RADIOSTATS */
  return numprinted;
}
#endif
/*---------------------------------------------------------------------------*/
static
PT_THREAD(sensor_readings(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_sensor_readings, s);
#if WEBSERVER_CONF_STATISTICS
  PSOCK_GENERATOR_SEND(&s->sout, generate_stats, s);
#endif
 
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_SENSORS */

#if WEBSERVER_CONF_TICTACTOE
/*---------------------------------------------------------------------------*/
static uint8_t whowon(char x) {

  if (httpd_query[0]==x) {
    if(((httpd_query[1]==x)&&(httpd_query[2]==x))
    || ((httpd_query[4]==x)&&(httpd_query[8]==x))
    || ((httpd_query[3]==x)&&(httpd_query[6]==x))){
      return 1;
    }
  }
  if (httpd_query[1]==x) {
    if ((httpd_query[4]==x)&&(httpd_query[7]==x)) {
      return 1;
    }
  }
  if (httpd_query[2]==x) {
    if(((httpd_query[4]==x)&&(httpd_query[6]==x))
    || ((httpd_query[5]==x)&&(httpd_query[8]==x))){
      return 1;
    }
  }
  if (httpd_query[3]==x) {
    if ((httpd_query[4]==x)&&(httpd_query[5]==x)) {
      return 1;
    }
  }
  if (httpd_query[6]==x) {
    if ((httpd_query[7]==x)&&(httpd_query[8]==x)) {
      return 1;
    }
  }
 return 0;
}

/*---------------------------------------------------------------------------*/
static unsigned short
make_tictactoe(void *p)
{
  uint8_t i,newgame,iwon,uwon,nx,no;
  char me,you,locater;
  unsigned short numprinted=0;
  
 /* If no query string restart game, else put into proper form */
  newgame=0;httpd_query[9]=0;
  if ((httpd_query[0]==0)||(httpd_query[0]==' ')) {
    newgame=1;
    for (i=0;i<9;i++) httpd_query[i]='b';
  } else for (i=0;i<9;i++) {
    if (!((httpd_query[i]=='x')||(httpd_query[i]=='o'))) {
      httpd_query[i]='b';
    }
  }

  /* I am x if I move first, or if number of x's is <= number of o's */
  for (nx=0,no=0,i=0;i<9;i++) {
    if (httpd_query[i]=='x') nx++;
    else if (httpd_query[i]=='o') no++;
  }
  if ((no>=nx)&&!newgame) {me='x';you='o';}
  else {me='o';you='x';};

  iwon=whowon(me);
  uwon=whowon(you);

  if (newgame||iwon||uwon||(nx+no)>=9) goto showboard;
 
  /* Make a move */
  if (me=='x') nx++;else no++;
  if (httpd_query[4]=='b') httpd_query[4]=me;
  else if (httpd_query[0]=='b') httpd_query[0]=me;
  else if (httpd_query[2]=='b') httpd_query[2]=me;
  else if (httpd_query[6]=='b') httpd_query[6]=me;
  else if (httpd_query[8]=='b') httpd_query[8]=me;
  else if (httpd_query[1]=='b') httpd_query[1]=me;
  else if (httpd_query[3]=='b') httpd_query[3]=me;
  else if (httpd_query[5]=='b') httpd_query[5]=me;
  else if (httpd_query[7]=='b') httpd_query[7]=me;
  
  /* Did I win? */
  iwon=whowon(me);

  showboard: 
  for (i=0;i<9;i++) {
  
    if (i==4) locater='c';
    else if ((i==1)||(i==7)) locater='v';
    else if ((i==3)||(i==5)) locater='h';
    else locater=0;
    
    if ((httpd_query[i]=='b')&&(!(iwon||uwon))) {
        httpd_query[i]=you;
        numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<a href=ttt.shtml?%s><img src=b",httpd_query);
        httpd_query[i]='b';
    } else {
        numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<img src=%c",httpd_query[i]);
    }
    if (locater) {
        numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "%c",locater);
    }
    numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, ".gif>");
    if (httpd_query[i]=='b') {       
        numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "</a>");       
    }
    if ((i==2)||(i==5)) {
      numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br>");
    }
  }
  
  if ((nx>(no+1))||(no>(nx+1))) {
     numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><h2>You cheated!!!</h2>");
  } else if (iwon) {
     numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><h2>I Win!</h2>");
  } else if (uwon) { 
     numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><h2>You Win!</h2>");
  } else if ((nx+no)==9) {
     numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><h2>Draw!</h2>");
  }
  if (iwon||uwon||((nx+no)==9)) {
       numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><a href=ttt.shtml>Play Again</a>");
  }

  /* If new game give option for me to start */
  if ((nx==0)&&(no==0)) {
     numprinted+=snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, "<br><br><a href=ttt.shtml?bbbbbbbb>Let computer move first</a>");
  }
  httpd_query[0]=0;  //zero the query string
  return numprinted;

}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(tictactoe(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, make_tictactoe, s);
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_TICTACTOE */

#if WEBSERVER_CONF_AJAX
/*---------------------------------------------------------------------------*/
static
PT_THREAD(ajax_call(struct httpd_state *s, char *ptr))
{
  static struct timer t;
  static int iter;
  static char buf[128];
  static uint8_t numprinted;
  PSOCK_BEGIN(&s->sout);

#if WEBSERVER_CONF_PASSQUERY
/* Get update time from ? string, if present */
{
   uint8_t i;uint16_t dt=0;
   for (i=0;i<WEBSERVER_CONF_PASSQUERY;i++) {
    if (httpd_query[i] < '0' || httpd_query[i] > '9') break;
    dt = dt*10 + httpd_query[i]-'0';
   }
   if (dt < 1) dt = 1;
 //  printf("dt %u\n",dt);
   timer_set(&t, dt*CLOCK_SECOND);
   httpd_query[i]=0; //necessary?
   if (dt > WEBSERVER_CONF_TIMEOUT/2) s->ajax_timeout = 2*dt; else s->ajax_timeout = WEBSERVER_CONF_TIMEOUT;
}
#else
    timer_set(&t, 2*CLOCK_SECOND);
#endif

  iter = 0;
  
  while(1) {
  	iter++;

#if CONTIKI_TARGET_SKY
    SENSORS_ACTIVATE(sht11_sensor);
    SENSORS_ACTIVATE(light_sensor);
    numprinted = snprintf(buf, sizeof(buf),
	     "t(%d);h(%d);l1(%d);l2(%d);",
	     sht11_sensor.value(SHT11_SENSOR_TEMP),
	     sht11_sensor.value(SHT11_SENSOR_HUMIDITY),
         light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC),
         light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR));
    SENSORS_DEACTIVATE(sht11_sensor);
    SENSORS_DEACTIVATE(light_sensor);

#elif CONTIKI_TARGET_MB851
  SENSORS_ACTIVATE(acc_sensor);    
  numprinted = snprintf(buf, sizeof(buf),"t(%d);ax(%d);ay(%d);az(%d);",
	     temperature_sensor.value(0),
	     acc_sensor.value(ACC_X_AXIS),
	     acc_sensor.value(ACC_Y_AXIS),
	     acc_sensor.value(ACC_Z_AXIS));   
  SENSORS_DEACTIVATE(acc_sensor);

#elif CONTIKI_TARGET_REDBEE_ECONOTAG
#if 0
/* Scan ADC channels if not already done elsewhere */
{ uint8_t c;
  adc_reading[8]=0;
  adc_init();
  while (adc_reading[8]==0) adc_service();
  adc_disable();
#endif


#if 1
   numprinted = snprintf(buf, sizeof(buf),"b(%u);adc(%u,%u,%u,%u,%u,%u,%u,%u);",
      1200*0xfff/adc_reading[8],adc_reading[0],adc_reading[1],adc_reading[2],adc_reading[3],adc_reading[4],adc_reading[5],adc_reading[6],adc_reading[7]);
#else
 //    numprinted = snprintf(buf, sizeof(buf),"b(%u);",1200*0xfff/adc_reading[8]);
        numprinted = snprintf(buf, sizeof(buf),"b(%u);adc(%u,%u,%u);",1200*0xfff/adc_reading[8],adc_reading[1],adc_reading[7],adc_reading[8]);
#endif
}
   if (iter<3) {
    static const char httpd_cgi_ajax11[] HTTPD_STRING_ATTR = "wt('Econtag [";
    static const char httpd_cgi_ajax12[] HTTPD_STRING_ATTR = "]');";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax11);
#if WEBSERVER_CONF_PRINTADDR
/* Note address table is filled from the end down */
{int i;
    for (i=0; i<UIP_DS6_ADDR_NB;i++) {
      if (uip_ds6_if.addr_list[i].isused) {
        numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf + numprinted);
        break;
      }
    }
}
#endif
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax12);
  }
#elif CONTIKI_TARGET_MINIMAL_NET
static uint16_t c0=0x3ff,c1=0x3ff,c2=0x3ff,c3=0x3ff,c4=0x3ff,c5=0x3ff,c6=0x3ff,c7=0x3ff;
    numprinted = snprintf(buf, sizeof(buf), "t(%d);b(%u);v(%u);",273+(rand()&0x3f),3300-iter/10,iter);
	numprinted += snprintf(buf+numprinted, sizeof(buf)-numprinted,"adc(%u,%u,%u,%u,%u,%u,%u,%u);",c0,c1,c2,c3,c4,c5,c6,c7);
	c0+=(rand()&0xf)-8;
	c1+=(rand()&0xf)-8;
	c2+=(rand()&0xf)-7;
	c3+=(rand()&0x1f)-15;
	c4+=(rand()&0x3)-1;
	c5+=(rand()&0xf)-8;
	c6+=(rand()&0xf)-8;
	c7+=(rand()&0xf)-8;
  if (iter==1) {
    static const char httpd_cgi_ajax11[] HTTPD_STRING_ATTR = "wt('Minimal-net ";
	static const char httpd_cgi_ajax12[] HTTPD_STRING_ATTR = "');";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax11);
#if WEBSERVER_CONF_PRINTADDR
/* Note address table is filled from the end down */
{int i;
    for (i=0; i<UIP_DS6_ADDR_NB;i++) {
      if (uip_ds6_if.addr_list[i].isused) {
	    numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf + numprinted);
	    break;
	  }
    }
}
#endif
	numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax12);
  }

#elif CONTIKI_TARGET_AVR_ATMEGA128RFA1
{ uint8_t i;int16_t tmp,bat;
  BATMON = 16; //give BATMON time to stabilize at highest range and lowest voltage
/* Measure internal temperature sensor, see atmega128rfa1 datasheet */
/* This code disabled by default for safety.
   Selecting an internal reference will short it to anything connected to the AREF pin
 */
#if 1
  ADCSRB|=1<<MUX5;          //this bit buffered till ADMUX written to!
  ADMUX =0xc9;              // Select internal 1.6 volt ref, temperature sensor ADC channel
  ADCSRA=0x85;              //Enable ADC, not free running, interrupt disabled, clock divider 32 (250 KHz@ 8 MHz)
//  while ((ADCSRB&(1<<AVDDOK))==0);  //wait for AVDD ok
//  while ((ADCSRB&(1<<REFOK))==0);  //wait for ref ok 
  ADCSRA|=1<<ADSC;          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  tmp=ADC;                  //Read adc
  tmp=11*tmp-2728+(tmp>>2); //Convert to celcius*10 (should be 11.3*h, approximate with 11.25*h)
  ADCSRA=0;                 //disable ADC
  ADMUX=0;                  //turn off internal vref      
#endif
/* Bandgap can't be measured against supply voltage in this chip. */
/* Use BATMON register instead */
  for ( i=16; i<31; i++) {
    BATMON = i;
    if ((BATMON&(1<<BATMON_OK))==0) break;
  }
  bat=2550-75*16-75+75*i;  //-75 to take the floor of the 75 mv transition window
  static const char httpd_cgi_ajax10[] HTTPD_STRING_ATTR ="t(%u),b(%u);adc(%d,%d,%u,%u,%u,%u,%u,%lu);";
  numprinted = httpd_snprintf(buf, sizeof(buf),httpd_cgi_ajax10,tmp,bat,iter,tmp,bat,sleepcount,OCR2A,0,clock_time(),clock_seconds());
  if (iter==1) {
    static const char httpd_cgi_ajax11[] HTTPD_STRING_ATTR = "wt('128rfa1 [";
	static const char httpd_cgi_ajax12[] HTTPD_STRING_ATTR = "]');";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax11);
#if WEBSERVER_CONF_PRINTADDR
/* Note address table is filled from the end down */
{int i;
    for (i=0; i<UIP_DS6_ADDR_NB;i++) {
      if (uip_ds6_if.addr_list[i].isused) {
	    numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf + numprinted);
	    break;
	  }
    }
}
#endif
	numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax12);
  }
}
#elif CONTIKI_TARGET_AVR_RAVEN
{ int16_t tmp,bat;
#if 1
/* Usual way to get AVR supply voltage, measure 1.1v bandgap using Vcc as reference.
 * This connects the bandgap to the AREF pin, so enable only if there is no external AREF!
 * A capacitor may be connected to this pin to reduce reference noise.
 */
  ADMUX =0x5E;              //Select AVCC as reference, measure 1.1 volt bandgap reference.
  ADCSRA=0x87;              //Enable ADC, not free running, interrupt disabled, clock divider  128 (62 KHz@ 8 MHz)
  ADCSRA|=1<<ADSC;          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
//bat=1126400UL/ADC;        //Get supply voltage (factor nominally 1100*1024)
  bat=1198070UL/ADC;        //My Raven
  ADCSRA=0;                 //disable ADC
  ADMUX=0;                  //turn off internal vref
#else
  bat=3300;  
#endif
   
  tmp=420;
  
  static const char httpd_cgi_ajax10[] HTTPD_STRING_ATTR ="t(%u),b(%u);adc(%d,%d,%u,%u,%u,%u,%u,%lu);";
  numprinted = httpd_snprintf(buf, sizeof(buf),httpd_cgi_ajax10,tmp,bat,iter,tmp,bat,sleepcount,OCR2A,0,clock_time(),clock_seconds());
  if (iter<3) {
    static const char httpd_cgi_ajax11[] HTTPD_STRING_ATTR = "wt('Raven [";
	static const char httpd_cgi_ajax12[] HTTPD_STRING_ATTR = "]');";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax11);
#if WEBSERVER_CONF_PRINTADDR
/* Note address table is filled from the end down */
{int i;
    for (i=0; i<UIP_DS6_ADDR_NB;i++) {
      if (uip_ds6_if.addr_list[i].isused) {
	    numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf + numprinted);
	    break;
	  }
    }
}
#endif
	numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax12);
  }
}

//#elif CONTIKI_TARGET_IS_SOMETHING_ELSE
#else
{
  static const char httpd_cgi_ajax10[] HTTPD_STRING_ATTR ="v(%u);";
  numprinted = httpd_snprintf(buf, sizeof(buf),httpd_cgi_ajax10,iter);
  if (iter==1) {
    static const char httpd_cgi_ajax11[] HTTPD_STRING_ATTR = "wt('Contiki Ajax ";
	static const char httpd_cgi_ajax12[] HTTPD_STRING_ATTR = "');";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax11);
#if WEBSERVER_CONF_PRINTADDR
/* Note address table is filled from the end down */
{int i;
    for (i=0; i<UIP_DS6_ADDR_NB;i++) {
      if (uip_ds6_if.addr_list[i].isused) {
	    numprinted += httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf + numprinted);
	    break;
	  }
    }
}
#endif
	numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajax12);
  }
}
#endif
 
#if CONTIKIMAC_CONF_COMPOWER
#include "sys/compower.h"
{
//sl=compower_idle_activity.transmit/RTIMER_ARCH_SECOND;
//sl=compower_idle_activity.listen/RTIMER_ARCH_SECOND;
}
#endif

#if RIMESTATS_CONF_ON

#include "net/rime/rimestats.h"
    static const char httpd_cgi_ajaxr1[] HTTPD_STRING_ATTR ="rime(%lu,%lu,%lu,%lu);";
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajaxr1,
		rimestats.tx,rimestats.rx,rimestats.lltx-rimestats.tx,rimestats.llrx-rimestats.rx);
#endif

#if ENERGEST_CONF_ON
{
#if 1
/* Send on times in percent since last update. Handle 16 bit rtimer wraparound.
 * Javascript must convert based on platform cpu, tx, rx power.
 * e.g. for cpu drawing 1.2ma@3v3, cp = 1.2*3.3*(% on time)/100 = 0.0396*c;
 * Low power mode l is essentially zero for cpus that sleep.
 * function p(c,l,t,r){cp=0.013*c;lt=0.465*t;lr=0.400*r;n=cp+lt+lr;//128rfa1@3v3,3dBm
 * function p(c,l,t,r){cp=0.013*c;lt=0.317*t;lr=0.400*r;n=cp+lt+lr;//128rfa1@3v3,0dBm
 * function p(c,l,t,r){cp=0.109*c;lt=0.848*t;lr=0.617*r;n=cp+lt+lr;//mc1322x@3v3,0dBm
 * Precision can be increased by multiplying by another 100 before sending.
 */
	static rtimer_clock_t last_send;
	rtimer_clock_t delta_time;
    static unsigned long last_cpu, last_lpm, last_listen, last_transmit;
    energest_flush();
	delta_time=RTIMER_NOW()-last_send;
	if (RTIMER_CLOCK_LT(RTIMER_NOW(),last_send)) delta_time+=RTIMER_ARCH_SECOND;
	last_send=RTIMER_NOW();
    static const char httpd_cgi_ajaxe1[] HTTPD_STRING_ATTR = "p(%lu,%lu,%lu,%lu);";	
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajaxe1,
        (10000UL*(energest_total_time[ENERGEST_TYPE_CPU].current - last_cpu))/delta_time,
        (10000UL*(energest_total_time[ENERGEST_TYPE_LPM].current - last_lpm))/delta_time,
        (10000UL*(energest_total_time[ENERGEST_TYPE_TRANSMIT].current - last_transmit))/delta_time,
        (10000UL*(energest_total_time[ENERGEST_TYPE_LISTEN].current - last_listen))/delta_time);
    last_cpu = energest_total_time[ENERGEST_TYPE_CPU].current;
    last_lpm = energest_total_time[ENERGEST_TYPE_LPM].current;
    last_transmit = energest_total_time[ENERGEST_TYPE_TRANSMIT].current;
    last_listen = energest_total_time[ENERGEST_TYPE_LISTEN].current;
#endif
#if 1
/* Send cumulative on times in percent*100 */
	uint16_t cpp,txp,rxp;
	uint32_t sl,clockseconds=clock_seconds();
//	energest_flush();
//	sl=((10000UL*energest_total_time[ENERGEST_TYPE_CPU].current)/RTIMER_ARCH_SECOND)/clockseconds;
    sl=energest_total_time[ENERGEST_TYPE_CPU].current/RTIMER_ARCH_SECOND;
    cpp=(10000UL*sl)/clockseconds;
//    txp=((10000UL*energest_total_time[ENERGEST_TYPE_TRANSMIT].current)/RTIMER_ARCH_SECOND)/clockseconds;
    sl=energest_total_time[ENERGEST_TYPE_TRANSMIT].current/RTIMER_ARCH_SECOND;
    txp=(10000UL*sl)/clockseconds;

 //   rxp=((10000UL*energest_total_time[ENERGEST_TYPE_LISTEN].current)/RTIMER_ARCH_SECOND)/clockseconds;
    sl=energest_total_time[ENERGEST_TYPE_LISTEN].current/RTIMER_ARCH_SECOND;
    rxp=(10000UL*sl)/clockseconds;

    static const char httpd_cgi_ajaxe2[] HTTPD_STRING_ATTR = "ener(%u,%u,%u);";	
    numprinted += httpd_snprintf(buf+numprinted, sizeof(buf)-numprinted,httpd_cgi_ajaxe2,cpp,txp,rxp);
#endif
}
#endif /* ENERGEST_CONF_ON */
 
    PSOCK_SEND_STR(&s->sout, buf);
    /* Can do fixed intervals or fixed starting points */
#if FIXED_INTERVALS
    timer_restart(&t);
    PSOCK_WAIT_UNTIL(&s->sout, timer_expired(&t));
#else
    PSOCK_WAIT_UNTIL(&s->sout, timer_expired(&t));
    timer_reset(&t);
#endif
}
  PSOCK_END(&s->sout);
}
#endif /* WEBSERVER_CONF_AJAX */
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
#if  WEBSERVER_CONF_HEADER
HTTPD_CGI_CALL(    hdr,    hdr_name, header         );
#endif
#if  WEBSERVER_CONF_FILESTATS
HTTPD_CGI_CALL(   file,   file_name, file_stats     );
#endif
#if  WEBSERVER_CONF_TCPSTATS
HTTPD_CGI_CALL(    tcp,    tcp_name, tcp_stats      );
#endif
#if  WEBSERVER_CONF_PROCESSES
HTTPD_CGI_CALL(   proc,   proc_name, processes      );
#endif
#if  WEBSERVER_CONF_ADDRESSES
HTTPD_CGI_CALL(   adrs,   adrs_name, addresses      );
#endif
#if  WEBSERVER_CONF_NEIGHBORS
HTTPD_CGI_CALL(   nbrs,   nbrs_name, neighbors      );
#endif
#if  WEBSERVER_CONF_ROUTES
HTTPD_CGI_CALL(   rtes,   rtes_name, routes         );
#endif
#if  WEBSERVER_CONF_SENSORS
HTTPD_CGI_CALL(sensors, sensor_name, sensor_readings);
#endif
#if WEBSERVER_CONF_TICTACTOE
HTTPD_CGI_CALL( tictac, tictac_name, tictactoe      );
#endif
#if WEBSERVER_CONF_AJAX
HTTPD_CGI_CALL( ajax, ajax_name, ajax_call          );
#endif

void
httpd_cgi_init(void)
{
#if  WEBSERVER_CONF_HEADER
  httpd_cgi_add(    &hdr);
#endif
#if  WEBSERVER_CONF_FILESTATS
  httpd_cgi_add(   &file);
#endif
#if  WEBSERVER_CONF_TCPSTATS
  httpd_cgi_add(    &tcp);
#endif
#if  WEBSERVER_CONF_PROCESSES
  httpd_cgi_add(   &proc);
#endif
#if  WEBSERVER_CONF_ADDRESSES
  httpd_cgi_add(   &adrs);
#endif
#if  WEBSERVER_CONF_NEIGHBORS
  httpd_cgi_add(   &nbrs);
#endif
#if  WEBSERVER_CONF_ROUTES
  httpd_cgi_add(   &rtes);
#endif
#if  WEBSERVER_CONF_SENSORS
  httpd_cgi_add(&sensors);
#endif
#if  WEBSERVER_CONF_TICTACTOE
  httpd_cgi_add( &tictac);
#endif
#if  WEBSERVER_CONF_AJAX
  httpd_cgi_add( &ajax);
#endif
}
#endif /* WEBSERVER_CONF_CGI */

#if WEBSERVER_CONF_PRINTADDR
/*---------------------------------------------------------------------------*/
uint8_t httpd_cgi_sprint_ip6(uip_ip6addr_t addr, char * result)
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
                        //all zeroes ?
                       if (zerocnt > 7) {*result++ = ':';*result++ = ':';}
                        }
                //Normal address, just print it
                else
                        {
                        result += sprintf(result, "%x", (unsigned int)(uip_ntohs(addr.u16[i])));
                        i++;
                        numprinted++;
                        }

                //Don't print : on last one
                if (numprinted != 8)
                        *result++ = ':';
                }

			return (result - starting);
        }
#endif /* WEBSERVER_CONF_PRINTADDR */