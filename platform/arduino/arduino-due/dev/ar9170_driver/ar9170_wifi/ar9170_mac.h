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
#include "ar9170.h"
#include "nl80211.h"


#ifndef AR9170_MAC_H_
#define AR9170_MAC_H_


inline bool conf_is_ht20(struct ieee80211_conf *conf)
{
	return conf->channel_type == NL80211_CHAN_HT20;
}

inline bool conf_is_ht40_minus(struct ieee80211_conf *conf)
{
	return conf->channel_type == NL80211_CHAN_HT40MINUS;
}

inline bool conf_is_ht40_plus(struct ieee80211_conf *conf)
{
	return conf->channel_type == NL80211_CHAN_HT40PLUS;
}

inline bool conf_is_ht40(struct ieee80211_conf *conf)
{
	return conf_is_ht40_minus(conf) || conf_is_ht40_plus(conf);
}

inline bool conf_is_ht(struct ieee80211_conf *conf)
{
	return conf->channel_type != NL80211_CHAN_NO_HT;
}

int ar9170_init_mac(struct ar9170* ar);
int ar9170_set_qos(struct ar9170 *ar);
int ar9170_upload_key(struct ar9170 *ar, const U8 id, const U8 *mac, const U8 ktype, const U8 keyidx, const U8 *keydata, const int keylen);
int ar9170_disable_key(struct ar9170 *ar, const U8 id);
int ar9170_set_operating_mode(struct ar9170 *ar);
int ar9170_set_mac_reg(struct ar9170 *ar, const U32 reg, const U8 *mac);
int ar9170_set_slot_time(struct ar9170 *ar);
int ar9170_set_slot_time(struct ar9170 *ar);
int ar9170_set_rts_cts_rate(struct ar9170 *ar);
int ar9170_set_mac_tpc(struct ar9170 *ar, struct ieee80211_channel *channel);
int ar9170_set_dyn_sifs_ack(struct ar9170 *ar);
int ar9170_set_mac_rates(struct ar9170 *ar);
int ar9170_set_hwretry_limit(struct ar9170 *ar, const unsigned int max_retry);
int ar9170_set_beacon_timers(struct ar9170 *ar);
int ar9170_update_multicast(struct ar9170 *ar, const U64 mc_hash);
int ar9170_update_multicast_mine(struct ar9170 *ar, const U32 mc_hash_low, const U32 mc_hash_high);
void ar9170_schedule_bcn_ctrl(struct ar9170* ar);
void ar9170_schedule_bcn_cancel(struct ar9170* ar);
#endif /* AR9170_MAC_H_ */


