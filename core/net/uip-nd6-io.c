/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         Neighbor discovery Input-Output (RFC 4861)
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 */
/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
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
 */

#include <string.h>
#include "net/uip-icmp6.h"
#include "net/uip-nd6.h"
#include "net/uip-netif.h"



/*------------------------------------------------------------------*/
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


/*------------------------------------------------------------------*/
/** @{ */
/** \name Pointers to the header structures.
 *  All pointers except UIP_IP_BUF depend on uip_ext_len, which at
 *  packet reception, is the total length of the extension headers.
 *  
 *  The pointer to ND6 options header also depends on nd6_opt_offset,
 *  which we set in each function.
 *
 *  Care should be taken when manipulating these buffers about the
 *  value of these length variables
 */

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
/**< Pointer to IP header */
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
/**< Pointer to ICMP header*/
/**@{  Pointers to messages just after icmp header */
#define UIP_ND6_RS_BUF            ((struct uip_nd6_rs *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_RA_BUF            ((struct uip_nd6_ra *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NS_BUF            ((struct uip_nd6_ns *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NA_BUF            ((struct uip_nd6_na *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/** @} */
/** Pointer to ND option */
#define UIP_ND6_OPT_HDR_BUF  ((struct uip_nd6_opt_hdr *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
/** @} */

/** \brief Offset from the end of the icmpv6 header to the option in uip_buf*/
static u8_t nd6_opt_offset;
/** \brief Pointer to llao option in uip_buf */
static struct uip_nd6_opt_llao *nd6_opt_llao;
/** \brief Pointer to mtu option in uip_buf */
static struct uip_nd6_opt_mtu *nd6_opt_mtu;
/** \brief Pointer to prefix information option in uip_buf */
static struct uip_nd6_opt_prefix_info *nd6_opt_prefix_info[UIP_CONF_ND6_MAX_PREFIXES];
/** \brief Pointer to a neighbor cache entry*/
static struct uip_nd6_neighbor *neighbor;
/** \brief Pointer to a prefix list entry */
static struct uip_nd6_prefix *prefix;
/** \brief Pointer to a router list entry */
static struct uip_nd6_defrouter *router;
/** \brief Pointer to an interface address */
static struct uip_netif_addr *ifaddr;
/** \brief Index used in loops */
static u8_t i;
  
/*------------------------------------------------------------------*/
void 
uip_nd6_io_ns_input(void)
{
  u8_t flags;
  
  PRINTF("Received NS from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("with target address");
  PRINT6ADDR((uip_ipaddr_t *)(&UIP_ND6_NS_BUF->tgtipaddr));
  PRINTF("\n");
  UIP_STAT(++uip_stat.nd6.recv);
 
 
#if UIP_CONF_IPV6_CHECKS
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
     (uip_is_addr_mcast(&UIP_ND6_NS_BUF->tgtipaddr)) ||
     (UIP_ICMP_BUF->icode != 0)) {
    goto badpkt;
  }
#endif /* UIP_CONF_IPV6_CHECKS */ 
  
  /* Options reading: we handle only SLLAO for now */
  nd6_opt_llao = NULL;
  nd6_opt_offset = UIP_ND6_NS_LEN;
  while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len) {
#if UIP_CONF_IPV6_CHECKS
    if(UIP_ND6_OPT_HDR_BUF->len == 0) {
      goto badpkt;
    }
#endif /* UIP_CONF_IPV6_CHECKS */ 
    switch(UIP_ND6_OPT_HDR_BUF->type) {
      case UIP_ND6_OPT_SLLAO:
        nd6_opt_llao = (struct uip_nd6_opt_llao *)UIP_ND6_OPT_HDR_BUF;
        break;
      default:
        UIP_LOG("ND option not supported in NS");
        break;
    }
    nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
  }

  /* Options processing: only SLLAO */
  if(nd6_opt_llao != NULL) {
#if UIP_CONF_IPV6_CHECKS
    /* There must be NO option in a DAD NS */
    if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
      goto badpkt;
    } else {
#endif /*UIP_CONF_IPV6_CHECKS*/
      neighbor = uip_nd6_nbrcache_lookup(&UIP_IP_BUF->srcipaddr);
      if(neighbor == NULL) {
        /* we need to add the neighbor*/
        uip_nd6_nbrcache_add(&UIP_IP_BUF->srcipaddr,
                             &nd6_opt_llao->addr, 0, STALE);
      } else {
        /* If LL address changed, set neighbor state to stale */
        if(memcmp(&nd6_opt_llao->addr, &neighbor->lladdr, UIP_LLADDR_LEN) != 0) {
          memcpy(&neighbor->lladdr, &nd6_opt_llao->addr, UIP_LLADDR_LEN);
          neighbor->state = STALE;
        } else {
          /* If neighbor state is INCOMPLETE, set to STALE */
          if(neighbor->state == INCOMPLETE) {
            neighbor->state = STALE;
          }
        }
      }
#if UIP_CONF_IPV6_CHECKS
    }   
