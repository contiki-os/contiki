/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * This file is part of the Contiki OS
 *
 * $Id: contiki-main.c,v 1.25 2010/10/19 18:29:05 adamdunkels Exp $
 *
 */

#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>
#include <memory.h>

#include "contiki.h"
#include "contiki-net.h"

#include "dev/serial-line.h"

#include "net/uip.h"
#ifdef __CYGWIN__
#include "net/wpcap-drv.h"
#else /* __CYGWIN__ */
#include "net/tapdev-drv.h"
#endif /* __CYGWIN__ */

#ifdef __CYGWIN__
PROCINIT(&etimer_process, &tcpip_process, &wpcap_process, &serial_line_process);
#else /* __CYGWIN__ */
PROCINIT(&etimer_process, &tapdev_process, &tcpip_process, &serial_line_process);
#endif /* __CYGWIN__ */

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
void
sprint_ip6(uip_ip6addr_t addr)
{
  unsigned char i = 0;
  unsigned char zerocnt = 0;
  unsigned char numprinted = 0;
  char thestring[40];
  char * result = thestring;

  *result++='[';
  while (numprinted < 8) {
    if ((addr.u16[i] == 0) && (zerocnt == 0)) {
      while(addr.u16[zerocnt + i] == 0) zerocnt++;
      if (zerocnt == 1) {
        *result++ = '0';
         numprinted++;
         break;
      }
      i += zerocnt;
      numprinted += zerocnt;
    } else {
      result += sprintf(result, "%x", (unsigned int)(uip_ntohs(addr.u16[i])));
      i++;
      numprinted++;
    }
    if (numprinted != 8) *result++ = ':';
  }
  *result++=']';
  *result=0;
  printf("%s",thestring);
}
#endif /* UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int
main(void)
{

  process_init();

  procinit_init();

  ctimer_init();

  autostart_start(autostart_processes);

  /* Set default IP addresses if not specified */
#if !UIP_CONF_IPV6
  uip_ipaddr_t addr;
  
  uip_gethostaddr(&addr);
  if (addr.u8[0]==0) {
    uip_ipaddr(&addr, 10,1,1,1);
  }
  printf("IP Address:  %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  uip_sethostaddr(&addr);

  uip_getnetmask(&addr);
  if (addr.u8[0]==0) {
    uip_ipaddr(&addr, 255,0,0,0);
    uip_setnetmask(&addr);
  }
  printf("Subnet Mask: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));

  uip_getdraddr(&addr);
  if (addr.u8[0]==0) {
    uip_ipaddr(&addr, 10,1,1,100);
    uip_setdraddr(&addr);
  }
  printf("Def. Router: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));

#else /* !UIP_CONF_IPV6 */
  uint8_t i;
  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);   
#if UIP_CONF_ROUTER
  uip_ds6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0, 0, 0, 0);
#else /* UIP_CONF_ROUTER */
  uip_ds6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0);
#endif /* UIP_CONF_ROUTER */
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
 // printf("IP6 Address: ");sprint_ip6(ipaddr);printf("\n");
  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
	if (uip_ds6_if.addr_list[i].isused) {	  
	  printf("IPV6 Address: ");sprint_ip6(uip_ds6_if.addr_list[i].ipaddr);printf("\n");
	}
  }
#endif /* !UIP_CONF_IPV6 */

  /* Make standard output unbuffered. */
  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  while(1) {
    fd_set fds;
    int n;
    struct timeval tv;
    
    n = process_run();
    /*    if(n > 0) {
      printf("%d processes in queue\n");
      }*/

    tv.tv_sec = 0;
    tv.tv_usec = 1;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(1, &fds, NULL, NULL, &tv);

    if(FD_ISSET(STDIN_FILENO, &fds)) {
      char c;
      if(read(STDIN_FILENO, &c, 1) > 0) {
	serial_line_input_byte(c);
      }
    }
    etimer_request_poll();
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}
/*---------------------------------------------------------------------------*/
unsigned short
sensors_light1(void)
{
  static unsigned short count;
  return count++;
}
/*---------------------------------------------------------------------------*/
