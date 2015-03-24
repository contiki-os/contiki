/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef IP64_ADDRMAP_H
#define IP64_ADDRMAP_H


#include "sys/timer.h"
#include "net/ip/uip.h"

struct ip64_addrmap_entry {
  struct ip64_addrmap_entry *next;
  struct timer timer;
  uip_ip6addr_t ip6addr;
  uip_ip4addr_t ip4addr;
  uint16_t mapped_port;
  uint16_t ip6port;
  uint16_t ip4port;
  uint8_t protocol;
  uint8_t flags;
};

#define FLAGS_NONE       0
#define FLAGS_RECYCLABLE 1

/**
 * Initialize the ip64_addrmap module.
 */
void ip64_addrmap_init(void);


/**
 * Look up an address mapping from inside the IPv6 network, given the
 * IPv6 address/port, the IPv4 address/port, and the protocol.
 */
struct ip64_addrmap_entry *ip64_addrmap_lookup(const uip_ip6addr_t *ip6addr,
					       uint16_t ip6port,
					       const uip_ip4addr_t *ip4addr,
					       uint16_t ip4port,
					       uint8_t protocol);

/**
 * Look up an address mapping from the outside IPv4 network, given the
 * mapped port number and protocol.
 */
struct ip64_addrmap_entry *ip64_addrmap_lookup_port(uint16_t mappedport,
						    uint8_t protocol);

/**
 * Create a new address mapping from an IPv6 address/port, an IPv4
 * address/port, and a protocol number.
 */
struct ip64_addrmap_entry *ip64_addrmap_create(const uip_ip6addr_t *ip6addr,
					       uint16_t ip6port,
					       const uip_ip4addr_t *ip4addr,
					       uint16_t ip4port,
					       uint8_t protocol);

/**
 * Set the lifetime of an address mapping.
 */
void ip64_addrmap_set_lifetime(struct ip64_addrmap_entry *e,
                               clock_time_t lifetime);

/**
 * Mark an address mapping to be recyclable.
 */
void ip64_addrmap_set_recycleble(struct ip64_addrmap_entry *e);

/**
 * Obtain the list of all address mappings.
 */
struct ip64_addrmap_entry *ip64_addrmap_list(void);
#endif /* IP64_ADDRMAP_H */
