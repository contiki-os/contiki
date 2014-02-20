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
#include "ar9170_mac.h"
#include "hw.h"
#include "usb_cmd_wrapper.h"
#include <sys\errno.h>
#include "ar9170_debug.h"
#include "mac80211.h"
#include "cfg80211.h"
#include "usb_lock.h"
#include "nl80211.h"
#include "compiler.h"
#include "ar9170.h"
#include "ieee80211_ibss.h"


int ar9170_set_dyn_sifs_ack(struct ar9170 *ar)
{
	U32 val;

	if (conf_is_ht40(&ar->hw->conf))
	val = 0x010a;
	else {
		if (ar->hw->conf.channel->band == IEEE80211_BAND_2GHZ)
		val = 0x105;
		else
		val = 0x104;
	}

	return !ar9170_write_rgstr(ar, AR9170_MAC_REG_DYNAMIC_SIFS_ACK, val);
}

int ar9170_set_mac_rates(struct ar9170 *ar)
{
	#if AR9170_MAC_DEBUG_DEEP
	printf("ar9170_set_mac_rates.\n");
	#endif
	
	struct ieee80211_vif *vif;
	U32 basic, mandatory;

	__read_lock();
	vif = ar9170_get_main_vif(ar);

	if (!vif) {
		__read_unlock();
		return 0;
	}

	basic = (vif->bss_conf.basic_rates & 0xf);
	basic |= (vif->bss_conf.basic_rates & 0xff0) << 4;
	__read_unlock();

	if (ar->hw->conf.channel->band == IEEE80211_BAND_5GHZ)
		mandatory = 0xff00; /* OFDM 6/9/12/18/24/36/48/54 */
	else
		mandatory = 0xff0f; /* OFDM (6/9../54) + CCK (1/2/5.5/11) */

	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_BASIC_RATE, basic);
	ar9170_regwrite(AR9170_MAC_REG_MANDATORY_RATE, mandatory);
	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}

int ar9170_set_hwretry_limit(struct ar9170 *ar, const unsigned int max_retry)
{
	#if AR9170_MAC_DEBUG
	printf("DEBUG: ar9170_set_hwretry_limit.\n");
	#endif
	
	U32 tmp = min((U32)0x33333, (U32)(max_retry * 0x11111));

	return ar9170_write_rgstr(ar, AR9170_MAC_REG_RETRY_MAX, tmp);
}