#endif /*UIP_CONF_IPV6_CHECKS*/
  }

  /* 
   * Rest of NS processing: Depends on the purpose of the NS: NUD or DAD or
   * Address Resolution 
   */
  /** \note we use ifaddr to remember the target address */
  ifaddr = uip_netif_addr_lookup(&UIP_ND6_NS_BUF->tgtipaddr, 128, 0);
  if(ifaddr != NULL) {
    if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)){
      /* DAD CASE */
#if UIP_CONF_IPV6_CHECKS 
      /* Dst address must be solicited node mcast address */
      if(!uip_netif_is_addr_my_solicited(&UIP_IP_BUF->destipaddr)){
        goto badpkt;
      }
#endif /* UIP_CONF_IPV6_CHECKS */ 
      /*
       * If my address is not tentative, then send a NA to all nodes with
       * TLLAO flags are: override = yes.
       */
        if(ifaddr->state!=TENTATIVE) {  
        /* 
         * we need to send a NA, we set the src, dest, flags. tgt remains the
         * same and the rest is done at "create_na" 
         */
        uip_create_linklocal_allnodes_mcast(&UIP_IP_BUF->destipaddr);
        uip_netif_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
        flags = UIP_ND6_NA_FLAG_OVERRIDE;
        goto create_na;
      } else {
        /** \todo if I sent a NS before him, I win */
        uip_netif_dad_failed(&UIP_ND6_NS_BUF->tgtipaddr);
        goto discard;
      }
    }
  

#if UIP_CONF_IPV6_CHECKS
    /* Duplicate check */
    if(uip_netif_is_addr_my_unicast(&UIP_IP_BUF->srcipaddr)) {
      /**
       * \NOTE do we do something here? we both are using the same address.
       * If we are doing dad, we could cancel it, though we should receive a
       * NA in response of DAD NS we sent, hence DAD will fail anyway. If we
       * were not doing DAD, it means there is a duplicate in the network!
       */
      goto badpkt;
    }
