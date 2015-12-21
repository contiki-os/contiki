/*
 * Copyright (c) 2015, SICS Swedish ICT.
 * Copyright (c) 2015, Singapore University of Technology and Design
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
 *         Ngo Van Mao <vanmao_ngo@sutd.edu.sg>
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

// Set  some parameter for this applications
#define PERIOD 5

#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON 1
#endif

#undef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL              22

/* IEEE802.15.4 PANID for CSMA or contikiMac*/
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xBABE

// TODO: should put in platform. If not, we have to put it everywhere to get RTIMER 32768
#define RTIMER_CONF_USE_32KHZ  1

// In order to make with TSCH mode, just run below command:
// make TARGET=<target> DEFINES=CONF_RUN_TSCH=1 ...
#if CONF_RUN_TSCH
/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xCAFE

/* Set to run orchestra */
#ifndef WITH_ORCHESTRA
#define WITH_ORCHESTRA 0
#endif /* WITH_ORCHESTRA */

/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

/*******************************************************/
/********************* Enable TSCH *********************/
/*******************************************************/

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* TSCH and RPL callbacks */
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network


/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/* TSCH logging. 0: disabled. 1: basic log. 2: with delayed
 * log messages from interrupt */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0
/* Reduce queue length for Zoul limitted ram */
#define TSCH_LOG_CONF_QUEUE_LEN 2


/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 1

/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3

#if WITH_SECURITY
/* Enable security */
#undef LLSEC802154_CONF_SECURITY_LEVEL
#define LLSEC802154_CONF_SECURITY_LEVEL 1
/* TSCH uses explicit keys to identify k1 and k2 */
#undef LLSEC802154_CONF_USES_EXPLICIT_KEYS
#define LLSEC802154_CONF_USES_EXPLICIT_KEYS 1
/* TSCH uses the ASN rather than frame counter to construct the Nonce */
#undef LLSEC802154_CONF_USES_FRAME_COUNTER
#define LLSEC802154_CONF_USES_FRAME_COUNTER 0
#endif /* WITH_SECURITY */

#if WITH_ORCHESTRA
/* See apps/orchestra/README.md for more Orchestra configuration options */
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0 /* No 6TiSCH minimal schedule */
#define TSCH_CONF_WITH_LINK_SELECTOR 1 /* Orchestra requires per-packet link selection */
/* Orchestra callbacks */
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed
#endif /* WITH_ORCHESTRA */

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/
/*========================Z1===========================*/
#if CONTIKI_TARGET_Z1
/* Needed for cc2420 platforms only */
/* Disable DCO calibration (uses timerB) */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED            0
/* Enable SFD timestamps (uses timerB) */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS       1
/* Save some space to fit the limited RAM of the z1 */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 4
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 8
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA 0
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG 0

#if WITH_SECURITY
/* Note: on sky or z1 in cooja, crypto operations are done in S/W and
 * cannot be accommodated in normal slots. Use 65ms slots instead, and
 * a very short 6TiSCH minimal schedule length */
#undef TSCH_CONF_DEFAULT_TIMESLOT_LENGTH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 65000
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 2
/* Reduce log level to make space for security on z1 */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0
#endif /* WITH_SECURITY */

#endif /* CONTIKI_TARGET_Z1 */
/*========================Z1===========================*/


/*==========================CC2538=================================*/
#if CONTIKI_TARGET_OPENMOTE || CONTIKI_TARGET_ZOUL
/* Enable SFD timestamps */
#undef CC2538_CONF_SFD_TIMESTAMPS
#define CC2538_CONF_SFD_TIMESTAMPS       1

#define SYS_CTRL_CONF_OSC32K_USE_XTAL 1
#define RTIMER_CONF_USE_32KHZ  1
#define SYS_CTRL_CONF_SYS_DIV SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ
#define TSCH_CONF_AUTOSELECT_TIME_SOURCE 1

// For testing purpose, we should use single channel for sniffering.
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_1_1

//#define TSCH_CONF_RESYNC_WITH_SFD_TIMESTAMPS 1
//#define TSCH_CONF_KEEPALIVE_TIMEOUT (5 * CLOCK_SECOND)
//#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
//#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 11
//#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0
#endif /* CONTIKI_TARGET_OPENMOTE || CONTIKI_TARGET_ZOUL */

/*==========================CC2538=================================*/
#endif /* CONF_RUN_TSCH */

#endif /* __PROJECT_CONF_H__ */
