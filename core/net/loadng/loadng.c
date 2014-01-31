/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: loadng.c,v 1.37 2010/01/20 09:58:16 chianhla Exp $
 */

/**
 * \file
 *         Implementation of the LOADng routing protocol 
 *         IETF draft draft-clausen-lln-loadng-00.txt
 *         Version for slotted 802.15.4 
 * \author 
 *         Chi-Anh La la@imag.fr         
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "net/loadng/loadng-def.h"
#include "net/loadng/loadng.h"
#include "contiki-net.h"


#include <string.h>

#define DEBUG 1
#include "net/uip-debug.h"

#define SEND_INTERVAL		25 * CLOCK_SECOND
#define RETRY_CHECK_INTERVAL	5 * CLOCK_SECOND
#define RV_CHECK_INTERVAL	10 * CLOCK_SECOND
#define MAX_SEQNO 65534
#define MAX_PAYLOAD_LEN		50
#define DEFAULT_PREFIX_LEN	128
#define DEFAULT_LOCAL_PREFIX    64
#define DEFAULT_OPT_SEQ_SKIP    4
extern uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];
#define UIP_IP_BUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define SEQNO_GREATER_THAN(s1, s2)                   \
          (((s1 > s2) && (s1 - s2 <= (MAX_SEQNO/2))) \
        || ((s2 > s1) && (s2 - s1 > (MAX_SEQNO/2))))
#if LOADNG_RREQ_RATELIMIT
static struct timer next_time;
#endif
static enum {
  COMMAND_NONE,
  COMMAND_SEND_RREQ,
  COMMAND_SEND_RERR,
} command;
#define LAST_RSSI cc2420_last_rssi 
// extern int8_t last_rssi; // for stm32w
extern signed char cc2420_last_rssi ;
static struct ctimer sendmsg_ctimer;
static u16_t my_hseqno, my_seq_id;
static int8_t my_rank;
static uint8_t my_weaklink;
static int8_t my_parent_rssi;
static u16_t local_prefix_len;
static u8_t opt_seq_skip_counter;
uip_ipaddr_t local_prefix;
uip_ipaddr_t ipaddr, myipaddr, mcastipaddr;
uip_ipaddr_t orig_addr, dest_addr, rreq_addr, def_rt_addr, my_sink_id;
static struct uip_udp_conn *udpconn;
static uip_ipaddr_t rerr_bad_addr, rerr_src_addr, rerr_next_addr;
static u8_t in_loadng_call=0 ; // make sure we don't trigger a rreq from within loadng
/*---------------------------------------------------------------------------*/
PROCESS(loadng_process, "LOADng process");
/*---------------------------------------------------------------------------*/


/* Route lookup without triggering RREQ ! */
/*---------------------------------------------------------------------------*/
static uip_ds6_route_t *
loadng_route_lookup(uip_ipaddr_t *addr)
{
  uip_ds6_route_t *r;
  uip_ds6_route_t *found_route;
  uint8_t longestmatch;

  PRINTF("loadng_route_lookup: Looking up route for ");
  PRINT6ADDR(addr);
  PRINTF("\n");


  found_route = NULL;
  longestmatch = 0;
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {
    if(r->length >= longestmatch &&
       uip_ipaddr_prefixcmp(addr, &r->ipaddr, r->length)) {
      longestmatch = r->length;
      found_route = r;
    }
  }

  if(found_route != NULL) {
    PRINTF("loadng_route_lookup: Found route: ");
    PRINT6ADDR(addr);
    PRINTF(" via ");
    PRINT6ADDR(uip_ds6_route_nexthop(found_route));
    PRINTF("\n");
  } else {
    PRINTF("loadng_route_lookup: No route found\n");
  }

  return found_route;
}
/*---------------------------------------------------------------------------*/



/* Implementation of route validity time check and purge */

static void
loadng_check_expired_route(u16_t interval)
{ 
  uip_ds6_route_t *r; 
    
  for(r = uip_ds6_route_head();
  r != NULL;
  r = uip_ds6_route_next(r)) {

    if(r->state.valid_time <= interval) {
      uip_ds6_route_rm(r);
    } else {
      r->state.valid_time -= interval; 
    }
  }

}

/*---------------------------------------------------------------------------*/
/* Implementation of request forwarding cache to avoid multiple forwarding */
#define FWCACHE 2

static struct {
  uip_ipaddr_t orig;
  u16_t seqno;
} fwcache[FWCACHE];

static int
fwc_lookup(const uip_ipaddr_t *orig, const u16_t *seqno)
{
  unsigned n = (((u8_t *)orig)[0] + ((u8_t *)orig)[15]) % FWCACHE;
  return fwcache[n].seqno == *seqno && uip_ipaddr_cmp(&fwcache[n].orig, orig);
}

static void
fwc_add(const uip_ipaddr_t *orig, const u16_t *seqno)
{
  unsigned n = (((u8_t *)orig)[0] + ((u8_t *)orig)[15]) % FWCACHE;
  fwcache[n].seqno = *seqno;
  uip_ipaddr_copy(&fwcache[n].orig, orig);
}


