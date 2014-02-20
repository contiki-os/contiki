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
#include "mac80211.h"
#include "common\if_ether.h"


#ifndef ATH_H_
#define ATH_H_

struct ath_cycle_counters {
	U32 cycles;
	U32 rx_busy;
	U32 rx_frame;
	U32 tx_frame;
};

enum ath_device_state {
	ATH_HW_UNAVAILABLE,
	ATH_HW_INITIALIZED,
};

enum ath_bus_type {
	ATH_PCI,
	ATH_AHB,
	ATH_USB,
};

struct reg_dmn_pair_mapping {
	U16 regDmnEnum;
	U16 reg_5ghz_ctl;
	U16 reg_2ghz_ctl;
};

struct ath_regulatory {
	char alpha2[2];
	U16 country_code;
	U16 max_power_level;
	U16 current_rd;
	int16_t power_limit;
	struct reg_dmn_pair_mapping *regpair;
};


struct ath_ani {
	bool caldone;
	unsigned int longcal_timer;
	unsigned int shortcal_timer;
	unsigned int resetcal_timer;
	unsigned int checkani_timer;
	//struct timer_list timer; FIXME
};



/**
 * struct ath_ops - Register read/write operations
 *
 * @read: Register read
 * @multi_read: Multiple register read
 * @write: Register write
 * @enable_write_buffer: Enable multiple register writes
 * @write_flush: flush buffered register writes and disable buffering
 */
struct ath_ops {
	unsigned int (*read)(void *, U32 reg_offset);
	void (*multi_read)(void *, U32 *addr, U32 *val, U16 count);
	void (*write)(void *, U32 val, U32 reg_offset);
	void (*enable_write_buffer)(void *);
	void (*write_flush) (void *);
	U32 (*rmw)(void *, U32 reg_offset, U32 set, U32 clr);
};


struct ath_common;
struct ath_bus_ops;

struct ath_common {
	void *ah;
	void *priv;
	struct ieee80211_hw *hw;
	int debug_mask;
	enum ath_device_state state;

	struct ath_ani ani;

	U16 cachelsz;
	U16 curaid;
	U8 macaddr[ETH_ALEN];
	U8 curbssid[ETH_ALEN];
	U8 bssidmask[ETH_ALEN];

	U32 rx_bufsize;

	U32 keymax;
	//DECLARE_BITMAP(keymap, ATH_KEYMAX);
	//DECLARE_BITMAP(tkip_keymap, ATH_KEYMAX);
	//DECLARE_BITMAP(ccmp_keymap, ATH_KEYMAX);
	//enum ath_crypt_caps crypt_caps; FIXME

	unsigned int clockrate;

	//spinlock_t cc_lock; FIXME: probably not needed
	struct ath_cycle_counters cc_ani;
	struct ath_cycle_counters cc_survey;

	struct ath_regulatory regulatory;
	struct ath_regulatory reg_world_copy;
	const struct ath_ops *ops;
	const struct ath_bus_ops *bus_ops;

	bool btcoex_enabled;
	bool disable_ani;
};

#endif /* ATH_H_ */