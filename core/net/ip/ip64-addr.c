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
#include "ip64-addr.h"

#include <stdio.h>
#include <string.h>

#define printf(...)
/*---------------------------------------------------------------------------*/
void
ip64_addr_copy4(uip_ip4addr_t *dest, const uip_ip4addr_t *src)
{
  memcpy(dest, src, sizeof(uip_ip4addr_t));
}
/*---------------------------------------------------------------------------*/
void
ip64_addr_copy6(uip_ip6addr_t *dest, const uip_ip6addr_t *src)
{
  memcpy(dest, src, sizeof(uip_ip6addr_t));
}
/*---------------------------------------------------------------------------*/
int
ip64_addr_4to6(const uip_ip4addr_t *ipv4addr,
	       uip_ip6addr_t *ipv6addr)
{
  /* This function converts an IPv4 addresses into an IPv6
     addresses. It returns 0 if it failed to convert the address and
     non-zero if it could successfully convert the address. */

  /* The IPv4 address is encoded as an IPv6-encoded IPv4 address in
     the ::ffff:0000/24 prefix.*/
  ipv6addr->u8[0] = 0;
  ipv6addr->u8[1] = 0;
  ipv6addr->u8[2] = 0;
  ipv6addr->u8[3] = 0;
  ipv6addr->u8[4] = 0;
  ipv6addr->u8[5] = 0;
  ipv6addr->u8[6] = 0;
  ipv6addr->u8[7] = 0;
  ipv6addr->u8[8] = 0;
  ipv6addr->u8[9] = 0;
  ipv6addr->u8[10] = 0xff;
  ipv6addr->u8[11] = 0xff;
  ipv6addr->u8[12] = ipv4addr->u8[0];
  ipv6addr->u8[13] = ipv4addr->u8[1];
  ipv6addr->u8[14] = ipv4addr->u8[2];
  ipv6addr->u8[15] = ipv4addr->u8[3];
  printf("ip64_addr_4to6: IPv6-encoded IPv4 address %d.%d.%d.%d\n",
	 ipv4addr->u8[0], ipv4addr->u8[1],
	 ipv4addr->u8[2], ipv4addr->u8[3]);

  /* Conversion succeeded, we return non-zero. */
  return 1;
}
/*---------------------------------------------------------------------------*/
int
ip64_addr_6to4(const uip_ip6addr_t *ipv6addr,
	       uip_ip4addr_t *ipv4addr)
{
  /* This function converts IPv6 addresses to IPv4 addresses. It
     returns 0 if it failed to convert the address and non-zero if it
     could successfully convert the address. */

  /* If the IPv6 address is an IPv6-encoded
     IPv4 address (i.e. in the ::ffff:0/8 prefix), we simply use the
     IPv4 addresses directly. */
  if(ipv6addr->u8[0] == 0 &&
     ipv6addr->u8[1] == 0 &&
     ipv6addr->u8[2] == 0 &&
     ipv6addr->u8[3] == 0 &&
     ipv6addr->u8[4] == 0 &&
     ipv6addr->u8[5] == 0 &&
     ipv6addr->u8[6] == 0 &&
     ipv6addr->u8[7] == 0 &&
     ipv6addr->u8[8] == 0 &&
     ipv6addr->u8[9] == 0 &&
     ipv6addr->u8[10] == 0xff &&
     ipv6addr->u8[11] == 0xff) {
    ipv4addr->u8[0] = ipv6addr->u8[12];
    ipv4addr->u8[1] = ipv6addr->u8[13];
    ipv4addr->u8[2] = ipv6addr->u8[14];
    ipv4addr->u8[3] = ipv6addr->u8[15];

    printf("ip64_addr_6to4: IPv6-encoded IPv4 address %d.%d.%d.%d\n",
	   ipv4addr->u8[0], ipv4addr->u8[1],
	   ipv4addr->u8[2], ipv4addr->u8[3]);

    /* Conversion succeeded, we return non-zero. */
    return 1;
  }
  /* We could not convert the IPv6 address, so we return 0. */
  return 0;
}
/*---------------------------------------------------------------------------*/
