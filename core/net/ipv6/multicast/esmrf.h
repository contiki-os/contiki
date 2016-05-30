/*
 * Copyright (c) 2011, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Header file for the Enhanced Stateless Multicast RPL Forwarding (ESMRF)
 *
 * \author
 *         Khaled Qorany	kqorany2@gmail.com
 */

#ifndef ESMRF_H_
#define ESMRF_H_

#include "contiki-conf.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* Protocol Constants */
/*---------------------------------------------------------------------------*/
#define ESMRF_ICMP_CODE              0   /* ICMPv6 code field */
#define ESMRF_IP_HOP_LIMIT        0xFF   /* Hop limit for ICMP messages */
/*---------------------------------------------------------------------------*/
/* Configuration */
/*---------------------------------------------------------------------------*/
/* Fmin */
#ifdef ESMRF_CONF_MIN_FWD_DELAY
#define ESMRF_MIN_FWD_DELAY ESMRF_CONF_MIN_FWD_DELAY
#else
#define ESMRF_MIN_FWD_DELAY 4
#endif

/* Max Spread */
#ifdef ESMRF_CONF_MAX_SPREAD
#define ESMRF_MAX_SPREAD ESMRF_CONF_MAX_SPREAD
#else
#define ESMRF_MAX_SPREAD 4
#endif
/*---------------------------------------------------------------------------*/
/* Stats datatype */
/*---------------------------------------------------------------------------*/
struct esmrf_stats {
  uint16_t mcast_in_unique;
  uint16_t mcast_in_all;        /* At layer 3 */
  uint16_t mcast_in_ours;       /* Unique and we are a group member */
  uint16_t mcast_fwd;           /* Forwarded by us but we are not the seed */
  uint16_t mcast_out;           /* We are the seed */
  uint16_t mcast_bad;
  uint16_t mcast_dropped;
  uint16_t icmp_out;
  uint16_t icmp_in;
  uint16_t icmp_bad;
};

#endif /* ESMRF_H_ */
