/*
 * Copyright (c) 2006, Technical University of Munich
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
 * Author: Simon Barner <barner@in.tum.de>
 *
 * @(#)$$
 */

#include "contiki-raven.h"

#include "zmac.h"
#include "sicslowpan.h"
extern uint64_t rndis_ethernet_addr;

void
init_net(void)
{


 
 
  /* Set local bit, Clear translate bit, Clear Multicast bit */
  macLongAddr &= ~(0x0700000000000000ULL);
  macLongAddr |=   0x0200000000000000ULL;
 

  /* Set the Ethernet address to the 15.4 MAC address */
  rndis_ethernet_addr =  macLongAddr;
  
  /* Remove the middle two bytes... */
  rndis_ethernet_addr = (rndis_ethernet_addr & 0xffffffUL) | ((rndis_ethernet_addr & 0xffffff0000000000ULL) >> 16);

  /* Change ieee802.15.4 address to correspond with what the ethernet's
     IPv6 address will be. This will have ff:fe in the middle.         */
  macLongAddr = (macLongAddr & 0xffffff0000ffffffULL) | (0x000000fffe000000ULL);

  ieee15_4ManagerAddress.set_long_addr(macLongAddr); 
}


