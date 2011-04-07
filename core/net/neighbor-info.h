/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: neighbor-info.h,v 1.4 2010/10/22 13:50:13 nvt-se Exp $
 */
/**
 * \file
 *         Declarations for the neighbor information module.
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef NEIGHBOR_INFO_H
#define NEIGHBOR_INFO_H

#include "net/rime.h"

/* ETX_DIVISOR is the value that a fix-point representation of the ETX 
   should be divided by in order to obtain the integer representation. */
#define NEIGHBOR_INFO_ETX_DIVISOR       16

/* Macros for converting between a fix-point representation of the ETX 
   and a integer representation. */
#define NEIGHBOR_INFO_ETX2FIX(etx)    ((etx) * NEIGHBOR_INFO_ETX_DIVISOR)
#define NEIGHBOR_INFO_FIX2ETX(fix)    ((fix) / NEIGHBOR_INFO_ETX_DIVISOR)

typedef void (*neighbor_info_subscriber_t)(const rimeaddr_t *, int known, int etx);
typedef uint8_t link_metric_t;

/**
 * Notify the neighbor information module about the status of
 * a packet transmission.
 *
 * \param status The MAC status code for this packet.
 *
 * \param numtx The amount of transmissions made for this packet.
 */
void neighbor_info_packet_sent(int status, int numtx);

/**
 * Notify the neighbor information module that a packet was received.
 *
 * \param status The MAC status code for this packet.
 *
 * \param numtx The amount of transmissions made for this packet.
 */
void neighbor_info_packet_received(void);

/**
 * Subscribe to notifications of changed neighbor information.
 *
 * \return Returns 1 if the subscription was successful, and 0 if not.
 */
int neighbor_info_subscribe(neighbor_info_subscriber_t);


/**
 * Get link metric value for a specific neighbor.
 *
 * \return Returns link metric if the neighbor exists, and 0 if not.
 */
link_metric_t neighbor_info_get_metric(const rimeaddr_t *addr);

#endif /* NEIGHBOR_INFO_H */
