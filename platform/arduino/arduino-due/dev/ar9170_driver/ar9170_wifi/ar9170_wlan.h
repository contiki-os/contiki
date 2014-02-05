/*
 * Atheros CARL9170 driver
 *
 * Copyright 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2009, 2010, Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, see
 * http://www.gnu.org/licenses/.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *    Copyright (c) 2007-2008 Atheros Communications, Inc.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef AR9170_WLAN_H_
#define AR9170_WLAN_H_

#include "fwcmd.h"
#include "compiler.h"
#include "ieee80211.h"

#define	AR9170_RX_PHY_RATE_CCK_1M		0x0a
#define	AR9170_RX_PHY_RATE_CCK_2M		0x14
#define	AR9170_RX_PHY_RATE_CCK_5M		0x37
#define	AR9170_RX_PHY_RATE_CCK_11M		0x6e

#define	AR9170_ENC_ALG_NONE			0x0
#define	AR9170_ENC_ALG_WEP64			0x1
#define	AR9170_ENC_ALG_TKIP			0x2
#define	AR9170_ENC_ALG_AESCCMP			0x4
#define	AR9170_ENC_ALG_WEP128			0x5
#define	AR9170_ENC_ALG_WEP256			0x6
#define	AR9170_ENC_ALG_CENC			0x7

#define	AR9170_RX_ENC_SOFTWARE			0x8

#define	AR9170_RX_STATUS_MODULATION		0x03
#define	AR9170_RX_STATUS_MODULATION_S		0
#define	AR9170_RX_STATUS_MODULATION_CCK		0x00
#define	AR9170_RX_STATUS_MODULATION_OFDM	0x01
#define	AR9170_RX_STATUS_MODULATION_HT		0x02
#define	AR9170_RX_STATUS_MODULATION_DUPOFDM	0x03

/* depends on modulation */
#define	AR9170_RX_STATUS_SHORT_PREAMBLE		0x08
#define	AR9170_RX_STATUS_GREENFIELD		0x08

#define	AR9170_RX_STATUS_MPDU			0x30
#define	AR9170_RX_STATUS_MPDU_S			4
#define	AR9170_RX_STATUS_MPDU_SINGLE		0x00
#define	AR9170_RX_STATUS_MPDU_FIRST		0x20
#define	AR9170_RX_STATUS_MPDU_MIDDLE		0x30
#define	AR9170_RX_STATUS_MPDU_LAST		0x10

#define	AR9170_RX_STATUS_CONT_AGGR		0x40
#define	AR9170_RX_STATUS_TOTAL_ERROR		0x80

#define	AR9170_RX_ERROR_RXTO			0x01
#define	AR9170_RX_ERROR_OVERRUN			0x02
#define	AR9170_RX_ERROR_DECRYPT			0x04
#define	AR9170_RX_ERROR_FCS			0x08
#define	AR9170_RX_ERROR_WRONG_RA		0x10
#define	AR9170_RX_ERROR_PLCP			0x20
#define	AR9170_RX_ERROR_MMIC			0x40

/* these are either-or */
#define	AR9170_TX_MAC_PROT_RTS			0x0001
#define	AR9170_TX_MAC_PROT_CTS			0x0002
#define	AR9170_TX_MAC_PROT			0x0003

#define	AR9170_TX_MAC_NO_ACK			0x0004
/* if unset, MAC will only do SIFS space before frame */
#define	AR9170_TX_MAC_BACKOFF			0x0008
#define	AR9170_TX_MAC_BURST			0x0010
#define	AR9170_TX_MAC_AGGR			0x0020

/* encryption is a two-bit field */
#define	AR9170_TX_MAC_ENCR_NONE			0x0000
#define	AR9170_TX_MAC_ENCR_RC4			0x0040
#define	AR9170_TX_MAC_ENCR_CENC			0x0080
#define	AR9170_TX_MAC_ENCR_AES			0x00c0

#define	AR9170_TX_MAC_MMIC			0x0100
#define	AR9170_TX_MAC_HW_DURATION		0x0200
#define	AR9170_TX_MAC_QOS_S			10
#define	AR9170_TX_MAC_QOS			0x0c00
#define	AR9170_TX_MAC_DISABLE_TXOP		0x1000
#define	AR9170_TX_MAC_TXOP_RIFS			0x2000
#define	AR9170_TX_MAC_IMM_BA			0x4000

/* either-or */
#define	AR9170_TX_PHY_MOD_CCK			0x00000000
#define	AR9170_TX_PHY_MOD_OFDM			0x00000001
#define	AR9170_TX_PHY_MOD_HT			0x00000002

