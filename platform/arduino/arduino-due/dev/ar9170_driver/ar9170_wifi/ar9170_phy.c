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
#include "ar9170_phy.h"
#include "fwcmd.h"
#include "ar9170eeprom.h"
#include "cfg80211.h"
#include <sys\errno.h>
#include "bitrev.h"
#include "ar9170_mac.h"
#include "regd.h"
#include "usb_cmd_wrapper.h"
#include "bitops.h"
#include "ar9170_debug.h"
#include "hw.h"
#include "compiler.h"
#include "ar9170.h"
#include "usb_fw_wrapper.h"


U8 ar9170_get_heavy_clip(struct ar9170 *ar, U32 freq,
enum ar9170_bw bw, struct ar9170_calctl_edges edges[])
{
	U8 f;
	int i;
	U8 rc = 0;

	if (freq < 3000)
		f = freq - 2300;
	else
		f = (freq - 4800) / 5;

	if (bw == CARL9170_BW_40_BELOW || bw == CARL9170_BW_40_ABOVE)
	rc |= 0xf0;

	for (i = 0; i < AR5416_NUM_BAND_EDGES; i++) {
		if (edges[i].channel == 0xff)
		break;
		if (f == edges[i].channel) {
			if (!(edges[i].power_flags & AR9170_CALCTL_EDGE_FLAGS))
			rc |= 0x0f;
			break;
		}
	}

	return rc;
}


int ar9170_find_freq_idx(int nfreqs, U8 *freqs, U8 f)
{
	int idx = nfreqs - 2;

	while (idx >= 0) {
		if (f >= freqs[idx])
		return idx;
		idx--;
	}

	return 0;
}

U8 ar9170_interpolate_u8(U8 x, U8 x1, U8 y1, U8 x2, U8 y2)
{
#define SHIFT		8
	S32 y;

	y = ar9170_interpolate_s32(x << SHIFT, x1 << SHIFT,
		y1 << SHIFT, x2 << SHIFT, y2 << SHIFT);

	/*
	 * XXX: unwrap this expression
	 *	Isn't it just DIV_ROUND_UP(y, 1<<SHIFT)?
	 *	Can we rely on the compiler to optimize away the div?
	 */
	return (y >> SHIFT) + ((y & (1 << (SHIFT - 1))) >> (SHIFT - 1));
#undef SHIFT
}

S32 ar9170_interpolate_s32(S32 x, S32 x1, S32 y1, S32 x2, S32 y2)
{
	/* nothing to interpolate, it's horizontal */
	if (y2 == y1)
	return y1;

	/* check if we hit one of the edges */
	if (x == x1)
	return y1;
	if (x == x2)
	return y2;

	/* x1 == x2 is bad, hopefully == x */
	if (x2 == x1)
	return y1;

	return y1 + (((y2 - y1) * (x - x1)) / (x2 - x1));
}


U8 ar9170_interpolate_val(U8 x, U8 *x_array, U8 *y_array)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (x <= x_array[i + 1])
		break;
	}

	return ar9170_interpolate_u8(x, x_array[i], y_array[i],
	x_array[i + 1], y_array[i + 1]);
}


int ar9170_init_rf_banks_0_7(struct ar9170 *ar, bool band5ghz)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_init_rf_banks.\n");
	#endif
	int err, i;

	ar9170_regwrite_begin(ar);

	for (i = 0; i < ARRAY_SIZE(ar9170_rf_initval); i++)
	ar9170_regwrite(ar9170_rf_initval[i].reg,
	band5ghz ? ar9170_rf_initval[i]._5ghz
	: ar9170_rf_initval[i]._2ghz);

	ar9170_regwrite_finish();
	err = ar9170_regwrite_result();
	if (err)
		printf("ERROR: rf init failed\n");
	
	return err;
}


int ar9170_init_rf_bank4_pwr(struct ar9170 *ar, bool band5ghz,
U32 freq, enum ar9170_bw bw)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: init_rf_bank4_pwr begin. 5ghz: %d, freq: %u, bw: %d.\n",band5ghz, (unsigned int)freq, bw);
	#endif
	
	int err;
	U32 d0, d1, td0, td1, fd0, fd1;
	U8 chansel;
	U8 refsel0 = 1, refsel1 = 0;
	U8 lf_synth = 0;

	switch (bw) {
		case CARL9170_BW_40_ABOVE:
			freq += 10;
			break;
		case CARL9170_BW_40_BELOW:
			freq -= 10;
			break;
		case CARL9170_BW_20:
			break;
		default:
			printf("BUG: Bandwidth was undefined.\n");
		return -ENOSYS;
	}

	if (band5ghz) {
		if (freq % 10) {
			chansel = (freq - 4800) / 5;
		} else {
			chansel = ((freq - 4800) / 10) * 2;
			refsel0 = 0;
			refsel1 = 1;
		}
		chansel = byte_rev_table[chansel];
	} else {
		if (freq == 2484) {
			chansel = 10 + (freq - 2274) / 5;
			lf_synth = 1;
		} else
		chansel = 16 + (freq - 2272) / 5;
		chansel *= 4;
		chansel = byte_rev_table[chansel];
	}

	d1 =	chansel;
	d0 =	0x21 |
	refsel0 << 3 |
	refsel1 << 2 |
	lf_synth << 1;
	td0 =	d0 & 0x1f;
	td1 =	d1 & 0x1f;
	fd0 =	td1 << 5 | td0;

	td0 =	(d0 >> 5) & 0x7;
	td1 =	(d1 >> 5) & 0x7;
	fd1 =	td1 << 5 | td0;

	ar9170_regwrite_begin(ar);

	ar9170_regwrite(0x1c58b0, fd0);
	ar9170_regwrite(0x1c58e8, fd1);

	ar9170_regwrite_finish();
	err = ar9170_regwrite_result();
	if (err) {
		printf("ERROR: RF bank initialization returned errors.\n");
		return err;
	}
	return 0;
}