#endif /*UIP_CONF_IPV6_CHECKS*/

    /* Address resolution case */  
    if(uip_netif_is_addr_my_solicited(&UIP_IP_BUF->destipaddr)){
      /*   
       * we need to send a NA, we set the src, dest, flags. The rest is
       * set at the "create_na" label.
       */
      uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
      uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_ND6_NS_BUF->tgtipaddr);
      flags = UIP_ND6_NA_FLAG_SOLICITED | UIP_ND6_NA_FLAG_OVERRIDE;
      goto create_na;
    } 

    /* 
     * NUD CASE. at this point the packet must be for us! we check this, 
     * and at the same time if target == dest
     */
    if(uip_netif_addr_lookup(&UIP_IP_BUF->destipaddr, 128, 0) == ifaddr){
      /*   
       * we need to send a NA, we set the src, dest, flags. The rest is set
       * at the "create_na" label.
       */
      uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
      uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_ND6_NS_BUF->tgtipaddr);
      flags = UIP_ND6_NA_FLAG_SOLICITED | UIP_ND6_NA_FLAG_OVERRIDE;
      goto create_na;
    } else {
#if UIP_CONF_IPV6_CHECKS
      goto badpkt;
#endif /* UIP_CONF_IPV6_CHECKS */ 
    }    
  } else {
    goto discard;
  }
  

 create_na:
  /* 
   * Fill the part of the NA which is common to all NAs. If the NS contained
   * extension headers, we must set the target address properly
   */
  uip_ext_len = 0; 
  
  /* IP header */
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->len[0] = 0; /* length will not be more than 255 */
  UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

  /* ICMP header */
  UIP_ICMP_BUF->type = ICMP6_NA;
  UIP_ICMP_BUF->icode = 0;

  /* NA static part */
  UIP_ND6_NA_BUF->flagsreserved = flags;
  memcpy(&UIP_ND6_NA_BUF->tgtipaddr, &ifaddr->ipaddr, sizeof(uip_ipaddr_t));

  /* NA option: TLLAO. note that length field is in unit of 8 bytes */
  uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN;
  nd6_opt_llao = (struct uip_nd6_opt_llao *)&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NA_LEN];
  nd6_opt_llao->type = UIP_ND6_OPT_TLLAO;
  nd6_opt_llao->len = UIP_ND6_OPT_LLAO_LEN >> 3;
  memcpy(&(nd6_opt_llao->addr), &uip_lladdr, UIP_LLADDR_LEN);
  /* padding if needed */
  memset((void *)(&nd6_opt_llao->addr) + UIP_LLADDR_LEN, 0, UIP_ND6_OPT_LLAO_LEN - 2 - UIP_LLADDR_LEN);

  /*ICMP checksum*/
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum(); 


  UIP_STAT(++uip_stat.nd6.sent);
  PRINTF("Sending NA to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("with target address");
  PRINT6ADDR(&UIP_ND6_NA_BUF->tgtipaddr);
  PRINTF("\n");
  return;

#if UIP_CONF_IPV6_CHECKS
 badpkt:
  UIP_STAT(++uip_stat.nd6.drop);
  UIP_LOG("NS received is bad"); 
#endif /* UIP_CONF_IPV6_CHECKS */
  
 discard:
  uip_len = 0;
  return;
}
/*------------------------------------------------------------------*/
void
uip_nd6_io_ns_output(uip_ipaddr_t *src, uip_ipaddr_t *dest, uip_ipaddr_t *tgt)
{
  /* IP header fields */
  uip_ext_len = 0;
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

  /*
   * The caller must put a valid tgt address. 
   * For dest, if the caller is doing DAD or Address resolution, he sets
   * dest to NULL and we forge dest as the sollicited node mcast address
   * for tgt.
   * If the caller is sending the NS for NUD, dest is unicast and the caller
   * specifies it in the arguments
   */
  if(dest == NULL) {
    uip_create_solicited_node(tgt, &UIP_IP_BUF->destipaddr);
  } else {
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
  }
  UIP_ICMP_BUF->type = ICMP6_NS;
  UIP_ICMP_BUF->icode = 0;
  UIP_ND6_NS_BUF->reserved = 0;

  /* Copy the tgt address */
  uip_ipaddr_copy((uip_ipaddr_t *)&UIP_ND6_NS_BUF->tgtipaddr, tgt);
  
  UIP_IP_BUF->len[0] = 0; /* length will not be more than 255 */
  /*
   * check if we add a SLLAO option: for DAD, MUST NOT, for NUD, MAY
   * (here yes), for Address resolution , MUST 
   * i.e. if and only if tgt is our address (in this case it is DAD), we do
   * not add it
   */ 
  if(!(uip_netif_is_addr_my_unicast(tgt))) {
    if(src != NULL) {
      uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, src);
    } else {
      uip_netif_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    }
    UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN;
   
    uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN;
    nd6_opt_llao = (struct uip_nd6_opt_llao *)&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NS_LEN];
    nd6_opt_llao->type = UIP_ND6_OPT_SLLAO; /* type of the option */
    /* 
     * length of the option: 2 bytes for type, length, plus the length of
     * the L2 address. It must be in units of 8 bytes 
     */
    nd6_opt_llao->len = UIP_ND6_OPT_LLAO_LEN >> 3;
    memcpy(&nd6_opt_llao->addr, &uip_lladdr, UIP_LLADDR_LEN);
    /* padding if needed */
    memset((void *)(&nd6_opt_llao->addr) + UIP_LLADDR_LEN, 0, UIP_ND6_OPT_LLAO_LEN - 2 - UIP_LLADDR_LEN);
  }
  else {
    uip_create_unspecified(&UIP_IP_BUF->srcipaddr);
    UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_NS_LEN;
    uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN;
  }

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum(); 

  UIP_STAT(++uip_stat.nd6.sent);
  PRINTF("Sending NS to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("with target address");
  PRINT6ADDR(tgt);
  PRINTF("\n");
  return;
}



