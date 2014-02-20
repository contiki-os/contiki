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
#include "ar9170_debug.h"
#include "ar9170.h"
#include "skbuff.h"
#include <sys\errno.h>
#include "kern.h"
#include "hw.h"
#include "ar9170_wlan.h"
#include "ar9170_main.h"
#include "fwcmd.h"
#include "usb_cmd_wrapper.h"
#include "ieee80211_ibss.h"
#include "compiler.h"
#include <stdio.h>
#include "bitops.h"
#include "usb_lock.h"
#include <sys\lock.h>
#include "ieee80211.h"
#include "delay.h"
#include "mac80211.h"
#include "ar9170_state.h"
#include "usb_data_wrapper.h"
#include "ar9170_psm.h"
#include "string.h"
#include "dsc.h"
#include "ieee80211_rx.h"
#include "etherdevice.h"


int ar9170_op_tx( struct ieee80211_hw *hw, struct sk_buff *skb )
{
	int i, err;
	UNUSED(i);
	
	#if AR9170_TX_DEBUG_DEEP
	printf("DEBUG: ar9170_op_tx. Length: %d.\n",skb->len);
	for (i=0; i<skb->len; i++)
		printf("%02x ", skb->data[i]);
	printf(" \n");
	#endif
	
	struct ar9170 *ar = (struct ar9170*)hw->priv;
	
	if (not_expected(!IS_STARTED(ar))) {
		printf("WARNING: Device not started. Dropping pending packet transmission.\n");
		goto err_free;
	}
	
	err = ar9170_tx_prepare(ar, skb);
	if(err) {
		printf("ERROR: ar9170_prepare returned errors.\n");
		goto err_free;
	}
		
	/* Send frame down to the tx queue */
	ar9170_tx(skb);
	
	return true;
err_free:
	printf("ERROR: Packet could not be handed down. Must be erased from buffer queue!\n");
	return false;	
}