int ar9170_init_mac( struct ar9170* ar )
{
	#if AR9170_MAC_DEBUG
	printf("DEBUG: ar9170_init_mac.\n");
	#endif
	
	ar9170_regwrite_begin(ar);
	
	/* switch MAC to OTUS interface */
	ar9170_regwrite(0x1c3600, 0x3);

	ar9170_regwrite(AR9170_MAC_REG_ACK_EXTENSION, 0x40);

	ar9170_regwrite(AR9170_MAC_REG_RETRY_MAX, 0x0);

	ar9170_regwrite(AR9170_MAC_REG_FRAMETYPE_FILTER,
	AR9170_MAC_FTF_MONITOR);

	/* enable MMIC */
	ar9170_regwrite(AR9170_MAC_REG_SNIFFER,
	AR9170_MAC_SNIFFER_DEFAULTS);

	ar9170_regwrite(AR9170_MAC_REG_RX_THRESHOLD, 0xc1f80);

	ar9170_regwrite(AR9170_MAC_REG_RX_PE_DELAY, 0x70);
	ar9170_regwrite(AR9170_MAC_REG_EIFS_AND_SIFS, 0xa144000);
	ar9170_regwrite(AR9170_MAC_REG_SLOT_TIME, 9 << 10);

	/* CF-END & CF-ACK rate => 24M OFDM */
	ar9170_regwrite(AR9170_MAC_REG_TID_CFACK_CFEND_RATE, 0x59900000);

	/* NAV protects ACK only (in TXOP) */
	ar9170_regwrite(AR9170_MAC_REG_TXOP_DURATION, 0x201);

	/* Set Beacon PHY CTRL's TPC to 0x7, TA1=1 */
	/* OTUS set AM to 0x1 */
	ar9170_regwrite(AR9170_MAC_REG_BCN_HT1, 0x8000170);

	ar9170_regwrite(AR9170_MAC_REG_BACKOFF_PROTECT, 0x105);

	/* Aggregation MAX number and timeout */
	ar9170_regwrite(AR9170_MAC_REG_AMPDU_FACTOR, 0x8000a);
	ar9170_regwrite(AR9170_MAC_REG_AMPDU_DENSITY, 0x140a07);

	ar9170_regwrite(AR9170_MAC_REG_FRAMETYPE_FILTER,
	AR9170_MAC_FTF_DEFAULTS);

	ar9170_regwrite(AR9170_MAC_REG_RX_CONTROL,
	AR9170_MAC_RX_CTRL_DEAGG |
	AR9170_MAC_RX_CTRL_SHORT_FILTER);

	/* rate sets */
	ar9170_regwrite(AR9170_MAC_REG_BASIC_RATE, 0x150f);
	ar9170_regwrite(AR9170_MAC_REG_MANDATORY_RATE, 0x150f);
	ar9170_regwrite(AR9170_MAC_REG_RTS_CTS_RATE, 0x0030033);

	/* MIMO response control */
	ar9170_regwrite(AR9170_MAC_REG_ACK_TPC, 0x4003c1e);

	ar9170_regwrite(AR9170_MAC_REG_AMPDU_RX_THRESH, 0xffff);

	/* set PHY register read timeout (??) */
	ar9170_regwrite(AR9170_MAC_REG_MISC_680, 0xf00008);

	/* Disable Rx TimeOut, workaround for BB. */
	ar9170_regwrite(AR9170_MAC_REG_RX_TIMEOUT, 0x0);

	/* Set WLAN DMA interrupt mode: generate int per packet */
	ar9170_regwrite(AR9170_MAC_REG_TXRX_MPI, 0x110011);

	ar9170_regwrite(AR9170_MAC_REG_FCS_SELECT,
	AR9170_MAC_FCS_FIFO_PROT);

	/* Disables the CF_END frame, undocumented register */
	ar9170_regwrite(AR9170_MAC_REG_TXOP_NOT_ENOUGH_IND,
	0x141e0f48);

	/* reset group hash table */
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_L, 0xffffffff);
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_H, 0xffffffff);

	/* disable PRETBTT interrupt */
	ar9170_regwrite(AR9170_MAC_REG_PRETBTT, 0x0);
	ar9170_regwrite(AR9170_MAC_REG_BCN_PERIOD, 0x0);

	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}

int ar9170_set_qos(struct ar9170 *ar)
{
	ar9170_regwrite_begin(ar);

	ar9170_regwrite(AR9170_MAC_REG_AC0_CW, ar->edcf[0].cw_min |
	(ar->edcf[0].cw_max << 16));
	ar9170_regwrite(AR9170_MAC_REG_AC1_CW, ar->edcf[1].cw_min |
	(ar->edcf[1].cw_max << 16));
	ar9170_regwrite(AR9170_MAC_REG_AC2_CW, ar->edcf[2].cw_min |
	(ar->edcf[2].cw_max << 16));
	ar9170_regwrite(AR9170_MAC_REG_AC3_CW, ar->edcf[3].cw_min |
	(ar->edcf[3].cw_max << 16));
	ar9170_regwrite(AR9170_MAC_REG_AC4_CW, ar->edcf[4].cw_min |
	(ar->edcf[4].cw_max << 16));

	ar9170_regwrite(AR9170_MAC_REG_AC2_AC1_AC0_AIFS,
	((ar->edcf[0].aifs * 9 + 10)) |
	((ar->edcf[1].aifs * 9 + 10) << 12) |
	((ar->edcf[2].aifs * 9 + 10) << 24));
	ar9170_regwrite(AR9170_MAC_REG_AC4_AC3_AC2_AIFS,
	((ar->edcf[2].aifs * 9 + 10) >> 8) |
	((ar->edcf[3].aifs * 9 + 10) << 4) |
	((ar->edcf[4].aifs * 9 + 10) << 16));

	ar9170_regwrite(AR9170_MAC_REG_AC1_AC0_TXOP,
	ar->edcf[0].txop | ar->edcf[1].txop << 16);
	ar9170_regwrite(AR9170_MAC_REG_AC3_AC2_TXOP,
	ar->edcf[2].txop | ar->edcf[3].txop << 16 |
	ar->edcf[4].txop << 24);

	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}


