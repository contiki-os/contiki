/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         This file shows the implementations of the Enhanced Stateless 
 *	       Multicast RPL Forwarding (ESMRF)
 *
 *         It will only work in RPL networks in MOP 3 "Storing with Multicast"
 *
 * \author
 *         Khaled Qorany	kqorany2@gmail.com
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ipv6/multicast/uip-mcast6-route.h"
#include "net/ipv6/multicast/uip-mcast6-stats.h"
#include "net/ipv6/multicast/esmrf.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/netstack.h"
#include <string.h>

extern uint16_t uip_slen;

#define DEBUG NONE
#include "net/ip/uip-debug.h"

#define ESMRF_VERBOSE NONE

#if DEBUG && ESMRF_VERBOSE
#define VERBOSE_PRINTF(...) PRINTF(__VA_ARGS__)
#define VERBOSE_PRINT_SEED(s) PRINT_SEED(s)
#else
#define VERBOSE_PRINTF(...)
#define VERBOSE_PRINT_SEED(...)
#endif

/*---------------------------------------------------------------------------*/
/* Maintain Stats */
#if UIP_MCAST6_STATS
static struct esmrf_stats stats;

#define ESMRF_STATS_ADD(x) stats.x++
#define ESMRF_STATS_INIT() do { memset(&stats, 0, sizeof(stats)); } while(0)
#else /* UIP_MCAST6_STATS */
#define ESMRF_STATS_ADD(x)
#define ESMRF_STATS_INIT()
#endif
/*---------------------------------------------------------------------------*/
/* Macros */
/*---------------------------------------------------------------------------*/
/* CCI */
#define ESMRF_FWD_DELAY()  NETSTACK_RDC.channel_check_interval()
/* Number of slots in the next 500ms */
#define ESMRF_INTERVAL_COUNT  ((CLOCK_SECOND >> 2) / fwd_delay)
/*---------------------------------------------------------------------------*/
/* Internal Data */
/*---------------------------------------------------------------------------*/
static struct ctimer mcast_periodic;
static uint8_t mcast_len;
static uip_buf_t mcast_buf;
static uint8_t fwd_delay;
static uint8_t fwd_spread;
static struct uip_udp_conn *c;
static uip_ipaddr_t src_ip;
static uip_ipaddr_t des_ip;
/*---------------------------------------------------------------------------*/
/* uIPv6 Pointers */
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF      ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD  ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_UDP_BUF       ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
/*---------------------------------------------------------------------------*/
/* Local function prototypes */
/*---------------------------------------------------------------------------*/
static void icmp_input(void);
static void icmp_output(void);
static void mcast_fwd(void *p);
int remove_ext_hdr(void);
/*---------------------------------------------------------------------------*/
/* Internal Data Structures */
/*---------------------------------------------------------------------------*/
struct multicast_on_behalf{   /*  ICMP message of multicast_on_behalf */
  uint16_t mcast_port;
  uip_ipaddr_t mcast_ip;
  uint8_t mcast_payload[UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN];
};
#define UIP_ICMP_MOB 18 /* Size of multicast_on_behalf ICMP header */
/*---------------------------------------------------------------------------*/
/* Temporary Stores */
/*---------------------------------------------------------------------------*/
static struct multicast_on_behalf *locmobptr;
static int loclen;
/*---------------------------------------------------------------------------*/
/* ESMRF ICMPv6 handler declaration */
UIP_ICMP6_HANDLER(esmrf_icmp_handler, ICMP6_ESMRF,
                  UIP_ICMP6_HANDLER_CODE_ANY, icmp_input);
