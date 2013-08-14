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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>
#include <memory.h>

#include "contiki.h"
#include "contiki-net.h"
#include "lib/assert.h"

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

#if RPL_BORDER_ROUTER
#include "net/rpl/rpl.h"

static uint16_t dag_id[] = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

PROCESS(border_router_process, "RPL Border Router");
PROCESS_THREAD(border_router_process, ev, data)
{

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  {
    rpl_dag_t *dag;
    char buf[sizeof(dag_id)];
    memcpy(buf,dag_id,sizeof(dag_id));
    dag = rpl_set_root((uip_ip6addr_t *)buf);
    
    /* Assign separate addresses to the uip stack and the host network
        interface, but with the same prefix E.g. bbbb::ff:fe00:200 to
        the stack and bbbb::1 to the host *fallback* network interface
        Otherwise the host will trap packets intended for the stack,
        just as the stack will trap packets intended for the host
        $ifconfig usb0 -arp on Ubuntu to skip the neighbor
        solicitations. Add explicit neighbors on other OSs */

    if(dag != NULL) {
      printf("Created a new RPL dag\n");
      
#if UIP_CONF_ROUTER_RECEIVE_RA
      /* Contiki stack will shut down until assigned an address from the
	 interface RA Currently this requires changes in the core
	 rpl-icmp6.c to pass the link-local RA broadcast.
      */
      
#else
      {
	static void sprint_ip6(uip_ip6addr_t addr);
	int i;
	uip_ip6addr_t ipaddr;
#ifdef HARD_CODED_ADDRESS
	uiplib_ipaddrconv(HARD_CODED_ADDRESS, &ipaddr);
#else
	uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
#endif
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
	rpl_set_prefix(dag, &ipaddr, 64);

	for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
	  if(uip_ds6_if.addr_list[i].isused) {
	    printf("IPV6 Address: ");
	    sprint_ip6(uip_ds6_if.addr_list[i].ipaddr);
	    printf("\n");
	  }
	}
      }
#endif
    }
  }
  /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  /* NETSTACK_MAC.off(1); */

  while(1) {
    PROCESS_YIELD();
    /* Local and global dag repair can be done from ? */
    /*  rpl_set_prefix(rpl_get_dag(RPL_ANY_INSTANCE), &ipaddr, 64);
	rpl_repair_dag(rpl_get_dag(RPL_ANY_INSTANCE)); */
  }

  PROCESS_END();
}
#endif /* RPL_BORDER_ROUTER */

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
static void
sprint_ip6(uip_ip6addr_t addr)
{
  unsigned char i = 0;
  unsigned char zerocnt = 0;
  unsigned char numprinted = 0;
  unsigned char notskipped = 0;
  char thestring[40];
  char *result = thestring;

  *result++ = '[';
  while(numprinted < 8) {
    if((addr.u16[i] == 0) && (zerocnt == 0)) {
      while(addr.u16[zerocnt + i] == 0) {
	zerocnt++;
      }
      if(zerocnt == 1 && notskipped) {
        *result++ = '0';
         numprinted++;
         notskipped = 1;
         continue;
      }
      i += zerocnt;
      numprinted += zerocnt;
    } else {
      result += sprintf(result, "%x", (unsigned int)(uip_ntohs(addr.u16[i])));
      i++;
      numprinted++;
    }
    if(numprinted != 8) {
      *result++ = ':';
    }
  }
  *result++=']';
  *result=0;
  printf("%s", thestring);
}
#endif /* UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/
int
main(void)
{
  clock_init();
#if UIP_CONF_IPV6
/* A hard coded address overrides the stack default MAC address to
   allow multiple instances. uip6.c defines it as
   {0x00,0x06,0x98,0x00,0x02,0x32} giving an ipv6 address of
   [fe80::206:98ff:fe00:232] We make it simpler, {0x02,0x00,0x00 + the
   last three bytes of the hard coded address (if any are nonzero).
   HARD_CODED_ADDRESS can be defined in the contiki-conf.h file, or
   here to allow quick builds using different addresses.  If
   HARD_CODED_ADDRESS has a prefix it also applied, unless built as a
   RPL end node.  E.g. bbbb::12:3456 becomes fe80::ff:fe12:3456 and
   prefix bbbb::/64 if non-RPL ::10 becomes fe80::ff:fe00:10 and
   prefix awaits RA or RPL formation bbbb:: gives an address of
   bbbb::206:98ff:fe00:232 if non-RPL */
#ifdef HARD_CODED_ADDRESS
  {
  uip_ipaddr_t ipaddr;
  uiplib_ipaddrconv(HARD_CODED_ADDRESS, &ipaddr);
  if((ipaddr.u8[13] != 0) ||
     (ipaddr.u8[14] != 0) ||
     (ipaddr.u8[15] != 0)) {
    if(sizeof(uip_lladdr) == 6) {  /* Minimal-net uses ethernet MAC */
      uip_lladdr.addr[0] = 0x02;
      uip_lladdr.addr[1] = 0;
      uip_lladdr.addr[2] = 0;
      uip_lladdr.addr[3] = ipaddr.u8[13];
      uip_lladdr.addr[4] = ipaddr.u8[14];
      uip_lladdr.addr[5] = ipaddr.u8[15];
    }
  }
 }
#endif /* HARD_CODED_ADDRESS */
#endif /* UIP_CONF_IPV6 */

  process_init();
/* procinit_init initializes RPL which sets a ctimer for the first DIS */
/* We must start etimers and ctimers,before calling it */
  process_start(&etimer_process, NULL);
  ctimer_init();

#if RPL_BORDER_ROUTER
  process_start(&border_router_process, NULL);
  printf("Border Router Process started\n");
#elif UIP_CONF_IPV6_RPL
  printf("RPL enabled\n");
#endif

  procinit_init();
  autostart_start(autostart_processes); 

  /* Set default IP addresses if not specified */
#if !UIP_CONF_IPV6
  {
    uip_ipaddr_t addr;

    uip_gethostaddr(&addr);
    if(addr.u8[0] == 0) {
      uip_ipaddr(&addr, 172,18,0,2);
    }
    printf("IP Address:  %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
    uip_sethostaddr(&addr);
    
    uip_getnetmask(&addr);
    if(addr.u8[0] == 0) {
      uip_ipaddr(&addr, 255,255,0,0);
      uip_setnetmask(&addr);
    }
    printf("Subnet Mask: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
    
    uip_getdraddr(&addr);
    if(addr.u8[0] == 0) {
      uip_ipaddr(&addr, 172,18,0,1);
      uip_setdraddr(&addr);
    }
    printf("Def. Router: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&addr));
  }
#else /* UIP_CONF_IPV6 */

#if !UIP_CONF_IPV6_RPL
  {
    uip_ipaddr_t ipaddr;
#ifdef HARD_CODED_ADDRESS
    uiplib_ipaddrconv(HARD_CODED_ADDRESS, &ipaddr);
#else
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
#endif
    if((ipaddr.u16[0] != 0) ||
       (ipaddr.u16[1] != 0) ||
       (ipaddr.u16[2] != 0) ||
       (ipaddr.u16[3] != 0)) {
#if UIP_CONF_ROUTER
      if(!uip_ds6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0, 0, 0, 0)) {
        fprintf(stderr,"uip_ds6_prefix_add() failed.\n");
        exit(EXIT_FAILURE);
      }
#else /* UIP_CONF_ROUTER */
      if(!uip_ds6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0)) {
        fprintf(stderr,"uip_ds6_prefix_add() failed.\n");
        exit(EXIT_FAILURE);
      }
#endif /* UIP_CONF_ROUTER */

      uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
      uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
    }
  }
#endif /* !UIP_CONF_IPV6_RPL */

#endif /* !UIP_CONF_IPV6 */

 // procinit_init();
 // autostart_start(autostart_processes); 

  /* Make standard output unbuffered. */
  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  printf("\n*******%s online*******\n",CONTIKI_VERSION_STRING);

#if UIP_CONF_IPV6 && !RPL_BORDER_ROUTER  /* Border router process prints addresses later */
  {
    int i = 0;
    int interface_count = 0;
    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
      if(uip_ds6_if.addr_list[i].isused) {
        printf("IPV6 Addresss: ");
        sprint_ip6(uip_ds6_if.addr_list[i].ipaddr);
        printf("\n");
        interface_count++;
      }
    }
    assert(0 < interface_count);
  }
#endif

  while(1) {
    fd_set fds;
    int n;
    struct timeval tv;
    clock_time_t next_event;
    
    n = process_run();
    next_event = etimer_next_expiration_time() - clock_time();

#if DEBUG_SLEEP
    if(n > 0)
      printf("sleep: %d events pending\n",n);
    else
      printf("sleep: next event @ T-%.03f\n",(double)next_event / (double)CLOCK_SECOND);
#endif

#ifdef __CYGWIN__
    /* wpcap doesn't appear to support select, so
     * we can't idle the process on windows. */
    next_event = 0;
#endif

    if(next_event > (CLOCK_SECOND * 2))
      next_event = CLOCK_SECOND * 2;
    tv.tv_sec = n ? 0 : (next_event / CLOCK_SECOND);
    tv.tv_usec = n ? 0 : ((next_event % 1000) * 1000);

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
#ifdef __CYGWIN__
    select(1, &fds, NULL, NULL, &tv);
#else
    FD_SET(tapdev_fd(), &fds);
    if(0 > select(tapdev_fd() + 1, &fds, NULL, NULL, &tv)) {
      perror("Call to select() failed.");
      exit(EXIT_FAILURE);
    }
#endif

    if(FD_ISSET(STDIN_FILENO, &fds)) {
      char c;
      if(read(STDIN_FILENO, &c, 1) > 0) {
	serial_line_input_byte(c);
      }
    }
#ifdef __CYGWIN__
    process_poll(&wpcap_process);
#else
    process_poll(&tapdev_process);
#endif
    etimer_request_poll();
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
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
void
_xassert(const char *file, int line)
{
  fprintf(stderr, "%s:%u: failed assertion\n", file, line);
  abort();
}


unsigned short
sensors_light1(void)
{
  static unsigned short count;
  return count++;
}
/*---------------------------------------------------------------------------*/