/*------------------------------------------------------------------*/
void
uip_nd6_io_na_input(void)
{

  /* 
   * booleans. the three last one are not 0 or 1 but 0 or 0x80, 0x40, 0x20
   * but it works. Be careful though, do not use tests such as is_router == 1 
   */
  u8_t is_llchange = 0;
  u8_t is_router = ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_ROUTER));
  u8_t is_solicited = ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_SOLICITED));
  u8_t is_override =  ((UIP_ND6_NA_BUF->flagsreserved & UIP_ND6_NA_FLAG_OVERRIDE));

  PRINTF("Received NA from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("with target address");
  PRINT6ADDR((uip_ipaddr_t *)(&UIP_ND6_NA_BUF->tgtipaddr));
  PRINTF("\n");
  UIP_STAT(++uip_stat.nd6.recv);

#if UIP_CONF_IPV6_CHECKS
  /*
   * Check hop limit / icmp code 
   * target address must not be multicast
   * if the NA is solicited, dest must not be multicast
   */
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
     (UIP_ICMP_BUF->icode != 0) ||
     (uip_is_addr_mcast(&UIP_ND6_NA_BUF->tgtipaddr)) ||
     (is_solicited && uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))) {
    goto badpkt;
  }
#endif /*UIP_CONF_IPV6_CHECKS*/

  /* Options reading: we can handle TLLAO, and must ignore othehrs */
  nd6_opt_offset = UIP_ND6_NA_LEN;
  nd6_opt_llao = NULL;
  while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len) {
#if UIP_CONF_IPV6_CHECKS
    if(UIP_ND6_OPT_HDR_BUF->len == 0) {
      goto badpkt;
    }
#endif /*UIP_CONF_IPV6_CHECKS*/
    switch(UIP_ND6_OPT_HDR_BUF->type) {
      case UIP_ND6_OPT_TLLAO:
        nd6_opt_llao = (struct uip_nd6_opt_llao *)UIP_ND6_OPT_HDR_BUF;
        break;
      default:
        UIP_LOG("ND option not supported in NA");
        break;
    }
    nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
  }
  ifaddr = uip_netif_addr_lookup(&UIP_ND6_NA_BUF->tgtipaddr, 128, 0);
  /* Message processing, including TLLAO if any */
  if(ifaddr != NULL) {
    if(ifaddr->state == TENTATIVE) {
      /*It means DAD failed*/
      uip_netif_dad_failed(&UIP_ND6_NA_BUF->tgtipaddr);
    }
    goto discard;
  } else {
    neighbor = uip_nd6_nbrcache_lookup(&UIP_ND6_NA_BUF->tgtipaddr);
    /* if the neighbor entry does not exist, silently discard */
    if(neighbor == NULL) {
      goto discard;
    }
    if(nd6_opt_llao != 0) {
      is_llchange = memcmp((void *)&nd6_opt_llao->addr, (void *)(&neighbor->lladdr), UIP_LLADDR_LEN);
    }
    if(neighbor->state == INCOMPLETE){
      if(nd6_opt_llao == 0) {
        goto discard;
      }
      memcpy(&neighbor->lladdr, &nd6_opt_llao->addr, UIP_LLADDR_LEN);
      if(is_solicited) {
        neighbor->state = REACHABLE;
        /* reachable time is stored in ms*/
        stimer_set(&(neighbor->reachable),
                  uip_netif_physical_if.reachable_time / 1000);
     
      } else {
        neighbor->state = STALE;
      }
      neighbor->isrouter = is_router;
    } else {
      if(!is_override && is_llchange) {
        if(neighbor->state == REACHABLE) {
          neighbor->state = STALE;
        }
        goto discard;
      } else {
        if(is_override 
           || (!is_override && nd6_opt_llao != 0 && !is_llchange)
           || nd6_opt_llao == 0) {
       
          /* update LL address if any */
          if(nd6_opt_llao != 0) {
            memcpy(&neighbor->lladdr, &nd6_opt_llao->addr, UIP_LLADDR_LEN);
          }
       
          /* 
           * if the NA was solicited, change the state of the neighbor to 
           * reachabe. Otherwise and if the neighbor LL changed, set the 
           * state to  STALE 
           */ 
          if(is_solicited) {
            neighbor->state = REACHABLE;
            /* reachable time is stored in ms*/
            stimer_set(&(neighbor->reachable),
                      uip_netif_physical_if.reachable_time / 1000);
      
          } else {
            if(nd6_opt_llao != 0 && is_llchange) {
              neighbor->state = STALE;
            }
          }
        }
      }
      if(neighbor->isrouter && !is_router){
        /* the neighbor stopped being a router, we remove it from the list */
        router = uip_nd6_defrouter_lookup(neighbor);
        if(router != NULL){
          uip_nd6_defrouter_rm(router);
        }
      }    
      neighbor->isrouter = is_router;
    }
  }
 
