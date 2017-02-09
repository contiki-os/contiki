/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
/*---------------------------------------------------------------------------*/
#ifndef TARGET_CONF_H_
#define TARGET_CONF_H_
/*---------------------------------------------------------------------------*/
/* Set to 1 for a sub-ghz sniffer with the CC1200 */
#ifndef ZOUL_CONF_SUB_GHZ_SNIFFER
#define ZOUL_CONF_SUB_GHZ_SNIFFER     0
#endif
/*---------------------------------------------------------------------------*/
#if ZOUL_CONF_SUB_GHZ_SNIFFER
#define NETSTACK_CONF_RADIO           cc1200_driver

/*
 * You will need to configure the defines below to match the configuration of
 * your sub-ghz network.
 */
#define CC1200_CONF_RF_CFG            cc1200_802154g_863_870_fsk_50kbps
#define CC1200_CONF_USE_GPIO2         0
#define CC1200_CONF_USE_RX_WATCHDOG   0
#define CC1200_CONF_802154G           0
#define CC1200_CONF_802154G_CRC16     0
#define CC1200_CONF_802154G_WHITENING 0
#define ANTENNA_SW_SELECT_DEF_CONF    ANTENNA_SW_SELECT_SUBGHZ
#endif
/*---------------------------------------------------------------------------*/
/*
 * Selection of Sensniff I/O Interface.
 * Define CC2538_IO_CONF_USB as 0 to use UART0 as sensniff's interface.
 */
#define CC2538_IO_CONF_USB            0
/*---------------------------------------------------------------------------*/
#if CC2538_IO_CONF_USB
#define USB_SERIAL_CONF_ENABLE        1
#else
#define UART0_CONF_BAUD_RATE     460800
#endif
/*---------------------------------------------------------------------------*/
#define SENSNIFF_IO_DRIVER_H "pool/cc2538-io.h"
/*---------------------------------------------------------------------------*/
#endif /* TARGET_CONF_H_ */
/*---------------------------------------------------------------------------*/
