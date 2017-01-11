/*
 * Copyright (c) 2017, Yasuyuki Tanaka
 * Copyright (c) 2015, SICS Swedish ICT.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *         Sample project-file.h for Contiki TSCH
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 *
 * \brief
 *         This file provide a sample project configuration using TSCH. Please
 *         refer to README.md under core/net/mac/tsch for further information.
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

/*-----------------------------*/
/* Project Local Configuration */
/*-----------------------------*/
/* Set 1 to WITH_SECURITY to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY       0
#endif /* WITH_SECURITY */

/* TSCH logging. 0: disabled. 1: basic log. 2: with logs in interrupt. */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 2

/* Do not start TSCH at init; needs to be started by an upper layer */
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0


/*------------------------------------------*/
/* IEEE 802.15.4/TSCH Network Configuration */
/*------------------------------------------*/
/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xabcd

/* 6TiSCH minimal schedule length.  The larger values, the less active slots */
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3


/*---------------------------------*/
/* Configuration Required for TSCH */
/*---------------------------------*/
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC        tschmac_driver

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC        nordc_driver

#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER     framer_802154

#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012


/*------------------------*/
/* Security Configuration */
/*------------------------*/
#if WITH_SECURITY
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED            1

#undef LLSEC802154_CONF_USES_EXPLICIT_KEYS
#define LLSEC802154_CONF_USES_EXPLICIT_KEYS 1

#undef LLSEC802154_CONF_USES_FRAME_COUNTER
#define LLSEC802154_CONF_USES_FRAME_COUNTER 0
#endif /* WITH_SECURITY */


/*-------------------*/
/* RPL Configuration */
/*-------------------*/
#define RPL_CALLBACK_PARENT_SWITCH    tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network


/*------------------------------------*/
/* Z1/Sky Platform Specific Configuration */
/*------------------------------------*/
#if CONTIKI_TARGET_Z1 || CONTIKI_TARGET_SKY

/* Save some space to fit the limited RAM of the z1 */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                 0

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM            3

#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM         8

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 8

#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA         0

#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG         0

#if WITH_SECURITY
/*
 * Note: on sky or z1 in cooja, crypto operations are done in S/W and cannot be
 * accommodated in normal slots. Use 65ms slots instead, and a very short 6TiSCH
 * minimal schedule length.
 */
#undef TSCH_CONF_DEFAULT_TIMESLOT_LENGTH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 65000

#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 2

/* Reduce log level to make space for security on z1 */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0
#endif /* WITH_SECURITY */

#endif /* CONTIKI_TARGET_Z1 || CONTIKI_TARGET_SKY */


/*----------------------------------------------*/
/* CC2538 based Platform Specific Configuration */
/*----------------------------------------------*/
#if CONTIKI_TARGET_CC2538DK || CONTIKI_TARGET_ZOUL || \
  CONTIKI_TARGET_OPENMOTE_CC2538

#define TSCH_CONF_HW_FRAME_FILTERING    0

/*
 * For TSCH we have to use the more accurate crystal oscillator by default the
 * RC oscillator is activated.
 */
#undef SYS_CTRL_CONF_OSC32K_USE_XTAL
#define SYS_CTRL_CONF_OSC32K_USE_XTAL 1

#endif /* CONTIKI_TARGET_CC2538DK || CONTIKI_TARGET_ZOUL \
       || CONTIKI_TARGET_OPENMOTE_CC2538 */


/*-------------------------------*/
/* CC2420 Specific Configuration */
/*-------------------------------*/
/* Disable DCO calibration (uses timerB) */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED            0

/* Enable SFD timestamps (uses timerB) */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS       1


/* ----------------------------------*/
/* Cooja Mote Specific Configuration */
/* ----------------------------------*/
#if CONTIKI_TARGET_COOJA
#define COOJA_CONF_SIMULATE_TURNAROUND 0
#endif /* CONTIKI_TARGET_COOJA */


#endif /* __PROJECT_CONF_H__ */