int ar9170_upload_key(struct ar9170 *ar, const U8 id, const U8 *mac,
const U8 ktype, const U8 keyidx, const U8 *keydata,
const int keylen)
{
	struct ar9170_set_key_cmd key = { };
	static const U8 bcast[ETH_ALEN] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	mac = mac ? : bcast;

	key.user = cpu_to_le16(id);
	key.keyId = cpu_to_le16(keyidx);
	key.type = cpu_to_le16(ktype);
	memcpy(&key.macAddr, mac, ETH_ALEN);
	if (keydata)
		memcpy(&key.key, keydata, keylen);

	return !ar9170_exec_cmd(ar, CARL9170_CMD_EKEY, sizeof(key), (U8 *)&key, 0, NULL);
}

int ar9170_disable_key(struct ar9170 *ar, const U8 id)
{
	struct ar9170_disable_key_cmd key = { };

	key.user = cpu_to_le16(id);

	return !ar9170_exec_cmd(ar, CARL9170_CMD_DKEY, sizeof(key), (U8 *)&key, 0, NULL);
}


int ar9170_set_operating_mode(struct ar9170 *ar)
{
	#if AR9170_MAC_DEBUG
	printf("Setting operating mode.\n");
	#endif
	
	struct ieee80211_vif *vif;
	struct ath_common *common = &ar->common;
	U8 *mac_addr, *bssid;
	U32 cam_mode = AR9170_MAC_CAM_DEFAULTS;
	U32 enc_mode = AR9170_MAC_ENCRYPTION_DEFAULTS;
	U32 rx_ctrl = AR9170_MAC_RX_CTRL_DEAGG |
	AR9170_MAC_RX_CTRL_SHORT_FILTER;
	U32 sniffer = AR9170_MAC_SNIFFER_DEFAULTS;
	int err = 0;

	__read_lock();
	vif = ar9170_get_main_vif(ar);

	if (vif) {
		#if AR9170_MAC_DEBUG
		printf("DEBUG: Virtual interface initialized.\n");
		#endif
		mac_addr = common->macaddr;
		bssid = common->curbssid; // FIXME
		
		#if AR9170_MAC_DEBUG
		printf("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]);
		#endif
		
		switch (vif->type) {
			case NL80211_IFTYPE_MESH_POINT:
			case NL80211_IFTYPE_ADHOC:
				cam_mode |= AR9170_MAC_CAM_IBSS;
				#if AR9170_MAC_DEBUG
				printf("DEBUG: Device is in Ad-Hoc Mode.\n");
				#endif
				break;
			case NL80211_IFTYPE_AP:
				cam_mode |= AR9170_MAC_CAM_AP;

				/* iwlagn 802.11n STA Workaround */
				rx_ctrl |= AR9170_MAC_RX_CTRL_PASS_TO_HOST;
				break;
			case NL80211_IFTYPE_WDS:
				cam_mode |= AR9170_MAC_CAM_AP_WDS;
				rx_ctrl |= AR9170_MAC_RX_CTRL_PASS_TO_HOST;
				break;
			case NL80211_IFTYPE_STATION:
				cam_mode |= AR9170_MAC_CAM_STA;
				rx_ctrl |= AR9170_MAC_RX_CTRL_PASS_TO_HOST;
				break;
			default:
				printf("WARNING: Unsupported operation mode %x\n", vif->type);
				err = -EOPNOTSUPP;
				break;
		}
	} else {
		printf("WARNING: Virtual interface was null.\n");
		mac_addr = NULL;
		bssid = NULL;
	}
	__read_unlock();

	if (err)
		return err;

	if (ar->rx_software_decryption) {
		#if AR9170_MAC_DEBUG
		printf("DEBUG: rx_sw decryption is enabled.\n");
		#endif
		enc_mode |= AR9170_MAC_ENCRYPTION_RX_SOFTWARE;
	}		

	if (ar->sniffer_enabled) {
		#if AR9170_MAC_DEBUG
		printf("DEBUG: MAC SNIFFER is enabled.\n");
		#endif
		rx_ctrl |= AR9170_MAC_RX_CTRL_ACK_IN_SNIFFER;
		sniffer |= AR9170_MAC_SNIFFER_ENABLE_PROMISC;
		enc_mode |= AR9170_MAC_ENCRYPTION_RX_SOFTWARE;
	}
		
	err = ar9170_set_mac_reg(ar, AR9170_MAC_REG_MAC_ADDR_L, mac_addr);
	if (err)
		return err;

	err = ar9170_set_mac_reg(ar, AR9170_MAC_REG_BSSID_L, bssid);
	if (err)
		return err;

	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_SNIFFER, sniffer);
	ar9170_regwrite(AR9170_MAC_REG_CAM_MODE, cam_mode);
	ar9170_regwrite(AR9170_MAC_REG_ENCRYPTION, enc_mode);
	ar9170_regwrite(AR9170_MAC_REG_RX_CONTROL, rx_ctrl);
	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}



