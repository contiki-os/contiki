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
 * $Id: project-conf.h,v 1.1 2011/09/28 13:11:08 simonduq Exp $
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#ifndef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#endif

/* Save some memory */
#define UIP_CONF_TCP                    0
#undef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU            12
#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU          12

/* Tmote Sky's serial works better with DMA */
#define UART1_CONF_RX_WITH_DMA          1

/* AH and ESP can be enabled/disabled independently */
#define WITH_IPSEC_AH              0
#define WITH_IPSEC_ESP             1
#define WITH_IPSEC    (WITH_IPSEC_ESP | WITH_IPSEC_AH)

/* Configuring an AES implementation */
#define CRYPTO_CONF_AES cc2420_aes
/* Configuring a cipher block mode of operation (encryption/decryption) */
#define IPSEC_CONF_BLOCK aesctr
/* Configuring a cipher block MAC mode of operation (authentication) */
#define IPSEC_CONF_MAC aesxcbc_mac

#undef RF_CHANNEL
#define RF_CHANNEL 26

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_driver

//#define NETSTACK_CONF_RDC     nullrdc_driver
/* Enable nullrdc autoack handling */
//#undef NULLRDC_CONF_802154_AUTOACK
//#define NULLRDC_CONF_802154_AUTOACK 1

#endif /* __PROJECT_H__ */
