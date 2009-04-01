/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: ip.c,v 1.2 2009/04/01 18:11:48 fros4943 Exp $
 */

#include "dev/ip.h"
#include "lib/simEnvChange.h"

#include "net/uip.h"

const struct simInterface ip_interface;

// COOJA variables

#if UIP_CONF_IPV6

char simIPChanged;
char simIP[16];

#endif /* UIP_CONF_IPV6 */

#if WITH_UIP

char simIPChanged;
char simIP[4];

#endif /* WITH_UIP */

/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
#if UIP_CONF_IPV6

  /* check if IPv6 address should change */

#endif /* UIP_CONF_IPV6 */

#if WITH_UIP

  /* check if IPv4 address should change */
/*
  if (simIPChanged) {
    uip_ipaddr_t hostaddr;
    uip_ipaddr(&hostaddr, simIP[0], simIP[1], simIP[2], simIP[3]);
    uip_sethostaddr(&hostaddr);
    simIPChanged = 0;
  }
*/

#endif /* WITH_UIP */
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(ip_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