le32_t get_unaligned_le32( const U8* param )
{
	le32_t return_value;
	memcpy(&return_value,param,4);
	return return_value;
}

le32_t get_unaligned_le16( const U8* param )
{
	le32_t return_value;
	memset(&return_value,0,4);
	memcpy(&return_value,param,2);
	return return_value;
}

int ar9170_set_mac_reg(struct ar9170 *ar, const U32 reg, const U8 *mac)
{
	#if AR9170_MAC_DEBUG
	printf("DEBUG: Set MAC REGISTER.\n");
	#endif
	
	static const U8 zero[ETH_ALEN] = { 0 };

	if (!mac)
		mac = zero;

	ar9170_regwrite_begin(ar);

	ar9170_regwrite(reg, get_unaligned_le32(mac)); 
	ar9170_regwrite(reg + 4, get_unaligned_le16(mac + 4));

	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}


int ar9170_set_slot_time(struct ar9170 *ar)
{
	#if AR9170_MAC_DEBUG_DEEP
	printf("DEBUG: ar9170_set_slot_time.\n");
	#endif
	
	struct ieee80211_vif *vif;
	U32 slottime = 20;

	__read_lock();
	vif = ar9170_get_main_vif(ar);
	if (!vif) {
		printf("WARNING: Virtual interface not initialized.\n");
		__read_unlock();
		return 0;
	}

	if ((ar->hw->conf.channel->band == IEEE80211_BAND_5GHZ) || vif->bss_conf.use_short_slot) {
		#if AR9170_MAC_DEBUG
		printf("DEBUG: Using short slot time.\n");
		#endif
		slottime = 9; /* This should give false at least in the beginning. */
	}
	__read_unlock();

	return !ar9170_write_rgstr(ar, AR9170_MAC_REG_SLOT_TIME, slottime << 10);
}

int ar9170_set_rts_cts_rate(struct ar9170 *ar)
{
	U32 rts_rate, cts_rate;

	if (conf_is_ht(&ar->hw->conf)) {
		/* 12 mbit OFDM */
		rts_rate = 0x1da;
		cts_rate = 0x10a;
	} else {
		if (ar->hw->conf.channel->band == IEEE80211_BAND_2GHZ) {
			/* 11 mbit CCK */
			rts_rate = 033;
			cts_rate = 003;
		} else {
			/* 6 mbit OFDM */
			rts_rate = 0x1bb;
			cts_rate = 0x10b;
		}
	}

	return !ar9170_write_rgstr(ar, AR9170_MAC_REG_RTS_CTS_RATE, rts_rate | (cts_rate) << 16);
}


