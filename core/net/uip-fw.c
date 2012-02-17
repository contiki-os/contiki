/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: uip-fw.c,v 1.12 2010/10/19 18:29:04 adamdunkels Exp $
 */
/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uipfw uIP packet forwarding
 * @{
 *
 */

/**
 * \file
 * uIP packet forwarding.
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file implements a number of simple functions which do packet
 * forwarding over multiple network interfaces with uIP.
 *
 */

#include <string.h>

#include "contiki-conf.h"

#include "net/uip.h"
#include "net/uip_arch.h"
#include "net/uip-fw.h"
#ifdef AODV_COMPLIANCE
#include "net/uaodv-def.h"
#endif

/*
 * The list of registered network interfaces.
 */
static struct uip_fw_netif *netifs = NULL;

/*
 * A pointer to the default network interface.
 */
static struct uip_fw_netif *defaultnetif = NULL;

struct tcpip_hdr {
  /* IP header. */
  uint8_t vhl,
    tos;
  uint16_t len,
    ipid,
    ipoffset;
  uint8_t ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
  
  /* TCP header. */
  uint16_t srcport,
    destport;
  uint8_t seqno[4],
    ackno[4],
    tcpoffset,
    flags,
    wnd[2];
  uint16_t tcpchksum;
  uint8_t urgp[2];
  uint8_t optdata[4];
};

struct icmpip_hdr {
  /* IP header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
  /* ICMP (echo) header. */
  uint8_t type, icode;
  uint16_t icmpchksum;
  uint16_t id, seqno;
  uint8_t payload[1];
};

/* ICMP ECHO. */
#define ICMP_ECHO 8

/* ICMP TIME-EXCEEDED. */
#define ICMP_TE 11

/*
 * Pointer to the TCP/IP headers of the packet in the uip_buf buffer.
 */
#define BUF ((struct tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*
 * Pointer to the ICMP/IP headers of the packet in the uip_buf buffer.
 */
#define ICMPBUF ((struct icmpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*
 * Certain fields of an IP packet that are used for identifying
 * duplicate packets.
 */
struct fwcache_entry {
  uint16_t timer;
  
  uip_ipaddr_t srcipaddr;
  uip_ipaddr_t destipaddr;
  uint16_t ipid;
  uint8_t proto;
  uint8_t unused;

#if notdef
  uint16_t payload[2];
#endif

#if UIP_REASSEMBLY > 0
  uint16_t len, offset;
#endif
};

/*
 * The number of packets to remember when looking for duplicates.
 */
#ifdef UIP_CONF_FWCACHE_SIZE
#define FWCACHE_SIZE UIP_CONF_FWCACHE_SIZE
#else
#define FWCACHE_SIZE 2
#endif


/*
 * A cache of packet header fields which are used for
 * identifying duplicate packets.
 */
static struct fwcache_entry fwcache[FWCACHE_SIZE];

/**
 * \internal
 * The time that a packet cache is active.
 */
#define FW_TIME 20

/*------------------------------------------------------------------------------*/
/**
 * Initialize the uIP packet forwarding module.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_init(void)
{
  struct uip_fw_netif *t;
  defaultnetif = NULL;
  while(netifs != NULL) {
    t = netifs;
    netifs = netifs->next;
    t->next = NULL;
  }
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Check if an IP address is within the network defined by an IP
 * address and a netmask.
 *
 * \param ipaddr The IP address to be checked.
 * \param netipaddr The IP address of the network.
 * \param netmask The netmask of the network.
 *
 * \return Non-zero if IP address is in network, zero otherwise.
 */
/*------------------------------------------------------------------------------*/
static unsigned char
ipaddr_maskcmp(uip_ipaddr_t *ipaddr,
	       uip_ipaddr_t *netipaddr,
	       uip_ipaddr_t *netmask)
{
  return (ipaddr->u16[0] & netmask->u16[0]) == (netipaddr->u16[0] & netmask->u16[0]) &&
    (ipaddr->u16[1] & netmask->u16[1]) == (netipaddr->u16[1] & netmask->u16[1]);
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Send out an ICMP TIME-EXCEEDED message.
 *
 * This function replaces the packet in the uip_buf buffer with the
 * ICMP packet.
 */
/*------------------------------------------------------------------------------*/
static void
time_exceeded(void)
{

  /* We don't send out ICMP errors for ICMP messages (unless they are pings). */
  if(ICMPBUF->proto == UIP_PROTO_ICMP &&
     ICMPBUF->type != ICMP_ECHO) {
    uip_len = 0;
    return;
  }
  /* Copy fields from packet header into payload of this ICMP packet. */
  memcpy(&(ICMPBUF->payload[0]), ICMPBUF, UIP_IPH_LEN + 8);

  /* Set the ICMP type and code. */
  ICMPBUF->type = ICMP_TE;
  ICMPBUF->icode = 0;

  /* Calculate the ICMP checksum. */
  ICMPBUF->icmpchksum = 0;
  ICMPBUF->icmpchksum = ~uip_chksum((uint16_t *)&(ICMPBUF->type), 36);

  /* Set the IP destination address to be the source address of the
     original packet. */
  uip_ipaddr_copy(&BUF->destipaddr, &BUF->srcipaddr);

  /* Set our IP address as the source address. */
  uip_ipaddr_copy(&BUF->srcipaddr, &uip_hostaddr);

  /* The size of the ICMP time exceeded packet is 36 + the size of the
     IP header (20) = 56. */
  uip_len = 56;
  ICMPBUF->len[0] = 0;
  ICMPBUF->len[1] = (uint8_t)uip_len;

  /* Fill in the other fields in the IP header. */
  ICMPBUF->vhl = 0x45;
  ICMPBUF->tos = 0;
  ICMPBUF->ipoffset[0] = ICMPBUF->ipoffset[1] = 0;
  ICMPBUF->ttl  = UIP_TTL;
  ICMPBUF->proto = UIP_PROTO_ICMP;
  
  /* Calculate IP checksum. */
  ICMPBUF->ipchksum = 0;
  ICMPBUF->ipchksum = ~(uip_ipchksum());


}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Register a packet in the forwarding cache so that it won't be
 * forwarded again.
 */
/*------------------------------------------------------------------------------*/
static void
fwcache_register(void)
{
  struct fwcache_entry *fw;
  int i, oldest;

  oldest = FW_TIME;
  fw = NULL;
  
  /* Find the oldest entry in the cache. */
  for(i = 0; i < FWCACHE_SIZE; ++i) {
    if(fwcache[i].timer == 0) {
      fw = &fwcache[i];
      break;
    } else if(fwcache[i].timer <= oldest) {
      fw = &fwcache[i];
      oldest = fwcache[i].timer;
    }
  }

  fw->timer = FW_TIME;
  fw->ipid = BUF->ipid;
  uip_ipaddr_copy(&fw->srcipaddr, &BUF->srcipaddr);
  uip_ipaddr_copy(&fw->destipaddr, &BUF->destipaddr);
  fw->proto = BUF->proto;
#if notdef
  fw->payload[0] = BUF->srcport;
  fw->payload[1] = BUF->destport;
#endif
#if UIP_REASSEMBLY > 0
  fw->len = BUF->len;
  fw->offset = BUF->ipoffset;
#endif
}
/*------------------------------------------------------------------------------*/
/**
 * \internal
 * Find a network interface for the IP packet in uip_buf.
 */
/*------------------------------------------------------------------------------*/
static struct uip_fw_netif *
find_netif(void)
{
  struct uip_fw_netif *netif;
  
  /* Walk through every network interface to check for a match. */
  for(netif = netifs; netif != NULL; netif = netif->next) {
    if(ipaddr_maskcmp(&BUF->destipaddr, &netif->ipaddr,
		      &netif->netmask)) {
      /* If there was a match, we break the loop. */
      return netif;
    }
  }
  
  /* If no matching netif was found, we use default netif. */
  return defaultnetif;
}
/*------------------------------------------------------------------------------*/
/**
 * Output an IP packet on the correct network interface.
 *
 * The IP packet should be present in the uip_buf buffer and its
 * length in the global uip_len variable.
 *
 * \retval UIP_FW_ZEROLEN Indicates that a zero-length packet
 * transmission was attempted and that no packet was sent.
 *
 * \retval UIP_FW_NOROUTE No suitable network interface could be found
 * for the outbound packet, and the packet was not sent.
 *
 * \return The return value from the actual network interface output
 * function is passed unmodified as a return value.
 */
/*------------------------------------------------------------------------------*/
uint8_t
uip_fw_output(void)
{
  struct uip_fw_netif *netif;
#if UIP_BROADCAST
  const struct uip_udpip_hdr *udp = (void *)BUF;
#endif /* UIP_BROADCAST */

  if(uip_len == 0) {
    return UIP_FW_ZEROLEN;
  }

  fwcache_register();

#if UIP_BROADCAST
  /* Link local broadcasts go out on all interfaces. */
  if(uip_ipaddr_cmp(&udp->destipaddr, &uip_broadcast_addr)) {
    if(defaultnetif != NULL) {
      defaultnetif->output();
    }
    for(netif = netifs; netif != NULL; netif = netif->next) {
      netif->output();
    }
    return UIP_FW_OK;
  }
#endif /* UIP_BROADCAST */
  
  netif = find_netif();
  /*  printf("uip_fw_output: netif %p ->output %p len %d\n", netif,
	 netif->output,
	 uip_len);*/

  if(netif == NULL) {
    return UIP_FW_NOROUTE;
  }
  /* If we now have found a suitable network interface, we call its
     output function to send out the packet. */
  return netif->output();
}
/*------------------------------------------------------------------------------*/
/**
 * Forward an IP packet in the uip_buf buffer.
 *
 *
 *
 * \return UIP_FW_FORWARDED if the packet was forwarded, UIP_FW_LOCAL if
 * the packet should be processed locally.
 */
/*------------------------------------------------------------------------------*/
uint8_t
uip_fw_forward(void)
{
  struct fwcache_entry *fw;

  /* First check if the packet is destined for ourselves and return 0
     to indicate that the packet should be processed locally. */
  if(uip_ipaddr_cmp(&BUF->destipaddr, &uip_hostaddr)) {
    return UIP_FW_LOCAL;
  }

#ifdef AODV_COMPLIANCE
#define udp ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
  if(udp->proto == UIP_PROTO_UDP && udp->destport == UIP_HTONS(UAODV_UDPPORT)) {
    return UIP_FW_LOCAL;
  }
#endif

  /* If we use ping IP address configuration, and our IP address is
     not yet configured, we should intercept all ICMP echo packets. */
#if UIP_PINGADDRCONF
  if(uip_ipaddr_cmp(&uip_hostaddr, &uip_all_zeroes_addr) &&
     BUF->proto == UIP_PROTO_ICMP &&
     ICMPBUF->type == ICMP_ECHO) {
    return UIP_FW_LOCAL;
  }
#endif /* UIP_PINGADDRCONF */

  /* Check if the packet is in the forwarding cache already, and if so
     we drop it. */

  for(fw = fwcache; fw < &fwcache[FWCACHE_SIZE]; ++fw) {
    if(fw->timer != 0 &&
#if UIP_REASSEMBLY > 0
       fw->len == BUF->len &&
       fw->offset == BUF->ipoffset &&
#endif
       fw->ipid == BUF->ipid &&
       uip_ipaddr_cmp(&fw->srcipaddr, &BUF->srcipaddr) &&
       uip_ipaddr_cmp(&fw->destipaddr, &BUF->destipaddr) &&
#if notdef
       fw->payload[0] == BUF->srcport &&
       fw->payload[1] == BUF->destport &&
#endif
       fw->proto == BUF->proto) {
      /* Drop packet. */
      return UIP_FW_FORWARDED;
    }
  }

  /* If the TTL reaches zero we produce an ICMP time exceeded message
     in the uip_buf buffer and forward that packet back to the sender
     of the packet. */

  if(BUF->ttl <= 1) {
    /* No time exceeded for broadcasts and multicasts! */
    if(uip_ipaddr_cmp(&BUF->destipaddr, &uip_broadcast_addr)) {
      return UIP_FW_LOCAL;
    }
    time_exceeded();
  }
  
  /* Decrement the TTL (time-to-live) value in the IP header */
  BUF->ttl = BUF->ttl - 1;
  
  /* Update the IP checksum. */
  if(BUF->ipchksum >= UIP_HTONS(0xffff - 0x0100)) {
    BUF->ipchksum = BUF->ipchksum + UIP_HTONS(0x0100) + 1;
  } else {
    BUF->ipchksum = BUF->ipchksum + UIP_HTONS(0x0100);
  }

  if(uip_len > 0) {
    uip_appdata = &uip_buf[UIP_LLH_LEN + UIP_TCPIP_HLEN];
    uip_fw_output();
  }

#if UIP_BROADCAST
  if(uip_ipaddr_cmp(&BUF->destipaddr, &uip_broadcast_addr)) {
    return UIP_FW_LOCAL;
  }
#endif /* UIP_BROADCAST */

  /* Return non-zero to indicate that the packet was forwarded and that no
     other processing should be made. */
  return UIP_FW_FORWARDED;
}
/*------------------------------------------------------------------------------*/
/**
 * Register a network interface with the forwarding module.
 *
 * \param netif A pointer to the network interface that is to be
 * registered.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_register(struct uip_fw_netif *netif)
{
  netif->next = netifs;
  netifs = netif;
}
/*------------------------------------------------------------------------------*/
/**
 * Register a default network interface.
 *
 * All packets that don't go out on any of the other interfaces will
 * be routed to the default interface.
 *
 * \param netif A pointer to the network interface that is to be
 * registered.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_default(struct uip_fw_netif *netif)
{
  defaultnetif = netif;
}
/*------------------------------------------------------------------------------*/
/**
 * Perform periodic processing.
 */
/*------------------------------------------------------------------------------*/
void
uip_fw_periodic(void)
{
  struct fwcache_entry *fw;
  for(fw = fwcache; fw < &fwcache[FWCACHE_SIZE]; ++fw) {
    if(fw->timer > 0) {
      --fw->timer;
    }
  }
}
/*------------------------------------------------------------------------------*/
/** @} */
/** @} */
