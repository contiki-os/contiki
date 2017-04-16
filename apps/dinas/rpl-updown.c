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
 *         RPL-based Up-Down overlay routing for DINAS
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "contiki.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/ip/uiplib.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "rpl-updown.h"
#include "proximity-cache.h"
#include "bloom.h"
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#define UDP_SERVER_PORT 5688

static unsigned long time_offset;
uip_ipaddr_t destination_ipaddr;
char destination[40];
int sent_messages = 0;

uip_ipaddr_t *children[UIP_DS6_ROUTE_NB];
int num_children;
uip_ipaddr_t parent_ipaddr;


/*---------------------------------------------------------------------------*/
int rpl_updown_check_cache(DINASMSG *msg)
{
  CACHEITEM ci;
  ci.bloomname = msg->bloom;
  int i = proximity_cache_check_item(&ci);
  if (i != CACHE_SIZE) 
    return 1;
  else
    return 0;  
}


/*---------------------------------------------------------------------------*/
uip_ipaddr_t *rpl_updown_get_parent_ipaddr() 
{
  return &parent_ipaddr;
}


/*---------------------------------------------------------------------------*/
void
rpl_updown_init()
{
  proximity_cache_init();
}


/*---------------------------------------------------------------------------*/
/* see https://github.com/rklauck/contiki/blob/master/examples/udp-ipv6/udp-server.c */
void
rpl_updown_recv(DINASMSG* msg, uip_ipaddr_t* provider_ipaddr, struct uip_udp_conn* client_conn)
{ 
 	
  /*
  * 1) if msg is notification, estract data, create cache item and add it to the local cache 
  * 2) for any msg, if TTL > 0, pass the msg (with TTL-1) to dinas_updown_send() 
  */	
	 
  if (dinas_msg_get_type(msg->config) == 0) /* notification */
  {	
  	rpl_updown_store_item(msg, provider_ipaddr);
  }   
  else if (dinas_msg_get_type(msg->config) == 1) /* request */
  {
  	/*
  	PRINTF("DINAS routing, received request for:\n");
  	bloom_print(&msg->bloom);
    proximity_cache_print();
    */
    
  	/*  
     * if the requested bloom matches one of those that are in cache, send a reply msg to the request owner 
     */
    CACHEITEM ci;
    ci.bloomname = msg->bloom;
    int i = proximity_cache_check_item(&ci);
    //PRINTF("i = %d\n", i);
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
      return; /* comment this, if you want that multiple replies can be received by the requester */
    }
  }
  
  /* Message forwarding: */
  if (dinas_msg_get_direction(msg->config) == 0) /* down */
  { 
  	if (dinas_msg_get_ttl(msg->config) >= 1)
  	{
  	  msg->config = dinas_msg_set_config(dinas_msg_get_direction(msg->config), dinas_msg_get_type(msg->config), dinas_msg_get_ttl(msg->config)-1);
	  rpl_updown_send(msg, provider_ipaddr, client_conn);
  	}
  }
  else if (dinas_msg_get_direction(msg->config) == 1) /* up */
    rpl_updown_send(msg, provider_ipaddr, client_conn);
  //PRINTF("end updown_recv\n");
}


/*---------------------------------------------------------------------------*/
void 
set_random_destination(DINASMSG* msg, uip_ipaddr_t* provider_ipaddr) 
{  
  //PRINTF("setRanDest nc = %d\n", num_children);
  if (num_children == 0) 
  {   
    sprintf(destination, "NO-DEST");
    return;
  }
  else /* there are children */
  {
  	do 
  	{
  	  /*PRINTF("num_children = %d\n", num_children);*/
      /*PRINTF("k = %d\n", k);*/
  	  unsigned short r = random_rand();
  	  //PRINTF("r = %u\n", r);
  	  unsigned short i = (r)%(num_children);
  	  //PRINTF("i = %u\n", i);
  	  destination_ipaddr = *children[i]; /* send to one of the children */
  	  //PRINT6ADDR(&destination_ipaddr);
  	} while (uip_ipaddr_cmp(&destination_ipaddr, provider_ipaddr)); /* choose a destination which is not the provider! */
  }     
}