int ar9170_set_mac_tpc(struct ar9170 *ar, struct ieee80211_channel *channel)
{
	#if AR9170_MAC_DEBUG_DEEP
	printf("DEBUG: ar9170_set_mac_tpc.\n");
	#endif
	
	unsigned int power, chains;

	
	if (ar->eeprom.tx_mask != 1)
		chains = AR9170_TX_PHY_TXCHAIN_2;
	else
		chains = AR9170_TX_PHY_TXCHAIN_1;

	switch (channel->band) {
		case IEEE80211_BAND_2GHZ:
			power = ar->power_2G_ofdm[0] & 0x3f;
			break;
		case IEEE80211_BAND_5GHZ:
			power = ar->power_5G_leg[0] & 0x3f;
			break;
		default:
			printf("WARNING: Channel band is not one of the possible 2 or 5 GHz.\n");
			power = 0; 
	}
	//Dirty XXX
	power = 1;
	power = min((unsigned int)power, (unsigned int)(ar->hw->conf.power_level * 2));

	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_ACK_TPC,	0x3c1e | power << 20 | chains << 26);
	ar9170_regwrite(AR9170_MAC_REG_RTS_CTS_TPC,	power << 5 | chains << 11 |	power << 21 | chains << 27);
	ar9170_regwrite(AR9170_MAC_REG_CFEND_QOSNULL_TPC, power << 5 | chains << 11 | power << 21 | chains << 27);
	ar9170_regwrite_finish();
	return ar9170_regwrite_result();
}


int ar9170_set_beacon_timers(struct ar9170 *ar)
{
	#if AR9170_MAC_DEBUG_DEEP
	printf("DEBUG: carl9170_set_beacon_timers.\n");
	#endif
	
	struct ieee80211_vif *vif;
	U32 v = 0;
	U32 w = 0;
	U32 pretbtt = 0;

	__read_lock();
	vif = ar9170_get_main_vif(ar);

	if (vif) {
		struct ar9170_vif_info *mvif;
		//mvif = (void *) vif->drv_priv;
		mvif = unique_cvif;
		
		if (mvif->enable_beacon && (!ar->beacon_enabled)) {
			printf("WARNING: Virtual interface and AR9170 structure disagree on beacon enable.\n");
		}
		
		if (mvif->enable_beacon && ar->beacon_enabled) { 

			ar->global_beacon_int = vif->bss_conf.beacon_int / ar->beacon_enabled;

			#if AR9170_MAC_DEBUG_DEEP
			printf("Current -v- value: %u.\n", v);
			#endif
			
			SET_VAL(AR9170_MAC_BCN_DTIM, v,	vif->bss_conf.dtim_period);

			#if AR9170_MAC_DEBUG_DEEP
			printf("DEBUG: Before case check. Current -v- value: %u.\n",v);
			#endif
			
			switch (vif->type) {
				case NL80211_IFTYPE_MESH_POINT:
				case NL80211_IFTYPE_ADHOC:
					v |= AR9170_MAC_BCN_IBSS_MODE;
					w |= AR9170_MAC_BCN_IBSS_MODE;
					#if AR9170_MAC_DEBUG_DEEP
					printf("IBSS Mode. Current -v- value: %d.\n",v);
					#endif
					break;
				case NL80211_IFTYPE_AP:
					v |= AR9170_MAC_BCN_AP_MODE;
					break;
				default:
					printf("WARNING: Beacon timers should not be set for other than adhoc or ap mode.\n");
				break;
			}
		} else if (vif->type == NL80211_IFTYPE_STATION) {
			ar->global_beacon_int = vif->bss_conf.beacon_int;

			SET_VAL(AR9170_MAC_BCN_DTIM, v,ar->hw->conf.ps_dtim_period);

			v |= AR9170_MAC_BCN_STA_PS | AR9170_MAC_BCN_PWR_MGT;
		}

		if (ar->global_beacon_int) {
			if (ar->global_beacon_int < 15) {
				__read_unlock();
				return -ERANGE;
			}

			ar->global_pretbtt = ar->global_beacon_int - AR9170_PRETBTT_KUS;
			#if AR9170_MAC_DEBUG_DEEP
			printf("DEBUG: Setting the global pre-TBTT to the beacon - 6 value: %d.\n",ar->global_pretbtt);
			#endif

		} else {
			printf("WARNING: Setting the global pre-TBTT to zero as BCN interval was zero as well.\n");
			ar->global_pretbtt = 0;
		}
	} else {
		printf("WARNING: Setting the global pre TBTT to zero as virtual interface was not initialized.\n");
		ar->global_beacon_int = 0;
		ar->global_pretbtt = 0;
	}

	__read_unlock();

	#if AR9170_MAC_DEBUG_DEEP
	printf("DEBUG: Before SET_VAL. Current -v- value: %u.\n",v);
	#endif
	
	SET_VAL(AR9170_MAC_ATIM_PERIOD, w, unique_vif->bss_conf.atim_window);
	SET_VAL(AR9170_MAC_BCN_PERIOD, v, ar->global_beacon_int);
	SET_VAL(AR9170_MAC_PRETBTT, pretbtt, ar->global_pretbtt);
	SET_VAL(AR9170_MAC_PRETBTT2, pretbtt, ar->global_pretbtt);

	#if AR9170_MAC_DEBUG_DEEP
	printf("DEBUG: After SET_VAL. Current -v- value: %u.\n",(unsigned int)v);
	#endif
	
	ar9170_regwrite_begin(ar);	
	ar9170_regwrite(AR9170_MAC_REG_PRETBTT, pretbtt);
	ar9170_regwrite(AR9170_MAC_REG_BCN_PERIOD, v);
	ar9170_regwrite(AR9170_MAC_REG_ATIM_WINDOW, w);
	ar9170_regwrite_finish();
	return ar9170_regwrite_result();
}

