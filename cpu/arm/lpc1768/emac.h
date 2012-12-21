/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
 */

#ifndef __EMAC_H
#define __EMAC_H

#include "lpc_types.h"
#include "lpc17xx_emac.h"
#include "contiki-conf.h"

#define EMAC_MAX_PACKET_SIZE (UIP_CONF_BUFFER_SIZE + 16)	// 1536 bytes
#define ENET_DMA_DESC_NUMB   	3
#define AUTO_NEGOTIATION_ENA 	1  		// Enable PHY Auto-negotiation
#define PHY_TO               	200000  // ~10sec
#define RMII                    1		// If zero, it's a MII interface

/* Configurable macro ---------------------- */
#define SPEED_100               1
#define SPEED_10                0
#define FULL_DUPLEX             1
#define HALF_DUPLEX             0

#define FIX_SPEED               SPEED_100
#define FIX_DUPLEX              FULL_DUPLEX

BOOL_8
tapdev_init(void);
UNS_32
tapdev_read(void * pPacket);
BOOL_8
tapdev_send(void *pPacket, UNS_32 size);

#endif
