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
#include "cfg80211.h"
#include "ieee80211.h"
#include "mac80211.h"
#include "nl80211.h"


#ifndef AR9170_MAIN_H_
#define AR9170_MAIN_H_


#define CARL9170_FILL_QUEUE(queue, ai_fs, cwmin, cwmax, _txop)		\
do {									\
queue.aifs = ai_fs;						\
queue.cw_min = cwmin;						\
queue.cw_max = cwmax;						\
queue.txop = _txop;						\
} while (0)


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


#define carl9170_g_ratetable	(__carl9170_ratetable + 0)
#define carl9170_g_ratetable_size	12
#define carl9170_a_ratetable	(__carl9170_ratetable + 4)
#define carl9170_a_ratetable_size	8

/*
 * NB: The hw_value is used as an index into the carl9170_phy_freq_params
 *     array in phy.c so that we don't have to do frequency lookups!
 */
#define CHAN(_freq, _idx) {		\
	.center_freq	= (_freq),	\
	.hw_value	= (_idx),	\
	.max_power	= 18, /* XXX */	\
}

static struct ieee80211_channel carl9170_2ghz_chantable[] = {
	CHAN(2412,  0),
	CHAN(2417,  1),
	CHAN(2422,  2),
	CHAN(2427,  3),
	CHAN(2432,  4),
	CHAN(2437,  5),
	CHAN(2442,  6),
	CHAN(2447,  7),
	CHAN(2452,  8),
	CHAN(2457,  9),
	CHAN(2462, 10),
	CHAN(2467, 11),
	CHAN(2472, 12),
	CHAN(2484, 13),
};

static struct ieee80211_channel carl9170_5ghz_chantable[] = {
	CHAN(4920, 14),
	CHAN(4940, 15),
	CHAN(4960, 16),
	CHAN(4980, 17),
	CHAN(5040, 18),
	CHAN(5060, 19),
	CHAN(5080, 20),
	CHAN(5180, 21),
	CHAN(5200, 22),
	CHAN(5220, 23),
	CHAN(5240, 24),
	CHAN(5260, 25),
	CHAN(5280, 26),
	CHAN(5300, 27),
	CHAN(5320, 28),
	CHAN(5500, 29),
	CHAN(5520, 30),
	CHAN(5540, 31),
	CHAN(5560, 32),
	CHAN(5580, 33),
	CHAN(5600, 34),
	CHAN(5620, 35),
	CHAN(5640, 36),
	CHAN(5660, 37),
	CHAN(5680, 38),
	CHAN(5700, 39),
	CHAN(5745, 40),
	CHAN(5765, 41),
	CHAN(5785, 42),
	CHAN(5805, 43),
	CHAN(5825, 44),
	CHAN(5170, 45),
	CHAN(5190, 46),
	CHAN(5210, 47),
	CHAN(5230, 48),
};
#undef CHAN

#define CARL9170_HT_CAP							\
{									\
	.ht_supported	= true,						\
	.cap		= IEEE80211_HT_CAP_MAX_AMSDU |			\
	IEEE80211_HT_CAP_SUP_WIDTH_20_40 |		\
	IEEE80211_HT_CAP_SGI_40 |			\
	IEEE80211_HT_CAP_DSSSCCK40 |			\
	IEEE80211_HT_CAP_SM_PS,			\
	.ampdu_factor	= IEEE80211_HT_MAX_AMPDU_64K,			\
	.ampdu_density	= IEEE80211_HT_MPDU_DENSITY_8,			\
	.mcs		= {						\
.rx_mask = { 0xff, 0xff, 0, 0, 0x1, 0, 0, 0, 0, 0, },	\
.rx_highest = cpu_to_le16(300),				\
.tx_params = IEEE80211_HT_MCS_TX_DEFINED,		\
	},								\
}

extern struct ieee80211_rate __carl9170_ratetable[];


#endif /* AR9170_MAIN_H_ */