#if UIP_CONF_IPV6_QUEUE_PKT
  /* The neighbor is now reachable, check if we had buffered a pkt for it */
  if(neighbor->queue_buf_len != 0) {
    uip_len = neighbor->queue_buf_len;     
    memcpy(UIP_IP_BUF, neighbor->queue_buf, uip_len);
    neighbor->queue_buf_len = 0;
    return;
  }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
  uip_len = 0;
  return; 
  
#if UIP_CONF_IPV6_CHECKS
 badpkt:
  UIP_STAT(++uip_stat.nd6.drop);
  UIP_LOG("NA received is bad"); 
#endif /* UIP_CONF_IPV6_CHECKS */

 discard:
  uip_len = 0;
  return;
}



/*---------------------------------------------------------------------------*/
void
uip_nd6_io_rs_output(void) {
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;
  uip_create_linklocal_allrouters_mcast(&UIP_IP_BUF->destipaddr);
  uip_netif_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  UIP_ICMP_BUF->type = ICMP6_RS;
  UIP_ICMP_BUF->icode = 0;
  UIP_ND6_RS_BUF->reserved = 0;
  
  UIP_IP_BUF->len[0] = 0; /* length will not be more than 255 */
  
  /* we add the SLLAO option only if src is not unspecified */ 
  if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
    UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_RS_LEN;
    uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN;
  } else {
    uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;
    UIP_IP_BUF->len[1] = UIP_ICMPH_LEN + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;
    nd6_opt_llao = (struct uip_nd6_opt_llao *)&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_RS_LEN];
    nd6_opt_llao->type = UIP_ND6_OPT_SLLAO; /* type of the option */
    nd6_opt_llao->len = UIP_ND6_OPT_LLAO_LEN >> 3;
    /* length of the option in units of 8 bytes */
    memcpy(&nd6_opt_llao->addr, &uip_lladdr, UIP_LLADDR_LEN);
    /* padding if needed */
    memset((void *)(&nd6_opt_llao->addr) + UIP_LLADDR_LEN, 0, UIP_ND6_OPT_LLAO_LEN - 2 - UIP_LLADDR_LEN);
  }
    

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum(); 

  UIP_STAT(++uip_stat.nd6.sent);

  PRINTF("Sendin RS to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  
  return;
}



