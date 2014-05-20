/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         ORPL-specific configuration of Contiki
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */


#ifndef __ORPL_CONTIKI_CONF_H__
#define __ORPL_CONTIKI_CONF_H__

/* Used for modification of core files */
#define WITH_ORPL 1

/* EDC is the objective function used by ORPL */
#undef RPL_CONF_OF
#define RPL_CONF_OF rpl_of_edc

/* ORPL does not have such notion of min hop rank increase */
#undef RPL_CONF_MIN_HOPRANKINC
#define RPL_CONF_MIN_HOPRANKINC 0

/* ORPL does not use the DIO redundancy mechanism */
#define RPL_CONF_DIO_REDUNDANCY   200  /* default 10 */

/* The current ORPL implementation assumes a single instance */
#define RPL_CONF_MAX_INSTANCES    1 /* default 1 */

/* The current ORPL implementation assumes a single DAG */
#define RPL_CONF_MAX_DAG_PER_INSTANCE 1 /* default 2 */

/* ORPL runs without IPv6 NA/ND */
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA 0

/* ORPL does not use RPL's normal downwards routing */
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_NO_DOWNWARD_ROUTES

/* ORPL does not use traditional routing entries */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  0

/* ORPL is not compatible with ContikiMAC phase-lock */
#undef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0

/* ORPL is not compatible with sending bursts, we therefore
 * set the number of CCA before transmitting to 2 only */
#undef CONTIKIMAC_CONF_CCA_COUNT_MAX_TX
#define CONTIKIMAC_CONF_CCA_COUNT_MAX_TX 2 /* default 6 */

/* Our softack implementation for cc2420 requires to disable DCO synch */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED 0

/* Our softack implementation for cc2420 requires SFD timestamps */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS 1

/* Contiki netstack: MAC */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver

/* Contiki netstack: RDC */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_orpl_driver

/* Contiki netstack: RADIO */
#undef NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO   cc2420_softack_driver

/* ORPL Callbacks for cc2420 softacks */
#define SOFTACK_ACKED_CALLBACK orpl_softack_acked_callback
#define SOFTACK_INPUT_CALLBACK orpl_softack_input_callback

/* Contiki netstack: FRAMER */
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* Disable compression threshold for consistent and predictable compression */
#undef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 0

/* Enable ContikiMAC header for MAC padding */
#undef CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER 1

#endif /* __ORPL_CONTIKI_CONF_H__ */
