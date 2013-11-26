/*
 * Copyright (c) 2001-2005, Adam Dunkels.
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

#ifndef HTTPD_H_
#define HTTPD_H_
/*
 * The default number of simultaneous server connections is 4. Multiple connections can be triggered
 * by requests for embedded images, style sheets, icons, etc. and a TCP RESET is issued when no more
 * connections are available.
 * The Firefox default connections per server is 15. It can be reduced to 1 in about:config and generally
 * will improve throughput in the half-duplex 6LoWPAN link as embedded files will be requested sequentially.
 * Tictactoe is a good test for multiple connections; it can use as many as 9, but should also work when
 * Firefox network.http.max-connections-per-server is set to a lower number.
 * The RAM needed for each entry depends on script enabling and buffer sizes; see struct httpd_state below.
 * Typical range is 100 - 200 bytes per connection
 * cgi's that use PSOCK_GENERATOR_SEND will have truncated output if UIP_CONF_RECEIVE_WINDOW and UIP_CONF_TCP_MSS
 * are not large enough. The header-menu cgi needs ~340 bytes if all options are enabled, while the file-stats * cgi
 * can exceed any MSS if there are enough files to display (e.g. tic-tac-toe).
 * The advertised MSS is easily seen in wireshark.
 * Some example set a small MSS by default. rpl-border-router for example uses a receive window of 60.
 */
 
 /* Titles of web pages served with the !header cgi can be configured to show characteristics of the node.
  * For example "CD1A:3456" to show the node id and clock time of last access.
  * Change this line and rebuild to make indentifiable instances.
  * Undefine to reduce program size, giving "Contiki-Nano" title on all pages.
  * WAD indexes into the uip destaddr field, which contains the address that we responded to.
  */
#define WEBSERVER_CONF_PAGETITLE sprintf(buf,"[%02x%02x]",WAD[14],WAD[15]);
//#define WEBSERVER_CONF_PAGETITLE sprintf(buf,"Nano[%02x%02x%02x]",WAD[13],WAD[14],WAD[15]);
//#define WEBSERVER_CONF_PAGETITLE sprintf(buf,"Nano[%02x%02x...%02x%02x]",WAD[0],WAD[1],WAD[14],WAD[15]);
//#define WEBSERVER_CONF_PAGETITLE sprintf(buf,"%2x%02x...%2x%02x [%lu]",WAD[0],WAD[1],WAD[14],WAD[15],clock_seconds());

#ifndef WEBSERVER_CONF_NANO
#if CONTIKI_TARGET_SKY || CONTIKI_TARGET_STK500
#define WEBSERVER_CONF_NANO 1
#elif CONTIKI_TARGET_REDBEE_ECONOTAG || CONTIKI_TARGET_AVR_RAVEN || CONTIKI_TARGET_AVR_ATMEGA128RFA1
#define WEBSERVER_CONF_NANO 2
#else
#define WEBSERVER_CONF_NANO 3
#endif
#endif

#if WEBSERVER_CONF_NANO==1
/* nano-size for constrained MCUs */
#define WEBSERVER_CONF_CONNS     2
#define WEBSERVER_CONF_NAMESIZE 16
#define WEBSERVER_CONF_BUFSIZE  40
/* Short tcp timeouts allow new connections sooner */
#define WEBSERVER_CONF_TIMEOUT  20
/* Allow include in .shtml pages, e.g. %!: /header.html */
#define WEBSERVER_CONF_INCLUDE   1
/* Allow cgi in .shtml pages, e.g. %! file-stats . */
#define WEBSERVER_CONF_CGI       1
/* MAX_SCRIPT_NAME_LENGTH should be at least the maximum file name length+2 for %!: includes */
#define MAX_SCRIPT_NAME_LENGTH   WEBSERVER_CONF_NAMESIZE+2
/* Enable specific cgi's */
#define WEBSERVER_CONF_HEADER    1
//#define WEBSERVER_CONF_HEADER_W3C  1 //Proper header
#define WEBSERVER_CONF_HEADER_MENU 1 //with links to other pages
//#define WEBSERVER_CONF_HEADER_ICON 1 //with favicon
#define WEBSERVER_CONF_LOADTIME  0  //show load time in filestats
#define WEBSERVER_CONF_FILESTATS 1
#define WEBSERVER_CONF_TCPSTATS  0
#define WEBSERVER_CONF_PROCESSES 0
#define WEBSERVER_CONF_ADDRESSES 1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_NEIGHBOR_STATUS 0
#define WEBSERVER_CONF_ROUTES    0
#define WEBSERVER_CONF_ROUTE_LINKS  0
#define WEBSERVER_CONF_SENSORS   0
#define WEBSERVER_CONF_STATISTICS   0
#define WEBSERVER_CONF_TICTACTOE 0   //Needs passquery of at least 10 chars 
#define WEBSERVER_CONF_AJAX      0
//#define WEBSERVER_CONF_PASSQUERY 10
#if WEBSERVER_CONF_PASSQUERY
extern char httpd_query[WEBSERVER_CONF_PASSQUERY];
#endif
/* Enable specific file types */
#define WEBSERVER_CONF_JPG       0
#define WEBSERVER_CONF_PNG       0
#define WEBSERVER_CONF_GIF       0
#define WEBSERVER_CONF_TXT       1
#define WEBSERVER_CONF_CSS       0
#define WEBSERVER_CONF_BIN       0

