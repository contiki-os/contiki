/* Adapted by Simon Berg from net/dhcpc.c */
/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 */

#include <stdio.h>
#include <string.h>
#include <uip_arp.h>
#include "contiki.h"
#include "contiki-net.h"
#include "dhcps.h"

struct dhcp_msg {
  uint8_t op, htype, hlen, hops;
  uint8_t xid[4];
  uint16_t secs, flags;
  uint8_t ciaddr[4];
  uint8_t yiaddr[4];
  uint8_t siaddr[4];
  uint8_t giaddr[4];
  uint8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
  uint8_t sname[64];
  uint8_t file[128];
#endif
  uint8_t options[312];
} CC_BYTE_ALIGNED;

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPS_SERVER_PORT  67
#define DHCPS_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7
#define DHCPINFORM    8

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255



#define LEASE_FLAGS_ALLOCATED 0x01	/* Lease with an allocated address*/
#define LEASE_FLAGS_VALID 0x02		/* Contains a valid but
					   possibly outdated lease */


static const struct dhcps_config *config;


static uint8_t *
find_option(uint8_t option)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  uint8_t *optptr = &m->options[4];
  uint8_t *end = (uint8_t*)uip_appdata + uip_datalen();
  while(optptr < end && *optptr != DHCP_OPTION_END) {
    if(*optptr == option) {
      return optptr;
    }
    optptr += optptr[1] + 2;
  }
  return NULL;
}

static const uint8_t magic_cookie[4] = {99, 130, 83, 99};

static int
check_cookie(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  return memcmp(m->options, magic_cookie, 4) == 0;
}

/* Finds any valid lease for a given MAC address */
static struct dhcps_client_lease *
lookup_lease_mac(const uint8_t *chaddr, uint8_t hlen)
{
  struct dhcps_client_lease *lease = config->leases;
  struct dhcps_client_lease *end = config->leases + config->num_leases;
  while(lease != end) {
    if (lease->flags & LEASE_FLAGS_VALID
	&& memcmp(lease->chaddr, chaddr, hlen) == 0) {
      return lease;
    }
    lease++;
  }
  return NULL;
}

static struct dhcps_client_lease *
lookup_lease_ip(const uip_ipaddr_t *ip)
{
  struct dhcps_client_lease *lease = config->leases;
  struct dhcps_client_lease *end = config->leases + config->num_leases;
  while(lease != end) {
    if (uip_ipaddr_cmp(&lease->ipaddr, ip)) {
      return lease;
    }
    lease++;
  }
  return NULL;
}

static struct dhcps_client_lease *
find_free_lease(void)
{
  struct dhcps_client_lease *found = NULL;
  struct dhcps_client_lease *lease = config->leases;
  struct dhcps_client_lease *end = config->leases + config->num_leases;
  while(lease != end) {
    if (!(lease->flags & LEASE_FLAGS_VALID)) return lease;
    if (!(lease->flags & LEASE_FLAGS_ALLOCATED)) found = lease;
    lease++;
  }
  return found;
}

struct dhcps_client_lease *
init_lease(struct dhcps_client_lease *lease,
	   const uint8_t *chaddr, uint8_t hlen)
{
  if (lease) {
    memcpy(lease->chaddr, chaddr, hlen);
    lease->flags = LEASE_FLAGS_VALID;
  }
  return lease;
}


static struct dhcps_client_lease *
choose_address()
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  struct dhcps_client_lease *lease;
  lease = lookup_lease_mac(m->chaddr, m->hlen);
  if (lease) {
    return lease;
  }
  {
    uint8_t *opt;
    opt = find_option(DHCP_OPTION_REQ_IPADDR);
    if (opt && (lease = lookup_lease_ip((uip_ipaddr_t*)&opt[2]))
	&& !(lease->flags & LEASE_FLAGS_ALLOCATED)) {
      return init_lease(lease, m->chaddr,m->hlen);
    }
  }
  lease = find_free_lease();
  if (lease) {
    return init_lease(lease, m->chaddr,m->hlen);
  }
  return NULL;
}

