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
#ifndef FWCMD_H_
#define FWCMD_H_

typedef uint64_t le64_t; // FIXME - there is no le64_t and this could be a source of problems

#define	AR9170_MAX_CMD_LEN		64
#define	AR9170_MAX_CMD_PAYLOAD_LEN	60

#define AR9170FW_API_MIN_VER		1
#define AR9170FW_API_MAX_VER		1

#define	AR9170_ERR_MAGIC		"ERR:"
#define	AR9170_BUG_MAGIC		"BUG:"

enum carl9170_cmd_oids {
	CARL9170_CMD_RREG		= 0x00,
	CARL9170_CMD_WREG		= 0x01,
	CARL9170_CMD_ECHO		= 0x02,
	CARL9170_CMD_SWRST		= 0x03,
	CARL9170_CMD_REBOOT		= 0x04,
	CARL9170_CMD_BCN_CTRL		= 0x05,
	CARL9170_CMD_READ_TSF		= 0x06,
	CARL9170_CMD_RX_FILTER		= 0x07,
	CARL9170_CMD_WOL		= 0x08,
	CARL9170_CMD_TALLY		= 0x09,

	/* CAM */
	CARL9170_CMD_EKEY		= 0x10,
	CARL9170_CMD_DKEY		= 0x11,

	/* RF / PHY */
	CARL9170_CMD_FREQUENCY		= 0x20,
	CARL9170_CMD_RF_INIT		= 0x21,
	CARL9170_CMD_SYNTH		= 0x22,
	CARL9170_CMD_FREQ_START		= 0x23,
	CARL9170_CMD_PSM		= 0x24,

	/* Asychronous command flag */
	CARL9170_CMD_ASYNC_FLAG		= 0x40,
	CARL9170_CMD_WREG_ASYNC		= (CARL9170_CMD_WREG |
	CARL9170_CMD_ASYNC_FLAG),
	CARL9170_CMD_REBOOT_ASYNC	= (CARL9170_CMD_REBOOT |
	CARL9170_CMD_ASYNC_FLAG),
	CARL9170_CMD_BCN_CTRL_ASYNC	= (CARL9170_CMD_BCN_CTRL |
	CARL9170_CMD_ASYNC_FLAG),
	CARL9170_CMD_PSM_ASYNC		= (CARL9170_CMD_PSM |
	CARL9170_CMD_ASYNC_FLAG),

	/* responses and traps */
	CARL9170_RSP_FLAG		= 0xc0,
	AR9170_RSP_PRETBTT		= 0xc0,
	AR9170_RSP_TXCOMP		= 0xc1,
	AR9170_RSP_BEACON_CONFIG	= 0xc2,
	AR9170_RSP_ATIM		= 0xc3,
	CARL9170_RSP_WATCHDOG		= 0xc6,
	CARL9170_RSP_TEXT		= 0xca,
	CARL9170_RSP_HEXDUMP		= 0xcc,
	CARL9170_RSP_RADAR		= 0xcd,
	CARL9170_RSP_GPIO		= 0xce,
	CARL9170_RSP_BOOT		= 0xcf,
};

struct ar9170_set_key_cmd {
	le16_t		user;
	le16_t		keyId;
	le16_t		type;
	U8		macAddr[6];
	U32		key[4];
} __attribute__((packed,aligned(4)));


#define AR9170_SET_KEY_CMD_SIZE		28

struct ar9170_disable_key_cmd {
	le16_t		user;
	le16_t		padding;
} __attribute__((packed,aligned(4)));

#define AR9170_DISABLE_KEY_CMD_SIZE		4

struct ar9170_u32_list {
	U32	vals[0];
} __attribute__ ((packed));

struct ar9170_reg_list {
	le32_t		regs[0];
} __attribute__ ((packed));

struct ar9170_write_reg {
	struct {
		le32_t		addr;
		le32_t		val;
	} regs[0] __attribute__ ((packed));
} __attribute__ ((packed));

#define	AR9170FW_PHY_HT_ENABLE		0x4
#define	AR9170FW_PHY_HT_DYN2040		0x8
#define	AR9170FW_PHY_HT_EXT_CHAN_OFF		0x3
#define	AR9170FW_PHY_HT_EXT_CHAN_OFF_S	2

struct ar9170_rf_init {
	le32_t		freq;
	U8		ht_settings;
	U8		padding2[3];
	le32_t		delta_slope_coeff_exp;
	le32_t		delta_slope_coeff_man;
	le32_t		delta_slope_coeff_exp_shgi;
	le32_t		delta_slope_coeff_man_shgi;
	le32_t		finiteLoopCount;
} __attribute__ ((packed));
#define AR9170_RF_INIT_SIZE		28

struct ar9170_rf_init_result {
	le32_t		ret;		/* AR9170_PHY_REG_AGC_CONTROL */
} __attribute__ ((packed));
#define	AR9170_RF_INIT_RESULT_SIZE	4

#define	AR9170_PSM_SLEEP		0x1000
#define	AR9170_PSM_SOFTWARE		0
#define	AR9170_PSM_WAKE		0 /* internally used. */
#define	AR9170_PSM_COUNTER		0xfff
#define	AR9170_PSM_COUNTER_S		0

struct ar9170_psm {
	le32_t		state;
} __attribute__ ((packed));
#define AR9170_PSM_SIZE		4

struct ar9170_rx_filter_cmd {
	le32_t		rx_filter;
} __attribute__ ((packed));
#define AR9170_RX_FILTER_CMD_SIZE	4

