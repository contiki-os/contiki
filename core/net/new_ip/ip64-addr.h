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
#ifndef IP64_ADDR_H
#define IP64_ADDR_H

#include "net/ip/uip.h"


/**
 * \brief Is IPv4-mapped Address
 *
 * See https://tools.ietf.org/html/rfc6890#page-14
 */
#define ip64_addr_is_ipv4_mapped_addr(a) \
  ((((a)->u16[0])  == 0) &&              \
   (((a)->u16[1])  == 0) &&              \
   (((a)->u16[2])  == 0) &&              \
   (((a)->u16[3])  == 0) &&              \
   (((a)->u16[4])  == 0) &&              \
   (((a)->u16[5])  == 0xFFFF))

void ip64_addr_copy4(uip_ip4addr_t *dest, const uip_ip4addr_t *src);

void ip64_addr_copy6(uip_ip6addr_t *dest, const uip_ip6addr_t *src);

int ip64_addr_6to4(const uip_ip6addr_t *ipv6addr,
		   uip_ip4addr_t *ipv4addr);

int ip64_addr_4to6(const uip_ip4addr_t *ipv4addr,
		   uip_ip6addr_t *ipv6addr);

int ip64_addr_is_ip64(const uip_ip6addr_t *ipv6addr);

void ip64_addr_set_prefix(const uip_ip6addr_t *prefix, uint8_t prefix_len);

#endif /* IP64_ADDR_H */

