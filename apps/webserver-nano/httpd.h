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

#ifndef __HTTPD_H__
#define __HTTPD_H__
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
 */
#if CONTIKI_TARGET_SKY || CONTIKI_TARGET_STK500
#define WEBSERVER_CONF_CONNS     2
#define WEBSERVER_CONF_NAMESIZE 16
#define WEBSERVER_CONF_BUFSIZE  40
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
#define WEBSERVER_CONF_FILESTATS 1
#define WEBSERVER_CONF_TCPSTATS  0
#define WEBSERVER_CONF_PROCESSES 0
#define WEBSERVER_CONF_ADDRESSES 1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_ROUTES    1
#define WEBSERVER_CONF_SENSORS   0
#define WEBSERVER_CONF_TICTACTOE 0   //Needs passquery of at least 10 chars 
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

#else /* !sky */
#define WEBSERVER_CONF_CONNS     6
#define WEBSERVER_CONF_NAMESIZE 20
#define WEBSERVER_CONF_BUFSIZE  40
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
#define WEBSERVER_CONF_FILESTATS 1
#define WEBSERVER_CONF_TCPSTATS  1
#define WEBSERVER_CONF_PROCESSES 1
#define WEBSERVER_CONF_ADDRESSES 1
#define WEBSERVER_CONF_NEIGHBORS 1
#define WEBSERVER_CONF_ROUTES    1
#define WEBSERVER_CONF_SENSORS   1
#define WEBSERVER_CONF_TICTACTOE 1   //Needs passquery of at least 10 chars 
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
#endif

/* Address printing used by cgi's and logging, but it can be turned off if desired */
#if WEBSERVER_CONF_LOG || WEBSERVER_CONF_ADDRESSES || WEBSERVER_CONF_NEIGHBORS || WEBSERVER_CONF_ROUTES
#define WEBSERVER_CONF_PRINTADDR 1
uint8_t httpd_cgi_sprint_ip6(uip_ip6addr_t addr, char * result);
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


#endif /* __HTTPD_H__ */
