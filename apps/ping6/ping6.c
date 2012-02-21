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

#include "mac.h"

#include <string.h>
#include <stdio.h>

#define MACDEBUG 0

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static struct etimer ping6_periodic_timer;
static uint8_t count = 0;
static char command[20];
static uint16_t addr[8];
uip_ipaddr_t dest_addr;

PROCESS(ping6_process, "PING6 process");
AUTOSTART_PROCESSES(&ping6_process);


/*---------------------------------------------------------------------------*/
static uint8_t
ping6handler(process_event_t ev, process_data_t data)
{
  if(count == 0){
#if MACDEBUG
    // Setup destination address.
    addr[0] = 0xFE80;
    addr[4] = 0x6466;
    addr[5] = 0x6666;
    addr[6] = 0x6666;
    addr[7] = 0x6666;
    uip_ip6addr(&dest_addr, addr[0], addr[1],addr[2],
                addr[3],addr[4],addr[5],addr[6],addr[7]);

    // Set the command to fool the 'if' below.
    memcpy(command, (void *)"ping6", 5);

#else
/* prompt */
    printf("> ");
    /** \note the scanf here is blocking (the all stack is blocked waiting
     *  for user input). This is far from ideal and could be improved
     */
    scanf("%s", command);

    if(strcmp(command,"ping6") != 0){
      PRINTF("> invalid command\n");
      return 0;
    }

    if(scanf(" %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
             &addr[0],&addr[1],&addr[2],&addr[3],
             &addr[4],&addr[5],&addr[6],&addr[7]) == 8){

      uip_ip6addr(&dest_addr, addr[0], addr[1],addr[2],
                  addr[3],addr[4],addr[5],addr[6],addr[7]);
    } else {
      PRINTF("> invalid ipv6 address format\n");
      return 0;
    }
#endif

  }

  if((strcmp(command,"ping6") == 0) && (count < PING6_NB)){

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
           count, PING6_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
    UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


    PRINTF("Sending Echo Request to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");
    UIP_STAT(++uip_stat.icmp.sent);

    tcpip_ipv6_output();

    count++;
    etimer_set(&ping6_periodic_timer, 3 * CLOCK_SECOND);
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping6_process, ev, data)
{

  uint8_t cont = 1;

  PROCESS_BEGIN();
  PRINTF("In Process PING6\n");
  PRINTF("Wait for DAD\n");

  etimer_set(&ping6_periodic_timer, 15*CLOCK_SECOND);

  while(cont) {
    PROCESS_YIELD();
    cont = ping6handler(ev, data);
  }

  PRINTF("END PING6\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