/*-----------------------------------------------------------------------------------*/
/* see https://github.com/rklauck/contiki/blob/master/examples/udp-ipv6/udp-client.c */
void
rpl_updown_send(DINASMSG* msg, uip_ipaddr_t* provider_ipaddr, struct uip_udp_conn* client_conn) 
{
  /*
  if msg direction is 1 (up)
  	if there is a parent, send the message to the parent
  	else change direction to 0, and send the msg to the most suitable children (!= from the sender)
  if msg direction is 0 (down)
  	send the msg to the most suitable children (if the cache is empty, randomly select the destination)
  */
  
  /*PRINTF("msg dir: %d\n", dinas_msg_get_direction(msg->config));*/
  if (dinas_msg_get_direction(msg->config) == 1) // up 
  {
  	char temp[40];
    sprint6addr(temp, &uip_ds6_if.addr_list[0].ipaddr);	
    //PRINTF("temp = %s\n", temp);
    if (strncmp(temp,"aaaa",4) == 0) /* this is the sink */
    {
      //msg->config -= 1; /* new direction is down */
      //PRINTF("rpl_updown_send: TTL = %d \n", dinas_msg_get_ttl(msg->config));
      //PRINTF("Sink: %d children \n", num_children);
      if (dinas_msg_get_ttl(msg->config) == 0)
      	return;
      else	// new direction is down and TTL is decreased by 1, as I am the sink
      	msg->config = dinas_msg_set_config(0, dinas_msg_get_type(msg->config), dinas_msg_get_ttl(msg->config)-1);
      if (num_children == 1) /* only one child, which was the message provider */
      { 
      	destination_ipaddr = *provider_ipaddr;
      	/*
      	PRINTF("Sending to: ");
        PRINT6ADDR(&destination_ipaddr);
        PRINTF("\n");
        */
        uip_udp_packet_sendto(client_conn, msg, sizeof(DINASMSG),
                          &destination_ipaddr, UIP_HTONS(UDP_SERVER_PORT)); 
        sent_messages += 1;
        return; 
      } 
      // 28/5/2014 we are here because this send() has been called by a recv(), thus TTL is 0 for next hop, not for us  
      /*  
      if (dinas_msg_get_ttl(msg->config) == 0) {
      	PRINTF("TTL is 0 \n");
        return;  
      }
      */
    }
    else /* this is not the sink */
    { 
      /*	     	
      PRINTF("up to parent \n"); 	
      PRINT6ADDR(&parent_ipaddr);
      PRINTF("\n");
      */
      destination_ipaddr = parent_ipaddr;
      sprint6addr(destination, &destination_ipaddr);
    }
  }
  
  if (dinas_msg_get_direction(msg->config) == 0) // down
  {
    //PRINTF("down\n");
    if (num_children == 0) /* no children */
      return; /* do not further propagate the msg */
  	
  	if ((FLOODING == 1) && (dinas_msg_get_type(msg->config) == 0)) /* only notifications are flooded! */
  	{
  	  /* send to all the children */
  	  PRINTF("Flooding!\n");
      int i;
      for (i = 0; i < num_children; i++) {
      	destination_ipaddr = *children[i];
    	sprint6addr(destination, &destination_ipaddr);
    	PRINTF("DEST: %s\n", destination);
    	uip_udp_packet_sendto(client_conn, msg, sizeof(DINASMSG),
                          &destination_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
        sent_messages += 1;
      } 
      return;
      
  	}
  	else /* no flooding */
  	{
  	  if (proximity_cache_size() > 0)
  	  {
        CACHEITEM* ci;
        ci = proximity_cache_get_most_similar_item(&(msg->bloom), provider_ipaddr); /* avoid the provider, unless it is the only one in cache */
        /*ci = proximity_cache_get_most_similar_item(&(msg->bloom));*/
        /* check that best destination is different from provider */ 
        /*if (strcmp(ci->provider_neighbor_addr, provider_ipaddr) != 0) */
        if (!uip_ipaddr_cmp(&(ci->provider_neighbor_addr), provider_ipaddr)) 
        { 
          /*PRINTF("dinas_updown_send: send to %s\n", ci->provider_neighbor_addr);*/
          /* concatenate fe80:0000:0000:0000: with ci->provider_neighbor_addr */
          /*
          sprintf(destination, "fe80:0000:0000:0000:");
          memcpy(destination+20, ci->provider_neighbor_addr, (SHORT_ADDR_SIZE+1)*sizeof(char));
          uiplib_ipaddrconv(destination, &destination_ipaddr);
          */
          destination_ipaddr = ci->provider_neighbor_addr;
        }
        else 
        {
      	  /* excluding the provider = the parent, select a children randomly */
          set_random_destination(msg, provider_ipaddr);
        }
      }
      else /* cache is empty or we are forced to send msg to a random destination */
      {
  	    /* excluding the provider = the parent, select a children randomly */
        set_random_destination(msg, provider_ipaddr);
      }
  	}
  }
    
  if (strcmp(destination, "NO-DEST") != 0) {
  	/*
    PRINTF("Sending to: ");
    PRINT6ADDR(&destination_ipaddr);
    PRINTF("\n");
    */
    uip_udp_packet_sendto(client_conn, msg, sizeof(DINASMSG),
                          &destination_ipaddr, UIP_HTONS(UDP_SERVER_PORT)); 
    sent_messages += 1; 
  }            
  /*PRINTF("end updown_send\n");*/         
}


/*---------------------------------------------------------------------------*/
int rpl_updown_sent_messages() 
{
  return sent_messages;	
}


/*---------------------------------------------------------------------------*/
void
rpl_updown_set_children() 
{
  uip_ds6_route_t *r;
  num_children = 0;
  
  //PRINTF("UIP_DS6_ROUTE_NB = %d", UIP_DS6_ROUTE_NB);
  	
  for(r = uip_ds6_route_head();
          r != NULL;
          r = uip_ds6_route_next(r)) {
    /*      	
    PRINTF("route to: ");
    uip_debug_ipaddr_print(&r->ipaddr);
    PRINTF(" via: ");
    uip_debug_ipaddr_print(uip_ds6_route_nexthop(r));
    PRINTF("\n");
    */
    if (!uip_ipaddr_cmp(uip_ds6_route_nexthop(r), &parent_ipaddr)) {
      if (num_children == 0) {
      	children[num_children] = uip_ds6_route_nexthop(r);
      	/*
      	PRINTF("child[%d] = ", num_children);
      	PRINT6ADDR(children[num_children]);
        PRINTF("\n"); 
        */ 
        num_children += 1;
      }
      else {  	
      	int i;
      	int flag = 0;
        for (i=0; i < num_children; i++) {
      	  if (uip_ipaddr_cmp(children[i], uip_ds6_route_nexthop(r)))
      	    flag = 1;
        }
        if (flag == 0) {
          children[num_children] = uip_ds6_route_nexthop(r);
          /*
          PRINTF("child[%d] = ", num_children);
          PRINT6ADDR(children[num_children]);
          PRINTF("\n");  
          */
          num_children += 1;
        }
      }	  	
    }

  }
}


/*---------------------------------------------------------------------------*/
void
rpl_updown_set_parent()
{
  parent_ipaddr = *uip_ds6_defrt_choose();
  /*
  PRINTF("Parent \n");
  PRINT6ADDR(&parent_ipaddr);
  PRINTF("\n");
  */
}


/*---------------------------------------------------------------------------*/
static unsigned long
get_time(void)
{
  return clock_seconds() + time_offset;
}


clock_time_t time;
/*---------------------------------------------------------------------------*/
void rpl_updown_store_item(DINASMSG *msg, uip_ipaddr_t* provider_ipaddr)
{
  //PRINTF("rpl_updown_store_item \n");	
  CACHEITEM ci;
  ci.bloomname = msg->bloom;
  ci.owner_addr = msg->owner_addr;
  ci.provider_neighbor_addr = *provider_ipaddr;
  time = get_time();
  ci.timestamp = time;
  proximity_cache_add_item(ci);
  //proximity_cache_print();
}





