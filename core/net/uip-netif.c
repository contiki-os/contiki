/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         Network interface and stateless autoconfiguration (RFC 4862)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 *
 */

/*
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
 *
 */

#include "net/uip-nd6.h"
#include "net/uip-netif.h"
#include "lib/random.h"

#include <string.h>
#include <stdlib.h>


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(lladdr)
#endif

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif /* UIP_LOGGING == 1 */

/*---------------------------------------------------------------------------*/
/** \brief The single physical interface */
struct uip_netif uip_netif_physical_if;
/** \brief DAD timer */
struct etimer uip_netif_timer_dad;
/** \brief The interface address on which dad is being performed */
static struct uip_netif_addr *dad_ifaddr;
/** \brief Number of ns already sent for DAD*/
static u8_t dad_ns;
/** \brief RS timer, to schedule RS sending */
struct etimer uip_netif_timer_rs;
/** \brief number of rs already sent */
static u8_t rs_count;
/** \brief index for loops */
static u8_t i;
/** \brief timer to check the address states */
struct etimer uip_netif_timer_periodic;


/** \brief remove an interface address, argument type is uip_netif_addr* */
#define uip_netif_addr_rm(addr) do {     \
    PRINTF("Removing address");          \
    PRINT6ADDR(&addr->ipaddr);           \
    PRINTF("\n");                        \
    addr->state = NOT_USED;              \
  } while(0)
/*---------------------------------------------------------------------------*/
void
uip_netif_init(void)
{    
  /* INITIALIZE INTERFACE (default values for now) */
  uip_netif_physical_if.link_mtu = UIP_LINK_MTU;
  uip_netif_physical_if.cur_hop_limit = UIP_TTL;
  uip_netif_physical_if.base_reachable_time = UIP_ND6_REACHABLE_TIME;
  uip_netif_physical_if.reachable_time = uip_netif_compute_reachable_time();
  uip_netif_physical_if.retrans_timer = UIP_ND6_RETRANS_TIMER;
  uip_netif_physical_if.dup_addr_detect_transmit = 1;

  /*
   * STATELESS AUTOCONFIGURATION of the link local address. We set it to 
   * infinite (this will become really true once DAD succeeds)
   */
  uip_ip6addr(&(uip_netif_physical_if.addresses[0].ipaddr),
              0xfe80,0,0,0,0,0,0,0);
  uip_netif_addr_autoconf_set(&(uip_netif_physical_if.addresses[0].ipaddr), &uip_lladdr);
  uip_netif_physical_if.addresses[0].state = TENTATIVE; 
  uip_netif_physical_if.addresses[0].type = MANUAL;
  uip_netif_physical_if.addresses[0].is_infinite = 1;
 
  /* set all other addresses to NOT_USED initialy */
  for(i = 1; i < UIP_CONF_NETIF_MAX_ADDRESSES; i ++) {
    uip_netif_physical_if.addresses[i].state = NOT_USED;
  }
 
  uip_ip6addr_u8(&(uip_netif_physical_if.solicited_node_mcastaddr),
                 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0xff,
                 uip_lladdr.addr[UIP_LLADDR_LEN - 3],
                 uip_lladdr.addr[UIP_LLADDR_LEN - 2],
                 uip_lladdr.addr[UIP_LLADDR_LEN - 1]);
  /* Start DAD */
  uip_netif_sched_dad(&(uip_netif_physical_if.addresses[0]));

  /* Find router (send rs to all-routers multicast group)) */
  uip_netif_sched_send_rs();
   
  /* Reset the timer */
  etimer_set(&uip_netif_timer_periodic, CLOCK_SECOND);
}
/*---------------------------------------------------------------------------*/
void
uip_netif_periodic(void)
{
  for(i = 1; i < UIP_CONF_NETIF_MAX_ADDRESSES; i++) {
    if((uip_netif_physical_if.addresses[i].state != NOT_USED) &&
       (uip_netif_physical_if.addresses[i].is_infinite != 1) &&
       (stimer_expired(&uip_netif_physical_if.addresses[i].vlifetime))) {
      uip_netif_addr_rm((&uip_netif_physical_if.addresses[i]));
    }
  }
  etimer_reset(&uip_netif_timer_periodic);
  return;
}

