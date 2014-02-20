/*
 * 802.11 netlink interface public header
 *
 * Copyright 2006-2010 Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2008 Michael Wu <flamingice@sourmilk.net>
 * Copyright 2008 Luis Carlos Cobo <luisca@cozybit.com>
 * Copyright 2008 Michael Buesch <m@bues.ch>
 * Copyright 2008, 2009 Luis R. Rodriguez <lrodriguez@atheros.com>
 * Copyright 2008 Jouni Malinen <jouni.malinen@atheros.com>
 * Copyright 2008 Colin McCabe <colin@cozybit.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef NL80211_H_
#define NL80211_H_


/**
 * enum nl80211_iftype - (virtual) interface types
 *
 * @NL80211_IFTYPE_UNSPECIFIED: unspecified type, driver decides
 * @NL80211_IFTYPE_ADHOC: independent BSS member
 * @NL80211_IFTYPE_STATION: managed BSS member
 * @NL80211_IFTYPE_AP: access point
 * @NL80211_IFTYPE_AP_VLAN: VLAN interface for access points; VLAN interfaces
 *	are a bit special in that they must always be tied to a pre-existing
 *	AP type interface.
 * @NL80211_IFTYPE_WDS: wireless distribution interface
 * @NL80211_IFTYPE_MONITOR: monitor interface receiving all frames
 * @NL80211_IFTYPE_MESH_POINT: mesh point
 * @NL80211_IFTYPE_P2P_CLIENT: P2P client
 * @NL80211_IFTYPE_P2P_GO: P2P group owner
 * @NL80211_IFTYPE_MAX: highest interface type number currently defined
 * @NUM_NL80211_IFTYPES: number of defined interface types
 *
 * These values are used with the %NL80211_ATTR_IFTYPE
 * to set the type of an interface.
 *
 */
enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,
	NL80211_IFTYPE_P2P_CLIENT,
	NL80211_IFTYPE_P2P_GO,

	/* keep last */
	NUM_NL80211_IFTYPES,
	NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};
/**
 * enum nl80211_band - Frequency band
 * @NL80211_BAND_2GHZ: 2.4 GHz ISM band
 * @NL80211_BAND_5GHZ: around 5 GHz band (4.9 - 5.7 GHz)
 */
enum nl80211_band {
	NL80211_BAND_2GHZ,
	NL80211_BAND_5GHZ,
	NL80211_BAND_60GHZ,
};

enum nl80211_channel_type {
	NL80211_CHAN_NO_HT,
	NL80211_CHAN_HT20,
	NL80211_CHAN_HT40MINUS,
	NL80211_CHAN_HT40PLUS
};


/**
 * enum nl80211_initiator - Indicates the initiator of a reg domain request
 * @NL80211_REGDOM_SET_BY_CORE: Core queried CRDA for a dynamic world
 * 	regulatory domain.
 * @NL80211_REGDOM_SET_BY_USER: User asked the wireless core to set the
 * 	regulatory domain.
 * @NL80211_REGDOM_SET_BY_DRIVER: a wireless drivers has hinted to the
 * 	wireless core it thinks its knows the regulatory domain we should be in.
 * @NL80211_REGDOM_SET_BY_COUNTRY_IE: the wireless core has received an
 * 	802.11 country information element with regulatory information it
 * 	thinks we should consider. cfg80211 only processes the country
 *	code from the IE, and relies on the regulatory domain information
 *	structure passed by userspace (CRDA) from our wireless-regdb.
 *	If a channel is enabled but the country code indicates it should
 *	be disabled we disable the channel and re-enable it upon disassociation.
 */
enum nl80211_reg_initiator {
	NL80211_REGDOM_SET_BY_CORE,
	NL80211_REGDOM_SET_BY_USER,
	NL80211_REGDOM_SET_BY_DRIVER,
	NL80211_REGDOM_SET_BY_COUNTRY_IE,
};


/**
 * enum nl80211_reg_type - specifies the type of regulatory domain
 * @NL80211_REGDOM_TYPE_COUNTRY: the regulatory domain set is one that pertains
 *	to a specific country. When this is set you can count on the
 *	ISO / IEC 3166 alpha2 country code being valid.
 * @NL80211_REGDOM_TYPE_WORLD: the regulatory set domain is the world regulatory
 * 	domain.
 * @NL80211_REGDOM_TYPE_CUSTOM_WORLD: the regulatory domain set is a custom
 * 	driver specific world regulatory domain. These do not apply system-wide
 * 	and are only applicable to the individual devices which have requested
 * 	them to be applied.
 * @NL80211_REGDOM_TYPE_INTERSECTION: the regulatory domain set is the product
 *	of an intersection between two regulatory domains -- the previously
 *	set regulatory domain on the system and the last accepted regulatory
 *	domain request to be processed.
 */
enum nl80211_reg_type {
	NL80211_REGDOM_TYPE_COUNTRY,
	NL80211_REGDOM_TYPE_WORLD,
	NL80211_REGDOM_TYPE_CUSTOM_WORLD,
	NL80211_REGDOM_TYPE_INTERSECTION,
};


/**
 * enum nl80211_user_reg_hint_type - type of user regulatory hint
 *
 * @NL80211_USER_REG_HINT_USER: a user sent the hint. This is always
 *      assumed if the attribute is not set.
 * @NL80211_USER_REG_HINT_CELL_BASE: the hint comes from a cellular
 *      base station. Device drivers that have been tested to work
 *      properly to support this type of hint can enable these hints
 *      by setting the NL80211_FEATURE_CELL_BASE_REG_HINTS feature
 *      capability on the struct wiphy. The wireless core will
 *      ignore all cell base station hints until at least one device
 *      present has been registered with the wireless core that
 *      has listed NL80211_FEATURE_CELL_BASE_REG_HINTS as a
 *      supported feature.
 */
enum nl80211_user_reg_hint_type {
	NL80211_USER_REG_HINT_USER      = 0,
	NL80211_USER_REG_HINT_CELL_BASE = 1,
};

#endif /* NL80211_H_ */