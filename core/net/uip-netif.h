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
 
#ifndef __UIP_NETIF_H__
#define __UIP_NETIF_H__

#include "net/uip-nd6.h"

#ifndef UIP_CONF_NETIF_MAX_ADDRESSES
#define UIP_CONF_NETIF_MAX_ADDRESSES 2
#endif /*UIP_CONF_NETIF_MAX_ADDRESSES*/

/**
 * \brief Possible states for the address of an interface (RFC 4862 autoconf +
 * NOT_USED + INFINITE)
 */
typedef enum {
  NOT_USED = -1,
  TENTATIVE = 0,
  PREFERRED = 1,
  DEPRECATED = 2, /* not needed if we don't use pliffetime in prefix struct */
} uip_netif_state;

/**
 * \brief How the address was acquired: Autoconf, DHCP or manually
 *
 */
typedef enum {
  AUTOCONF = 1,
  DHCP = 2,
  MANUAL = 3
} uip_netif_type;

/**
 * \brief An address structure for an interface 
 *
 * Contains an ip address assigned to the interface, and its state.
 */
struct uip_netif_addr {
  uip_ipaddr_t ipaddr;
  uip_netif_state state;
  struct stimer vlifetime;
  u8_t is_infinite;
  uip_netif_type type;
};

/** \brief  Interface structure (contains all the interface variables) */
struct uip_netif {
  u32_t link_mtu;
  u8_t  cur_hop_limit;
  u32_t base_reachable_time; /* in msec */
  u32_t reachable_time;     /* in msec */
  u32_t retrans_timer;      /* in msec */
  u8_t  dup_addr_detect_transmit;
  /** Note: the link-local address is at position 0 */
  struct uip_netif_addr addresses[UIP_CONF_NETIF_MAX_ADDRESSES];
  uip_ipaddr_t solicited_node_mcastaddr;
};


/*---------------------------------------------------------------------------*/
extern struct uip_netif uip_netif_physical_if;
extern struct etimer uip_netif_timer_dad;
extern struct etimer uip_netif_timer_rs;
extern struct etimer uip_netif_timer_periodic;

/*---------------------------------------------------------------------------*/
/** \brief Initialize the network interfac and run stateless autoconf */
void  uip_netif_init(void);


/**
 * \brief periodically check the state of the addresses.
 */
void uip_netif_periodic(void); 

/**
 * \brief recompute random reachable timer
 * \return the new value for timer
 */
u32_t uip_netif_compute_reachable_time(void);

/**
 * \brief Check if an unicast address is attached to my interface
 * \param ipaddr an IP address to be checked
 * \return 1 if address is attached to my interface (otherwise false)
 */
#define uip_netif_is_addr_my_unicast(a) (uip_netif_addr_lookup(a, 128, 0) != NULL)

/**
 * \brief Check if this is my solicited-node multicast address
 * \param ipaddr an IP address to be checked
 * \return 1 if the address is my solicited-node (otherwise false)
 */
u8_t  uip_netif_is_addr_my_solicited(uip_ipaddr_t *ipaddr);

/**
 * \brief Autoconfigure and add an address corresponding to a specific prefix
 * \param ipaddr the prefix if we are doing autoconf, the address for DHCP and manual
 * \param length the prefix length if autoconf, 0 for DHCP and manual
 * \param vlifetime valid lifetime of the address, 0 if the address has INFINITE lifetime, 
 *        non 0 otherwise
 * \param type AUTOCONF or MANUAL or DHCP 
 */
void uip_netif_addr_add(uip_ipaddr_t *ipaddr, u8_t length, unsigned long vlifetime, uip_netif_type type);

/**
 * \brief Set the 8 last bytes of the IP address
 *        based on the L2 identifier using autoconf
 * \param *ipaddr the IP address to be completed with layer 2 info
 * \param *lladdr the L2 address
 */
void uip_netif_addr_autoconf_set(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr);

/**
 * \brief Lookup an address
 * \param ipaddr the prefix if we are looking for an autoconf address, the address otherwise
 * \param length the prefix length if we are looking for autoconf address, 128 otherwise
 * \param type AUTOCONF or MANUAL or DHCP or 0
 * 
 *
 * If we are looking for an AUTOCONFIGURED address, ipaddr is a prefix
 * length is its length, type is AUTOCONF.
 * Otherwise ipaddr is a full address, length must be 128, type is MANUAL 
 * or DHCP.
 * Note: if we do not care about the type, type MUST be 0 
 */
struct uip_netif_addr *
uip_netif_addr_lookup(uip_ipaddr_t *ipaddr, u8_t length, uip_netif_type type);

/**
 * \brief Select the appropriate source address for a packet
 * \param ipaddr the selected address (returned value)
 * \param ipaddr the destination of the packet 
 */
void uip_netif_select_src(uip_ipaddr_t *src, uip_ipaddr_t *dst);

/**
 * \brief Schedule DAD for a given address
 * \param ifaddr the address for which schedule DAD 
 */
void  uip_netif_sched_dad(struct uip_netif_addr *ifaddr);

/**
 * \brief Perform DAD (i.e. Duplicate Address Detection)
 */
void  uip_netif_dad(void);

/**
 * \brief DAD failed, should never happen!
 * \param ipaddr the address for which DAD failed 
 */
void  uip_netif_dad_failed(uip_ipaddr_t *ipaddr);

/**
 * \brief Schedule the sending of RS
 */
void  uip_netif_sched_send_rs(void);

/**
 * \brief Send up to MAX_RTR_SOLICITATION_DELAY RS separated by a delay of
 * RTR_SOLICITATION_INTERVAL
 */
void uip_netif_send_rs(void);

#endif

/** @} */

