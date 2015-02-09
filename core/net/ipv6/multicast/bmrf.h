/*
 * Copyright (c) 2014, VUB - ETRO
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
 *         Header file for 'Bidirectional Multicast RPL Forwarding' (BMRF)
 *
 * \author
 *         Guillermo Gastón Lorente
 */

#ifndef BMRF_H_
#define BMRF_H_

#include "contiki-conf.h"

#include <stdint.h>

#define BMRF_UNICAST_MODE      0
#define BMRF_BROADCAST_MODE    1
#define BMRF_MIXED_MODE        2

/*---------------------------------------------------------------------------*/
/* Configuration */
/*---------------------------------------------------------------------------*/
/* LL forwarding mode */
#ifdef BMRF_CONF_MODE
#define BMRF_MODE              BMRF_CONF_MODE
#else
#define BMRF_MODE              BMRF_MIXED_MODE
#endif /* BMRF_CONF_MODE */

#if BMRF_MODE == BMRF_MIXED_MODE
#ifdef BMRF_CONF_BROADCAST_THRESHOLD
#define BMRF_BROADCAST_THRESHOLD    BMRF_CONF_BROADCAST_THRESHOLD
#else
#define BMRF_BROADCAST_THRESHOLD    3
#endif /* BMRF_CONF_BROADCAST_THRESHOLD */
#endif /* BMRF_MODE */

/* Fmin */
#ifdef BMRF_CONF_MIN_FWD_DELAY
#define BMRF_MIN_FWD_DELAY BMRF_CONF_MIN_FWD_DELAY
#else
#define BMRF_MIN_FWD_DELAY 4
#endif

/* Max Spread */
#ifdef BMRF_CONF_MAX_SPREAD
#define BMRF_MAX_SPREAD BMRF_CONF_MAX_SPREAD
#else
#define BMRF_MAX_SPREAD 4
#endif

/*---------------------------------------------------------------------------*/
/* Stats datatype */
/*---------------------------------------------------------------------------*/
struct bmrf_stats {
  UIP_MCAST6_STATS_DATATYPE bmrf_fwd_brdcst; /* Forwarded by us with LL Broadcast*/
  UIP_MCAST6_STATS_DATATYPE bmrf_fwd_uncst;  /* Forwarded by us with LL Unicast*/
};

#endif /* BMRF_H_ */
