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
#ifndef IP64_SPECIAL_PORTS_H
#define IP64_SPECIAL_PORTS_H

/* The IP64 special ports module allows specific ports on the IP64
   translator to be mapped to specific address in the IPv6
   network. The module provides three function prototypes that must be
   implemented by the user.

   The IP64 special ports module must be enabled by

#define IP64_SPECIAL_PORTS_CONF_ENABLE 1

   Otherwise, the functions are replaced by empty default definitions
   in the ip64-special-ports.c module.

   Port numbers are always in network byte order. */


/* Translate the special port to an IPv6 address for inbound
   packets. */
int ip64_special_ports_translate_incoming(uint16_t incoming_port,
					  uip_ip6addr_t *newaddr,
					  uint16_t *newport);
int ip64_special_ports_translate_outgoing(uint16_t incoming_port,
					  const uip_ip6addr_t *ip6addr,
					  uint16_t *newport);
/* Check if an incoming (destination) port is special. */
int ip64_special_ports_incoming_is_special(uint16_t port);

/* Check if an outgoing (source) port is special. */
int ip64_special_ports_outgoing_is_special(uint16_t port);


#endif /* IP64_SPECIAL_PORTS_H */