int ar9170_tx_prepare(struct ar9170* ar, struct sk_buff* skb) {
	
	int i;
	bool no_ack, ampdu;
	le16_t mac_tmp;
	struct ieee80211_hdr *hdr;
	struct _ar9170_tx_superframe *txc;	
	struct ieee80211_tx_rate *txrate;
	struct ieee80211_tx_info *info;
	uint16_t len = skb->len;
	
	UNUSED(ampdu);
			
	#if AR9170_TX_DEBUG_DEEP
	printf("DEBUG: ar9170_tx_prepare. MAC length: %d.\n", len);
	#endif
	
	#if AR9170_TX_DEBUG_DEEP
	printf("SKB [%d]: ", len);
	for (i=0; i<len; i++)
		printf("%02x ", skb->data[i]);
	printf(" \n");
	#endif
	
	/* Append the PHY header in the beginning of the packet frame */
	txc = (struct _ar9170_tx_superframe*)smalloc(len + sizeof(*txc));
	if (txc == NULL) {
		printf("ERROR: Could not allocate memory for super frame.\n");
		return -ENOMEM;
	}
	memset(txc, 0, len + sizeof(*txc));
	
	/* Copy MAC frame to the new buffer */
	uint8_t* txc_start = (uint8_t*)txc;
	memcpy(txc_start + sizeof(*txc), skb->data, len);
	
	#if AR9170_TX_DEBUG_DEEP
	uint8_t* test = (uint8_t*)txc;
	printf("SKB [%d]: ",len + sizeof(*txc));
	for (i=0; i<len + sizeof(*txc); i++)
		printf("%02x ", test[i]);
	printf(" \n");
	#endif
			
	/* Free MAC frame memory */
	free(skb->data);
	skb->data = NULL;
	
	/* Reassign pointer for buffer data */
	skb->data = &txc[0];
	/* Update with the new frame length. */
	skb->len += sizeof(*txc); 
			
	/* Currently we have a single tx queue */
	unsigned int hw_queue = 1; 
	
	/* FIXME Currently a cookie flag is on */
	txc->s.cookie = 0x01;
	
	/*
	 * Cookie #0 serves two special purposes:
	 *  1. The firmware might use it generate BlockACK frames
	 *     in responds of an incoming BlockAckReqs.
	 *
	 *  2. Prevent double-free bugs.
	 */
	//super->s.cookie = (u8) cookie + 1;
		
	/* Set the hw queue byte */
	SET_VAL(AR9170_TX_SUPER_MISC_QUEUE, txc->s.misc, hw_queue);
	
	if (likely(unique_cvif))
		SET_VAL(AR9170_TX_SUPER_MISC_VIF_ID, txc->s.misc, unique_cvif->id);
/*  XXX FIXME
	Comes from the original carl9170 driver - need to have	
	if (unlikely(info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM))
		txc->s.misc |= CARL9170_TX_SUPER_MISC_CAB;

	if (unlikely(info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ))
		txc->s.misc |= CARL9170_TX_SUPER_MISC_ASSIGN_SEQ;
*/
	txc->s.misc |= AR9170_TX_SUPER_MISC_ASSIGN_SEQ;
	
	#if AR9170_TX_DEBUG_DEEP
	printf("SKB [%u]: ",skb->len);
	for (i=0; i<skb->len; i++)
		printf("%02x ", skb->data[i]);
	printf(" \n");
	#endif
		
	hdr = (struct ieee80211_hdr*) (skb->data + sizeof(*txc));
	if (unlikely(ieee80211_is_probe_resp(hdr->frame_control)))
		txc->s.misc |= AR9170_TX_SUPER_MISC_FILL_IN_TSF;
		
	mac_tmp = cpu_to_le16(AR9170_TX_MAC_HW_DURATION | AR9170_TX_MAC_BACKOFF);
	mac_tmp |= cpu_to_le16((hw_queue << AR9170_TX_MAC_QOS_S) & AR9170_TX_MAC_QOS);
	
	no_ack = false;
	
	/* If this is a broadcast frame, ACK should be disabled and TSF should be present. */
	const uint8_t broadcast_ethernet_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (not_expected(ether_addr_equal(broadcast_ethernet_addr, hdr->addr1))) {
		#if AR9170_TX_DEBUG_DEEP
		printf("DEBUG: AR9170_TX; Sending broadcast packet.\n");
		#endif
		no_ack = true;		
		//txc->s.misc |= AR9170_TX_SUPER_MISC_FILL_IN_TSF; //XXX Why was this here?
	}
	
	if (unlikely(no_ack))
		mac_tmp |= cpu_to_le16(AR9170_TX_MAC_NO_ACK);
		
	/* We do not currently support aggregate MPDU transmissions FIXME */
/*	
	info = (struct ieee80211_tx_info*)smalloc(sizeof(struct ieee80211_tx_info));
	info->band = ibss_info->ibss_channel->band; // Band info is all we need here
	
	txrate = (struct ieee80211_tx_rate*)smalloc(sizeof(struct ieee80211_tx_rate));
	for (i = 1; i < AR9170_TX_MAX_RATES; i++) {
		// --------------------------FIXME----------------------------//	
		
		if (i==1) {			
			txrate->count = 0x03;
			txrate->flags = 0x0;
			txrate->idx = 0x09;
		} else if (i==2) {
			txrate->count = 0x03;
			txrate->flags = 0x0;
			txrate->idx = 0x0b;
		} else if (i==3) {
			txrate->count = 0x02;
			txrate->flags = 0x0;
			txrate->idx = 0x00;
		}
		
		// Normal operations continue here //
		SET_VAL(AR9170_TX_SUPER_RI_TRIES, txc->s.ri[i],txrate->count);
		
		txc->s.rr[i - 1] = ar9170_tx_physet(ar, info, txrate);
		#if AR9170_TX_DEBUG_DEEP
		printf("rr[i-1]: %04x.\n",txc->s.rr[i - 1]);		
		#endif			
	}		
	// Control rate //
	txrate->count = 0x03;
	txrate->flags = 0x00;
	txrate->idx = 0x0b;
	
	SET_VAL(AR9170_TX_SUPER_RI_TRIES, txc->s.ri[0], txrate->count);

	if (not_expected(ether_addr_equal(broadcast_ethernet_addr, hdr->addr1))) {
		// HACK here. If it is broadcast frame, reduce the retries manually. //
		txrate->count = 0x01;
		SET_VAL(AR9170_TX_SUPER_RI_TRIES, txc->s.ri[0], txrate->count);	
	}		
*/	
	/* XXX Hack here. Until we sort this out, we set the rate manually. */
	txrate->count = 1;
	SET_VAL(AR9170_TX_SUPER_RI_TRIES, txc->s.ri[0], txrate->count);
	
	/* Set PHY header control info */
	
	/* Total length, including the PHY header */
	txc->s.len = cpu_to_le16(skb->len);
	/* Total MAC frame length including the FCS length [4]*/
	txc->f.length = cpu_to_le16(len + FCS_LEN);
	/* MAC control */
	txc->f.mac_control = mac_tmp;
	/* PHX control */
	txc->f.phy_control = ar9170_tx_physet(ar, info, txrate);
	
	/* XXX Hack here. Until I sort it out, we set it manually. */
	if (len < 75) 
		txc->f.phy_control = 0x002ec001;
	else
		txc->f.phy_control = 0x0002c400;
		
		
	#if AR9170_TX_DEBUG_DEEP
	printf("Total Length: %04x, MAC Length %04x MAC Ctrl: %04x PHY Ctrl: %08x.\n", 
			skb->len, len+FCS_LEN, mac_tmp, txc->f.phy_control);
	#endif
			
	/* free txrate info memory */
	free(txrate);
	free(info);
	return 0;
}	