/*---------------------------------------------------------------------------*/
/* Implementation of route request cache for LOADNG_RREQ_RETRIES and LOADNG_NET_TRAVERSAL_TIME */
#define RRCACHE 2

static struct {
  uip_ipaddr_t dest;
  u16_t expire_time;
  u8_t request_time;
} rrcache[RRCACHE];

static int
rrc_lookup(const uip_ipaddr_t *dest)
{
  unsigned n = (((u8_t *)dest)[0] + ((u8_t *)dest)[15]) % RRCACHE;
  return uip_ipaddr_cmp(&rrcache[n].dest, dest);
}

static void
rrc_remove(const uip_ipaddr_t *dest)
{
  unsigned n = (((u8_t *)dest)[0] + ((u8_t *)dest)[15]) % RRCACHE;
  if(uip_ipaddr_cmp(&rrcache[n].dest, dest))
  {
     memset(&rrcache[n].dest, 0, sizeof(&rrcache[n].dest));
     rrcache[n].expire_time = 0;
     rrcache[n].request_time = 0;
  }
}

static void
rrc_add(const uip_ipaddr_t *dest)
{
  unsigned n = (((u8_t *)dest)[0] + ((u8_t *)dest)[15]) % RRCACHE;
  rrcache[n].expire_time = LOADNG_NET_TRAVERSAL_TIME;
  rrcache[n].request_time = 1;
  uip_ipaddr_copy(&rrcache[n].dest, dest);
}

static void
rrc_check_expired_rreq(u16_t interval)
{ 
  int i;
  for(i = 0; i < RRCACHE; ++i){  
     rrcache[i].expire_time -= interval; 
     if(rrcache[i].expire_time <=0){
          loadng_request_route_to(&rrcache[i].dest);
          rrcache[i].request_time++;
          if(rrcache[i].request_time == LOADNG_RREQ_RETRIES){  
             rrc_remove(&rrcache[i].dest);
          } else {
             rrcache[i].expire_time = LOADNG_NET_TRAVERSAL_TIME;   
          } 
     } 
  }

}


/*---------------------------------------------------------------------------*/
void
uip_ds6_route_print(void)
{
  uip_ds6_route_t *locroute = NULL;


  PRINTF("LOADng: Print route entry: ");
  PRINT6ADDR(&myipaddr);
  
for(locroute = uip_ds6_route_head();
  locroute != NULL;
  locroute = uip_ds6_route_next(locroute)) {
    PRINTF(" DEST "); 
    PRINT6ADDR(&locroute->ipaddr);
    PRINTF("/%u ", locroute->length);
    PRINTF(" NEXT ");  
    PRINT6ADDR(uip_ds6_route_nexthop(locroute));
    PRINTF(" HC %u ", locroute->state.route_cost);
    PRINTF(" | "); 
  }
  PRINTF("\n");

}

static inline uint8_t get_weaklink(uint8_t metric){
  return (metric & 0x0f) ;
}
static inline uint8_t parent_weaklink(int8_t rssi){
  return ((rssi > LOADNG_RSSI_THRESHOLD) ? 0 : 1) ;
}

/*---------------------------------------------------------------------------*/
static int
get_global_addr(uip_ipaddr_t *addr)
{
  int i;
  int state;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      //if(!uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
        memcpy(addr, &uip_ds6_if.addr_list[i].ipaddr, sizeof(uip_ipaddr_t));
        return 1;
      //}
    }
  }
  return 0;
}

static void uip_loadng_nbr_add(uip_ipaddr_t* next_hop){
#if !UIP_ND6_SEND_NA
  uip_ds6_nbr_t *nbr = NULL;
  // it's my responsability to create+maintain neighbor
  nbr = uip_ds6_nbr_lookup(next_hop);
  
  if (nbr==NULL){
    PRINTF("adding nbr from loadng\n");
    uip_lladdr_t nbr_lladdr;
    memcpy(&nbr_lladdr, &next_hop->u8[8],
           UIP_LLADDR_LEN);

    nbr_lladdr.addr[0] ^= 2;
    nbr = uip_ds6_nbr_add(next_hop, &nbr_lladdr, 0, NBR_REACHABLE);
//    nbr->nscount = 1;

  }
  else{
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");
  }
  
#endif /* !UIP_ND6_SEND_NA */
}