int ar9170_init_power_cal(struct ar9170 *ar)
{
	ar9170_regwrite_begin(ar);

	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE_MAX, 0x7f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE1, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE2, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE3, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE4, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE5, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE6, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE7, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE8, 0x3f3f3f3f);
	ar9170_regwrite(AR9170_PHY_REG_POWER_TX_RATE9, 0x3f3f3f3f);

	ar9170_regwrite_finish();
	return ar9170_regwrite_result();
}



/*
 * initialize some phy regs from eeprom values in modal_header[]
 * acc. to band and bandwidth
 */
int ar9170_init_phy_from_eeprom(struct ar9170 *ar,
				bool is_2ghz, bool is_40mhz)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_init_phy_from_eeprom begins.\n");
	#endif
	
	static const U8 xpd2pd[16] = {
		0x2, 0x2, 0x2, 0x1, 0x2, 0x2, 0x6, 0x2,
		0x2, 0x3, 0x7, 0x2, 0xb, 0x2, 0x2, 0x2
	};
	/* pointer to the modal_header acc. to band */
	struct ar9170_eeprom_modal *m = &ar->eeprom.modal_header[is_2ghz];
	U32 val;

	ar9170_regwrite_begin(ar);

	/* ant common control (index 0) */
	ar9170_regwrite(AR9170_PHY_REG_SWITCH_COM,
		le32_to_cpu(m->antCtrlCommon));

	/* ant control chain 0 (index 1) */
	ar9170_regwrite(AR9170_PHY_REG_SWITCH_CHAIN_0,
		le32_to_cpu(m->antCtrlChain[0]));

	/* ant control chain 2 (index 2) */
	ar9170_regwrite(AR9170_PHY_REG_SWITCH_CHAIN_2,
		le32_to_cpu(m->antCtrlChain[1]));

	/* SwSettle (index 3) */
	if (!is_40mhz) {
		val = ar9170_def_val(AR9170_PHY_REG_SETTLING,
				     is_2ghz, is_40mhz);
		SET_VAL(AR9170_PHY_SETTLING_SWITCH, val, m->switchSettling);
		ar9170_regwrite(AR9170_PHY_REG_SETTLING, val);
	}

	/* adcDesired, pdaDesired (index 4) */
	val = ar9170_def_val(AR9170_PHY_REG_DESIRED_SZ, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_DESIRED_SZ_PGA, val, m->pgaDesiredSize);
	SET_VAL(AR9170_PHY_DESIRED_SZ_ADC, val, m->adcDesiredSize);
	ar9170_regwrite(AR9170_PHY_REG_DESIRED_SZ, val);

	/* TxEndToXpaOff, TxFrameToXpaOn (index 5) */
	val = ar9170_def_val(AR9170_PHY_REG_RF_CTL4, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_RF_CTL4_TX_END_XPAB_OFF, val, m->txEndToXpaOff);
	SET_VAL(AR9170_PHY_RF_CTL4_TX_END_XPAA_OFF, val, m->txEndToXpaOff);
	SET_VAL(AR9170_PHY_RF_CTL4_FRAME_XPAB_ON, val, m->txFrameToXpaOn);
	SET_VAL(AR9170_PHY_RF_CTL4_FRAME_XPAA_ON, val, m->txFrameToXpaOn);
	ar9170_regwrite(AR9170_PHY_REG_RF_CTL4, val);

	/* TxEndToRxOn (index 6) */
	val = ar9170_def_val(AR9170_PHY_REG_RF_CTL3, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_RF_CTL3_TX_END_TO_A2_RX_ON, val, m->txEndToRxOn);
	ar9170_regwrite(AR9170_PHY_REG_RF_CTL3, val);

	/* thresh62 (index 7) */
	val = ar9170_def_val(0x1c8864, is_2ghz, is_40mhz);
	val = (val & ~0x7f000) | (m->thresh62 << 12);
	ar9170_regwrite(0x1c8864, val);

	/* tx/rx attenuation chain 0 (index 8) */
	val = ar9170_def_val(AR9170_PHY_REG_RXGAIN, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_RXGAIN_TXRX_ATTEN, val, m->txRxAttenCh[0]);
	ar9170_regwrite(AR9170_PHY_REG_RXGAIN, val);

	/* tx/rx attenuation chain 2 (index 9) */
	val = ar9170_def_val(AR9170_PHY_REG_RXGAIN_CHAIN_2,
			       is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_RXGAIN_TXRX_ATTEN, val, m->txRxAttenCh[1]);
	ar9170_regwrite(AR9170_PHY_REG_RXGAIN_CHAIN_2, val);

	/* tx/rx margin chain 0 (index 10) */
	val = ar9170_def_val(AR9170_PHY_REG_GAIN_2GHZ, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_GAIN_2GHZ_RXTX_MARGIN, val, m->rxTxMarginCh[0]);
	/* bsw margin chain 0 for 5GHz only */
	if (!is_2ghz)
		SET_VAL(AR9170_PHY_GAIN_2GHZ_BSW_MARGIN, val, m->bswMargin[0]);
	ar9170_regwrite(AR9170_PHY_REG_GAIN_2GHZ, val);

	/* tx/rx margin chain 2 (index 11) */
	val = ar9170_def_val(AR9170_PHY_REG_GAIN_2GHZ_CHAIN_2,
			       is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_GAIN_2GHZ_RXTX_MARGIN, val, m->rxTxMarginCh[1]);
	ar9170_regwrite(AR9170_PHY_REG_GAIN_2GHZ_CHAIN_2, val);

	/* iqCall, iqCallq chain 0 (index 12) */
	val = ar9170_def_val(AR9170_PHY_REG_TIMING_CTRL4(0),
			       is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF, val, m->iqCalICh[0]);
	SET_VAL(AR9170_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF, val, m->iqCalQCh[0]);
	ar9170_regwrite(AR9170_PHY_REG_TIMING_CTRL4(0), val);

	/* iqCall, iqCallq chain 2 (index 13) */
	val = ar9170_def_val(AR9170_PHY_REG_TIMING_CTRL4(2),
			       is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF, val, m->iqCalICh[1]);
	SET_VAL(AR9170_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF, val, m->iqCalQCh[1]);
	ar9170_regwrite(AR9170_PHY_REG_TIMING_CTRL4(2), val);

	/* xpd gain mask (index 14) */
	val = ar9170_def_val(AR9170_PHY_REG_TPCRG1, is_2ghz, is_40mhz);
	SET_VAL(AR9170_PHY_TPCRG1_PD_GAIN_1, val,
		xpd2pd[m->xpdGain & 0xf] & 3);
	SET_VAL(AR9170_PHY_TPCRG1_PD_GAIN_2, val,
		xpd2pd[m->xpdGain & 0xf] >> 2);
	ar9170_regwrite(AR9170_PHY_REG_TPCRG1, val);

	ar9170_regwrite(AR9170_PHY_REG_RX_CHAINMASK, ar->eeprom.rx_mask);
	ar9170_regwrite(AR9170_PHY_REG_CAL_CHAINMASK, ar->eeprom.rx_mask);

	ar9170_regwrite_finish();
	
	return ar9170_regwrite_result();
}