/* Log page accesses */
#define WEBSERVER_CONF_LOG       0
/* Include referrer in log */
#define WEBSERVER_CONF_REFERER   0
/*-----------------------------------------------------------------------------*/
#elif WEBSERVER_CONF_NANO==2
/* webserver-mini having more content */
#define WEBSERVER_CONF_CONNS     2
#define WEBSERVER_CONF_NAMESIZE 20
#define WEBSERVER_CONF_BUFSIZE  40
#define WEBSERVER_CONF_TIMEOUT  20
/* Allow include in .shtml pages, e.g. %!: /header.html */
#define WEBSERVER_CONF_INCLUDE   1
/* Allow cgi in .shtml pages, e.g. %! file-stats . */
#define WEBSERVER_CONF_CGI       1
/* MAX_SCRIPT_NAME_LENGTH should be at least the maximum file name length+2 for %!: includes */
#define MAX_SCRIPT_NAME_LENGTH   WEBSERVER_CONF_NAMESIZE+2
/* Enable specific cgi's */
#define WEBSERVER_CONF_HEADER    1
//#define WEBSERVER_CONF_HEADER_W3C  1 //Proper header
#define WEBSERVER_CONF_HEADER_MENU 1 //with links to other pages
//#define WEBSERVER_CONF_HEADER_ICON 1 //with favicon
#define WEBSERVER_CONF_LOADTIME  1
#define WEBSERVER_CONF_FILESTATS 1
#define WEBSERVER_CONF_TCPSTATS  1
#define WEBSERVER_CONF_PROCESSES 1
#define WEBSERVER_CONF_ADDRESSES 1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_NEIGHBOR_STATUS 1
#define WEBSERVER_CONF_ROUTES    1
#define WEBSERVER_CONF_ROUTE_LINKS  1
#define WEBSERVER_CONF_SENSORS   1
#define WEBSERVER_CONF_STATISTICS   1
//#define WEBSERVER_CONF_TICTACTOE 1   //Needs passquery of at least 10 chars 
#define WEBSERVER_CONF_AJAX      1
#define WEBSERVER_CONF_SHOW_ROOM 0
#define WEBSERVER_CONF_PASSQUERY 10
#if WEBSERVER_CONF_PASSQUERY
extern char httpd_query[WEBSERVER_CONF_PASSQUERY];
#endif
/* Enable specific file types */
#define WEBSERVER_CONF_JPG       0
#define WEBSERVER_CONF_PNG       0
#define WEBSERVER_CONF_GIF       0
#define WEBSERVER_CONF_TXT       1
#define WEBSERVER_CONF_CSS       0
#define WEBSERVER_CONF_BIN       0

/* Log page accesses */
#define WEBSERVER_CONF_LOG       0
/* Include referrer in log */
#define WEBSERVER_CONF_REFERER   1

/*-----------------------------------------------------------------------------*/
#elif WEBSERVER_CONF_NANO==3
/* webserver-mini having all content */
#define WEBSERVER_CONF_CONNS     6
#define WEBSERVER_CONF_NAMESIZE 20
#define WEBSERVER_CONF_BUFSIZE  40
#define WEBSERVER_CONF_TIMEOUT  20
/* Allow include in .shtml pages, e.g. %!: /header.html */
#define WEBSERVER_CONF_INCLUDE   1
/* Allow cgi in .shtml pages, e.g. %! file-stats . */
#define WEBSERVER_CONF_CGI       1
/* MAX_SCRIPT_NAME_LENGTH should be at least the maximum file name length+2 for %!: includes */
#define MAX_SCRIPT_NAME_LENGTH   WEBSERVER_CONF_NAMESIZE+2
/* Enable specific cgi's */
#define WEBSERVER_CONF_HEADER    1
//#define WEBSERVER_CONF_HEADER_W3C  1 //Proper header
#define WEBSERVER_CONF_HEADER_MENU 1 //with links to other pages
//#define WEBSERVER_CONF_HEADER_ICON 1 //with favicon
#define WEBSERVER_CONF_LOADTIME  1
#define WEBSERVER_CONF_FILESTATS 1
#define WEBSERVER_CONF_TCPSTATS  1
#define WEBSERVER_CONF_PROCESSES 1
#define WEBSERVER_CONF_ADDRESSES 1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_ROUTES    1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_NEIGHBOR_STATUS 1
#define WEBSERVER_CONF_ROUTES    1
#define WEBSERVER_CONF_ROUTE_LINKS  1
#define WEBSERVER_CONF_SENSORS   1
#define WEBSERVER_CONF_STATISTICS   1
#define WEBSERVER_CONF_TICTACTOE 1   //Needs passquery of at least 10 chars 
#define WEBSERVER_CONF_AJAX      1
#define WEBSERVER_CONF_PASSQUERY 10
#if WEBSERVER_CONF_PASSQUERY
extern char httpd_query[WEBSERVER_CONF_PASSQUERY];
#endif
/* Enable specific file types */
#define WEBSERVER_CONF_JPG       1
#define WEBSERVER_CONF_PNG       1
#define WEBSERVER_CONF_GIF       1
#define WEBSERVER_CONF_TXT       1
#define WEBSERVER_CONF_CSS       1
#define WEBSERVER_CONF_BIN       1

