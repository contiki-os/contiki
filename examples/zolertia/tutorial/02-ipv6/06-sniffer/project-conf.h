/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID      0xABCD

#if CONTIKI_TARGET_ZOUL

/* The following are Zoul (RE-Mote, etc) specific */
#undef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL     26

#define CC2538_RF_CONF_SNIFFER     1
#define CC2538_RF_CONF_AUTOACK     0
#define UART0_CONF_BAUD_RATE       460800

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC          stub_rdc_driver

#else /* Default is Z1 */

/* The following are Z1 specific */
#undef RF_CHANNEL
#define RF_CHANNEL	           26

#undef CC2420_CONF_CHANNEL
#define CC2420_CONF_CHANNEL        26

#undef CC2420_CONF_AUTOACK
#define CC2420_CONF_AUTOACK        0
#undef CC2420_CONF_HEXDUMP
#define CC2420_CONF_HEXDUMP        1

#undef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON         0
#undef LPM_CONF_MODE
#define LPM_CONF_MODE              0
#undef UART0_CONF_HIGH_SPEED
#define UART0_CONF_HIGH_SPEED      1

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC          nullrdc_driver

#endif /* CONTIKI_TARGET_ZOUL */

#endif /* PROJECT_CONF_H_ */