int ar9170_init_phy(struct ar9170 *ar, enum ieee80211_band band)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_init_phy begin.\n");
	#endif
	
	int i, err;
	U32 val;
	bool is_2ghz = band == IEEE80211_BAND_2GHZ;
	bool is_40mhz = conf_is_ht40(&ar->hw->conf);

	ar9170_regwrite_begin(ar);

	for (i = 0; i < ARRAY_SIZE(ar5416_phy_init); i++) {
		if (is_40mhz) {
			if (is_2ghz)
				val = ar5416_phy_init[i]._2ghz_40;
			else
				val = ar5416_phy_init[i]._5ghz_40;
		} else {
			if (is_2ghz)
				val = ar5416_phy_init[i]._2ghz_20;
			else
				val = ar5416_phy_init[i]._5ghz_20;
		}

		ar9170_regwrite(ar5416_phy_init[i].reg, val);
	}

	ar9170_regwrite_finish();
	err = ar9170_regwrite_result();
	if (err) {
		printf("ERROR: Init PHY could not write to registers.\n");
		return err;	
	}
	
	err = ar9170_init_phy_from_eeprom(ar, is_2ghz, is_40mhz);
	if (err) {
		printf("ERROR: Could not initialize PHY from eeprom.\n");
		return err;	
	}	

	err = ar9170_init_power_cal(ar);
	if (err) {
		printf("ERROR: Could not initialize power calculation.\n");
		return err;
	}

	if (!ar->fw.hw_counters) {
		err = !ar9170_write_rgstr(ar, AR9170_PWR_REG_PLL_ADDAC,
		is_2ghz ? 0x5163 : 0x5143);
	}
	
	return err;
}


/*
 * calculate the conformance test limits and the heavy clip parameter
 * and apply them to ar->power* (derived from otus hal/hpmain.c, line 3706)
 */
void ar9170_calc_ctl(struct ar9170 *ar, U32 freq, enum ar9170_bw bw)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_calc_ctl starts.\n");
	#endif
	
	U8 ctl_grp; /* CTL group */
	U8 ctl_idx; /* CTL index */
	int i, j;
	struct ctl_modes {
		U8 ctl_mode;
		U8 max_power;
		U8 *pwr_cal_data;
		int pwr_cal_len;
	} *modes;

	/*
	 * order is relevant in the mode_list_*: we fall back to the
	 * lower indices if any mode is missed in the EEPROM.
	 */
	struct ctl_modes mode_list_2ghz[] = {
		{ CTL_11B, 0, ar->power_2G_cck, 4 },
		{ CTL_11G, 0, ar->power_2G_ofdm, 4 },
		{ CTL_2GHT20, 0, ar->power_2G_ht20, 8 },
		{ CTL_2GHT40, 0, ar->power_2G_ht40, 8 },
	};
	struct ctl_modes mode_list_5ghz[] = {
		{ CTL_11A, 0, ar->power_5G_leg, 4 },
		{ CTL_5GHT20, 0, ar->power_5G_ht20, 8 },
		{ CTL_5GHT40, 0, ar->power_5G_ht40, 8 },
	};
	int nr_modes;

