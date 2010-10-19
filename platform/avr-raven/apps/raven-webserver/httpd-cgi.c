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
 * $Id: httpd-cgi.c,v 1.8 2010/10/19 18:29:05 adamdunkels Exp $
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
#include "lib/petsciiconv.h"

#include "sensors.h"

#define DEBUGLOGIC 0        //See httpd.c, if 1 must also set it there!
#if DEBUGLOGIC
#define uip_mss(...) 512
#define uip_appdata TCPBUF
extern char TCPBUF[512];
#endif

/* RADIOSTATS must also be set in clock.c and the radio driver */
#if RF230BB
#define RADIOSTATS 1
#endif

static struct httpd_cgi_call *calls = NULL;

/*cgi function names*/
#if HTTPD_FS_STATISTICS
static const char   file_name[] HTTPD_STRING_ATTR = "file-stats";
#endif
static const char    tcp_name[] HTTPD_STRING_ATTR = "tcp-connections";
static const char   proc_name[] HTTPD_STRING_ATTR = "processes";
static const char sensor_name[] HTTPD_STRING_ATTR = "sensors";

/*Process states for processes cgi*/
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

  static char sensor_temperature[12]="Not Enabled";
  static char sensor_extvoltage[12]="Not Enabled";
  static unsigned long last_tempupdate,last_extvoltageupdate;
  extern unsigned long seconds;
#if RADIOSTATS
  extern unsigned long radioontime;
  extern uint8_t RF230_radio_on, RF230_rsigsi;
  extern uint16_t RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail;
#endif
  

void
web_set_temp(char *s)
{
  strcpy(sensor_temperature, s);
  last_tempupdate=seconds;
}
void
web_set_voltage(char *s)
{
  strcpy(sensor_extvoltage, s);
  last_extvoltageupdate=seconds;
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
    if(httpd_strncmp(name, f->name, httpd_strlen(f->name)) == 0) {
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
  static const char httpd_cgi_filestat1[] HTTPD_STRING_ATTR = "<p class=right><br><br><i>This page has been sent %u times</i></div></body></html>";
  static const char httpd_cgi_filestat2[] HTTPD_STRING_ATTR = "<tr><td><a href=\"%s\">%s</a></td><td>%d</td>";
  static const char httpd_cgi_filestat3[] HTTPD_STRING_ATTR = "%5u";
  char tmp[20];
  struct httpd_fsdata_file_noconst *f,fram;
  u16_t i;
  unsigned short numprinted;

  /* Transfer arg from whichever flash that contains the html file to RAM */
  httpd_fs_cpy(&tmp, (char *)arg, 20);

  /* Count for this page, with common page footer */
  if (tmp[0]=='.') {
    numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_filestat1, httpd_fs_open(thisfilename, 0));

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
#if HTTPD_FS_STATISTICS==1
      numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_filestat2, tmp, tmp, f->count);
#elif HTTPD_FS_STATISTICS==2
      numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_filestat2, tmp, tmp, httpd_filecount[i]);
