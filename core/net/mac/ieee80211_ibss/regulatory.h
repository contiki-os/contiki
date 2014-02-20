#include "nl80211.h"


#ifndef REGULATORY_H_
#define REGULATORY_H_
/*
 * regulatory support structures
 *
 * Copyright 2008-2009  Luis R. Rodriguez <mcgrof@qca.qualcomm.com>
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
 */

#include "list.h"
/**
 * enum environment_cap - Environment parsed from country IE
 * @ENVIRON_ANY: indicates country IE applies to both indoor and
 *	outdoor operation.
 * @ENVIRON_INDOOR: indicates country IE applies only to indoor operation
 * @ENVIRON_OUTDOOR: indicates country IE applies only to outdoor operation
 */
enum environment_cap {
	ENVIRON_ANY,
	ENVIRON_INDOOR,
	ENVIRON_OUTDOOR,
};

struct ieee80211_freq_range {
	U32 start_freq_khz;
	U32 end_freq_khz;
	U32 max_bandwidth_khz;
};


struct ieee80211_power_rule {
	U32 max_antenna_gain;
	U32 max_eirp;
};


struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	U32 flags;
};


struct ieee80211_regdomain {
	U32 n_reg_rules;
	char alpha2[2];
	U8 dfs_region;
	struct ieee80211_reg_rule reg_rules[];
};


/**
 * struct regulatory_request - used to keep track of regulatory requests
 *
 * @wiphy_idx: this is set if this request's initiator is
 * 	%REGDOM_SET_BY_COUNTRY_IE or %REGDOM_SET_BY_DRIVER. This
 * 	can be used by the wireless core to deal with conflicts
 * 	and potentially inform users of which devices specifically
 * 	cased the conflicts.
 * @initiator: indicates who sent this request, could be any of
 * 	of those set in nl80211_reg_initiator (%NL80211_REGDOM_SET_BY_*)
 * @alpha2: the ISO / IEC 3166 alpha2 country code of the requested
 * 	regulatory domain. We have a few special codes:
 * 	00 - World regulatory domain
 * 	99 - built by driver but a specific alpha2 cannot be determined
 * 	98 - result of an intersection between two regulatory domains
 *	97 - regulatory domain has not yet been configured
 * @dfs_region: If CRDA responded with a regulatory domain that requires
 *	DFS master operation on a known DFS region (NL80211_DFS_*),
 *	dfs_region represents that region. Drivers can use this and the
 *	@alpha2 to adjust their device's DFS parameters as required.
 * @user_reg_hint_type: if the @initiator was of type
 *	%NL80211_REGDOM_SET_BY_USER, this classifies the type
 *	of hint passed. This could be any of the %NL80211_USER_REG_HINT_*
 *	types.
 * @intersect: indicates whether the wireless core should intersect
 * 	the requested regulatory domain with the presently set regulatory
 * 	domain.
 * @processed: indicates whether or not this requests has already been
 *	processed. When the last request is processed it means that the
 *	currently regulatory domain set on cfg80211 is updated from
 *	CRDA and can be used by other regulatory requests. When a
 *	the last request is not yet processed we must yield until it
 *	is processed before processing any new requests.
 * @country_ie_checksum: checksum of the last processed and accepted
 * 	country IE
 * @country_ie_env: lets us know if the AP is telling us we are outdoor,
 * 	indoor, or if it doesn't matter
 * @list: used to insert into the reg_requests_list linked list
 */
struct regulatory_request {
	int wiphy_idx;
	enum nl80211_reg_initiator initiator;
	enum nl80211_user_reg_hint_type user_reg_hint_type;
	char alpha2[2];
	U8 dfs_region;
	bool intersect;
	bool processed;
	enum environment_cap country_ie_env;
	struct _list_head list;
};

#endif /* REGULATORY_H_ */