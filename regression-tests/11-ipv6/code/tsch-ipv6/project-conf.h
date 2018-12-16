/*
 * Copyright (c) 2017, Yasuyuki Tanaka
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
 *
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

#undef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL 0

#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 1

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

/* Need NDP */
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA         1

#undef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT      1

#if CONTIKI_TARGET_Z1
/* Z1 specific part reducing consumed RAM */
/* Save some space to fit the limited RAM of the z1 */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                 0
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM            3
#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM         8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 2
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG         0
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED        0
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS   1
#endif /* CONTIKI_TARGET_Z1 */

#if CONTIKI_TARGET_COOJA
#define COOJA_CONF_SIMULATE_TURNAROUND 0
#endif /* CONTIKI_TARGET_COOJA */