/*---------------------------------------------------------------------------*/
u32_t
uip_netif_compute_reachable_time(void)
{
  return (u32_t)(UIP_ND6_MIN_RANDOM_FACTOR(uip_netif_physical_if.base_reachable_time)) +
    ((u16_t)(random_rand() << 8) + (u16_t)random_rand()) %
    (u32_t)(UIP_ND6_MAX_RANDOM_FACTOR(uip_netif_physical_if.base_reachable_time) -
            UIP_ND6_MIN_RANDOM_FACTOR(uip_netif_physical_if.base_reachable_time));
}
/*---------------------------------------------------------------------------*/
u8_t
uip_netif_is_addr_my_solicited(uip_ipaddr_t *ipaddr)
{
  if(uip_ipaddr_cmp(ipaddr, &uip_netif_physical_if.solicited_node_mcastaddr)) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
struct uip_netif_addr *
uip_netif_addr_lookup(uip_ipaddr_t *ipaddr, u8_t length, uip_netif_type type)
{
  for(i = 0; i < UIP_CONF_NETIF_MAX_ADDRESSES; i ++) {
    if((uip_netif_physical_if.addresses[i].state != NOT_USED) &&
       (uip_netif_physical_if.addresses[i].type == type || type == 0) &&
       (uip_ipaddr_prefixcmp(&(uip_netif_physical_if.addresses[i].ipaddr), ipaddr, length))) {
      return &uip_netif_physical_if.addresses[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_netif_addr_add(uip_ipaddr_t *ipaddr, u8_t length, unsigned long vlifetime, uip_netif_type type) {
  
  /* check prefix has the right length if we are doing autoconf */
  if((type == AUTOCONF) && (length != UIP_DEFAULT_PREFIX_LEN)) {
    UIP_LOG("Error: UNSUPPORTED PREFIX LENGTH");
    return;
  }
  
  /* check if addr does not already exist and find a free entry */
  for(i = 0; i < UIP_CONF_NETIF_MAX_ADDRESSES; ++i) {
    if(uip_netif_physical_if.addresses[i].state == NOT_USED){
      /*
       * Copying address
       * If we are doing autoconf, ipaddr is a prefix, we copy the 128 bits
       * of it, then overwrite the last 64 bits with the interface ID at 
       * next if statement.
       * Otherwise ipaddr is an address, we just copy it
       */
      uip_ipaddr_copy(&uip_netif_physical_if.addresses[i].ipaddr, ipaddr);
      if(type == AUTOCONF) {
        /* construct address from prefix and layer2 id */
        uip_netif_addr_autoconf_set(&uip_netif_physical_if.addresses[i].ipaddr, &uip_lladdr);
      } 
      /* setting state, type */
      uip_netif_physical_if.addresses[i].state = TENTATIVE;
      uip_netif_physical_if.addresses[i].type = type;
      /* setting lifetime timer if lieftime is not infinite */
      if(vlifetime != 0) {
        stimer_set(&(uip_netif_physical_if.addresses[i].vlifetime), vlifetime);
        uip_netif_physical_if.addresses[i].is_infinite = 0;
      } else {
        uip_netif_physical_if.addresses[i].is_infinite = 1;
      }
      PRINTF("Created new address");
      PRINT6ADDR(&uip_netif_physical_if.addresses[i].ipaddr);
      PRINTF("for interface\n");
   
      /* schedule DAD */
      uip_netif_sched_dad(&uip_netif_physical_if.addresses[i]);
      
      return;
    }
  }
 
  /* If we did not find space, log */ 
  UIP_LOG("ADDRESS LIST FULL");
  return;
}
/*---------------------------------------------------------------------------*/
void
uip_netif_addr_autoconf_set(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  /* We consider only links with IEEE EUI-64 identifier or
     IEEE 48-bit MAC addresses */
#if (UIP_LLADDR_LEN == 8)
  memcpy(ipaddr->u8 + 8, lladdr, UIP_LLADDR_LEN);
  ipaddr->u8[8] ^= 0x02;  
#elif (UIP_LLADDR_LEN == 6)
  memcpy(ipaddr->u8 + 8, lladdr, 3);
  ipaddr->u8[11] = 0xff;
  ipaddr->u8[12] = 0xfe;
  memcpy(ipaddr->u8 + 13, (u8_t*)lladdr + 3, 3);
  ipaddr->u8[8] ^= 0x02;
#else
  UIP_LOG("CAN NOT BUILD INTERFACE IDENTIFIER");
  UIP_LOG("THE STACK IS GOING TO SHUT DOWN");
  UIP_LOG("THE HOST WILL BE UNREACHABLE");
#endif
}
/*---------------------------------------------------------------------------*/
u8_t
get_match_length(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{
  u8_t j, k, x_or;
  u8_t len = 0;
  for(j = 0; j < 16; j ++) {
    if(src->u8[j] == dst->u8[j]) {
      len += 8;
    } else {
      x_or = src->u8[j] ^ dst->u8[j];
      for(k = 0; k < 8; k ++) {
        if((x_or & 0x80) == 0){
          len ++;
          x_or <<= 1;
        }
        else {
          break;
        }
      } 
      break;
    }
  }
  return len;
}
/*---------------------------------------------------------------------------*/
void
uip_netif_select_src(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{   
  u8_t best = 0; /* number of bit in common with best match*/
  u8_t n = 0;
  u8_t index = 0;
  
  if(!uip_is_addr_link_local(dst) && !uip_is_addr_mcast(dst)) {
    for(i = 1; i < UIP_CONF_NETIF_MAX_ADDRESSES; ++i) {
      if(uip_netif_physical_if.addresses[i].state == PREFERRED){
        n = get_match_length(dst, &(uip_netif_physical_if.addresses[i].ipaddr));
        if(n >= best){
          best = n;
          index = i;
        }
      }
    }
  }

  uip_ipaddr_copy(src, &(uip_netif_physical_if.addresses[index].ipaddr));
  return;
}
/*---------------------------------------------------------------------------*/
void
uip_netif_sched_dad(struct uip_netif_addr *ifaddr)
{    
  if(ifaddr->state != TENTATIVE){
    UIP_LOG("DAD called with non tentative address");
    return;
  }
  /*
   * check dad is not running
   */
  if(dad_ifaddr != NULL){
    UIP_LOG("DAD already running");
    return;
  }
  /*
   * Set the interface address that is going through DAD
   */
  dad_ifaddr = ifaddr;

  PRINTF("Scheduling DAD for ipaddr:");
  PRINT6ADDR(&dad_ifaddr->ipaddr);
  PRINTF("\n");

  etimer_set(&uip_netif_timer_dad, random_rand()%(UIP_ND6_MAX_RTR_SOLICITATION_DELAY * CLOCK_SECOND)); 
}
/*---------------------------------------------------------------------------*/
void
uip_netif_dad(void)
{
  /*
   * check if dad is running
   */
  if(dad_ifaddr == NULL){
    PRINTF("uip_netif_dad: DAD not running\n");
    return;
  }
  /*
   * send dup_addr_detect_transmit NS for DAD
   */
  if(dad_ns < uip_netif_physical_if.dup_addr_detect_transmit) {
    uip_nd6_io_ns_output(NULL, NULL, &dad_ifaddr->ipaddr);
    dad_ns++;
    etimer_set(&uip_netif_timer_dad, uip_netif_physical_if.retrans_timer / 1000 * CLOCK_SECOND);
    return;
  }
  /*
   * If we arrive here it means DAD succeeded, otherwise the dad process
   * would have been interrupted in nd6_dad_ns/na_input
   */
  PRINTF("DAD succeeded for ipaddr:");
  PRINT6ADDR(&(dad_ifaddr->ipaddr));
  PRINTF("\n");

  etimer_stop(&uip_netif_timer_dad);
  dad_ifaddr->state = PREFERRED;
  dad_ifaddr = NULL;
  dad_ns = 0;
  /*
   * check if we need to run DAD on another address
   * This is necessary because if you receive a RA,
   * you might want to run DAD for several addresses
   * Considering that we have structures to do DAD 
   * for one address only, we start DAD for the subsequent
   * addresses here
   */
  PRINTF("END of DAD\n");
  for(i = 0; i < UIP_CONF_NETIF_MAX_ADDRESSES; i ++){
    if(uip_netif_physical_if.addresses[i].state != NOT_USED){
      PRINTF("address %u : ",i);
      PRINT6ADDR(&(uip_netif_physical_if.addresses[i].ipaddr));
      PRINTF("\n");
    }
    if(uip_netif_physical_if.addresses[i].state == TENTATIVE){
      uip_netif_sched_dad(&uip_netif_physical_if.addresses[i]);
      return;   
    }
  }
}
/*---------------------------------------------------------------------------*/
void
uip_netif_dad_failed(uip_ipaddr_t *ipaddr)
{
  UIP_LOG("DAD FAILED");
  UIP_LOG("THE STACK IS GOING TO SHUT DOWN");
  UIP_LOG("THE HOST WILL BE UNREACHABLE");
  
  if(uip_ipaddr_cmp(&dad_ifaddr->ipaddr, ipaddr)){
    etimer_stop(&uip_netif_timer_dad);
    dad_ifaddr->state = NOT_USED;
    dad_ifaddr = NULL;
    dad_ns = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
uip_netif_sched_send_rs(void)
{
  /* before a host sends an initial solicitation, it SHOULD delay the
     transmission for a random amount of time between 0 and
     UIP_ND6_MAX_RTR_SOLICITATION_DELAY. */
  if(rs_count == 0){
    etimer_set(&uip_netif_timer_rs, random_rand()%(UIP_ND6_MAX_RTR_SOLICITATION_DELAY * CLOCK_SECOND));
    PRINTF("Scheduling RS\n");  
  }
}
/*---------------------------------------------------------------------------*/
void
uip_netif_send_rs(void)
{
  if((uip_nd6_choose_defrouter() == NULL) && (rs_count < UIP_ND6_MAX_RTR_SOLICITATIONS)){
    PRINTF("Sending RS %u\n", rs_count);
    uip_nd6_io_rs_output();
    rs_count++;
    etimer_set(&uip_netif_timer_rs, UIP_ND6_RTR_SOLICITATION_INTERVAL * CLOCK_SECOND);     
  } else {
    PRINTF("Router found ? (boolean): %u\n", (uip_nd6_choose_defrouter() != NULL));
    etimer_stop(&uip_netif_timer_rs);
    rs_count = 0;
  }
}

/** @} */
