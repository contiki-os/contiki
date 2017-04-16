/* Copyright (c) 2015, Michele Amoretti.
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
*/


/**
 * \file
 *         Server node for demo-centralized
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */
 
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"

#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dinas-msg.h"
#include "proximity-cache.h" 

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"


#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688

#ifndef PERIOD
#define PERIOD 120
#endif
#define RANDWAIT (PERIOD)
#define MAX_PAYLOAD_LEN 120

static uip_ipaddr_t local_ipaddr;
static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *server_conn;
static unsigned long time_offset;
uip_ipaddr_t destination_ipaddr;
char destination[40];

/*---------------------------------------------------------------------------*/
PROCESS(dinas_sink_process, "SINK process");
AUTOSTART_PROCESSES(&dinas_sink_process);


/*---------------------------------------------------------------------------*/
static unsigned long
get_time(void)
{
  return clock_seconds() + time_offset;
}

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("print_local_addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    /*PRINTF("state = %d\n", state);*/
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {	
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      local_ipaddr = uip_ds6_if.addr_list[i].ipaddr;
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
clock_time_t time;
PROCESS_THREAD(dinas_sink_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  /*PRINTF("DINAS sink started\n");*/

#if UIP_CONF_ROUTER
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("Created a new RPL dag\n");
  } else {
    PRINTF("Failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */

  print_local_addresses();
  
  proximity_cache_init();

  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_RDC.off(1);

  /* This is a server .. */
  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));
  
  /* ... and a client */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  /*
  PRINTF("Created a server connection ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));
  */

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      if(uip_newdata()) {
               
        DINASMSG *msg = (DINASMSG *)uip_appdata;
        
        uip_ipaddr_t ipaddr;
        ipaddr = UIP_IP_BUF->srcipaddr;
	    
	    /* 
	      if msg is notification, add to cache
	      else if msg is request, send reply 
	    */
	    /*PRINTF("msg type = %d \n", msg->type);*/
  
  		//if (msg->type == 0) /* notification */
  		if (dinas_msg_get_type(msg->config) == 0) /* notification */
  		{
  		  /*		
  		  PRINTF("Received notification for:\n");
  	      bloom_print(&msg->bloom);
  	      PRINTF("\n");
  	      */
  		  CACHEITEM ci;
  		  ci.bloomname = msg->bloom;
  		  ci.owner_addr = msg->owner_addr;
          ci.provider_neighbor_addr = ipaddr;
          time = get_time();
          ci.timestamp = time;
          proximity_cache_add_item(ci);
          //proximity_cache_print();
        }  
        //else if (msg->type == 1) /* msg is a request */ 
        else if (dinas_msg_get_type(msg->config) == 1) /* request */
        {
  	      //PRINTF("Received request for:\n");
  	      //bloom_print(&msg->bloom);
          //proximity_cache_print();
  	      /* 
           * - if the requested bloom matches one of those that are in cache, send a reply msg to the request owner 
           */
          CACHEITEM ci;
          ci.bloomname = msg->bloom;
          int i = proximity_cache_check_item(&ci);
          if (i != CACHE_SIZE) 
          { 
          	//PRINTF("I've got it in my cache! Now sending reply to \n");
            DINASMSG reply;
      		reply.bloom = msg->bloom;
      		reply.owner_addr = proximity_cache_get_item(i)->owner_addr;
      		reply.config = dinas_msg_set_config(0,2,0);
      		reply.req_num = msg->req_num;
      		destination_ipaddr = msg->owner_addr;
      		//PRINT6ADDR(&destination_ipaddr);
      		//PRINTF("\n");
      		uip_udp_packet_sendto(client_conn, &reply, sizeof(DINASMSG), &destination_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
      		return 0; /* comment this, if you want that multiple replies can be received by the requester */ 
          }
        }
      }	
    }
  }

  PROCESS_END();
}


