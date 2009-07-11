#ifndef __DHCPS_H__6M2XYUGNTK__
#define __DHCPS_H__6M2XYUGNTK__
#include "contiki-net.h"
#include <stdint.h>

#define MAX_HLEN 6

struct dhcps_client_lease
{
  uint8_t chaddr[MAX_HLEN];
  uip_ipaddr_t ipaddr;
  unsigned long lease_end;
  uint8_t flags;
};

struct dhcps_config
{
  unsigned long default_lease_time;
  uip_ipaddr_t netmask;
  uip_ipaddr_t dnsaddr;
  uip_ipaddr_t default_router;
  struct dhcps_client_lease *leases;
  uint8_t flags;
  uint8_t num_leases;
};

#define DHCP_CONF_NETMASK 0x01
#define DHCP_CONF_DNSADDR 0x02
#define DHCP_CONF_DEFAULT_ROUTER 0x04

#define DHCP_INIT_LEASE(addr0, addr1, addr2, addr3) \
{{0},{addr0, addr1, addr2, addr3},0,0}

/**
 * Start the DHCP server
 *
 * This function starts th DHCP server with the given configuration.
 * The flags field determines which options are actually sent to the
 * client
 *
 * \param conf Pointer to a configuration struct. The configuration is
 * not copied and should remain constant while the server is running.
 * The leases pointed to by the configuration must be in writable memory.
 **/
void dhcps_init(const struct dhcps_config *conf);

#endif /* __DHCPS_H__6M2XYUGNTK__ */