#define EDGES(c, n) (ar->eeprom.ctl_data[c].control_edges[n])

	ar->heavy_clip = 0;

	/*
	 * TODO: investigate the differences between OTUS'
	 * hpreg.c::zfHpGetRegulatoryDomain() and
	 * ath/regd.c::ath_regd_get_band_ctl() -
	 * e.g. for FCC3_WORLD the OTUS procedure
	 * always returns CTL_FCC, while the one in ath/ delivers
	 * CTL_ETSI for 2GHz and CTL_FCC for 5GHz.
	 */
	ctl_grp = ath_regd_get_band_ctl(&ar->common.regulatory,
					ar->hw->conf.channel->band);

	#if AR9170_PHY_DEBUG_DEEP
	printf("ctl_grp: %d.\n",ctl_grp);
	#endif
	
	/* ctl group not found - either invalid band (NO_CTL) or ww roaming */
	if (ctl_grp == NO_CTL || ctl_grp == SD_NO_CTL)
		ctl_grp = CTL_FCC;

	if (ctl_grp != CTL_FCC)
		/* skip CTL and heavy clip for CTL_MKK and CTL_ETSI */
		return;

	if (ar->hw->conf.channel->band == IEEE80211_BAND_2GHZ) {
		modes = mode_list_2ghz;
		nr_modes = ARRAY_SIZE(mode_list_2ghz);
	} else {
		modes = mode_list_5ghz;
		nr_modes = ARRAY_SIZE(mode_list_5ghz);
	}

	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: here.\n");
	#endif
		
	for (i = 0; i < nr_modes; i++) {
		U8 c = ctl_grp | modes[i].ctl_mode;
		for (ctl_idx = 0; ctl_idx < AR5416_NUM_CTLS; ctl_idx++)
			if (c == ar->eeprom.ctl_index[ctl_idx])
				break;
		if (ctl_idx < AR5416_NUM_CTLS) {
			int f_off = 0;

			/*
			 * determine heavy clip parameter
			 * from the 11G edges array
			 */
			if (modes[i].ctl_mode == CTL_11G) {
				ar->heavy_clip =
					ar9170_get_heavy_clip(ar,
						freq, bw, EDGES(ctl_idx, 1));
			}

			/* adjust freq for 40MHz */
			if (modes[i].ctl_mode == CTL_2GHT40 ||
			    modes[i].ctl_mode == CTL_5GHT40) {
				if (bw == CARL9170_BW_40_BELOW)
					f_off = -10;
				else
					f_off = 10;
			}

			modes[i].max_power =
				ar9170_get_max_edge_power(ar,
					freq + f_off, EDGES(ctl_idx, 1));

			/*
			 * TODO: check if the regulatory max. power is
			 * controlled by cfg80211 for DFS.
			 * (hpmain applies it to max_power itself for DFS freq)
			 */

		} else {
			/*
			 * Workaround in otus driver, hpmain.c, line 3906:
			 * if no data for 5GHT20 are found, take the
			 * legacy 5G value. We extend this here to fallback
			 * from any other HT* or 11G, too.
			 */
			int k = i;

			modes[i].max_power = AR5416_MAX_RATE_POWER;
			while (k-- > 0) {
				if (modes[k].max_power !=
				    AR5416_MAX_RATE_POWER) {
					modes[i].max_power = modes[k].max_power;
					break;
				}
			}
		}

		/* apply max power to pwr_cal_data (ar->power_*) */
		for (j = 0; j < modes[i].pwr_cal_len; j++) {
			modes[i].pwr_cal_data[j] = min(modes[i].pwr_cal_data[j],
						       modes[i].max_power);
		}
	}

	if (ar->heavy_clip & 0xf0) {
		ar->power_2G_ht40[0]--;
		ar->power_2G_ht40[1]--;
		ar->power_2G_ht40[2]--;
	}
	if (ar->heavy_clip & 0xf) {
		ar->power_2G_ht20[0]++;
		ar->power_2G_ht20[1]++;
		ar->power_2G_ht20[2]++;
	}
	
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: Function finishes.\n");
	#endif

#undef EDGES
}