/* depends on modulation */
#define	AR9170_TX_PHY_SHORT_PREAMBLE		0x00000004
#define	AR9170_TX_PHY_GREENFIELD		0x00000004

#define	AR9170_TX_PHY_BW_S			3
#define	AR9170_TX_PHY_BW			(3 << AR9170_TX_PHY_BW_SHIFT)
#define	AR9170_TX_PHY_BW_20MHZ			0
#define	AR9170_TX_PHY_BW_40MHZ			2
#define	AR9170_TX_PHY_BW_40MHZ_DUP		3

#define	AR9170_TX_PHY_TX_HEAVY_CLIP_S		6
#define	AR9170_TX_PHY_TX_HEAVY_CLIP		(7 << \
AR9170_TX_PHY_TX_HEAVY_CLIP_S)

#define	AR9170_TX_PHY_TX_PWR_S			9
#define	AR9170_TX_PHY_TX_PWR			(0x3f << \
AR9170_TX_PHY_TX_PWR_S)

#define	AR9170_TX_PHY_TXCHAIN_S			15
#define	AR9170_TX_PHY_TXCHAIN			(7 << \
AR9170_TX_PHY_TXCHAIN_S)
#define	AR9170_TX_PHY_TXCHAIN_1			1
/* use for cck, ofdm 6/9/12/18/24 and HT if capable */
#define	AR9170_TX_PHY_TXCHAIN_2			5

#define	AR9170_TX_PHY_MCS_S			18
#define	AR9170_TX_PHY_MCS			(0x7f << \
AR9170_TX_PHY_MCS_S)

#define	AR9170_TX_PHY_RATE_CCK_1M		0x0
#define	AR9170_TX_PHY_RATE_CCK_2M		0x1
#define	AR9170_TX_PHY_RATE_CCK_5M		0x2
#define	AR9170_TX_PHY_RATE_CCK_11M		0x3

/* same as AR9170_RX_PHY_RATE */
#define	AR9170_TXRX_PHY_RATE_OFDM_6M		0xb
#define	AR9170_TXRX_PHY_RATE_OFDM_9M		0xf
#define	AR9170_TXRX_PHY_RATE_OFDM_12M		0xa
#define	AR9170_TXRX_PHY_RATE_OFDM_18M		0xe
#define	AR9170_TXRX_PHY_RATE_OFDM_24M		0x9
#define	AR9170_TXRX_PHY_RATE_OFDM_36M		0xd
#define	AR9170_TXRX_PHY_RATE_OFDM_48M		0x8
#define	AR9170_TXRX_PHY_RATE_OFDM_54M		0xc

#define	AR9170_TXRX_PHY_RATE_HT_MCS0		0x0
#define	AR9170_TXRX_PHY_RATE_HT_MCS1		0x1
#define	AR9170_TXRX_PHY_RATE_HT_MCS2		0x2
#define	AR9170_TXRX_PHY_RATE_HT_MCS3		0x3
#define	AR9170_TXRX_PHY_RATE_HT_MCS4		0x4
#define	AR9170_TXRX_PHY_RATE_HT_MCS5		0x5
#define	AR9170_TXRX_PHY_RATE_HT_MCS6		0x6
#define	AR9170_TXRX_PHY_RATE_HT_MCS7		0x7
#define	AR9170_TXRX_PHY_RATE_HT_MCS8		0x8
#define	AR9170_TXRX_PHY_RATE_HT_MCS9		0x9
#define	AR9170_TXRX_PHY_RATE_HT_MCS10		0xa
#define	AR9170_TXRX_PHY_RATE_HT_MCS11		0xb
#define	AR9170_TXRX_PHY_RATE_HT_MCS12		0xc
#define	AR9170_TXRX_PHY_RATE_HT_MCS13		0xd
#define	AR9170_TXRX_PHY_RATE_HT_MCS14		0xe
#define	AR9170_TXRX_PHY_RATE_HT_MCS15		0xf

#define	AR9170_TX_PHY_SHORT_GI			0x80000000









#define	AR9170_RX_HEAD_LEN			12

struct ar9170_rx_head {
	U8 plcp[12];
} __attribute__ ((packed));

struct ar9170_rx_phystatus {
	union {
		struct {
			U8 rssi_ant0, rssi_ant1, rssi_ant2,
				rssi_ant0x, rssi_ant1x, rssi_ant2x,
				rssi_combined;
		} __attribute__ ((packed));
		U8 rssi[7];
	} __attribute__ ((packed));