/* Log page accesses */
#define WEBSERVER_CONF_LOG       1
/* Include referrer in log */
#define WEBSERVER_CONF_REFERER   1

#else
#error Specified WEBSERVER_CONF_NANO configuration not supported.
#endif /* WEBSERVER_CONF_NANO */

/* Address printing used by cgi's and logging, but it can be turned off if desired */
#if WEBSERVER_CONF_LOG || WEBSERVER_CONF_ADDRESSES || WEBSERVER_CONF_NEIGHBORS || WEBSERVER_CONF_ROUTES
extern uip_ds6_netif_t uip_ds6_if;
#define WEBSERVER_CONF_PRINTADDR 1
#endif
#if WEBSERVER_CONF_PRINTADDR
uint8_t httpd_cgi_sprint_ip6(uip_ip6addr_t addr, char * result);
#else
#define httpd_cgi_sprint_ip6(...) 0
#endif

#include "contiki-net.h"
#include "httpd-fs.h"

#if defined(__AVR__)
/* When using non-ram storage httpd-fsdata.c must be generated with the HTTPD_STRING_ATTR, eg
 *        ../../tools/makefsdata -A HTTPD_STRING_ATTR
 */
#include <avr/pgmspace.h>
#define HTTPD_STRING_ATTR PROGMEM
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
/* These will fail if the server strings are above 64K in program flash */
#define httpd_memcpy         memcpy_P
#define httpd_strcpy         strcpy_P
#define httpd_strcmp         strcmp_P
#define httpd_strncmp        strncmp_P
#define httpd_strlen         strlen_P
#define httpd_snprintf       snprintf_P
#define httpd_fs_cpy         memcpy_P
#define httpd_fs_strcmp      strcmp_P
#define httpd_fs_strchr      strchr_P
#define httpd_fs_getchar(x)  pgm_read_byte(x)

#else /* All storage in RAM */
#define HTTPD_STRING_ATTR
#define PRINTA(FORMAT,args...) printf(FORMAT,##args)
#define httpd_snprintf       snprintf
#define httpd_fs_cpy         memcpy
#define httpd_memcpy         memcpy
#define httpd_strcpy         strcpy
#define httpd_strcmp         strcmp
#define httpd_strncmp        strncmp
#define httpd_strlen         strlen
#define httpd_fs_strchr      strchr
#define httpd_fs_strcmp      strcmp
#define httpd_fs_getchar(c)  *(c)
#endif

struct httpd_state {
  unsigned char timer;
  struct psock sin, sout;
  struct pt outputpt;
#if WEBSERVER_CONF_INCLUDE || WEBSERVER_CONF_CGI
  struct pt scriptpt;
#endif
  char inputbuf[WEBSERVER_CONF_BUFSIZE];
  char filename[WEBSERVER_CONF_NAMESIZE];
  char state;
  struct httpd_fs_file file;  
  int len;
#if WEBSERVER_CONF_INCLUDE || WEBSERVER_CONF_CGI
  char *scriptptr;
  int scriptlen;
#endif
#if WEBSERVER_CONF_LOADTIME
  clock_time_t pagetime;
#endif
#if WEBSERVER_CONF_AJAX
  uint16_t ajax_timeout;
#endif
#if WEBSERVER_CONF_NEIGHBORS || WEBSERVER_CONF_ROUTES
  uint8_t starti,savei,startj,savej;
#endif
#if WEBSERVER_CONF_CGI
  union {
    unsigned short count;
    void *ptr;
  } u;
#endif
};

void httpd_init(void);
void httpd_appcall(void *state);

/* DEBUGLOGIC is a convenient way to debug without a tcp/ip connection.
 * After initialization, break the program and set the program counter
 * to the beginning of httpd.c::httpd_appcall:
 *   s = sg = (struct httpd_state *)memb_alloc(&conns);
 * The global sg points to the s data to get around "not in scope" optimization.
 * The input file is forced to /index.html and the output directed to TCPBUF.
 * However the socket will hang on acknowledgement when any data is sent.
 * To prevent this just add a break to /core/net/psock.c::psock_generator_send
 * ...
 * // Wait until all data is sent and acknowledged.
 * break;                            //<---add this line
 * PT_YIELD_UNTIL(&s->psockpt, uip_acked() || uip_rexmit());
 * ...
 */
#define DEBUGLOGIC 0
#if DEBUGLOGIC
struct httpd_state *sg;
#define uip_mss(...) 512
#define uip_appdata TCPBUF
char TCPBUF[512];
#endif


#endif /* HTTPD_H_ */