#endif
      i++;
    }

  /* Count for specified file */
  } else {
    numprinted=httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_filestat3, httpd_fs_open(tmp, 0));
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
  static const char httpd_cgi_tcpstat1[] HTTPD_STRING_ATTR = "<tr align=\"center\"><td>%d</td><td>";
  static const char httpd_cgi_tcpstat2[] HTTPD_STRING_ATTR = "-%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n";
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  char tstate[20];
  uint16_t numprinted;

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
  static const char httpd_cgi_proc[] HTTPD_STRING_ATTR = "<tr align=\"center\"><td>%p</td><td>%s</td><td>%p</td><td>%s</td></tr>\r\n";
  char name[40],tstate[20];

  strncpy(name, ((struct process *)p)->name, 40);
  petsciiconv_toascii(name, 40);
  httpd_strcpy(tstate,states[9 + ((struct process *)p)->state]);
  return httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_proc, p, name,
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
  uint16_t numprinted;
  uint16_t h,m,s;
  static const char httpd_cgi_sensor0[] HTTPD_STRING_ATTR = "[Updated %d seconds ago]<br><br>";
  static const char httpd_cgi_sensor1[] HTTPD_STRING_ATTR = "<em>Temperature:</em> %s<br>";
  static const char httpd_cgi_sensor2[] HTTPD_STRING_ATTR = "<em>Voltage:</em> %s<br>";
  static const char httpd_cgi_sensor3[] HTTPD_STRING_ATTR = "<em>Up time:</em> %02d:%02d:%02d<br>";

  numprinted=0;
  if (last_tempupdate) {
    numprinted =httpd_snprintf((char *)uip_appdata, uip_mss(), httpd_cgi_sensor0,seconds-last_tempupdate);
  }

  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor1, sensor_temperature);

#if 0
//Measuring AVcc might be useful to check on battery condition but on ext power it's always 3v3
  ADMUX =0x1E;              //Select AREF as reference, measure 1.1 volt bandgap reference.
//ADMUX =0x5E;              //Select AVCC as reference, measure 1.1 volt bandgap reference.
  ADCSRA=0x07;              //Enable ADC, not free running, interrupt disabled, clock divider  128 (62 KHz@ 8 MHz)
  ADCSRA|=1<<ADSC;          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  h=1131632UL/ADC;          //Get supply voltage
#endif

  numprinted+=httpd_snprintf((char *)uip_appdata+numprinted, uip_mss()-numprinted, httpd_cgi_sensor2, sensor_extvoltage);

  h=seconds/3600;
  s=seconds-h*3600;
  m=s/60;
  s=s-m*60;
  numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor3, h,m,s);
  return numprinted;
}
#if RADIOSTATS
/*---------------------------------------------------------------------------*/
static unsigned short
generate_radio_stats(void *arg)
{
  uint16_t numprinted;
  uint16_t h,m,s;
  uint8_t p1,p2;
  static const char httpd_cgi_sensor4[] HTTPD_STRING_ATTR = "<em>Radio on:</em> %02d:%02d:%02d (%d.%02d%%)<br>";
  static const char httpd_cgi_sensor5[] HTTPD_STRING_ATTR = "<em>Packets:</em> Tx=%5d Rx=%5d TxL=%5d RxL=%5d RSSI=%2d\n";

  s=(10000UL*radioontime)/seconds;
  p1=s/100;
  p2=s-p1*100;
  h=radioontime/3600;
  s=radioontime-h*3600;
  m=s/60;
  s=s-m*60;

  numprinted =httpd_snprintf((char *)uip_appdata             , uip_mss()             , httpd_cgi_sensor4,\
    h,m,s,p1,p2);  
  numprinted+=httpd_snprintf((char *)uip_appdata + numprinted, uip_mss() - numprinted, httpd_cgi_sensor5,\
    RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail,RF230_rsigsi);  
 
  return numprinted;
}
#endif
/*---------------------------------------------------------------------------*/
static
PT_THREAD(sensor_readings(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_sensor_readings, s);
#if RADIOSTATS
  PSOCK_GENERATOR_SEND(&s->sout, generate_radio_stats, s);
#endif

  
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
HTTPD_CGI_CALL(   file,   file_name,      file_stats);
#endif
HTTPD_CGI_CALL(    tcp,    tcp_name, tcp_stats      );
HTTPD_CGI_CALL(   proc,   proc_name, processes      );
HTTPD_CGI_CALL(sensors, sensor_name, sensor_readings);

void
httpd_cgi_init(void)
{
#if HTTPD_FS_STATISTICS
  httpd_cgi_add(   &file);
#endif
  httpd_cgi_add(    &tcp);
  httpd_cgi_add(   &proc);
  httpd_cgi_add(&sensors);
}
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