	U8 evm_stream0[6], evm_stream1[6];
	U8 phy_err;
} __attribute__ ((packed));

#define	AR9170_RX_PHYSTATUS_LEN			20

struct ar9170_rx_macstatus {
	U8 SAidx, DAidx;
	U8 error;
	U8 status;
} __attribute__ ((packed));

#define	AR9170_RX_MACSTATUS_LEN			4

struct ar9170_rx_frame_single {
	struct ar9170_rx_head phy_head;
	struct ieee80211_hdr i3e;
	struct ar9170_rx_phystatus phy_tail;
	struct ar9170_rx_macstatus macstatus;
} __attribute__ ((packed));

struct ar9170_rx_frame_head {
	struct ar9170_rx_head phy_head;
	struct ieee80211_hdr i3e;
	struct ar9170_rx_macstatus macstatus;
} __attribute__ ((packed));

struct ar9170_rx_frame_middle {
	struct ieee80211_hdr i3e;
	struct ar9170_rx_macstatus macstatus;
} __attribute__ ((packed));

struct ar9170_rx_frame_tail {
	struct ieee80211_hdr i3e;
	struct ar9170_rx_phystatus phy_tail;
	struct ar9170_rx_macstatus macstatus;
} __attribute__ ((packed));

struct ar9170_rx_frame {
	union {
		struct ar9170_rx_frame_single single;
		struct ar9170_rx_frame_head head;
		struct ar9170_rx_frame_middle middle;
		struct ar9170_rx_frame_tail tail;
	} __attribute__ ((packed));
} __attribute__ ((packed));

static inline U8 ar9170_get_decrypt_type(struct ar9170_rx_macstatus *t)
{
	return (t->SAidx & 0xc0) >> 4 |
	       (t->DAidx & 0xc0) >> 6;
}

/*
 * This is an workaround for several undocumented bugs.
 * Don't mess with the QoS/AC <-> HW Queue map, if you don't
 * know what you are doing.
 *
 * Known problems [hardware]:
 *  * The MAC does not aggregate frames on anything other
 *    than the first HW queue.
 *  * when an AMPDU is placed [in the first hw queue] and
 *    additional frames are already queued on a different
 *    hw queue, the MAC will ALWAYS freeze.
 *
 * In a nutshell: The hardware can either do QoS or
 * Aggregation but not both at the same time. As a
 * result, this makes the device pretty much useless
 * for any serious 802.11n setup.
 */
enum ar9170_txq {
	AR9170_TXQ_BK = 0,	/* TXQ0 */
	AR9170_TXQ_BE,		/* TXQ1	*/
	AR9170_TXQ_VI,		/* TXQ2	*/
	AR9170_TXQ_VO,		/* TXQ3 */

	__AR9170_NUM_TXQ,
};

#define	AR9170_TXQ_DEPTH			32



struct _ar9170_tx_hwdesc {
	le16_t length;
	le16_t mac_control;
	le32_t phy_control;
} __attribute__((packed));

#define AR9170_TX_SUPER_MISC_QUEUE			0x3
#define AR9170_TX_SUPER_MISC_QUEUE_S			0
#define AR9170_TX_SUPER_MISC_ASSIGN_SEQ		0x4
#define	AR9170_TX_SUPER_MISC_VIF_ID			0x38
#define	AR9170_TX_SUPER_MISC_VIF_ID_S			3
#define AR9170_TX_SUPER_MISC_FILL_IN_TSF		0x40
#define	AR9170_TX_SUPER_MISC_CAB			0x80

#define AR9170_TX_SUPER_RI_TRIES			0x7
#define AR9170_TX_SUPER_RI_TRIES_S			0
#define AR9170_TX_SUPER_RI_ERP_PROT			0x18
#define AR9170_TX_SUPER_RI_ERP_PROT_S			3
#define AR9170_TX_SUPER_RI_AMPDU			0x20
#define AR9170_TX_SUPER_RI_AMPDU_S			5

struct _ar9170_tx_superdesc {
         le16_t len;
         U8 rix;
         U8 cnt;
         U8 cookie;
         U8 ampdu_settings;
         U8 misc;
         U8 padding;
         U8 ri[AR9170_TX_MAX_RATES];
         le32_t rr[AR9170_TX_MAX_RETRY_RATES];
} __attribute__((packed));

struct _ar9170_tx_superframe {
         struct _ar9170_tx_superdesc s;
         struct _ar9170_tx_hwdesc f;
         U8 frame_data[0];
}  __attribute__((packed,aligned(4)));



#endif /* AR9170_WLAN_H_ */