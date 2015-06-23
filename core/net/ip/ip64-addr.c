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
static int
is_private_ipv4addr(const uip_ip4addr_t *ipv4addr)
{
  uip_ipaddr_t ipaddr, mask;

  /* 10.0.0.0/8 */
  uip_ipaddr(&ipaddr, 10, 0, 0, 0);
  uip_ipaddr(&mask, 255, 0, 0, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 172.16.0.0/12 */
  uip_ipaddr(&ipaddr, 172, 16, 0, 0);
  uip_ipaddr(&mask, 255, 240, 0, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 198.18.0.0/15 */
  uip_ipaddr(&ipaddr, 198, 18, 0, 0);
  uip_ipaddr(&mask, 255, 254, 0, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 192.168.0.0/16 */
  uip_ipaddr(&ipaddr, 192, 168, 0, 0);
  uip_ipaddr(&mask, 255, 255, 0, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 192.0.2.0/24 */
  uip_ipaddr(&ipaddr, 192, 0, 2, 0);
  uip_ipaddr(&mask, 255, 255, 255, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 198.51.100.0/24 */
  uip_ipaddr(&ipaddr, 198, 51, 100, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  /* 203.0.113.0/24 */
  uip_ipaddr(&ipaddr, 203, 0, 113, 0);
  if(uip_ipaddr_maskcmp(ipv4addr, &ipaddr, &mask)) {
    return 1;
  }

  return 0;
}
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
  } else if(ipv6addr->u8[0] == 00 &&
            ipv6addr->u8[1] == 0x64 &&
            ipv6addr->u8[2] == 0xff &&
            ipv6addr->u8[3] == 0x9b &&
            ipv6addr->u8[4] == 0 &&
            ipv6addr->u8[5] == 0 &&
            ipv6addr->u8[6] == 0 &&
            ipv6addr->u8[7] == 0 &&
            ipv6addr->u8[8] == 0 &&
            ipv6addr->u8[9] == 0 &&
            ipv6addr->u8[10] == 0 &&
            ipv6addr->u8[11] == 0) {

    /*
     * Handle IPv6 addresses using the "Well-Know Prefix" 64:ff9b::/96.
     * Not allowed for private (RFC1918) IPv4 addresses as per RFC 6052.
     */

    ipv4addr->u8[0] = ipv6addr->u8[12];
    ipv4addr->u8[1] = ipv6addr->u8[13];
    ipv4addr->u8[2] = ipv6addr->u8[14];
    ipv4addr->u8[3] = ipv6addr->u8[15];

    if(!is_private_ipv4addr(ipv4addr)) {
      printf("ip64_addr_6to4: IPv6-encoded IPv4 address %d.%d.%d.%d\n",
             ipv4addr->u8[0], ipv4addr->u8[1],
             ipv4addr->u8[2], ipv4addr->u8[3]);
      /* Conversion succeeded, we return non-zero. */
      return 1;
    } else {
      /* WKP not allowed for private IPv4 addresses. Drop the packet */
      printf("ip64_addr_6to4: WKP not allowed for private IPv4 address\n");
    }
  }
  /* We could not convert the IPv6 address, so we return 0. */
  return 0;
}
/*---------------------------------------------------------------------------*/