U8 ar9170_get_max_edge_power(struct ar9170 *ar,
U32 freq, struct ar9170_calctl_edges edges[])
{
	int i;
	U8 rc = AR5416_MAX_RATE_POWER;
	U8 f;
	if (freq < 3000)
		f = freq - 2300;
	else
		f = (freq - 4800) / 5;

	for (i = 0; i < AR5416_NUM_BAND_EDGES; i++) {
		if (edges[i].channel == 0xff)
		break;
		if (f == edges[i].channel) {
			/* exact freq match */
			rc = edges[i].power_flags & ~AR9170_CALCTL_EDGE_FLAGS;
			break;
		}
		if (i > 0 && f < edges[i].channel) {
			if (f > edges[i - 1].channel &&
			edges[i - 1].power_flags &
			AR9170_CALCTL_EDGE_FLAGS) {
				/* lower channel has the inband flag set */
				rc = edges[i - 1].power_flags &
				~AR9170_CALCTL_EDGE_FLAGS;
			}
			break;
		}
	}

	if (i == AR5416_NUM_BAND_EDGES) {
		if (f > edges[i - 1].channel &&
		edges[i - 1].power_flags & AR9170_CALCTL_EDGE_FLAGS) {
			/* lower channel has the inband flag set */
			rc = edges[i - 1].power_flags &
			~AR9170_CALCTL_EDGE_FLAGS;
		}
	}
	return rc;
}


void ar9170_set_power_cal(struct ar9170 *ar, U32 freq,
				   enum ar9170_bw bw)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_set_power_cal begins here.\n");
	#endif
	
	struct ar9170_calibration_target_power_legacy *ctpl;
	struct ar9170_calibration_target_power_ht *ctph;
	U8 *ctpres;
	int ntargets;
	int idx, i, n;
	U8 f;
	U8 pwr_freqs[AR5416_MAX_NUM_TGT_PWRS];

	if (freq < 3000)
		f = freq - 2300;
	else
		f = (freq - 4800) / 5;
	
	/*
	 * cycle through the various modes
	 *
	 * legacy modes first: 5G, 2G CCK, 2G OFDM
	 */
	for (i = 0; i < 3; i++) {
		switch (i) {
		case 0: /* 5 GHz legacy */
			ctpl = &ar->eeprom.cal_tgt_pwr_5G[0];
			ntargets = AR5416_NUM_5G_TARGET_PWRS;
			ctpres = ar->power_5G_leg;
			break;
		case 1: /* 2.4 GHz CCK */
			ctpl = &ar->eeprom.cal_tgt_pwr_2G_cck[0];
			ntargets = AR5416_NUM_2G_CCK_TARGET_PWRS;
			ctpres = ar->power_2G_cck;
			break;
		case 2: /* 2.4 GHz OFDM */
			ctpl = &ar->eeprom.cal_tgt_pwr_2G_ofdm[0];
			ntargets = AR5416_NUM_2G_OFDM_TARGET_PWRS;
			ctpres = ar->power_2G_ofdm;
			break;
		default:
			printf("BUG: Inside power calculation.\n");
			ntargets = -1; // FIXME do it better
			ctpres = NULL;
			ctpl = NULL;
		}
		
		for (n = 0; n < ntargets; n++) {
			if (ctpl[n].freq == 0xff)
				break;
			pwr_freqs[n] = ctpl[n].freq;
		}
		ntargets = n;
		idx = ar9170_find_freq_idx(ntargets, pwr_freqs, f);
		for (n = 0; n < 4; n++)
			ctpres[n] = ar9170_interpolate_u8(f,
				ctpl[idx + 0].freq, ctpl[idx + 0].power[n],
				ctpl[idx + 1].freq, ctpl[idx + 1].power[n]);
	}

	
	
	/* HT modes now: 5G HT20, 5G HT40, 2G CCK, 2G OFDM, 2G HT20, 2G HT40 */
	for (i = 0; i < 4; i++) {
		switch (i) {
		case 0: /* 5 GHz HT 20 */
			ctph = &ar->eeprom.cal_tgt_pwr_5G_ht20[0];
			ntargets = AR5416_NUM_5G_TARGET_PWRS;
			ctpres = ar->power_5G_ht20;
			break;
		case 1: /* 5 GHz HT 40 */
			ctph = &ar->eeprom.cal_tgt_pwr_5G_ht40[0];
			ntargets = AR5416_NUM_5G_TARGET_PWRS;
			ctpres = ar->power_5G_ht40;
			break;
		case 2: /* 2.4 GHz HT 20 */
			ctph = &ar->eeprom.cal_tgt_pwr_2G_ht20[0];
			ntargets = AR5416_NUM_2G_OFDM_TARGET_PWRS;
			ctpres = ar->power_2G_ht20;
			break;
		case 3: /* 2.4 GHz HT 40 */
			ctph = &ar->eeprom.cal_tgt_pwr_2G_ht40[0];
			ntargets = AR5416_NUM_2G_OFDM_TARGET_PWRS;
			ctpres = ar->power_2G_ht40;
			break;
		default:
			printf("BUG: Should be 0,1,2,3.\n");
			ctph = NULL; // FIXME do it better
		}

		for (n = 0; n < ntargets; n++) {
			if (ctph[n].freq == 0xff)
				break;
			pwr_freqs[n] = ctph[n].freq;
		}
		ntargets = n;
		idx = ar9170_find_freq_idx(ntargets, pwr_freqs, f);
		for (n = 0; n < 8; n++)
			ctpres[n] = ar9170_interpolate_u8(f,
				ctph[idx + 0].freq, ctph[idx + 0].power[n],
				ctph[idx + 1].freq, ctph[idx + 1].power[n]);
	}
	
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: Calculation set finishes.\n");
	#endif

	/* calc. conformance test limits and apply to ar->power*[] */
	ar9170_calc_ctl(ar, freq, bw);
	
}



