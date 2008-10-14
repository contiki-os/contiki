/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

#define UDP_NB 5
#define UDP_DATA_LEN 32

static struct etimer udp_periodic_timer;

static struct uip_udp_conn *udpconn;
static u8_t udpdata[UDP_DATA_LEN];
static u8_t count = 0;

PROCESS(udp_process_sender, "UPD test sender");
AUTOSTART_PROCESSES(&udp_process_sender);

/*---------------------------------------------------------------------------*/
static u8_t
udphandler(process_event_t ev, process_data_t data)
{
  if(count < UDP_NB){
    PRINTF("Sending UDP packet\n");
    uip_udp_packet_send(udpconn, udpdata, UDP_DATA_LEN);
    etimer_restart(&udp_periodic_timer);
    count++;
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process_sender, ev, data)
{
  uip_ipaddr_t ipaddr;
  u8_t cont = 1;
  
  PROCESS_BEGIN();
  PRINTF("In Process UDP test\n");

  memset(udpdata, 1, UDP_DATA_LEN);
  uip_ip6addr(&ipaddr,0xfe80,0,0,0,0x6466,0x6666,0x6666,0x6666);
  /* new connection with remote host */
  udpconn = udp_new(&ipaddr, HTONS(0xF0B0), NULL);
  udp_bind(udpconn, HTONS(0xF0B0+1));
  
  PRINTF("Created connection with remote peer ");
  PRINT6ADDR(&udpconn->ripaddr);
  PRINTF("local/remote port %u/%u\n",
         HTONS(udpconn->lport),HTONS(udpconn->rport));
  
  etimer_set(&udp_periodic_timer,15*CLOCK_SECOND);

  while(cont) {
    PROCESS_YIELD();
    cont = udphandler(ev, data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
