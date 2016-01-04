/*
 * Copyright (c) 2010, Loughborough University - Computer Science
 * Copyright (c) 2015, Singapore University of Technology and Design (SUTD)
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
 *         Project specific configuration defines for the sniffer example.
 *
 *         We make sure that the radio driver outputs all packets in hexdump
 *         format.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *         Ngo Van Mao      - <vanmao_ngo@sutd.edu.sg>
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#if CONTIKI_TARGET_Z1 || CONTIKI_TARGET_SKY
#undef CC2420_CONF_RF_SNIFFER
#define CC2420_CONF_RF_SNIFFER 1
#endif

#if CONTIKI_TARGET_OPENMOTE || CONTIKI_TARGET_ZOUL
#undef C2538_RF_CONF_SNIFFER
#define CC2538_RF_CONF_SNIFFER 1
#endif

// Inorder to build with differrent channel, e.g. 22, please use the following make:
// make TARGET=<target> DEFINES=CONF_SNIFFER_CHANNEL=22 ...
#if CONF_SNIFFER_CHANNEL
#undef RF_CHANNEL
#define RF_CHANNEL          CONF_SNIFFER_CHANNEL

#undef CC2420_CONF_CHANNEL
#define CC2420_CONF_CHANNEL CONF_SNIFFER_CHANNEL

#undef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL CONF_SNIFFER_CHANNEL
#endif // CONF_SNIFFER_CHANNEL

#if 1
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC      nullrdc_driver
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC      nullmac_driver
#else
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC      stub_rdc_driver
#endif //0

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

#undef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON     0
#undef LPM_CONF_MODE
#define LPM_CONF_MODE          0
#undef UART0_CONF_HIGH_SPEED
#define UART0_CONF_HIGH_SPEED  1

#undef UART0_CONF_RX_WITH_DMA
#define UART0_CONF_RX_WITH_DMA 1

#if CONTIKI_TARGET_SENSINODE // for sensinode only
#define CC2430_RF_CONF_HEXDUMP 1
#define CC2430_RF_CONF_AUTOACK 0
#endif

#endif /* PROJECT_CONF_H_ */