static uip_ds6_route_t* uip_loadng_route_add(uip_ipaddr_t* orig_addr, uint8_t length,
            uip_ipaddr_t* next_hop,uint8_t route_cost,uint16_t seqno)
{

  struct uip_ds6_route *rt;
  
  in_loadng_call=1;

  PRINTF(" nexthop ");
  PRINT6ADDR(next_hop);
  PRINTF(" \n ");
  uip_loadng_nbr_add(next_hop);
  
  rt = uip_ds6_route_add(orig_addr, length, next_hop);
        PRINTF("passed add route\n");

  rt->state.route_cost=route_cost ;
  rt->state.seqno=seqno ;
  rt->state.valid_time = LOADNG_R_HOLD_TIME ;

  in_loadng_call=0;

  return rt ;
}
/*---------------------------------------------------------------------------*/
static void
send_qry()
{
  
  char buf[MAX_PAYLOAD_LEN];
  PRINTF("LOADng: Send QRY from ");
  PRINT6ADDR(&myipaddr);
  PRINTF("\n"); 

  struct loadng_msg_qry *rm = (struct loadng_msg_qry *)buf;

  rm->type = LOADNG_QRY_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;
  udpconn->ttl = 1;
  uip_create_linklocal_lln_routers_mcast(&udpconn->ripaddr);
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_qry));
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
 
}
/*---------------------------------------------------------------------------*/
#if RDC_LAYER_ID == ID_mac_802154_rdc_driver
void
#else
static void
#endif /* RDC_LAYER_ID == ID_mac_802154_rdc_driver */
send_opt()
{
  if(!LOADNG_IS_COORDINATOR())
     return; //no OPT from device
  if(!LOADNG_IS_SINK && my_seq_id==0)
     return; //wait for sink OPT
  char buf[MAX_PAYLOAD_LEN];
  PRINTF("LOADng: Send OPT from ");
  PRINT6ADDR(&myipaddr);
  PRINTF("\n"); 

  struct loadng_msg_opt *rm = (struct loadng_msg_opt *)buf;

  rm->type = LOADNG_OPT_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;	
  rm->seqno = my_seq_id;
  rm->rank = my_rank;
  rm->metric = LOADNG_METRIC_HC;
  rm->metric = (rm->metric << 4) | my_weaklink + parent_weaklink(my_parent_rssi);
  if(LOADNG_IS_SINK){
     uip_ipaddr_copy(&rm->sink_addr, &myipaddr);
  } else {
     uip_ipaddr_copy(&rm->sink_addr, &my_sink_id);
  }
  udpconn->ttl = 1;
  uip_create_linklocal_lln_routers_mcast(&udpconn->ripaddr);
  PRINTF("LOADng: OPT length %u\n", sizeof(struct loadng_msg_opt));
// #if RDC_LAYER_ID == ID_mac_802154_rdc_driver  
//   tcpip_set_outputfunc(output_802154);
//   uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_opt));
//   tcpip_set_outputfunc(output);
// #else
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_opt));
// #endif /* RDC_LAYER_ID == ID_mac_802154_rdc_driver */
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
#if LOADNG_IS_SINK
  if(opt_seq_skip_counter >= DEFAULT_OPT_SEQ_SKIP){
    opt_seq_skip_counter = 0;
    my_seq_id++;
    if(my_seq_id > MAX_SEQNO)
      my_seq_id = 1;
  } else {
        opt_seq_skip_counter++;
  } 	
#endif
}
/*---------------------------------------------------------------------------*/
static void
send_rreq()
{
  
  char buf[MAX_PAYLOAD_LEN];
  PRINTF("LOADng: Send RREQ for ");
  PRINT6ADDR(&rreq_addr);
  PRINTF(" from ");
  PRINT6ADDR(&myipaddr);
  PRINTF("\n"); 

  struct loadng_msg_rreq *rm = (struct loadng_msg_rreq *)buf;

  rm->type = LOADNG_RREQ_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;
  my_hseqno++;
  if(my_hseqno>MAX_SEQNO)
    my_hseqno = 1; 		
  rm->seqno = my_hseqno;
  rm->metric = LOADNG_METRIC_HC;
  rm->metric = (rm->metric << 4) | LOADNG_WEAK_LINK;
  rm->route_cost = 0;
  uip_ipaddr_copy(&rm->dest_addr, &rreq_addr);
  uip_ipaddr_copy(&rm->orig_addr, &myipaddr);
  udpconn->ttl = LOADNG_MAX_DIST;
  uip_create_linklocal_lln_routers_mcast(&udpconn->ripaddr);
  PRINTF("LOADng: RREQ length %u\n", sizeof(struct loadng_msg_rreq));
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_rreq));
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
 
}
/*---------------------------------------------------------------------------*/
static void
send_rrep(uip_ipaddr_t *dest, uip_ipaddr_t *nexthop, uip_ipaddr_t *orig,
	  u16_t *seqno, unsigned hop_count)
{ 
  char buf[MAX_PAYLOAD_LEN];
  struct loadng_msg_rrep *rm = (struct loadng_msg_rrep *)buf;
  PRINTF("LOADng: Send RREP for orig ");
  PRINT6ADDR(orig);
  PRINTF(" dest ");
  PRINT6ADDR(dest);
  PRINTF(" nexthop ");
  PRINT6ADDR(nexthop);
  PRINTF(" hopcount=%u\n", hop_count);
  
  rm->type = LOADNG_RREP_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;
  rm->seqno = *seqno;
  rm->metric = LOADNG_METRIC_HC;
  rm->metric = (rm->metric << 4) | LOADNG_WEAK_LINK;
  rm->route_cost = hop_count;
  uip_ipaddr_copy(&rm->orig_addr, orig);
  uip_ipaddr_copy(&rm->dest_addr, dest);
  udpconn->ttl = LOADNG_MAX_DIST;
  uip_ipaddr_copy(&udpconn->ripaddr, nexthop);
  PRINTF("LOADng: RREP length: %u\n", sizeof(struct loadng_msg_rrep)); 
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_rrep));
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
  
}