int ar9170_set_freq_cal_data(struct ar9170 *ar,
struct ieee80211_channel *channel)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170 set frequency calculation data.\n");
	#endif
	
	U8 *cal_freq_pier;
	U8 vpds[2][AR5416_PD_GAIN_ICEPTS];
	U8 pwrs[2][AR5416_PD_GAIN_ICEPTS];
	int chain, idx, i;
	U32 phy_data = 0;
	U8 f, tmp;

	switch (channel->band) {
		case IEEE80211_BAND_2GHZ:
			f = channel->center_freq - 2300;
			cal_freq_pier = ar->eeprom.cal_freq_pier_2G;
			i = AR5416_NUM_2G_CAL_PIERS - 1;
			break;

		case IEEE80211_BAND_5GHZ:
			f = (channel->center_freq - 4800) / 5;
			cal_freq_pier = ar->eeprom.cal_freq_pier_5G;
			i = AR5416_NUM_5G_CAL_PIERS - 1;
			break;

		default:
			printf("ERROR: In freq_cal_data, the band is unspecified.\n");
			return -EINVAL;
			break;
	}

	for (; i >= 0; i--) {
		if (cal_freq_pier[i] != 0xff)
		break;
	}
	if (i < 0)
		return -EINVAL;

	idx = ar9170_find_freq_idx(i, cal_freq_pier, f);

	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: Index: %d.\n",idx);
	#endif
	
	ar9170_regwrite_begin(ar);

	for (chain = 0; chain < AR5416_MAX_CHAINS; chain++) {
		for (i = 0; i < AR5416_PD_GAIN_ICEPTS; i++) {
			struct ar9170_calibration_data_per_freq *cal_pier_data;
			int j;

			switch (channel->band) {
				case IEEE80211_BAND_2GHZ:
					cal_pier_data = &ar->eeprom.
					cal_pier_data_2G[chain][idx];
					#if AR9170_PHY_DEBUG_DEEP
					int ii,jj;
					for (ii=0; ii<4; ii++) {
						for (jj=0; jj<5; jj++) 
							printf("%d\t", cal_pier_data->pwr_pdg[ii][jj]);
						printf(" \n");
					}					
					#endif
					break;
					
				case IEEE80211_BAND_5GHZ:
					cal_pier_data = &ar->eeprom.
					cal_pier_data_5G[chain][idx];
					break;

				default:
					return -EINVAL;
			}

			for (j = 0; j < 2; j++) {
				vpds[j][i] = ar9170_interpolate_u8(f,
				cal_freq_pier[idx],
				cal_pier_data->vpd_pdg[j][i],
				cal_freq_pier[idx + 1],
				cal_pier_data[1].vpd_pdg[j][i]);

				pwrs[j][i] = ar9170_interpolate_u8(f,
				cal_freq_pier[idx],
				cal_pier_data->pwr_pdg[j][i],
				cal_freq_pier[idx + 1],
				cal_pier_data[1].pwr_pdg[j][i]) / 2;
			}
		}
		
		#if AR9170_PHY_DEBUG_DEEP
		int k;
		for (i=0; i<2; i++) {
			for (k=0; k<AR5416_PD_GAIN_ICEPTS; k++) {
				printf("%u ",vpds[i][k]);		
			}
			printf(" \n");
		}		
		#endif
		
		for (i = 0; i < 76; i++) {
			if (i < 25) {
				tmp = ar9170_interpolate_val(i, &pwrs[0][0],
				&vpds[0][0]);
			} else {
				tmp = ar9170_interpolate_val(i - 12,
				&pwrs[1][0],
				&vpds[1][0]);
			}

			phy_data |= tmp << ((i & 3) << 3);
			if ((i & 3) == 3) {
				ar9170_regwrite(0x1c6280 + chain * 0x1000 +
				(i & ~3), phy_data);
				phy_data = 0;
			}
			#if AR9170_PHY_DEBUG_DEEP
			printf("temp %u.\n",tmp);
			#endif
		}

		for (i = 19; i < 32; i++)
		ar9170_regwrite(0x1c6280 + chain * 0x1000 + (i << 2),
		0x0);
	}

	ar9170_regwrite_finish();
	return ar9170_regwrite_result();
}