static struct dhcps_client_lease *
allocate_address()
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  struct dhcps_client_lease *lease;
  lease = lookup_lease_mac(m->chaddr, m->hlen);
  if (!lease) {
    uint8_t *opt;
    opt = find_option(DHCP_OPTION_REQ_IPADDR);
    if (!(opt && (lease = lookup_lease_ip((uip_ipaddr_t*)&opt[2]))
	&& !(lease->flags & LEASE_FLAGS_ALLOCATED))) {
      return NULL;
    }
  }
  lease->lease_end = clock_seconds()+config->default_lease_time;
  lease->flags |= LEASE_FLAGS_ALLOCATED;
  return lease;
}

static struct dhcps_client_lease *
release_address()
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  struct dhcps_client_lease *lease;
  lease = lookup_lease_mac(m->chaddr, m->hlen);
  if (!lease) {
    return NULL;
  }
  lease->flags &= ~LEASE_FLAGS_ALLOCATED;
  return lease;
}

/*---------------------------------------------------------------------------*/
static uint8_t *
add_msg_type(uint8_t *optptr, uint8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_server_id(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy(optptr, &uip_hostaddr, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_lease_time(uint8_t *optptr)
{
  uint32_t lt;
  *optptr++ = DHCP_OPTION_LEASE_TIME;
  *optptr++ = 4;
  lt = UIP_HTONL(config->default_lease_time);
  memcpy(optptr, &lt, 4);
  return optptr + 4;
}

/*---------------------------------------------------------------------------*/
static uint8_t *
add_end(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;
  return optptr;
}

static uint8_t *
add_config(uint8_t *optptr)
{
  if (config->flags & DHCP_CONF_NETMASK) {
    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = 4;
    memcpy(optptr, &config->netmask, 4);
    optptr += 4;
  }
  if (config->flags & DHCP_CONF_DNSADDR) {
    *optptr++ = DHCP_OPTION_DNS_SERVER;
    *optptr++ = 4;
    memcpy(optptr, &config->dnsaddr, 4);
    optptr += 4;
  }
  if (config->flags & DHCP_CONF_DEFAULT_ROUTER) {
    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = 4;
    memcpy(optptr, &config->default_router, 4);
    optptr += 4;
  }
  return optptr;
}

static void
create_msg(CC_REGISTER_ARG struct dhcp_msg *m)
{
  m->op = DHCP_REPLY;
  /* m->htype = DHCP_HTYPE_ETHERNET; */
/*   m->hlen = DHCP_HLEN_ETHERNET; */
/*   memcpy(m->chaddr, &uip_ethaddr,DHCP_HLEN_ETHERNET); */
  m->hops = 0;
  m->secs = 0;
  memcpy(m->siaddr, &uip_hostaddr, 4);
  m->sname[0] = '\0';
  m->file[0] = '\0';
  memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}

static uip_ipaddr_t any_addr;
static uip_ipaddr_t bcast_addr;

/*---------------------------------------------------------------------------*/
static void
send_offer(struct uip_udp_conn *conn, struct dhcps_client_lease *lease)
{
  uint8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  create_msg(m);
  memcpy(&m->yiaddr, &lease->ipaddr,4);

  end = add_msg_type(&m->options[4], DHCPOFFER);
  end = add_server_id(end);
  end = add_lease_time(end);
  end = add_config(end);
  end = add_end(end);
  uip_ipaddr_copy(&conn->ripaddr, &bcast_addr);
  uip_send(uip_appdata, (int)(end - (uint8_t *)uip_appdata));
}

static void
send_ack(struct uip_udp_conn *conn, struct dhcps_client_lease *lease)
{
  uint8_t *end;
  uip_ipaddr_t ciaddr;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  create_msg(m);
  memcpy(&m->yiaddr, &lease->ipaddr,4);
   
  end = add_msg_type(&m->options[4], DHCPACK);
  end = add_server_id(end);
  end = add_lease_time(end);
  end = add_config(end);
  end = add_end(end);
  memcpy(&ciaddr, &lease->ipaddr,4);
  uip_ipaddr_copy(&conn->ripaddr, &bcast_addr);
  uip_send(uip_appdata, (int)(end - (uint8_t *)uip_appdata));
  printf("ACK\n");
}
static void
send_nack(struct uip_udp_conn *conn)
{
  uint8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  create_msg(m);
  memset(&m->yiaddr, 0, 4);

  end = add_msg_type(&m->options[4], DHCPNAK);
  end = add_server_id(end);
  end = add_end(end);

  uip_ipaddr_copy(&conn->ripaddr, &bcast_addr);
  uip_send(uip_appdata, (int)(end - (uint8_t *)uip_appdata));
  printf("NACK\n");
}

/*---------------------------------------------------------------------------*/
PROCESS(dhcp_server_process, "DHCP server");
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dhcp_server_process, ev , data)
{
  static struct uip_udp_conn *conn;
  static struct uip_udp_conn *send_conn;
  static struct dhcps_client_lease *lease;
  PROCESS_BEGIN();
  printf("DHCP server starting\n");
  uip_ipaddr(&any_addr, 0,0,0,0);
  uip_ipaddr(&bcast_addr, 255,255,255,255);
  conn = udp_new(&any_addr, UIP_HTONS(DHCPS_CLIENT_PORT), NULL);
  if (!conn) goto exit;
  send_conn = udp_new(&bcast_addr, UIP_HTONS(DHCPS_CLIENT_PORT), NULL);
  if (!send_conn) goto exit;
  
  uip_udp_bind(conn, UIP_HTONS(DHCPS_SERVER_PORT));
  uip_udp_bind(send_conn, UIP_HTONS(DHCPS_SERVER_PORT));
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      if (uip_newdata()) {
	struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
	struct uip_udpip_hdr *header = (struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN];

	if (m->op == DHCP_REQUEST && check_cookie() && m->hlen <= MAX_HLEN) {
	  uint8_t *opt = find_option(DHCP_OPTION_MSG_TYPE);
	  if (opt) {
	    uint8_t mtype = opt[2];
	    if (opt[2] == DHCPDISCOVER) {
	      printf("Discover\n");
	      lease = choose_address();
	      if (lease) {
		lease->lease_end = clock_seconds()+config->default_lease_time;
		tcpip_poll_udp(send_conn);
		PROCESS_WAIT_EVENT_UNTIL(uip_poll());
		send_offer(conn,lease);
	      }
	    } else {
	      uint8_t *opt = find_option(DHCP_OPTION_SERVER_ID);
	      if (!opt || uip_ipaddr_cmp((uip_ipaddr_t*)&opt[2], &uip_hostaddr)) {
		if (mtype == DHCPREQUEST) {
		  printf("Request\n");
		  lease = allocate_address();
		  tcpip_poll_udp(send_conn);
		  PROCESS_WAIT_EVENT_UNTIL(uip_poll());
		  if (!lease) {
		    send_nack(send_conn);
		  } else {
		    send_ack(send_conn,lease);
		  }
		} else if (mtype == DHCPRELEASE) {
		  printf("Release\n");
		  release_address();
		} else if (mtype ==  DHCPDECLINE) {
		  printf("Decline\n");
		} else if (mtype == DHCPINFORM) {
		  printf("Inform\n");
		}
	      }
	    }
	  }
	}
      }
    } else if (uip_poll()) {
      
    }
  }
 exit:
  printf("DHCP server exiting\n");
  PROCESS_END();
}

void
dhcps_init(const struct dhcps_config *conf)
{
  config = conf;
  process_start(&dhcp_server_process,NULL);
}
