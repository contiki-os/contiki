/**
 * \file sicslow_ethernet.c
 *         Routines to interface between Ethernet and 6LowPan
 *
 * \author
 *         Colin O'Flynn <coflynn@newae.com>
 *
 * \addtogroup usbstick 
 */

/* Copyright (c) 2008 by:

	Colin O'Flynn coflynn@newae.com
	Eric Gnoske egnoske@gmail.com
	Blake Leverett bleverett@gmail.com
	Mike Vidales mavida404@gmail.com
	Kevin Brown kbrown3@uccs.edu
	Nate Bohlmann nate@elfwerks.com

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SICSLOW_ETHERNET_H
#define SICSLOW_ETHERNET_H

#if !RF230BB
#include "sicslowmac.h"
#endif

typedef enum {
  ll_802154_type,
  ll_8023_type
} lltype_t;

#define PREFIX_BUFFER_SIZE 32

extern uint8_t prefixCounter;
extern uint8_t prefixBuffer[PREFIX_BUFFER_SIZE][3];


#define UIP_ETHTYPE_802154 0x809A

int8_t mac_translateIcmpLinkLayer(lltype_t target);
int8_t mac_translateIPLinkLayer(lltype_t target);
void mac_LowpanToEthernet(uip_lladdr_t * lowpan, uint8_t * ethHeader);
void mac_ethernetToLowpan(uint8_t * ethHeader);
void mac_ethernetSetup(void);

uint8_t mac_createSicslowpanLongAddr(uint8_t * ethernet,
                                     uip_lladdr_t * lowpan);
uint8_t mac_createEthernetAddr(uint8_t * ethernet, uip_lladdr_t * lowpan);
uint8_t mac_createDefaultEthernetAddr(uint8_t * ethernet);

#endif