/*---------------------------------------------------------------------------*/
void
uip_nd6_io_ra_input(void) {
  PRINTF("Received RA from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("to");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");
  UIP_STAT(++uip_stat.nd6.recv);
 
   
#if UIP_CONF_IPV6_CHECKS
  /* a few validity checks */
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
     (!uip_is_addr_link_local(&UIP_IP_BUF->srcipaddr)) ||
     (UIP_ICMP_BUF->icode != 0)) {
    goto badpkt;
  }
#endif /*UIP_CONF_IPV6_CHECKS*/

  /*
   * process the fields of the constant part 
   * As per RFC 4861, for reachable time and retrans timer, a value of 0 means 
   * unspecified by this router.
   * If router lifetime is 0, the router SHOULD NOT be placed in default
   * router list
   */
  if(UIP_ND6_RA_BUF->cur_ttl != 0) {
    uip_netif_physical_if.cur_hop_limit = UIP_ND6_RA_BUF->cur_ttl;
    PRINTF("uip_netif_physical_if.cur_hop_limit %u\n", uip_netif_physical_if.cur_hop_limit);
  }
  /* 
   * As per RFC4861 section 6.3.4, if reachable time field is non zero
   * and defers from the current base reachable time for the interface,
   * set base reachable time to the new value and recompute the reachable time 
   * as a random value distributed between 0.5 and 1.5 x Base reachable time
   */
  if(UIP_ND6_RA_BUF->reachable_time != 0) {
    if(uip_netif_physical_if.base_reachable_time != ntohl(UIP_ND6_RA_BUF->reachable_time)) {
      uip_netif_physical_if.base_reachable_time = ntohl(UIP_ND6_RA_BUF->reachable_time);
      uip_netif_physical_if.reachable_time = uip_netif_compute_reachable_time();
    }
  }
  if(UIP_ND6_RA_BUF->retrans_timer !=0) {
    uip_netif_physical_if.retrans_timer = ntohl(UIP_ND6_RA_BUF->retrans_timer);
  }
  
  /*
   * Note: in our implementation, an entry in the default router list contains
   * does not contain the IP address of the router. The entry has a pointer
   * to the corresponding neighbor cache entry, where the IP address is stored
   */
  /* Options reading: possible options are MTU, SLLAO, prefix */
  nd6_opt_llao = NULL;
  nd6_opt_mtu = NULL;
  for(i = 0; i < UIP_CONF_ND6_MAX_PREFIXES; i++) { 
    nd6_opt_prefix_info[i] = NULL;
  }
  nd6_opt_offset = UIP_ND6_RA_LEN;
  while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len) {
    if(UIP_ND6_OPT_HDR_BUF->len == 0) {
      goto badpkt;
    }
    switch(UIP_ND6_OPT_HDR_BUF->type) {
      case UIP_ND6_OPT_SLLAO:
        nd6_opt_llao = (struct uip_nd6_opt_llao *)UIP_ND6_OPT_HDR_BUF;
        break;
      case UIP_ND6_OPT_MTU:
        nd6_opt_mtu = (struct uip_nd6_opt_mtu *)UIP_ND6_OPT_HDR_BUF;
        break;
      case UIP_ND6_OPT_PREFIX_INFO:
        i = 0;
        while((i < UIP_CONF_ND6_MAX_PREFIXES) && (nd6_opt_prefix_info[i] != NULL)) {
          i++;
        }
        if(i < UIP_CONF_ND6_MAX_PREFIXES) {
          nd6_opt_prefix_info[i] = (struct uip_nd6_opt_prefix_info *)UIP_ND6_OPT_HDR_BUF; 
        } 
        break;
      default:
        UIP_LOG("ND option not supported in RA");
        break;
    }  
    nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
  }

  /* Process the MTU option if any */
  if(nd6_opt_mtu != NULL) {
    /* just set the link MTU to the value specified. Note that this option */
    PRINTF("Processing MTU option in RA\n");
    uip_netif_physical_if.link_mtu = ntohl(nd6_opt_mtu->mtu);  
  } 

  /* Prefix information options processing */
  i = 0;
  while((i < UIP_CONF_ND6_MAX_PREFIXES - 1) && (nd6_opt_prefix_info[i] != NULL)) {
    PRINTF("Processing PREFIX option in RA\n");
        
    if((ntohl((nd6_opt_prefix_info[i])->validlt) >=
        ntohl((nd6_opt_prefix_info[i])->preferredlt)) && 
       (!uip_is_addr_link_local(&nd6_opt_prefix_info[i]->prefix))) {
      /* on-link flag related processing*/  
      if(((nd6_opt_prefix_info[i])->flagsreserved1 & 0x80) == 0x80) {
        prefix = uip_nd6_prefix_lookup(&(nd6_opt_prefix_info[i])->prefix);
        if(prefix == NULL) {
          if((nd6_opt_prefix_info[i])->validlt != 0){
            if((nd6_opt_prefix_info[i])->validlt != UIP_ND6_INFINITE_LIFETIME){ 
              prefix = uip_nd6_prefix_add(&(nd6_opt_prefix_info[i])->prefix,
                                          (nd6_opt_prefix_info[i])->preflen, 
                                          ntohl((nd6_opt_prefix_info[i])->validlt));
            } else {
              prefix = uip_nd6_prefix_add(&(nd6_opt_prefix_info[i])->prefix,
                                          (nd6_opt_prefix_info[i])->preflen, 
                                          0);
            }
          }
        } else {
          /* we update or remove an existing prefix */
          switch((nd6_opt_prefix_info[i])->validlt) {
            case 0: 
              uip_nd6_prefix_rm(prefix);
              break;
            case UIP_ND6_INFINITE_LIFETIME:
              prefix->is_infinite = 1;
              break;
            default:
              PRINTF("Updating timer of prefix");
              PRINT6ADDR(prefix);
              PRINTF("new value %lu\n", ntohl((nd6_opt_prefix_info[i])->validlt));
              stimer_set(&prefix->vlifetime, ntohl((nd6_opt_prefix_info[i])->validlt));
              /*in case the prefix lifetime was previously infinite */
              prefix->is_infinite = 0;
              break;              
          }
        }
      }
      /* End of on-link flag related processing */
      /* autonomous flag related processing*/  
      if((((nd6_opt_prefix_info[i])->flagsreserved1 & 0x40) == 0x40) &&
         ((nd6_opt_prefix_info[i])->validlt != 0)){
        ifaddr = uip_netif_addr_lookup(&(nd6_opt_prefix_info[i])->prefix,
                                       (nd6_opt_prefix_info[i])->preflen,
                                       AUTOCONF);
        if(ifaddr != NULL) {
          if((nd6_opt_prefix_info[i])->validlt != UIP_ND6_INFINITE_LIFETIME) {
            /* The processing below is defined in RFC4862 section 5.5.3 e */
            if((ntohl((nd6_opt_prefix_info[i])->validlt) > 2 * 60 * 60) ||
               (ntohl((nd6_opt_prefix_info[i])->validlt)> stimer_remaining(&ifaddr->vlifetime))) {
              PRINTF("Updating timer of address");
              PRINT6ADDR(&ifaddr->ipaddr);
              PRINTF("new value %lu\n", ntohl((nd6_opt_prefix_info[i])->validlt));
              stimer_set(&ifaddr->vlifetime, ntohl((nd6_opt_prefix_info[i])->validlt));
            } else {
              stimer_set(&ifaddr->vlifetime, 2 * 60 * 60);
          
              PRINTF("Updating timer of address ");
              PRINT6ADDR(&ifaddr->ipaddr);
              PRINTF("new value %lu\n", (unsigned long)(2 * 60 * 60));
            }
            /*in case the address lifetime was previously infinite */
            ifaddr->is_infinite = 0;
          } else {
            ifaddr->is_infinite = 1;
          }
        } else {
          /* Autoconfigure an address*/
          if((nd6_opt_prefix_info[i])->validlt != UIP_ND6_INFINITE_LIFETIME) {
            /* Add an address with FINITE lifetime */
            uip_netif_addr_add(&(nd6_opt_prefix_info[i])->prefix,
                               (nd6_opt_prefix_info[i])->preflen, 
                               ntohl((nd6_opt_prefix_info[i])->validlt),
                               AUTOCONF);
          } else {
            /* Add an address with INFINITE lifetime */
            uip_netif_addr_add(&(nd6_opt_prefix_info[i])->prefix,
                               (nd6_opt_prefix_info[i])->preflen, 
                               0,
                               AUTOCONF);
          }
        }
      } 
      /* End of autonomous flag related processing */
    }
    i++;
  }
  /* End of Prefix Information Option processing*/   

  
  /* SLLAO option processing */
  neighbor = uip_nd6_nbrcache_lookup(&UIP_IP_BUF->srcipaddr);
  if(nd6_opt_llao != NULL) {
    PRINTF("Processing SLLAO option in RA\n");
    if(neighbor == NULL) {
      neighbor = uip_nd6_nbrcache_add(&UIP_IP_BUF->srcipaddr,
                                      &nd6_opt_llao->addr, 1, STALE);
    } else {
      if(neighbor->state == INCOMPLETE) {
        neighbor->state = STALE;
      }
      /* check if there was an address change. it applies to the case 
       * where if the router ND entry was in state NO_STATE */
      if(memcmp(&nd6_opt_llao->addr, &neighbor->lladdr, UIP_LLADDR_LEN) != 0) {
        memcpy(&neighbor->lladdr, &nd6_opt_llao->addr, UIP_LLADDR_LEN);
        neighbor->state = STALE;
      } 
      /*in case the neighbor was not a router before */
      neighbor->isrouter = 1;
    }
  }


  /*
   * As per RFC4861, section 6.3.4, and considering that in our
   * implementation, a router entry contains a pointer to an existing
   * neighbor entry:
   * if router lifetime != 0, if entry present in def router list
   * (and therefore neighbor cache), update timer, and update neighbor entry
   * if needed
   * else add entry to both router list (and neighbor cache if absent)
   * else delete entry in router list if it exists
   */
  if(UIP_ND6_RA_BUF->router_lifetime != 0) {
    /* Add router or update timer, we need a neighbor entry for this */
    if(neighbor == NULL) {
      neighbor = uip_nd6_nbrcache_add(&UIP_IP_BUF->srcipaddr, NULL, 1, NO_STATE);
    } else {
      /* in case the neighbor was not a router before */
      neighbor->isrouter = 1;
    }
    if((router = uip_nd6_defrouter_lookup(neighbor)) == NULL) {
      uip_nd6_defrouter_add(neighbor, (unsigned long)(ntohs(UIP_ND6_RA_BUF->router_lifetime)));
    } else {
      stimer_set(&(router->lifetime), (unsigned long)(ntohs(UIP_ND6_RA_BUF->router_lifetime)));
    }
  } else {
    /* delete router entry*/
    if(neighbor != NULL) {
      router = uip_nd6_defrouter_lookup(neighbor);
      if(router != NULL) {
        uip_nd6_defrouter_rm(router);
      }
    }
  }

#if UIP_CONF_IPV6_QUEUE_PKT
  /*
   * If the neighbor just became reachable (e.g. it was in INCOMPLETE state
   * and we got a SLLAO), check if we had buffered a pkt for it 
   */
  if((neighbor != NULL) && (neighbor->queue_buf_len != 0)) {
    uip_len = neighbor->queue_buf_len;     
    memcpy(UIP_IP_BUF, neighbor->queue_buf, uip_len);
    neighbor->queue_buf_len = 0;     
    return;
  }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
  uip_len = 0;
  return;

 badpkt:
  UIP_STAT(++uip_stat.nd6.drop);
  UIP_LOG("RA received is bad"); 
  uip_len = 0;
  return;
}


/** @} */
