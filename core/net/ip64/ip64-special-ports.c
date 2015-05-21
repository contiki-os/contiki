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
#include "ip64.h"
#include "ip64-special-ports.h"

#ifndef IP64_SPECIAL_PORTS_CONF_ENABLE
#define EMPTY_DEFINITIONS 1
#else
#if IP64_SPECIAL_PORTS_CONF_ENABLE == 0
#define EMPTY_DEFINITIONS 1
#endif /* IP64_SPECIAL_PORTS_CONF_ENABLE */
#endif /* IP64_SPECIAL_PORTS_CONF_ENABLE */



#if EMPTY_DEFINITIONS
/*---------------------------------------------------------------------------*/
int
ip64_special_ports_translate_incoming(uint16_t incoming_port,
				      uip_ip6addr_t *newaddr,
				      uint16_t *newport)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ip64_special_ports_translate_outgoing(uint16_t incoming_port,
				      const uip_ip6addr_t *ip6addr,
				      uint16_t *newport)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ip64_special_ports_incoming_is_special(uint16_t port)
{
  /* Default is to have no special ports. */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ip64_special_ports_outgoing_is_special(uint16_t port)
{
  /* Default is to have no special ports. */
  return 0;
}
/*---------------------------------------------------------------------------*/
#endif /* EMPTY_DEFINITIONS */