/*---------------------------------------------------------------------------*/
static void
send_rerr(uip_ipaddr_t *src, uip_ipaddr_t *dest, uip_ipaddr_t *nexthop)
{ 
  char buf[MAX_PAYLOAD_LEN];
  struct loadng_msg_rerr *rm = (struct loadng_msg_rerr *)buf;
  PRINTF("LOADng: Send RERR towards src: ");
  PRINT6ADDR(src);
  PRINTF(" for address in error: ");
  PRINT6ADDR(dest);
  PRINTF(" nexthop: ");
  PRINT6ADDR(nexthop);
  PRINTF("\n");
  rm->type = LOADNG_RERR_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;
  uip_ipaddr_copy(&rm->addr_in_error, dest);
  uip_ipaddr_copy(&rm->src_addr, src);
  udpconn->ttl = LOADNG_MAX_DIST;
  uip_ipaddr_copy(&udpconn->ripaddr, nexthop);
  PRINTF("LOADng: RERR length: %u\n", sizeof(struct loadng_msg_rerr)); 
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_rerr));
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
}
/*---------------------------------------------------------------------------*/
static void
send_rack(uip_ipaddr_t *src, uip_ipaddr_t *nexthop, u16_t seqno)
{ 
  char buf[MAX_PAYLOAD_LEN];
  struct loadng_msg_rack *rm = (struct loadng_msg_rack *)buf;
  PRINTF("LOADng: Send RACK for src ");
  PRINT6ADDR(src);
  PRINTF(" nexthop ");
  PRINT6ADDR(nexthop);
  PRINTF("\n");
  rm->type = LOADNG_RACK_TYPE;
  rm->type = (rm->type << 4) | LOADNG_RSVD1;
  rm->addr_len = LOADNG_RSVD2; 
  rm->addr_len = (rm->type << 4) | LOADNG_ADDR_LEN_IPV6;
  uip_ipaddr_copy(&rm->src_addr, src);
  rm->seqno = seqno;
  udpconn->ttl = LOADNG_MAX_DIST;
  uip_ipaddr_copy(&udpconn->ripaddr, nexthop);
  PRINTF("LOADng: RACK length: %u\n", sizeof(struct loadng_msg_rack)); 
  uip_udp_packet_send(udpconn, buf, sizeof(struct loadng_msg_rack));
  memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rreq(void)
{
  struct loadng_msg_rreq *rm = (struct loadng_msg_rreq *)uip_appdata;
  uip_ipaddr_t dest_addr, orig_addr;

  PRINTF("LOADng: RREQ ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" -> ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" ttl=%u ", UIP_IP_BUF->ttl);
  PRINTF(" hop=%u ", rm->route_cost);
  PRINTF(" seq=%u ", rm->seqno);
  PRINTF(" orig ");
  PRINT6ADDR(&rm->orig_addr);
  PRINTF(" dest ");
  PRINT6ADDR(&rm->dest_addr);
  PRINTF("\r\n");

  if(loadng_is_my_global_address(&rm->orig_addr)) {
    PRINTF("LOADng: RREQ loops back, not processing\n");
    return;			
  }


  /* Do not add reverse route while receiving RREQ
  rt = loadng_route_lookup(&rm->orig_addr);

  if(rt == NULL){
    PRINTF("LOADng: Inserting route from RREQ\n");
    rt = uip_ds6_route_add(&rm->orig_addr, DEFAULT_PREFIX_LEN,
            &UIP_IP_BUF->srcipaddr, rm->route_cost, rm->seqno, LOADNG_R_HOLD_TIME);
  } else if(SEQNO_GREATER_THAN(rm->seqno,rt->state.seqno) 
            || (rm->seqno==rt->state.seqno && rm->route_cost < rt->state.route_cost)){
    PRINTF("LOADng: Update route from RREQ\n");
    uip_ipaddr_copy(&rt->nexthop, &UIP_IP_BUF->srcipaddr);
    rt->state.seqno = rm->seqno;
    rt->state.route_cost = rm->route_cost;
    rt->state.valid_time = LOADNG_R_HOLD_TIME;
  } else {
    PRINTF("LOADng: Not a better route for inserting (RREQ)\n");
  }
    
  */

  if(loadng_is_my_global_address(&rm->dest_addr)) { /* RREQ for our address */
    PRINTF("LOADng: RREQ for our address\n");
    uip_ipaddr_copy(&dest_addr, &rm->orig_addr);
    uip_ipaddr_copy(&orig_addr, &rm->dest_addr);
    my_hseqno++;
    if(my_hseqno>MAX_SEQNO)
	my_hseqno = 1; 	
    send_rrep(&dest_addr, &UIP_IP_BUF->srcipaddr, &orig_addr, &my_hseqno, 0); 
  } else if (LOADNG_IS_COORDINATOR()) {  //only coordinator forward RREQ
    if(UIP_IP_BUF->ttl > 1) { /* TTL still valid for forwarding */

      /* Have we seen this RREQ before? */
      if(fwc_lookup(&rm->orig_addr, &rm->seqno)) {
        PRINTF("LOADng: RREQ cached, not forward\n");
        return;
      }
      fwc_add(&rm->orig_addr, &rm->seqno);

      PRINTF("LOADng: RREQ forward\n");
      rm->route_cost++;
      udpconn->ttl = UIP_IP_BUF->ttl - 1;
      uip_create_linklocal_lln_routers_mcast(&udpconn->ripaddr);
#if LOADNG_RANDOM_WAIT == 1
      PRINTF("waiting rand time\n");
      clock_wait(random_rand()%50 * CLOCK_SECOND / 1000);
#endif

      uip_udp_packet_send(udpconn, rm, sizeof(struct loadng_msg_rreq));
      memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rrep(void)
{
  struct loadng_msg_rrep *rm = (struct loadng_msg_rrep *)uip_appdata;
  struct uip_ds6_route *rt;
  uip_ipaddr_t *nexthop;
  /* No multicast RREP: drop */
  if(uip_ipaddr_cmp(&UIP_IP_BUF->destipaddr, &mcastipaddr)) {
    return;
  }
  PRINTF("LOADng: RREP ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" -> ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" ttl=%u ", UIP_IP_BUF->ttl);
  PRINTF(" hop=%u ", rm->route_cost);
  PRINTF(" seq=%u ", rm->seqno);
  PRINTF(" orig ");
  PRINT6ADDR(&rm->orig_addr);
  PRINTF(" dest ");
  PRINT6ADDR(&rm->dest_addr);
  PRINTF("\r\n");

  rt = loadng_route_lookup(&rm->orig_addr);

  /* New forward route? */
  if(rt == NULL){
    PRINTF("LOADng: Inserting route from RREP\n");
    rt=uip_loadng_route_add(&rm->orig_addr, DEFAULT_PREFIX_LEN,
            &UIP_IP_BUF->srcipaddr,rm->route_cost,rm->seqno);
#if LOADNG_RREP_ACK
    rt->state.ack_received = 0; /* Pending route for ACK */
#else
    rt->state.ack_received = 1; 
#endif
  }
  else if(SEQNO_GREATER_THAN(rm->seqno,rt->state.seqno) 
          || (rm->seqno==rt->state.seqno && rm->route_cost < rt->state.route_cost)){
    PRINTF("LOADng: Update route from RREP\n");
    uip_ds6_route_rm(rt);
    uip_loadng_route_add(&rm->orig_addr, DEFAULT_PREFIX_LEN,
            &UIP_IP_BUF->srcipaddr,rm->route_cost,rm->seqno);
  } else {
    PRINTF("LOADng: Not a better route for inserting (RREP)\n");
  }

  /* Forward RREP towards originator? */
  if(uip_ipaddr_cmp(&rm->dest_addr, &myipaddr)) {
    PRINTF("LOADng: Received RREP to our own RREQ\n");
#if LOADNG_RREQ_RETRIES
    //remove route request cache
    rrc_remove(&rm->orig_addr);
#endif
  } else {
    //rt = loadng_route_lookup(&rm->dest_addr);
    nexthop = uip_ds6_defrt_choose();
    if(nexthop == NULL) {
      PRINTF("LOADng: RREP received, but no default route to originator\n");
      // No ACK and RREP forwarding
      return;
    }
    // Send ACK
#if LOADNG_RREP_ACK
    send_rack(&rm->orig_addr, &UIP_IP_BUF->srcipaddr, rm->seqno);
#endif    

    PRINTF("LOADng: Forward RREP to ");
    PRINT6ADDR(nexthop);
    PRINTF("\n");
    rm->route_cost++;
    udpconn->ttl = 1;
    uip_ipaddr_copy(&udpconn->ripaddr, nexthop);
    uip_udp_packet_send(udpconn, rm, sizeof(struct loadng_msg_rrep));
    memset(&udpconn->ripaddr, 0, sizeof(udpconn->ripaddr));
    
  }
}

/*---------------------------------------------------------------------------*/
static void
handle_incoming_rack(void)
{
  struct loadng_msg_rack *rm = (struct loadng_msg_rack *)uip_appdata;
  struct uip_ds6_route *rt;


  PRINTF("LOADng: RACK ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" -> ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" ttl=%u ", UIP_IP_BUF->ttl);
  PRINTF(" seq=%u ", rm->seqno);
  PRINTF(" src ");
  PRINT6ADDR(&rm->src_addr);
  PRINTF("\r\n");

  rt = loadng_route_lookup(&rm->src_addr);

  /* No route? */
  if(rt == NULL){
    PRINTF("LOADng: Receved RACK for non-existing route\n");
  } else {
    rt->state.ack_received = 1; /* Make pending route valid */
  }
  
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_rerr(void)
{
  struct loadng_msg_rerr *rm = (struct loadng_msg_rerr *)uip_appdata;
  struct uip_ds6_route *rt_in_err;
  struct uip_ds6_route *rt;
  uip_ds6_defrt_t *defrt;
  
  PRINTF("LOADng: RERR ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" -> ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" ttl=%u ", UIP_IP_BUF->ttl);
  PRINTF(" towards ");
  PRINT6ADDR(&rm->src_addr);
  PRINTF(" addr in error : ");
  PRINT6ADDR(&rm->addr_in_error);
  PRINTF("\r\n");

  rt_in_err = loadng_route_lookup(&rm->addr_in_error);
  rt = loadng_route_lookup(&rm->src_addr);

  /* No route? */
  if(rt_in_err == NULL){
    PRINTF("LOADng: Received RERR for non-existing route\n");
  }
  else{
    uip_ds6_route_rm(rt_in_err); /* Remove route */
  }

  #if USE_OPT
  // if the RERR comes from a default router and it's for me, send spontaneous RREP
  defrt=uip_ds6_defrt_lookup(&UIP_IP_BUF->srcipaddr);
  if(defrt!=NULL && loadng_is_my_global_address(&rm->src_addr)){
  PRINTF("send RREP\n");
    send_rrep(&my_sink_id, &defrt->ipaddr, &myipaddr, &my_hseqno, 0);
  }
  // otherwise, if there is a matching tupple, send along default route
  
  #endif //USE_OPT
  // Draft draft-clausen-lln-loadng-10#section-14.3 : still forward even if no matching routing tupple found
  if(rt != NULL)
    send_rerr(&rm->src_addr, &rm->addr_in_error, uip_ds6_route_nexthop(rt)); /* Forward RERR to nexthop */
}
/*---------------------------------------------------------------------------*/
void
reinitialize_default_route(void)
{
  my_rank = 255;
  my_weaklink = 255;
  my_parent_rssi = -126;
  my_seq_id = 0;
  uip_ds6_defrt_t *defrt;
  defrt = uip_ds6_defrt_lookup(&def_rt_addr);
  if(defrt != NULL) {
    uip_ds6_defrt_rm(defrt);
  }
}
/*---------------------------------------------------------------------------*/
static void
change_default_route(struct loadng_msg_opt *rm)
{
  uip_ds6_defrt_t *defrt;
  my_rank = rm->rank + 1;
  my_weaklink = get_weaklink(rm->metric); 
  // add default foute 
  defrt = uip_ds6_defrt_lookup(&def_rt_addr);
  if(defrt !=  NULL){ //remove
    uip_ds6_defrt_rm(defrt);
  }
  
  in_loadng_call=1;
  
  PRINTF("LOADng: call uip_loadng_nbr_add\n");
  uip_loadng_nbr_add(&UIP_IP_BUF->srcipaddr);

  
  uip_ds6_defrt_add(&UIP_IP_BUF->srcipaddr, LOADNG_DEFAULT_ROUTE_LIFETIME);
  uip_ipaddr_copy(&def_rt_addr, &UIP_IP_BUF->srcipaddr);
  uip_ipaddr_copy(&def_rt_addr, &UIP_IP_BUF->srcipaddr);
  if (!LOADNG_IS_COORDINATOR()) {
    ctimer_set(&sendmsg_ctimer, random_rand() / 1000,
        (void (*)(void *))send_opt, NULL); 
  }
  // fixme: code specific to beacon-enabled
//   uip_lladdr_t coord_addr;
//   memcpy(&coord_addr, &UIP_IP_BUF->srcipaddr.u8[8], UIP_LLADDR_LEN);  
//   coord_addr.addr[0] ^= 2;
//   NETSTACK_RDC_CONFIGURATOR.coordinator_choice((void *) &coord_addr, 
//       rm->rank);
  in_loadng_call=0;
}
/*---------------------------------------------------------------------------*/
static void
handle_incoming_opt(void)
{ uint8_t parent_changed = 0;
  uint8_t opt_weaklink = 0;
  // fixme: code specific to beacon-enabled
//   if (NETSTACK_RDC_CONFIGURATOR.use_routing_information()) {
    struct loadng_msg_opt *rm = (struct loadng_msg_opt *)uip_appdata;
    PRINTF("LOADng: OPT ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(" -> ");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF(" ttl=%u ", UIP_IP_BUF->ttl);
    PRINTF(" rank=%d ", rm->rank);
    PRINTF(" seq=%d ", rm->seqno);
    PRINTF(" rssi=%i ", (int8_t)LAST_RSSI);
    PRINTF("\r\n");

    if(uip_ipaddr_cmp(&UIP_IP_BUF->srcipaddr, &myipaddr)) {
      PRINTF("LOADng: OPT loops back, not processing\n");
      return;			
    } 

    if(rm->seqno > my_seq_id  ) {
      /* First OPT */
      PRINTF("LOADng: New OPT sequence number received\n");
      my_seq_id = rm->seqno;
      parent_changed = 1;
    } else {
      opt_weaklink = get_weaklink(rm->metric) + parent_weaklink((int8_t)LAST_RSSI);
      if(opt_weaklink < my_weaklink + parent_weaklink(my_parent_rssi)){ /* less weak links */
         parent_changed = 1;
         printf("less weak links\n");
      } else if(opt_weaklink == my_weaklink + parent_weaklink(my_parent_rssi)){ /* weak link ties  */   
         if(rm->rank < (my_rank - 1)) { /* better rank */
            parent_changed = 1;
            printf("better rank\n");

         } 
//          else if (!uip_ipaddr_cmp(&UIP_IP_BUF->srcipaddr, &def_rt_addr) && my_parent_rssi < (int8_t)LAST_RSSI){
//             parent_changed = 1;
//          printf("addr  -- "); PRINT6ADDR(&UIP_IP_BUF->srcipaddr);printf("   "); PRINT6ADDR(&def_rt_addr);printf("\n");
//          }
      }
    }


    if(parent_changed){
       uip_ipaddr_copy(&my_sink_id, &rm->sink_addr);
       my_parent_rssi = (int8_t)LAST_RSSI;
       PRINTF("LOADng: Update from received OPT r=%u wl=%u rssi=%i\n", my_rank, my_weaklink + parent_weaklink(my_parent_rssi), my_parent_rssi); 
       change_default_route(rm);
#if LOADNG_IS_SKIP_LEAF
       my_hseqno++;
       if(my_hseqno>MAX_SEQNO)
         my_hseqno = 1; 
       nexthop =  uip_ds6_defrt_choose();	
       send_rrep(&rm->sink_addr, nexthop, &myipaddr, &my_hseqno, 0); 
#endif
    } else {
       PRINTF("LOADng: Not a better rank/RSSI\n");
    }
    
//   } else {
//        PRINTF("LOADng: Node already associated. OPT not processed\n");
//   } 
}

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("LOADng IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/

static void
tcpip_handler(void)
{
  
  u8_t type;
  if(uip_newdata()) {

  
    struct loadng_msg *m = (struct loadng_msg *)uip_appdata;
    type = m->type >> 4;
    PRINTF("LOADng: Packet type %u\n", type);
    if(type==LOADNG_RREQ_TYPE){
      PRINTF("LOADng: Received RREQ\n");
      handle_incoming_rreq();
    } else
    if(type==LOADNG_RREP_TYPE){
      PRINTF("LOADng: Received RREP\n");
      handle_incoming_rrep();
    } else
    if(type==LOADNG_RERR_TYPE){
      PRINTF("LOADng: Received RERR\n");
      handle_incoming_rerr();
    } else
    if(type==LOADNG_RACK_TYPE){
      PRINTF("LOADng: Received RACK\n");
      handle_incoming_rack();
    }
    if(type==LOADNG_OPT_TYPE){
      PRINTF("LOADng: Received OPT\n");
      handle_incoming_opt();
    }
    if(type==LOADNG_QRY_TYPE){
      PRINTF("LOADng: Received QRY\n");
      send_opt();
    }
  }
}
/*---------------------------------------------------------------------------*/
uint8_t loadng_addr_matches_local_prefix(uip_ipaddr_t *host){
  return uip_ipaddr_prefixcmp(&local_prefix, host, local_prefix_len);
}


/*---------------------------------------------------------------------------*/
void
loadng_request_route_to(uip_ipaddr_t *host)
{

  if(in_loadng_call){
    return;
  }
#if !LOADNG_IS_SINK && USE_OPT
    PRINTF("Only sink sends RREQ\n");
    return ; //only sink sends RREQ
#endif
  if(!loadng_addr_matches_local_prefix(host)) {
    PRINTF("no RREQ for non-local address\n");
    //no local prefix matches this addr, no RREQ for non-local address
    return ;
  } 
#if LOADNG_RREQ_RATELIMIT
  if(!timer_expired(&next_time)) {
     PRINTF("LOADng: RREQ exceeds rate limit"); 
     return ;
  }
#endif

  PRINTF("LOADng: Request for route "); 
  PRINT6ADDR(host);
  PRINTF("\n");
  if(uip_ds6_addr_lookup(host)==NULL)
  {
    uip_ipaddr_copy(&rreq_addr, host);
    command = COMMAND_SEND_RREQ;
    process_post(&loadng_process, PROCESS_EVENT_MSG, NULL);
#if LOADNG_RREQ_RETRIES
    if(!rrc_lookup(&rreq_addr))
    {
   	rrc_add(&rreq_addr);
    }
#endif
  }
  else{
    PRINTF("LOADng: Request for our address. Do nothing\n"); 
  }
#if LOADNG_RREQ_RATELIMIT
  timer_set(&next_time, CLOCK_SECOND/LOADNG_RREQ_RATELIMIT); 
#endif
}
/*---------------------------------------------------------------------------*/
void
loadng_no_route(uip_ipaddr_t *dest, uip_ipaddr_t *src)
{
  struct uip_ds6_route *rt = NULL;
  uip_ipaddr_t nexthop;
  uip_ipaddr_t *nexthptr=&nexthop;
  rt = loadng_route_lookup(src);
  PRINTF("loadng_no_route(): dest:"); PRINT6ADDR(dest); PRINTF(" src:");PRINT6ADDR(src);PRINTF("\n");
  if(rt == NULL){
    PRINTF("rt is NULL\n");
    #if USE_OPT
    // multicast RERR so the neighbors are notified -- this is helpful if a node has lost memory
    uip_create_linklocal_lln_routers_mcast(nexthptr);
    PRINTF("send to: "); PRINT6ADDR(nexthptr);PRINTF("\n");
    #else
    PRINTF("LOADng: Send a RERR with no route source address\n"); 
    return;
    #endif //USE_OPT
  }
  else{
    nexthptr=uip_ds6_route_nexthop(rt);
  }
  uip_ipaddr_copy(&rerr_bad_addr, dest);
  uip_ipaddr_copy(&rerr_src_addr, src);
  uip_ipaddr_copy(&rerr_next_addr, nexthptr);
  command = COMMAND_SEND_RERR;
  process_post(&loadng_process, PROCESS_EVENT_MSG, NULL);
}
/*---------------------------------------------------------------------------*/
void
loadng_set_local_prefix(uip_ipaddr_t *prefix, uint8_t len)
{
  uip_ipaddr_copy(&local_prefix, prefix);
  local_prefix_len = len;
}
/*---------------------------------------------------------------------------*/
void 
get_prefix_from_addr(uip_ipaddr_t *addr, uip_ipaddr_t *prefix, uint8_t len)
{
  uint8_t i;
  local_prefix_len = len; 
  for(i = 0; i < 16; i++) {
    if(i < len/8)
    {
       prefix->u8[i] = addr->u8[i];
    } else {
       prefix->u8[i] = 0;
     
    }
  } 
}
/*---------------------------------------------------------------------------*/
uint8_t loadng_is_my_global_address(uip_ipaddr_t *addr){
  return uip_ipaddr_cmp(addr, &myipaddr);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(loadng_process, ev, data)
{
  static struct etimer et, dfet, rv;
  

  PROCESS_BEGIN();
  PRINTF("LOADng process started\n");
#if LOADNG_IS_SINK
  my_seq_id = 1;
  my_rank = 1;
  my_weaklink = 0;
  my_parent_rssi = 126;
  opt_seq_skip_counter = 0;
#else
  my_seq_id = 0;
  my_rank = 255;
  my_weaklink = 255;
  my_parent_rssi = -126;
#endif
  PRINTF("LOADng is sink:%d \n",(int)LOADNG_IS_SINK);

  my_hseqno = 1;
  print_local_addresses();
  get_global_addr(&myipaddr);
  get_prefix_from_addr(&myipaddr, &local_prefix, DEFAULT_LOCAL_PREFIX);  
  uip_create_linklocal_lln_routers_mcast(&mcastipaddr);
  uip_create_linklocal_empty_addr(&def_rt_addr);
  uip_ds6_maddr_add(&mcastipaddr);
  /* new connection UDP */
  udpconn = udp_new(NULL, UIP_HTONS(LOADNG_UDPPORT), NULL);
  udp_bind(udpconn, UIP_HTONS(LOADNG_UDPPORT));

  PRINTF("LOADng: Created an UDP socket  ");
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(udpconn->lport), UIP_HTONS(udpconn->rport));

  if (LOADNG_IS_COORDINATOR()) {
    PRINTF("LOADng: Set timer for OPT multicast %u\n", SEND_INTERVAL);
    etimer_set(&et, SEND_INTERVAL);
    memset(&ipaddr, 0, sizeof(ipaddr));
  }
#if LOADNG_RREQ_RETRIES
  PRINTF("LOADng: Set timer for RREQ retry %u\n", RETRY_CHECK_INTERVAL);
  etimer_set(&dfet, RETRY_CHECK_INTERVAL);
#endif
#if LOADNG_RREQ_RATELIMIT
  timer_set(&next_time, CLOCK_SECOND/LOADNG_RREQ_RATELIMIT); 
#endif
#if LOADNG_R_HOLD_TIME
  PRINTF("LOADng: Set timer for route validity time check %u\n", RV_CHECK_INTERVAL);
  etimer_set(&rv, RV_CHECK_INTERVAL);
#endif
  while(1) {
    PROCESS_YIELD();
    
    if(ev == tcpip_event) {
      tcpip_handler();
    }

#if LOADNG_IS_COORDINATOR()
    if(etimer_expired(&et)) {
      send_opt();
      uip_ds6_route_print();
      etimer_restart(&et); 
    }
#endif

#if LOADNG_RREQ_RETRIES
    if(etimer_expired(&dfet)) {
      rrc_check_expired_rreq(RETRY_CHECK_INTERVAL);
      etimer_restart(&dfet); 
    }
#endif
#if LOADNG_R_HOLD_TIME
    if(etimer_expired(&rv)) {
      loadng_check_expired_route(RV_CHECK_INTERVAL);
      etimer_restart(&rv); 
    }
#endif

    if(ev == PROCESS_EVENT_MSG) {
        if(command == COMMAND_SEND_RREQ) {
            PRINTF("LOADng: Send RREQ\n");
            
            ctimer_set(&sendmsg_ctimer, random_rand()%50 * CLOCK_SECOND / 1000,
                     (void (*)(void *))send_rreq, NULL);  
	} else if (command == COMMAND_SEND_RERR) {
	  send_rerr(&rerr_src_addr, &rerr_bad_addr, &rerr_next_addr);
	}
	command = COMMAND_NONE;
	
    } 

  
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
