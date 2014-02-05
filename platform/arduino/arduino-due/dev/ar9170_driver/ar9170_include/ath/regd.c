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
#include "regd.h"
#include "regd_common.h"


bool is_wwr_sku(U16 regd)
{
	return ((regd & COUNTRY_ERD_FLAG) != COUNTRY_ERD_FLAG) &&
	(((regd & WORLD_SKU_MASK) == WORLD_SKU_PREFIX) ||
	(regd == WORLD));
}


U32 ath_regd_get_band_ctl(struct ath_regulatory *reg,
enum ieee80211_band band)
{
	if (!reg->regpair ||
	(reg->country_code == CTRY_DEFAULT &&
	is_wwr_sku(ath_regd_get_eepromRD(reg)))) {
		printf("Should not enter here.\n");
		return SD_NO_CTL;
		
	}

	switch (band) {
		case IEEE80211_BAND_2GHZ:
		return reg->regpair->reg_2ghz_ctl;
		case IEEE80211_BAND_5GHZ:
		return reg->regpair->reg_5ghz_ctl;
		default:
		return NO_CTL;
	}
}

U16 ath_regd_get_eepromRD(struct ath_regulatory *reg)
{
	return reg->current_rd & ~WORLDWIDE_ROAMING_FLAG;
}