#define AR9170_RX_FILTER_BAD		0x01
#define AR9170_RX_FILTER_OTHER_RA	0x02
#define AR9170_RX_FILTER_DECRY_FAIL	0x04
#define AR9170_RX_FILTER_CTL_OTHER	0x08
#define AR9170_RX_FILTER_CTL_PSPOLL	0x10
#define AR9170_RX_FILTER_CTL_BACKR	0x20
#define AR9170_RX_FILTER_MGMT		0x40
#define AR9170_RX_FILTER_DATA		0x80
#define AR9170_RX_FILTER_EVERYTHING	(~0)

struct ar9170_bcn_ctrl_cmd {
	le32_t		vif_id;
	le32_t		mode;
	le32_t		bcn_addr;
	le32_t		bcn_len;
} __attribute__ ((packed));
#define AR9170_BCN_CTRL_CMD_SIZE	16

#define AR9170_BCN_CTRL_DRAIN	0
#define AR9170_BCN_CTRL_CAB_TRIGGER	1

struct ar9170_wol_cmd {
	le32_t		flags;
	U8		mac[6];
	U8		bssid[6];
	le32_t		null_interval;
	le32_t		free_for_use2;
	le32_t		mask;
	U8		pattern[32];
} __attribute__ ((packed));

#define AR9170_WOL_CMD_SIZE		60

#define AR9170_WOL_DISCONNECT		1
#define AR9170_WOL_MAGIC_PKT		2


struct ar9170_cmd_head {
	union {
		struct {
			U8	len;
			U8	cmd;
			U8	seq;
			U8	ext;
		} __attribute__ ((packed));

		U32 hdr_data;
	} __attribute__ ((packed));
} __attribute__ ((packed));


struct ar9170_cmd {
	struct ar9170_cmd_head hdr;
	union {
		struct ar9170_set_key_cmd	setkey;
		struct ar9170_disable_key_cmd	disablekey;
		struct ar9170_u32_list	echo;
		struct ar9170_reg_list	rreg;
		struct ar9170_write_reg	wreg;
		struct ar9170_rf_init		rf_init;
		struct ar9170_psm		psm;
		struct ar9170_wol_cmd		wol;
		struct ar9170_bcn_ctrl_cmd	bcn_ctrl;
		struct ar9170_rx_filter_cmd	rx_filter;
		COMPILER_WORD_ALIGNED U8 data[AR9170_MAX_CMD_PAYLOAD_LEN];
	} __attribute__ ((packed));
} __attribute__((packed,aligned(4)));


#define	AR9170_TX_STATUS_QUEUE	3
#define	AR9170_TX_STATUS_QUEUE_S	0
#define	AR9170_TX_STATUS_RIX_S	2
#define	AR9170_TX_STATUS_RIX		(3 << AR9170_TX_STATUS_RIX_S)
#define	AR9170_TX_STATUS_TRIES_S	4
#define	AR9170_TX_STATUS_TRIES	(7 << AR9170_TX_STATUS_TRIES_S)
#define	AR9170_TX_STATUS_SUCCESS	0x80

#ifdef __CARL9170FW__
/*
 * NOTE:
 * Both structs [carl9170_tx_status and _carl9170_tx_status]
 * need to be "bit for bit" in sync.
 */
struct ar9170_tx_status {
	/*
	 * Beware of compiler bugs in all gcc pre 4.4!
	 */

	U8 cookie;
	u8 queue:2;
	U8 rix:2;
	U8 tries:3;
	U8 success:1;
} __attribute__ ((packed));
#endif /* __CARL9170FW__ */

struct _ar9170_tx_status {
	/*
	 * This version should be immune to all alignment bugs.
	 */

	U8 cookie;
	U8 info;
} __attribute__ ((packed));
#define AR9170_TX_STATUS_SIZE		2

#define	AR9170_RSP_TX_STATUS_NUM	(AR9170_MAX_CMD_PAYLOAD_LEN /	\
					 sizeof(struct _ar9170_tx_status))

#define	AR9170_TX_MAX_RATE_TRIES	7

#define	AR9170_TX_MAX_RATES		4
#define	AR9170_TX_MAX_RETRY_RATES	(AR9170_TX_MAX_RATES - 1)
#define	AR9170_ERR_MAGIC		"ERR:"
#define	AR9170_BUG_MAGIC		"BUG:"

struct ar9170_gpio {
	le32_t gpio;
} __attribute__ ((packed));

#define AR9170_GPIO_SIZE		4

struct ar9170_tsf_rsp {
	union {
		le32_t tsf[2];
		le64_t tsf_64;
	} __attribute__ ((packed));
} __attribute__ ((packed));
#define AR9170_TSF_RSP_SIZE		8

struct ar9170_tally_rsp {
	le32_t active;
	le32_t cca;
	le32_t tx_time;
	le32_t rx_total;
	le32_t rx_overrun;
	le32_t tick;
} __attribute__ ((packed));

struct ar9170_rsp {
	struct ar9170_cmd_head hdr;

	union {
		struct ar9170_rf_init_result	rf_init_res;
		struct ar9170_u32_list	rreg_res;
		struct ar9170_u32_list	echo;
#ifdef __CARL9170FW__
		struct ar9170_tx_status	tx_status[0];
#endif /* __CARL9170FW__ */
		struct _ar9170_tx_status	_tx_status[0];
		struct ar9170_gpio		gpio;
		struct ar9170_tsf_rsp		tsf;
		struct ar9170_psm		psm;
		struct ar9170_tally_rsp	tally;
		COMPILER_WORD_ALIGNED U8 data[AR9170_MAX_CMD_PAYLOAD_LEN];
	} __attribute__ ((packed));
} __attribute__((packed,aligned(4)));



#endif /* FWCMD_H_ */