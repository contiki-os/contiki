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
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __COMMON_CONF_H__
#define __COMMON_CONF_H__

/* Global config flags */

#define WITH_TSCH 1
#define WITH_TSCH_SECURITY 0
#define TSCH_LOG_CONF_LEVEL 2
#define WITH_COAP_RESOURCES 0

#undef ENABLE_COOJA_DEBUG
#define ENABLE_COOJA_DEBUG 0

#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x5254

#undef TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
#define TSCH_HOPPING_SEQUENCE_MY_SEQUENCE (uint8_t[]){17, 23, 15, 25, 19, 11, 13, 21}
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_MY_SEQUENCE

#undef TSCH_CONF_JOIN_MY_PANID_ONLY
#define TSCH_CONF_JOIN_MY_PANID_ONLY 1

#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0

#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval

/* RPL Trickle timer tuning */
#undef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_CONF_DIO_INTERVAL_MIN 12 /* 4.096 s */

#undef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_CONF_DIO_INTERVAL_DOUBLINGS 2 /* Max factor: x4. 4.096 s * 4 = 16.384 s */

#define TSCH_CONF_EB_PERIOD (4 * CLOCK_SECOND)
#define TSCH_CONF_KEEPALIVE_TIMEOUT (24 * CLOCK_SECOND)

#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0
#define TSCH_CONF_WITH_LINK_SELECTOR 1
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed

/* Dimensioning */
#define ORCHESTRA_CONF_EBSF_PERIOD                     41
#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD             7 /* Common shared slot, 7 is a very short slotframe (high energy, high capacity). Must be prime and at least equal to number of nodes (incl. BR) */
#define ORCHESTRA_CONF_UNICAST_PERIOD                  11 /* First prime greater than 10 */

/* Use sender-based slots */
#define ORCHESTRA_CONF_UNICAST_SENDER_BASED 1
/* Our "hash" is collision-free */
#define ORCHESTRA_CONF_COLLISION_FREE_HASH 1
/* Max hash value */
#define ORCHESTRA_CONF_MAX_HASH (ORCHESTRA_CONF_UNICAST_PERIOD - 1)

/* RPL probing */
#define RPL_CONF_PROBING_INTERVAL (5 * CLOCK_SECOND)
#define RPL_CONF_PROBING_EXPIRATION_TIME (2 * 60 * CLOCK_SECOND)

/* CoAP */

#undef COAP_SERVER_PORT
#define COAP_SERVER_PORT 5684

#undef COAP_OBSERVE_RETURNS_REPRESENTATION
#define COAP_OBSERVE_RETURNS_REPRESENTATION 1

/* RPL */
#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 1

/* RPL storing mode */
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_STORING_NO_MULTICAST

/* Default link metric */
#undef RPL_CONF_INIT_LINK_METRIC
#define RPL_CONF_INIT_LINK_METRIC 2 /* default 5 */

#define RPL_CONF_MAX_INSTANCES    1 /* default 1 */
#define RPL_CONF_MAX_DAG_PER_INSTANCE 1 /* default 2 */

/* No RA, No NA */
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA 0

#undef UIP_CONF_ND6_SEND_RA
#define UIP_CONF_ND6_SEND_RA 0

#undef UIP_CONF_TCP
#define UIP_CONF_TCP             0
#undef UIP_CONF_DS6_ADDR_NBU
#define UIP_CONF_DS6_ADDR_NBU    1
#undef UIP_CONF_FWCACHE_SIZE
#define UIP_CONF_FWCACHE_SIZE    1
#undef UIP_CONF_UDP_CHECKSUMS
#define UIP_CONF_UDP_CHECKSUMS   1

/* Link-layer security */

/* Even when link-layer security is needed, we do not use a LLSEC layer, as it does not
 * allow to secure MAC-layer packets, nor can run encrypt/decrupt from interrupt.
 * Instead, we call AES-CCM* primitives directly from TSCH */
#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC nullsec_driver

#if WITH_TSCH_SECURITY
/* Set security level to the maximum, even if unused, to all crypto code */
#define LLSEC802154_CONF_SECURITY_LEVEL 7
/* Attempt to associate from both secured and non-secured EBs */
#define TSCH_CONF_JOIN_SECURED_ONLY 0
/* We need explicit keys to identify k1 and k2 */
#undef LLSEC802154_CONF_USES_EXPLICIT_KEYS
#define LLSEC802154_CONF_USES_EXPLICIT_KEYS 1
/* TSCH uses the ASN to construct the Nonce */
#undef LLSEC802154_CONF_USES_FRAME_COUNTER
#define LLSEC802154_CONF_USES_FRAME_COUNTER 0
#endif /* WITH_TSCH_SECURITY */

#if WITH_TSCH

#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

/* Contiki netstack: MAC */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver

/* Contiki netstack: RDC */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver

#else /* No TSCH, use Csma+NullRDC with ACK */

/* Contiki netstack: MAC */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver

/* Contiki netstack: RDC */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver

#undef RF_CHANNEL
#define RF_CHANNEL 26

#undef MICROMAC_CONF_AUTOACK
#define MICROMAC_CONF_AUTOACK 1

/* increase internal radio buffering */
#undef MIRCOMAC_CONF_BUF_NUM
#define MIRCOMAC_CONF_BUF_NUM 4

#endif

#undef CONTIKI_VERSION_STRING
#define CONTIKI_VERSION_STRING "Contiki 3.x"

#include "common-conf-jn516x.h"

#endif /* __COMMON_CONF_H__ */
