/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "compiler.h"
#include <stddef.h>
#include "ieee80211.h"
#include "bitops.h"
#include "ibss_util.h"


void ieee802_11_parse_elems(U8 *start, size_t len, struct ieee802_11_elems *elems)
{
	ieee802_11_parse_elems_crc(start, len, elems, 0, 0);
}


U32 ieee802_11_parse_elems_crc(U8 *start, size_t len, struct ieee802_11_elems *elems, U64 filter, U32 crc)
{
	size_t left = len;
	U8 *pos = start;
	bool calc_crc = filter != 0;
	
	DECLARE_BITMAP(seen_elems, 256);

	bitmap_zero(seen_elems, 256);
	
	memset(elems, 0, sizeof(*elems));
	elems->ie_start = start;
	elems->total_len = len;

	while (left >= 2) {
		U8 id, elen;
		bool elem_parse_failed;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			elems->parse_error = true;
			break;
		}

		if (id != WLAN_EID_VENDOR_SPECIFIC && id != WLAN_EID_QUIET && test_bit(id, seen_elems)) {
			elems->parse_error = true;
			left -= elen;
			pos += elen;
			continue;
		}
/*
		if (calc_crc && id < 64 && (filter & (1ULL << id)))
			crc = crc32_be(crc, pos - 2, elen + 2);
*/
		elem_parse_failed = false;

		switch (id) {
			case WLAN_EID_SSID:
				elems->ssid = pos;
				elems->ssid_len = elen;
				break;
			case WLAN_EID_SUPP_RATES:
				elems->supp_rates = pos;
				elems->supp_rates_len = elen;
				break;
			case WLAN_EID_FH_PARAMS:
				elems->fh_params = pos;
				elems->fh_params_len = elen;
				break;
			case WLAN_EID_DS_PARAMS:
				elems->ds_params = pos;
				elems->ds_params_len = elen;
				break;
			case WLAN_EID_CF_PARAMS:
				elems->cf_params = pos;
				elems->cf_params_len = elen;
				break;
			case WLAN_EID_TIM:
				if (elen >= sizeof(struct ieee80211_tim_ie)) {
					elems->tim = (void *)pos;
					elems->tim_len = elen;
				} else
					elem_parse_failed = true;
				break;
			case WLAN_EID_IBSS_PARAMS:
				elems->ibss_params = pos;
				elems->ibss_params_len = elen;
				break;
			case WLAN_EID_CHALLENGE:
				elems->challenge = pos;
				elems->challenge_len = elen;
				break;
			case WLAN_EID_VENDOR_SPECIFIC:
				if (elen >= 4 && pos[0] == 0x00 && pos[1] == 0x50 &&
				pos[2] == 0xf2) {
					/* Microsoft OUI (00:50:F2) */

					if (calc_crc) {
						//FIXMEcrc = crc32_be(crc, pos - 2, elen + 2);
					}
					if (pos[3] == 1) {
						/* OUI Type 1 - WPA IE */
						elems->wpa = pos;
						elems->wpa_len = elen;
					} else if (elen >= 5 && pos[3] == 2) {
						/* OUI Type 2 - WMM IE */
						if (pos[4] == 0) {
							elems->wmm_info = pos;
							elems->wmm_info_len = elen;
						} else if (pos[4] == 1) {
							elems->wmm_param = pos;
							elems->wmm_param_len = elen;
						}
					}
				}
				break;
			case WLAN_EID_RSN:
				elems->rsn = pos;
				elems->rsn_len = elen;
				break;
			case WLAN_EID_ERP_INFO:
				elems->erp_info = pos;
				elems->erp_info_len = elen;
				break;
			case WLAN_EID_EXT_SUPP_RATES:
				elems->ext_supp_rates = pos;
				elems->ext_supp_rates_len = elen;
				break;
			case WLAN_EID_HT_CAPABILITY:
				if (elen >= sizeof(struct ieee80211_ht_cap))
					elems->ht_cap_elem = (void *)pos;
				else
					elem_parse_failed = true;
				break;
			case WLAN_EID_HT_OPERATION:
				if (elen >= sizeof(struct ieee80211_ht_operation))
					elems->ht_operation = (void *)pos;
				else
					elem_parse_failed = true;
				break;
			case WLAN_EID_MESH_ID:
				elems->mesh_id = pos;
				elems->mesh_id_len = elen;
				break;
			case WLAN_EID_MESH_CONFIG:
				if (elen >= sizeof(struct ieee80211_meshconf_ie))
					elems->mesh_config = (void *)pos;
				else
					elem_parse_failed = true;
				break;
			case WLAN_EID_PEER_MGMT:
				elems->peering = pos;
				elems->peering_len = elen;
				break;
			case WLAN_EID_PREQ:
				elems->preq = pos;
				elems->preq_len = elen;
				break;
			case WLAN_EID_PREP:
				elems->prep = pos;
				elems->prep_len = elen;
				break;
			case WLAN_EID_PERR:
				elems->perr = pos;
				elems->perr_len = elen;
				break;
			case WLAN_EID_RANN:
				if (elen >= sizeof(struct ieee80211_rann_ie))
					elems->rann = (void *)pos;
				else
					elem_parse_failed = true;
				break;
			case WLAN_EID_CHANNEL_SWITCH:
				elems->ch_switch_elem = pos;
				elems->ch_switch_elem_len = elen;
				break;
			case WLAN_EID_QUIET:
				if (!elems->quiet_elem) {
					elems->quiet_elem = pos;
					elems->quiet_elem_len = elen;
				}
				elems->num_of_quiet_elem++;
				break;
			case WLAN_EID_COUNTRY:
				elems->country_elem = pos;
				elems->country_elem_len = elen;
				break;
			case WLAN_EID_PWR_CONSTRAINT:
				elems->pwr_constr_elem = pos;
				elems->pwr_constr_elem_len = elen;
				break;
			case WLAN_EID_TIMEOUT_INTERVAL:
				elems->timeout_int = pos;
				elems->timeout_int_len = elen;
				break;
			default:
				break;
		}

		if (elem_parse_failed)
			elems->parse_error = true;
		else
			set_bit(id, seen_elems);

		left -= elen;
		pos += elen;
	}

	if (left != 0)
		elems->parse_error = true;

	return crc;
}