int ar9170_set_channel(struct ar9170 *ar, struct ieee80211_channel *channel,
			 enum nl80211_channel_type _bw,
			 enum ar9170_rf_init_mode rfi)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: carl9170_set_channel begin. F_C: %d, Band: %d Type: %d, rfi: %d.\n",
										channel->center_freq, channel->band, _bw , rfi);
	#endif
	
	const struct ar9170_phy_freq_params *freqpar;
	struct ar9170_rf_init_result rf_res;
	struct ar9170_rf_init rf;
	// XXX For cleanness - should be safe
	memset(&rf, 0, sizeof(struct ar9170_rf_init));
	
	U32 cmd, tmp, offs = 0, new_ht = 0;
	int err;
	enum ar9170_bw bw;
	bool warm_reset;
	struct ieee80211_channel *old_channel = NULL;

	bw = nl80211_to_carl(_bw);

	if (conf_is_ht(&ar->hw->conf))
		new_ht |= AR9170FW_PHY_HT_ENABLE;

	if (conf_is_ht40(&ar->hw->conf))
		new_ht |= AR9170FW_PHY_HT_DYN2040;

	/* may be NULL at first setup */
	if (ar->channel) {
		old_channel = ar->channel;
		warm_reset = (old_channel->band != channel->band) ||
			     (old_channel->center_freq ==
			      channel->center_freq) ||
			     (ar->ht_settings != new_ht);

		ar->channel = NULL;
	} else {
		warm_reset = true;
	}

	/* HW workaround */
	if (!ar->hw->wiphy->bands[IEEE80211_BAND_5GHZ] &&
	    channel->center_freq <= 2417)
		warm_reset = true;
		
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: Decision on warm reset: %d.\n",warm_reset);
	#endif
	
	if (rfi != CARL9170_RFI_NONE || warm_reset) {
		U32 val;
		
		if (rfi == CARL9170_RFI_COLD)
			val = AR9170_PWR_RESET_BB_COLD_RESET;
		else
			val = AR9170_PWR_RESET_BB_WARM_RESET;

		#if AR9170_PHY_DEBUG_DEEP
		printf("DEBUG: val: %u.\n",(unsigned int)val);
		#endif

		/* warm/cold reset BB/ADDA */
		err = !ar9170_write_rgstr(ar, AR9170_PWR_REG_RESET, val);
		if (err) {
			printf("ERROR: Power register reset could not be written.\n");	
			return err;
		}			
		
		err = !ar9170_write_rgstr(ar, AR9170_PWR_REG_RESET, 0x0);
		if (err) {
			printf("ERROR: Power register reset could not be written.\n");
			return err;
		}
				
		err = ar9170_init_phy(ar, channel->band);
		if (err)
			return err;

		err = ar9170_init_rf_banks_0_7(ar,
			channel->band == IEEE80211_BAND_5GHZ);
		if (err)
			return err;

		cmd = CARL9170_CMD_RF_INIT;
	} else {
		cmd = CARL9170_CMD_FREQUENCY;
	}

	err = !ar9170_exec_cmd(ar, CARL9170_CMD_FREQ_START, 0, NULL, 0, NULL);
	if (err)
		return err;

	err = !ar9170_write_rgstr(ar, AR9170_PHY_REG_HEAVY_CLIP_ENABLE,
				 0x200);
	if (err)
		return err;

	err = ar9170_init_rf_bank4_pwr(ar,
		channel->band == IEEE80211_BAND_5GHZ,
		channel->center_freq, bw);
	if (err) {
		printf("ERROR: ar9170_init_rf_bank4_pwer returned errors.\n");
		return err;
	}		

	tmp = AR9170_PHY_TURBO_FC_SINGLE_HT_LTF1 |
	      AR9170_PHY_TURBO_FC_HT_EN;

	switch (bw) {
	case CARL9170_BW_20:
		break;
	case CARL9170_BW_40_BELOW:
		tmp |= AR9170_PHY_TURBO_FC_DYN2040_EN |
		       AR9170_PHY_TURBO_FC_SHORT_GI_40;
		offs = 3;
		break;
	case CARL9170_BW_40_ABOVE:
		tmp |= AR9170_PHY_TURBO_FC_DYN2040_EN |
		       AR9170_PHY_TURBO_FC_SHORT_GI_40 |
		       AR9170_PHY_TURBO_FC_DYN2040_PRI_CH;
		offs = 1;
		break;
	default:
		printf("BUG: Bandwidth is unspecified.\n");
		return -ENOSYS;
	}

	if (ar->eeprom.tx_mask != 1)
		tmp |= AR9170_PHY_TURBO_FC_WALSH;

	err = !ar9170_write_rgstr(ar, AR9170_PHY_REG_TURBO, tmp);
	if (err)
		return 0;

	err = ar9170_set_freq_cal_data(ar, channel);
	if (err)
		return err;

	ar9170_set_power_cal(ar, channel->center_freq, bw);

	err = ar9170_set_mac_tpc(ar, channel);
	if (err) {
		printf("ERROR: Inside phy could not set mac power.\n");
		return err;
	}
	
	freqpar = ar9170_get_hw_dyn_params(channel, bw);

	rf.ht_settings = new_ht;
	#if AR9170_PHY_DEBUG_DEEP
	printf("new_ht: %d.\n",new_ht);
	#endif
	
	if (conf_is_ht40(&ar->hw->conf)) {
		#if AR9170_PHY_DEBUG_DEEP
		printf("Is ht40. offs: %d.\n",offs);
		#endif
		SET_VAL(AR9170FW_PHY_HT_EXT_CHAN_OFF, rf.ht_settings, offs);
	}
	rf.freq = cpu_to_le32(channel->center_freq * 1000);
	rf.delta_slope_coeff_exp = cpu_to_le32(freqpar->coeff_exp);
	rf.delta_slope_coeff_man = cpu_to_le32(freqpar->coeff_man);
	rf.delta_slope_coeff_exp_shgi = cpu_to_le32(freqpar->coeff_exp_shgi);
	rf.delta_slope_coeff_man_shgi = cpu_to_le32(freqpar->coeff_man_shgi);

	if (rfi != CARL9170_RFI_NONE) {
		#if AR9170_PHY_DEBUG_DEEP
		printf("Is not RFI NONE.\n");
		#endif
		rf.finiteLoopCount = cpu_to_le32(2000);
	}		
	else
		rf.finiteLoopCount = cpu_to_le32(1000);

	#if AR9170_PHY_DEBUG_DEEP
	printf("exp: %d, exp_shgi: %d, man: %d, man_shgi: %d, finiteLoopCount: %d, freq: %d, ht_settings: %d, padding: %d %d %d.\n",
			  rf.delta_slope_coeff_exp,
			  rf.delta_slope_coeff_exp_shgi,
			  rf.delta_slope_coeff_man,
			  rf.delta_slope_coeff_man_shgi,
			  rf.finiteLoopCount,
			  rf.freq,
			  rf.ht_settings,
			  rf.padding2[0],
			  rf.padding2[1],
			  rf.padding2[2]);
	#endif

	err = !ar9170_exec_cmd(ar, cmd, sizeof(rf), &rf,
				sizeof(rf_res), &rf_res);
	if (err)
		return err;

	err = le32_to_cpu(rf_res.ret);
	if (err != 0) {
		ar->chan_fail++;
		ar->total_chan_fail++;

		printf("ERROR: channel change: %d -> %d "
			  "failed (%d).\n", old_channel ?
			  old_channel->center_freq : -1, channel->center_freq,
			  err);

		if ((rfi == CARL9170_RFI_COLD) || (ar->chan_fail > 3)) {
			/*
			 * We have tried very hard to change to _another_
			 * channel and we've failed to do so!
			 * Chances are that the PHY/RF is no longer
			 * operable (due to corruptions/fatal events/bugs?)
			 * and we need to reset at a higher level.
			 */
			printf("SHOULD RESTART NOW.\n");
			//carl9170_restart(ar, CARL9170_RR_TOO_MANY_PHY_ERRORS); FIXME
			return 0;
		}

		err = ar9170_set_channel(ar, channel, _bw,
					   CARL9170_RFI_COLD);
		if (err)
			return err;
	} else {
		ar->chan_fail = 0;
	}

	if (ar->heavy_clip) {
		err = !ar9170_write_rgstr(ar, AR9170_PHY_REG_HEAVY_CLIP_ENABLE,
					 0x200 | ar->heavy_clip);
		if (err) {
/*			if (net_ratelimit()) {
				wiphy_err(ar->hw->wiphy, "failed to set "
				       "heavy clip\n");
			}
*/			printf("ERROR: Writing register for heavy clip returned errors.\n");
			return err;
		}
	}

	ar->channel = channel;
	ar->ht_settings = new_ht;
	
	return 0;
}