void ar9170_tx_rate_tpc_chains(struct ar9170 *ar, struct ieee80211_tx_info *info, struct ieee80211_tx_rate *txrate,
unsigned int *phyrate, unsigned int *tpc, unsigned int *chains)
{
	struct ieee80211_rate *rate = NULL;
	uint8_t *txpower;
	unsigned int idx;
	
	idx = txrate->idx;
	*tpc = 0;
	*phyrate = 0;

	if (txrate->flags & IEEE80211_TX_RC_MCS) {
		if (txrate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH) {
			/* +1 dBm for HT40 */
			*tpc += 2;

			if (info->band == IEEE80211_BAND_2GHZ)
				txpower = ar->power_2G_ht40;
			else
				txpower = ar->power_5G_ht40;
		} else {
			if (info->band == IEEE80211_BAND_2GHZ)
				txpower = ar->power_2G_ht20;
			else
				txpower = ar->power_5G_ht20;
		}

		*phyrate = txrate->idx;
		*tpc += txpower[idx & 7];
	} else {
		if (info->band == IEEE80211_BAND_2GHZ) {
			if (idx < 4)
				txpower = ar->power_2G_cck;
			else
				txpower = ar->power_2G_ofdm;
		} else {
				txpower = ar->power_5G_leg;
				idx += 4;
		}

		rate = &__carl9170_ratetable[idx];
		*tpc += txpower[(rate->hw_value & 0x30) >> 4];
		*phyrate = rate->hw_value & 0xf;
	}

	if (ar->eeprom.tx_mask == 1) {
		*chains = AR9170_TX_PHY_TXCHAIN_1;
	} else {
		if (!(txrate->flags & IEEE80211_TX_RC_MCS) &&
			rate && rate->bitrate >= 360)
			*chains = AR9170_TX_PHY_TXCHAIN_1;
		else
			*chains = AR9170_TX_PHY_TXCHAIN_2;
	}

	*tpc = min((unsigned int) *tpc, (unsigned int) (ar->hw->conf.power_level * 2)); // TODO - check whether the power level is set
	
}