/*---------------------------------------------------------------------------*/
static void
icmp_output()
{
  uint16_t payload_len=0;
  rpl_dag_t *dag_t;

  struct multicast_on_behalf *mob;
  mob = (struct multicast_on_behalf *)UIP_ICMP_PAYLOAD;
  memcpy(&mob->mcast_payload, &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], uip_slen);

  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = ESMRF_IP_HOP_LIMIT;

  mob->mcast_port = (uint16_t) uip_udp_conn->rport;
  uip_ipaddr_copy(&mob->mcast_ip, &UIP_IP_BUF->destipaddr);

  payload_len = UIP_ICMP_MOB + uip_slen;

  dag_t = rpl_get_any_dag();
  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dag_t->dag_id);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

  VERBOSE_PRINTF("ESMRF: ICMPv6 Out - Hdr @ %p, payload @ %p to: ", UIP_ICMP_BUF, mob);
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + payload_len) >> 8;
  UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + payload_len) & 0xff;

  UIP_ICMP_BUF->type = ICMP6_ESMRF;
  UIP_ICMP_BUF->icode = ESMRF_ICMP_CODE;

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + payload_len;

  VERBOSE_PRINTF("ESMRF: ICMPv6 Out - %u bytes, uip_len %u bytes, uip_ext_len %u bytes\n",
					payload_len, uip_len, uip_ext_len);

  tcpip_ipv6_output();
  ESMRF_STATS_ADD(icmp_out);
  return;
}
/*---------------------------------------------------------------------------*/
static void
icmp_input()
{
#if UIP_CONF_IPV6_CHECKS
  if(UIP_ICMP_BUF->icode != ESMRF_ICMP_CODE) {
    PRINTF("ESMRF: ICMPv6 In, bad ICMP code\n");
    ESMRF_STATS_ADD(icmp_bad);
    return;
  }
  if(UIP_IP_BUF->ttl <= 1) {
    PRINTF("ESMRF: ICMPv6 In, bad TTL\n");
    ESMRF_STATS_ADD(icmp_bad);
    return;
  }
#endif

  remove_ext_hdr();

  PRINTF("ESMRF: ICMPv6 In from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" len %u, ext %u\n", uip_len, uip_ext_len);

  ESMRF_STATS_ADD(icmp_in);

  VERBOSE_PRINTF("ESMRF: ICMPv6 In, parse from %p to %p\n",
                 UIP_ICMP_PAYLOAD,
                 (uint8_t *)UIP_ICMP_PAYLOAD + uip_len -
                 uip_l2_l3_icmp_hdr_len);


  locmobptr = (struct multicast_on_behalf *) UIP_ICMP_PAYLOAD;
  loclen = uip_len - (uip_l2_l3_icmp_hdr_len + UIP_ICMP_MOB);

  uip_ipaddr_copy(&src_ip, &UIP_IP_BUF->srcipaddr);
  uip_ipaddr_copy(&des_ip, &UIP_IP_BUF->destipaddr);

  /* Extract the original multicast message */
  uip_ipaddr_copy(&c->ripaddr, &locmobptr->mcast_ip);
  c->rport = locmobptr->mcast_port;
  uip_slen = loclen;
  uip_udp_conn=c;
  memcpy(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], locmobptr->mcast_payload,
         loclen > UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN?
         UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN: loclen);

  uip_process(UIP_UDP_SEND_CONN);

  memcpy(&mcast_buf, uip_buf, uip_len);
  mcast_len = uip_len;
  /* pass the packet to our uip_process to check if it is allowed to 
   * accept this packet or not */
  uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &src_ip);
  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &des_ip);
  UIP_UDP_BUF->udpchksum = 0;
  
  uip_process(UIP_DATA);

  memcpy(uip_buf, &mcast_buf, mcast_len);
  uip_len = mcast_len;
  /* Return the IP of the original Multicast sender */
  uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &src_ip);
  UIP_UDP_BUF->udpchksum = 0;
  /* If we have an entry in the multicast routing table, something with
   * a higher RPL rank (somewhere down the tree) is a group member */
  if(uip_mcast6_route_lookup(&UIP_IP_BUF->destipaddr)) {
    PRINTF("ESMRF: Forward this packet\n");
    /* If we enter here, we will definitely forward */
    tcpip_ipv6_output();
  }
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
static void
mcast_fwd(void *p)
{
  memcpy(uip_buf, &mcast_buf, mcast_len);
  uip_len = mcast_len;
  UIP_IP_BUF->ttl--;
  tcpip_output(NULL);
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
static uint8_t
in()
{
  rpl_dag_t *d;                 /* Our DODAG */
  uip_ipaddr_t *parent_ipaddr;  /* Our pref. parent's IPv6 address */
  const uip_lladdr_t *parent_lladdr;  /* Our pref. parent's LL address */

  /*
   * Fetch a pointer to the LL address of our preferred parent
   *
   * ToDo: This rpl_get_any_dag() call is a dirty replacement of the previous
   *   rpl_get_dag(RPL_DEFAULT_INSTANCE);
   * so that things can compile with the new RPL code. This needs updated to
   * read instance ID from the RPL HBHO and use the correct parent accordingly
   */
  d = rpl_get_any_dag();
  if(!d) {
    PRINTF("ESMRF: No DODAG\n");
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  /* Retrieve our preferred parent's LL address */
  parent_ipaddr = rpl_get_parent_ipaddr(d->preferred_parent);
  parent_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(parent_ipaddr);

  if(parent_lladdr == NULL) {
    PRINTF("ESMRF: NO Parent exist \n");
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  /*
   * We accept a datagram if it arrived from our preferred parent, discard
   * otherwise.
   */
  if(memcmp(parent_lladdr, packetbuf_addr(PACKETBUF_ADDR_SENDER),
            UIP_LLADDR_LEN)) {
    PRINTF("ESMRF: Routable in but ESMRF ignored it\n");
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  if(UIP_IP_BUF->ttl <= 1) {
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  UIP_MCAST6_STATS_ADD(mcast_in_all);
  UIP_MCAST6_STATS_ADD(mcast_in_unique);

  /* If we have an entry in the mcast routing table, something with
   * a higher RPL rank (somewhere down the tree) is a group member */
  if(uip_mcast6_route_lookup(&UIP_IP_BUF->destipaddr)) {
    /* If we enter here, we will definitely forward */
    UIP_MCAST6_STATS_ADD(mcast_fwd);

    /*
     * Add a delay (D) of at least ESMRF_FWD_DELAY() to compensate for how
     * contikimac handles broadcasts. We can't start our TX before the sender
     * has finished its own.
     */
    fwd_delay = ESMRF_FWD_DELAY();

    /* Finalise D: D = min(ESMRF_FWD_DELAY(), ESMRF_MIN_FWD_DELAY) */
#if ESMRF_MIN_FWD_DELAY
    if(fwd_delay < ESMRF_MIN_FWD_DELAY) {
      fwd_delay = ESMRF_MIN_FWD_DELAY;
    }
#endif

    if(fwd_delay == 0) {
      /* No delay required, send it, do it now, why wait? */
      UIP_IP_BUF->ttl--;
      tcpip_output(NULL);
      UIP_IP_BUF->ttl++;        /* Restore before potential upstack delivery */
    } else {
      /* Randomise final delay in [D , D*Spread], step D */
      fwd_spread = ESMRF_INTERVAL_COUNT;
      if(fwd_spread > ESMRF_MAX_SPREAD) {
        fwd_spread = ESMRF_MAX_SPREAD;
      }
      if(fwd_spread) {
        fwd_delay = fwd_delay * (1 + ((random_rand() >> 11) % fwd_spread));
      }

      memcpy(&mcast_buf, uip_buf, uip_len);
      mcast_len = uip_len;
      ctimer_set(&mcast_periodic, fwd_delay, mcast_fwd, NULL);
    }
    PRINTF("ESMRF: %u bytes: fwd in %u [%u]\n",
           uip_len, fwd_delay, fwd_spread);
  }

  /* Done with this packet unless we are a member of the mcast group */
  if(!uip_ds6_is_my_maddr(&UIP_IP_BUF->destipaddr)) {
    PRINTF("ESMRF: Not a group member. No further processing\n");
    return UIP_MCAST6_DROP;
  } else {
    PRINTF("ESMRF: Ours. Deliver to upper layers\n");
    UIP_MCAST6_STATS_ADD(mcast_in_ours);
    return UIP_MCAST6_ACCEPT;
  }
}
/*---------------------------------------------------------------------------*/
static void
init()
{
  UIP_MCAST6_STATS_INIT(NULL);
  uip_mcast6_route_init();
  /* Register the ICMPv6 input handler */
  uip_icmp6_register_input_handler(&esmrf_icmp_handler);
  c = udp_new(NULL, 0, NULL);
}
/*---------------------------------------------------------------------------*/
static void
out(void)
{
  rpl_dag_t *dag_t;
  dag_t = rpl_get_any_dag();
  if (!dag_t){
    PRINTF("ESMRF: There is no DODAG\n");
    return;
  }
  if(dag_t->rank == 256){
    PRINTF("ESMRF: I am the Root, thus send the multicast packet normally. \n");
    return;
  }
  else{
    PRINTF("ESMRF: I am not the Root\n");
	PRINTF("Send multicast-on-befalf message (ICMPv6) instead to  ");
    PRINT6ADDR(&dag_t->dag_id);
    PRINTF("\n");
    icmp_output();
    uip_slen=0;
    return;
  }
}
/*---------------------------------------------------------------------------*/
const struct uip_mcast6_driver esmrf_driver = {
  "ESMRF",
  init,
  out,
  in,
};
/*---------------------------------------------------------------------------*/