const struct ar9170_phy_freq_params* ar9170_get_hw_dyn_params(struct ieee80211_channel *channel, enum ar9170_bw bw)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170 getting hw dynamic parameters.\n");
	#endif
	
	unsigned int chanidx = 0;
	U16 freq = 2412;

	if (channel) {
		chanidx = channel->hw_value;
		freq = channel->center_freq;
		
		#if AR9170_PHY_DEBUG_DEEP
		printf("Channel freq: %d, hw_value: %d.\n",channel->center_freq, channel->hw_value);
		#endif
	}
 
	if(chanidx >= ARRAY_SIZE(ar9170_phy_freq_params))
		printf("BUG: chanidx >= ARRAY_SIZE(carl9170_phy_freq_params).\n");

	if(__CARL9170_NUM_BW != 3)
		printf("BUG: __CARL9170_NUM_BW != 3");

	if(ar9170_phy_freq_params[chanidx].freq != freq)
		printf("carl9170_phy_freq_params[chanidx].freq != freq.\n");

	return &ar9170_phy_freq_params[chanidx].params[bw];
}


int ar9170_get_noisefloor(struct ar9170 *ar)
{
	#if AR9170_PHY_DEBUG_DEEP
	printf("DEBUG: ar9170_get_noisefloor begin.\n");
	#endif
	
	static const U32 phy_regs[] = {
		AR9170_PHY_REG_CCA, AR9170_PHY_REG_CH2_CCA,
	AR9170_PHY_REG_EXT_CCA, AR9170_PHY_REG_CH2_EXT_CCA };
	U32 phy_res[ARRAY_SIZE(phy_regs)];
	int err, i;

	if (ARRAY_SIZE(phy_regs) != ARRAY_SIZE(ar->noise)) {
		printf("BUILD_BUG_ON(ARRAY_SIZE(phy_regs) != ARRAY_SIZE(ar->noise));");
	}	

	err = !ar9170_read_mreg(ar, ARRAY_SIZE(phy_regs), phy_regs, phy_res);
	if (err) {
		printf("ERROR: Noise floor [read_mreg] returned errors.\n");		
		return err;
	}

	for (i = 0; i < 2; i++) {
		ar->noise[i] = sign_extend32(GET_VAL(AR9170_PHY_CCA_MIN_PWR, phy_res[i]), 8);

		ar->noise[i + 2] = sign_extend32(GET_VAL(
		AR9170_PHY_EXT_CCA_MIN_PWR, phy_res[i + 2]), 8);
	}

	if (ar->channel)
		ar->survey[ar->channel->hw_value].noise = ar->noise[0];
	
	return 0;
}