le32_t ar9170_tx_physet(struct ar9170 *ar,
	struct ieee80211_tx_info *info, struct ieee80211_tx_rate *txrate)
{
	unsigned int power = 0, chains = 0, phyrate = 0;
	le32_t tmp;

	tmp = cpu_to_le32(0);

	if (txrate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
		tmp |= cpu_to_le32(AR9170_TX_PHY_BW_40MHZ <<
			AR9170_TX_PHY_BW_S);
	/* this works because 40 MHz is 2 and dup is 3 */
	if (txrate->flags & IEEE80211_TX_RC_DUP_DATA)
		tmp |= cpu_to_le32(AR9170_TX_PHY_BW_40MHZ_DUP <<
			AR9170_TX_PHY_BW_S);

	if (txrate->flags & IEEE80211_TX_RC_SHORT_GI)
		tmp |= cpu_to_le32(AR9170_TX_PHY_SHORT_GI);

	if (txrate->flags & IEEE80211_TX_RC_MCS) {
		SET_VAL(AR9170_TX_PHY_MCS, phyrate, txrate->idx);

		/* heavy clip control */
		tmp |= cpu_to_le32((txrate->idx & 0x7) <<
			AR9170_TX_PHY_TX_HEAVY_CLIP_S);

		tmp |= cpu_to_le32(AR9170_TX_PHY_MOD_HT);

		/*
		 * green field preamble does not work.
		 *
		 * if (txrate->flags & IEEE80211_TX_RC_GREEN_FIELD)
		 * tmp |= cpu_to_le32(AR9170_TX_PHY_GREENFIELD);
		 */
	} else {
		if (info->band == IEEE80211_BAND_2GHZ) {
			if (txrate->idx <= AR9170_TX_PHY_RATE_CCK_11M)
				tmp |= cpu_to_le32(AR9170_TX_PHY_MOD_CCK);
			else
				tmp |= cpu_to_le32(AR9170_TX_PHY_MOD_OFDM);
		} else {
			tmp |= cpu_to_le32(AR9170_TX_PHY_MOD_OFDM);
		}

		/*
		 * short preamble seems to be broken too.
		 *
		 * if (txrate->flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE)
		 *	tmp |= cpu_to_le32(AR9170_TX_PHY_SHORT_PREAMBLE);
		 */
	}
	ar9170_tx_rate_tpc_chains(ar, info, txrate, &phyrate, &power, &chains);

	tmp |= cpu_to_le32(SET_CONSTVAL(AR9170_TX_PHY_MCS, phyrate));
	tmp |= cpu_to_le32(SET_CONSTVAL(AR9170_TX_PHY_TX_PWR, power));
	tmp |= cpu_to_le32(SET_CONSTVAL(AR9170_TX_PHY_TXCHAIN, chains));
	return tmp;
}


int ar9170_update_beacon(struct ar9170 *ar, bool submit) {
		
	#if AR9170_TX_DEBUG_DEEP
	printf("DEBUG: Updating BCN.\n");
	#endif
	
	struct ieee80211_tx_info *txinfo;
	struct ieee80211_tx_rate *rate;
	
	UNUSED(txinfo);
	UNUSED(rate);
	
	// The arrays for the updated and the current [old] beacons
	COMPILER_WORD_ALIGNED uint8_t *data, *old = NULL;
	uint32_t word, ht1, off, addr, len;
	int i = 0, err = 0;
	unsigned int plcp, power, chains;
	
	// The virtual interface info 
	struct ar9170_vif_info *cvif;
	
	// The buffer to hold the beacon data
	struct sk_buff *skb = NULL;

	//__read_lock();
	//cvif = ar->beacon_iter; 
	cvif = unique_cvif;
	
	if (!ar->beacon_enabled || i++) {
		printf("WARNING: Beaconing is not enabled.\n");
		goto out_unlock;	
	}
	
	// Extract the socket buffer structure from the [IBSS] structure TODO
	skb = ieee80211_beacon_get_tim(ar->hw, unique_vif, NULL, NULL);
	
	if (!skb || skb->data == NULL) {
		printf("ERROR: Beacon data were not found!.\n");
		err = -ENOMEM;
		goto err_free;
	}
	
	// Lock execution
	//__lock_acquire(&ar->beacon_lock);
	
	// Extract beacon data from the socket buffer
	data = (uint8_t*)skb->data;
		
	// Check if the beacon information is already stored in the virtual interface buffer
	if (cvif != 0)
		if(cvif->beacon != 0)
			if(cvif->beacon->data != 0) {
				#if AR9170_TX_DEBUG_DEEP
				printf("INFO: Beacon data already stored.\n");
				#endif
				old = (uint8_t*)(cvif->beacon->data); 
			}		
	
	#if AR9170_TX_DEBUG_DEEP
	printf("BCN [%d]: ",(unsigned int)(skb->len));
	for (i=0; i<skb->len; i++)
		printf("%02x ",data[i]);
	printf(" \n");
	if (old != NULL) {
		printf("OLD [%d]: ",(unsigned int)(cvif->beacon->len));
		for (i=0; i<cvif->beacon->len; i++)
			printf("%02x ",old[i]);
		printf(" \n");
	}		
	#endif	
	
	off = cvif->id * AR9170_MAC_BCN_LENGTH_MAX; // TODO this is zero, so put it on the object
	
	// The address in the device to store the beacon data [starting address]
	addr = ar->fw.beacon_addr + off;
	
	// The total length of the data to store [data + FCS which is 4 bytes]
	len = roundup(skb->len + FCS_LEN, 4);
	
	// If the length exceeds the maximum allowed length, print an error message and DO NOT send the beacon
	if ((off + len) > ar->fw.beacon_max_len) {
		printf("ERROR: beacon does not fit into device memory!\n");
		err = -EINVAL;
		goto err_unlock;
	}
	
	if (len > AR9170_MAC_BCN_LENGTH_MAX) {
			printf("ERROR: no support for beacons bigger than %d (yours:%d).\n", 
					AR9170_MAC_BCN_LENGTH_MAX, (unsigned int)len);
		err = -EMSGSIZE;
		goto err_unlock;
	}
	
	ht1 = AR9170_MAC_BCN_HT1_TX_ANT0;	


/* // TODO - check later what to do with this	
	rate = &txinfo->control.rates[0];
	ar9170_tx_rate_tpc_chains(ar, txinfo, rate, &plcp, &power, &chains);
	
	
	if (!(txinfo->control.rates[0].flags & IEEE80211_TX_RC_MCS)) {
		if (plcp <= AR9170_TX_PHY_RATE_CCK_11M)
			plcp |= ((skb->len + FCS_LEN) << (3 + 16)) + 0x0400;
		else
			plcp |= ((skb->len + FCS_LEN) << 16) + 0x0010;
	} else {
		ht1 |= AR9170_MAC_BCN_HT1_HT_EN;
		if (rate->flags & IEEE80211_TX_RC_SHORT_GI)
			plcp |= AR9170_MAC_BCN_HT2_SGI;

		if (rate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH) {
			ht1 |= AR9170_MAC_BCN_HT1_BWC_40M_SHARED;
			plcp |= AR9170_MAC_BCN_HT2_BW40;
		}
		if (rate->flags & IEEE80211_TX_RC_DUP_DATA) {
			ht1 |= AR9170_MAC_BCN_HT1_BWC_40M_DUP;
			plcp |= AR9170_MAC_BCN_HT2_BW40;
		}

		SET_VAL(AR9170_MAC_BCN_HT2_LEN, plcp, skb->len + FCS_LEN);
	}

*/
	
	// FIXME - copied manually MUST consider fixing it
	plcp = 0 | (((skb->len + FCS_LEN) << (3 + 16)) + 0x0400); 
	power = 34;
	chains = 5;
	
	SET_VAL(AR9170_MAC_BCN_HT1_PWR_CTRL, ht1, 7);
	SET_VAL(AR9170_MAC_BCN_HT1_TPC, ht1, power);
	SET_VAL(AR9170_MAC_BCN_HT1_CHAIN_MASK, ht1, chains);
	if (chains == AR9170_TX_PHY_TXCHAIN_2)
		ht1 |= AR9170_MAC_BCN_HT1_TX_ANT1;

	if(!submit) {
		ar9170_async_regwrite_begin(ar);
		ar9170_async_regwrite(AR9170_MAC_REG_BCN_HT1, ht1);
	//	if (!(txinfo->control.rates[0].flags & IEEE80211_TX_RC_MCS))
			//ar9170_regwrite(AR9170_MAC_REG_BCN_PLCP, plcp);
			ar9170_async_regwrite(AR9170_MAC_REG_BCN_PLCP, plcp);
	//	else
	//		ar9170_async_regwrite(AR9170_MAC_REG_BCN_HT2, plcp);

		for (i=0; i<DIV_ROUND_UP(skb->len, 4); i++) {
			/*
			 * XXX: This accesses beyond skb data for up
			 *	to the last 3 bytes!!
			 */
			if (old && (data[i] == old[i]))
				continue;

			memcpy(&word, &data[4*i], 4);
	
			//word = (uint32_t)data[i];//le32_to_cpu((uint32_t)data[i]);
			ar9170_async_regwrite(addr + 4 * i, word);
			delay_ms(1);
		}	
		ar9170_async_regwrite_finish();
	/*
		if(cvif->beacon) {
			printf("WILL FREE cvif->beacon.\n");
			//free(cvif->beacon); FIXME 
			cvif->beacon = NULL;		
		}
	*/
		/* Copy the beacon on the vif_info struct if not there already */
		if (old == NULL) {
			#if AR9170_TX_DEBUG_DEEP
			printf("DEBUG: Copying beacon data on the ieee80211_vif_info structure.\n");
			#endif
			unique_cvif->beacon = smalloc(sizeof(struct sk_buff));
			if (unique_cvif->beacon == NULL) {
				printf("ERROR: Could not allocate memory for beacon data.\n");
				err = -1;
				goto err_free;
			}
			unique_cvif->beacon->data = smalloc(AR9170_MAC_BCN_LENGTH_MAX);
			if (unique_cvif->beacon->data == NULL) {
				printf("ERROR: Could not allocate memory for beacon data.\n");
				err = -1;
				goto err_free;
			}
			memcpy(unique_cvif->beacon->data, skb->data, skb->len);
			unique_cvif->beacon->len = skb->len;
		}

		err = ar9170_async_regwrite_result();
		if (!err) {
			#if AR9170_TX_DEBUG_DEEP
			printf("DEBUG: BCN info sent down successfully.\n");
			#endif
			//cvif->beacon = skb; // Assign pointer: TODO - consider holding an actual copy
			goto submit_bcn;
	
		} else {
			printf("ERROR: Could not write to registers for the beacon.\n");
			goto err_free;
		}
	
	}

submit_bcn:	
	if (submit) {
		#if AR9170_TX_DEBUG_DEEP
		printf("DEBUG: BCN submit [%d].\n", skb->len);
		#endif
		err = ar9170_bcn_ctrl(ar, cvif->id,	AR9170_BCN_CTRL_CAB_TRIGGER, addr, skb->len + FCS_LEN);

		if (err) {
			printf("ERROR: Beacon control command returned errors.\n");
			goto err_free;
		
		} else {
			#if AR9170_TX_DEBUG_DEEP
			printf("DEBUG: Beacon submitted successfully.\n");
			#endif
		}			
	}	
	
out_unlock:	
	//__read_unlock();
	//__lock_release(&ar->beacon_lock);
	return err;

err_free:
	//__lock_release(&ar->beacon_lock);
	//__read_unlock();
err_unlock:
	return err;		
}


int ar9170_send_beacon_cmd(struct ar9170* ar) {
	
	return ar9170_bcn_ctrl(ar, unique_cvif->id, 
		AR9170_BCN_CTRL_CAB_TRIGGER, ar->fw.beacon_addr, ibss_info->ibss_beacon_buf->len + FCS_LEN);
}


void ar9170_tx_drop(struct ar9170 *ar, struct sk_buff *skb)
{
/*	FIXME
	struct _carl9170_tx_superframe *super;
	uint8_t q = 0;
*/
	ar->tx_dropped++;
/*
	super = (void *)skb->data;
	SET_VAL(CARL9170_TX_SUPER_MISC_QUEUE, q,
	ar9170_qmap[carl9170_get_queue(ar, skb)]);
	__carl9170_tx_process_status(ar, super->s.cookie, q);
*/	
}


void ar9170_tx(struct sk_buff* skb) {
	
	int i;
	UNUSED(i);
	
	struct ar9170* ar = ar9170_get_device();
	
	if (unlikely(!IS_STARTED(ar))) {
		
		printf("ERROR: Device can not send because it is not started.\n");
		return;
	}
	
	#if AR9170_TX_DEBUG_DEEP
	printf("SKB: ");
	for (i=0; i<skb->len; i++)
		printf("%02x ", skb->data[i]);
	printf(" \n");
	#endif
	
	ar9170_usb_tx(ar, skb);
	
}

/* This function is called inside interrupt context. */
void ar9170_tx_process_status(struct ar9170 *ar, const struct ar9170_rsp *cmd)
{
	unsigned int i, j;
	uint8_t* cmd_dump = (void*)cmd;

	for (i = 0;  i < cmd->hdr.ext; i++) {
		if ((i > ((cmd->hdr.len / 2) + 1))) {
			for (j=0; j<cmd->hdr.len + 4; j++)
				printf("%02x ", cmd_dump[j]);
			break;
		}		
		__ar9170_tx_process_status(ar, cmd->_tx_status[i].cookie, cmd->_tx_status[i].info);
	}
}

/* This function is called inside interrupt context. */
void __ar9170_tx_status( struct ar9170 * ar, struct sk_buff * skb, bool success ) 
{
	UNUSED(skb);
	
	if ((ar->tx_data_wait == false) && (ar->tx_atim_wait == false)) {
		
		//printf("WARNING: SR for no set flag.\n");
		printf("SR?\n");
		/* Due to early freeing of these flags, we may lose 
		 * packet statistics. So we check the relative flags
		 * of freeing the ATIM and TX RECV flags and update
		 * the respective packet statistics. We only consider
		 * ATIM packets here.
		 */
		if (ar->atim_wait_cleared_count  > 0) {
			ar->atim_wait_cleared_count--;
		}
		return;
	}
	
	/*
	 * Handle processing to different directions depending on whether the
	 * device is within the ATIM Window [in Power-Save mode], or in the 
	 * Data Transmission period.
	 */
	if (ar->hw->conf.flags & IEEE80211_CONF_PS) {
		
		/*
		 * Device operates under PS mode, so we need to check,
		 * whether we got this status response inside the ATIM
		 * Window. If yes, then the previously sent packet is 
		 * an ATIM frame and we need to release the ATIM wait 
		 * flag.
		 */
		if (ar->ps_mgr.psm_state == AR9170_ATIM_WINDOW) {
			/* Notice that it does not matter whether the response
			 * is a successful one or not, for the release of the
			 * flag.
			 */
			if (ar->tx_atim_wait == true) {
				
				__complete(&ar->tx_atim_wait);

				/* Hand-in the control to the PSM core. */
				ieee80211_handle_ATIM_status_rsp(success);									
				
			} else if (ar->tx_data_wait == true) {
				
				__complete(&ar->tx_data_wait);
				__complete(&ar->clear_tx_data_wait_at_next_tbtt);
				
				/* The response is not from an ATIM transmission, 
				 * but from a DATA transmission that happened to 
				 * complete after the expiration of the Data TX 
				 * period. Question: can this really, happen, as
				 * we actually have a pre-TBTT window period right
				 * before the ATIM period?
				 */
				#if AR9170_TX_DEBUG_DEEP
				printf("WARNING: ACK for DATA packet arrived during the ATIM Window!\n");
				#endif
							
			} 	
		
		} else {
			/*
			 * We are currently in the TX or in the pre-TBTT period;
			 * if we got a status response while the ATIM wait flag
			 * is ON, we know this was an ATIM packet that was sent
			 * right before the ATIM Window expiration.
			 */
			if (not_expected(ar->tx_atim_wait == true)) {
				
				__complete(&ar->tx_atim_wait);
				/* Handle the control to the PSM core */
				ieee80211_handle_ATIM_status_rsp(success);
				
				/* Signal a warning [perhaps a debug should be ok] */
				#if AR9170_TX_DEBUG_DEEP
				printf("WARNING: ATIM Status response arrived after the expiration of the ATIM Window.\n");
				#endif
				
			} else if (ar->tx_data_wait == true) {
				
				__complete(&ar->tx_data_wait);
				if (ar->clear_tx_data_wait_at_next_tbtt == true) {
					__complete(&ar->clear_tx_data_wait_at_next_tbtt);
				}				
				
				/* This is a status response for a Data packet, and
				 * as such, it should be handled separately.
				 */
				
			} 			
		}
				
	} else {
		
		/* We are not in PSM so we do not send ATIM frames. This is
		 * a status response for a Data packet.
		 */
		if (ar->tx_atim_wait == true) {
			
			printf("ERROR: Got ATIM status response while not in PSM.\n");
			/* Should we release the flag? */
		} else if (ar->tx_data_wait == true) {
			/* TODO - handle status responses of stations in non-PSM. */
			__complete(&ar->tx_data_wait);
			if (ar->clear_tx_data_wait_at_next_tbtt == true) {
				__complete(&ar->clear_tx_data_wait_at_next_tbtt);
			}
			
		}
	}
}


/* This function is called inside interrupt context. */
void __ar9170_tx_process_status(struct ar9170 *ar,const uint8_t cookie, const uint8_t info)
{
	struct sk_buff *skb = NULL;
	struct ieee80211_tx_info *txinfo;
	unsigned int r, t, q;
	bool success = true;

	q = ar9170_qmap[info & AR9170_TX_STATUS_QUEUE];

	//skb = carl9170_get_queued_skb(ar, cookie, &ar->tx_status[q]);
	//if (!skb) {
		/*
		 * We have lost the race to another thread.
		 */

		//return ;
	//}

	//txinfo = IEEE80211_SKB_CB(skb);

	if (!(info & AR9170_TX_STATUS_SUCCESS)) {
		success = false;
		#if AR9170_RX_DEBUG_DEEP
		printf("WARNING: TX status reported error!\n");
		#endif
	} else {
		#if AR9170_TX_DEBUG_DEEP
		printf("DEBUG: TX; Packet sent successfully!\n");
		#endif
	}		

	r = (info & AR9170_TX_STATUS_RIX) >> AR9170_TX_STATUS_RIX_S;
	t = (info & AR9170_TX_STATUS_TRIES) >> AR9170_TX_STATUS_TRIES_S;

	//carl9170_tx_fill_rateinfo(ar, r, t, txinfo);
	__ar9170_tx_status(ar, skb, success);
}





