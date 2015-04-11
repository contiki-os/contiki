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
 *         Client node for demo-centralized
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "dev/serial-line.h"
#include "lib/random.h"
#include "sys/node-id.h"  /* TRICK to have the node_id (good only for Cooja sim.) */
#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dinas-msg.h"
#include "project-conf.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688
#define SERVICE_ID 190

static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *client_conn;
static uip_ipaddr_t local_ipaddr;
static uip_ipaddr_t global_ipaddr;
static uip_ipaddr_t server_ipaddr;
BLOOM bloomname;



/*---------------------------------------------------------------------------*/
PROCESS(dinas_peer_process, "DINAS peer process");
PROCESS(send_process, "send process");
AUTOSTART_PROCESSES(&dinas_peer_process, &send_process);

/*---------------------------------------------------------------------------*/
extern uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];

/*---------------------------------------------------------------------------*/

static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("print_local_addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      local_ipaddr = uip_ds6_if.addr_list[i].ipaddr;
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
  
  global_ipaddr = local_ipaddr;
  global_ipaddr.u16[0] = 0xaaaa;
  
  /*
  PRINTF("Remote server address: ");
  PRINT6ADDR(&server_ipaddr);
  PRINTF("\n"); 
  */
}


/*---------------------------------------------------------------------------*/

static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  
  /* FIXME: the sink address may not be known a priori! */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7401, 0x0001, 0x0101);
  
}


/*---------------------------------------------------------------------------*/
uip_ipaddr_t destination_ipaddr;
char destination[40];
int num_received_replies = 0;
PROCESS_THREAD(dinas_peer_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  print_local_addresses();

  set_global_address();
  
  /* PRINTF("DINAS peer process started\n");*/

  /*print_local_addresses();*/

  /* this is a server... */ 	
  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));
  
  /*PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));
  */

  /* ... and a client */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  /*
  PRINTF("Created a client connection ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
        UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
  */      

  /* handle incoming events (see above in this file) */	
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      if(uip_newdata()) {
        
        /*
        PRINTF("-----------\n I am: ");
        PRINT6ADDR(&local_ipaddr);
        PRINTF("\n");
        */
        
        DINASMSG *msg = (DINASMSG *)uip_appdata; /* (struct MSG *)*/
        //if (msg->type == 2) /* msg is a reply from the server */
        if (dinas_msg_get_type(msg->config) == 2) /* msg is a reply */
        {
          num_received_replies += 1;
          PRINTF("rp %d\n", msg->req_num);
          /*PRINTF("Received reply from %s, now I have %d replies\n", short_ipaddr, num_received_replies);*/
          /*
          PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
          PRINTF("\n");
          */
          return -1;	
        }
      }
    }
  }

  PROCESS_END();
}

int count = NOTIFICATION_COUNTER;
int msg_count = 0;
int req_count = 0;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(send_process, ev, data)
{
  static struct etimer period_timer, wait_timer;
  char location[12];
  
  PROCESS_BEGIN();

  /* Create this node's name once for all */
  bloomname = bloom_create();
  bloom_add(&bloomname, "netservice-_dinas._udp.local");
  if (node_id%2 == 0)
    bloom_add(&bloomname, "application:TemperatureMonitoring");	
  else
  	bloom_add(&bloomname, "application:CO2Monitoring");
  if (node_id <= NUM_ROOMS/2)
    bloom_add(&bloomname, "space:Floor1");
  else
    bloom_add(&bloomname, "space:Floor2");
  sprintf(location, "space:Room-%d", node_id);
  /*PRINTF("location: %s\n", location);*/
  bloom_add(&bloomname, location);

  /* Send a packet every 120-122 seconds. */
  etimer_set(&period_timer, CLOCK_SECOND * PERIOD);
  //PRINTF("MAX_NUM_MSG = %d \n", MAX_NUM_MSG);
  while(msg_count < MAX_NUM_MSG) {
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_TIMER) 
    {
      if(data == &period_timer) 
      {
        etimer_reset(&period_timer); /* Restart the timer from the previous expiration time */ 
        clock_time_t ctt = random_rand() % (CLOCK_SECOND * RANDWAIT);
        /*PRINTF("ctt = %d\n", ctt);*/
        etimer_set(&wait_timer, ctt);
      } 
      else if (data == &wait_timer) 
      {
          /*  
           * 1) build the msg (pub or req): BF + owner addr + TTL
           * 2) pass the msg to dinas_routing_send()
           */	
              
  		  msg_count++;
          PRINTF("msg_count = %d\n", msg_count);
  		  
  		  DINASMSG msg; 
  		  
  		  if (count == NOTIFICATION_COUNTER) 
  		  {
  		  	msg.config = dinas_msg_set_config(0,0,TTL); /* notification */
  		    count = 1;
            msg.bloom = bloomname;
  		    //PRINTF("Notification \n"); 
  		  }
  		  else {
  		  	msg.config = dinas_msg_set_config(0,1,TTL); /* request */	
  		    req_count++;
  		    PRINTF("rq %d\n", req_count); 
  		  	
  		    
  		    
  		    int room_number = 0;
  		    do 
  		    {
  		      unsigned short r = random_rand();
              /*PRINTF("r = %u\n", r);*/
              room_number = (r)%(NUM_ROOMS) + 2; /* random number between 2 and NUM_ROOMS+1 */
              /*
              PRINTF("room_number = %d\n", room_number);
              PRINTF("node_id = %d\n", node_id);
              */
  		    } while (room_number == node_id); /* we want to search for another room's sensor */
  		    
  		    sprintf(location, "location-%d", room_number);
  		    //PRINTF("room_number: %d\n", room_number);
  		    
            BLOOM bloom;
            bloom = bloom_create();
            bloom_add(&bloom, "netservice-_dinas._udp.local");
            if (room_number%2 == 0)
    	      bloom_add(&bloom, "application:TemperatureMonitoring");	
  			else
  			  bloom_add(&bloom, "application:CO2Monitoring");
  			if (room_number <= NUM_ROOMS/2)
    		  bloom_add(&bloom, "space:Floor1");
  			else
    		bloom_add(&bloom, "space:Floor2");
    		sprintf(location, "space:Room-%d", room_number);
            bloom_add(&bloom, location);
            //bloom_print(&bloom);
            msg.bloom = bloom;
            msg.req_num = req_count;
            
            count++;
  		  }
          
          msg.owner_addr = global_ipaddr;	  
          /* Time to send the data to the server */
          uip_udp_packet_sendto(client_conn, &msg, sizeof(DINASMSG),
                          &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
          /*                
          PRINTF("sent from %s to ", msg.owner_addr);
          PRINT6ADDR(&server_ipaddr);
          PRINTF("\n");
          */
          
        }
    }
  }

  PROCESS_END();
}