int ar9170_update_multicast(struct ar9170 *ar, const U64 mc_hash)
{
	#if AR9170_MAC_DEBUG
	printf("ar9170_update_multicast_start. constant mc_hash: %8x.\n",mc_hash);
	printf("%04x.\n",mc_hash >> 32);
	#endif
	int err;
	
	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_H, mc_hash >> 32);
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_L, mc_hash);
	ar9170_regwrite_finish();
	err = ar9170_regwrite_result();
	if (err)
		return err;

	ar->cur_mc_hash = mc_hash;
	
	return 0;
}

int ar9170_update_multicast_mine(struct ar9170 *ar, const U32 mc_hash_high, const U32 mc_hash_low)
{
	int err;
	
	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_H, mc_hash_high);
	ar9170_regwrite(AR9170_MAC_REG_GROUP_HASH_TBL_L, mc_hash_low);
	ar9170_regwrite_finish();
	err = ar9170_regwrite_result();
	if (err)
	return err;

	ar->cur_mc_hash = 0xc00001001;
	
	return 0;
}

void ar9170_schedule_bcn_ctrl(struct ar9170* ar)
{
	if (ar->beacon_ctrl == true)
		printf("ERROR: Beacon Control flag should have been cleared.\n");
	/* Set beacon control flag now */
	ar->beacon_ctrl = true;
}

void ar9170_schedule_bcn_cancel(struct ar9170* ar)
{
	if (ar->beacon_cancel == true)
		printf("ERROR: Beacon Control flag should be cleared.\n");
	/* Set beacon control flag now */
	ar->beacon_cancel = true;
}

/*
int ar9170_set_mac_tpc(struct ar9170 *ar, struct ieee80211_channel *channel)
{
	#if AR9170_MAC_DEBUG
	print("DEBUG: carl9170_set_mac_tpc begin.\n");
	#endif
	unsigned int power, chains;

	if (ar->eeprom.tx_mask != 1)
		chains = AR9170_TX_PHY_TXCHAIN_2;
	else
		chains = AR9170_TX_PHY_TXCHAIN_1;

	switch (channel->band) {
		case IEEE80211_BAND_2GHZ:
			power = ar->power_2G_ofdm[0] & 0x3f;
			break;
		case IEEE80211_BAND_5GHZ:
			power = ar->power_5G_leg[0] & 0x3f;
			break;
		default:
			printf("BUG: Band is unspecified.\n");
	}

	power = min((unsigned int)power, (unsigned int)(ar->hw->conf.power_level * 2));

	ar9170_regwrite_begin(ar);
	ar9170_regwrite(AR9170_MAC_REG_ACK_TPC,	0x3c1e | power << 20 | chains << 26);
	ar9170_regwrite(AR9170_MAC_REG_RTS_CTS_TPC,	power << 5 | chains << 11 |	power << 21 | chains << 27);
	ar9170_regwrite(AR9170_MAC_REG_CFEND_QOSNULL_TPC,	power << 5 | chains << 11 |	power << 21 | chains << 27);
	ar9170_regwrite_finish();

	return ar9170_regwrite_result();
}*/

