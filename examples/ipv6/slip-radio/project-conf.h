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
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    140

#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 0

#undef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL               0

#define CMD_CONF_OUTPUT slip_radio_cmd_output

/* add the cmd_handler_cc2420 + some sensors if TARGET_SKY */
#ifdef CONTIKI_TARGET_SKY
#define CMD_CONF_HANDLERS slip_radio_cmd_handler,cmd_handler_cc2420
#define SLIP_RADIO_CONF_SENSORS slip_radio_sky_sensors
#else
#define CMD_CONF_HANDLERS slip_radio_cmd_handler
#endif


/* configuration for the slipradio/network driver */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     nullmac_driver

#undef NETSTACK_CONF_RDC
/* #define NETSTACK_CONF_RDC     nullrdc_noframer_driver */
#define NETSTACK_CONF_RDC     contikimac_driver

#undef NETSTACK_CONF_NETWORK
#define NETSTACK_CONF_NETWORK slipnet_driver

#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER no_framer

#undef CC2420_CONF_AUTOACK
#define CC2420_CONF_AUTOACK              1

#undef UART1_CONF_RX_WITH_DMA
#define UART1_CONF_RX_WITH_DMA           1

#endif /* __PROJECT_CONF_H__ */
