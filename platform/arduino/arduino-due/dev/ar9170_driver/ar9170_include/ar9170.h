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
#include "usb_lock.h"
#include "ath.h"
#include "ar9170eeprom.h"
#include "mac80211.h"
#include "fwdesc.h"
#include "fwcmd.h"
#include "hw_random.h"
#include "conf_ar9170.h"
#include "hw.h"
#include "list.h"
#include "bitops.h"
#include "ar9170_wlan.h"
#include "cfg80211.h"
#include "compiler.h"
#include "skbuff.h"
#include "linked_list.h"


#ifndef AR9170_H_
#define AR9170_H_

/* Maximum number of packets pending on the MAC outgoing queue. */
#define AR9170_MAX_PENDING_TX_PKT_QUEUE_LEN		16
/* Maximum number of packets pending on the MAC incoming queue. */
#define AR9170_MAX_PENDING_RX_PKT_QUEUE_LEN		8


// TODO Move to version.h
#define __CARL9170_SHARED_VERSION_H
#define CARL9170FW_VERSION_YEAR 12
#define CARL9170FW_VERSION_MONTH 7
#define CARL9170FW_VERSION_DAY 7
#define CARL9170FW_VERSION_GIT "1.9.6"

/* Single buffer for accepting BULK IN endpoint transfers. */
#define AR9170_BULK_TRANSFER_IN_BUFFER_NUM		1

/* We do not handle packets larger than 512 bytes. */
#define AR9170_RX_MAX_PACKET_LENGTH				512

#define AR9170_USB_REG_MAX_BUF_SIZE				64

#define AR9170_CMD_HDR_LEN						4
#define MAX_CMD_PAYLOAD_LEN  AR9170_USB_REG_MAX_BUF_SIZE - AR9170_CMD_HDR_LEN

#define PAYLOAD_MAX	(AR9170_MAX_CMD_LEN / 4 - 1)

#define AR9170_QUEUE_TIMEOUT		256
#define AR9170_BUMP_QUEUE			1000
#define AR9170_TX_TIMEOUT			2500
#define AR9170_JANITOR_DELAY		128
#define AR9170_QUEUE_STUCK_TIMEOUT	5500
#define AR9170_STAT_WORK			30000

#define CARL9170_NUM_TX_AGG_MAX		30

static const U8 ar9170_qmap[__AR9170_NUM_TXQ] = { 3, 2, 1, 0 };

enum ar9170_restart_reasons {
	CARL9170_RR_NO_REASON = 0,
	CARL9170_RR_FATAL_FIRMWARE_ERROR,
	CARL9170_RR_TOO_MANY_FIRMWARE_ERRORS,
	CARL9170_RR_WATCHDOG,
	CARL9170_RR_STUCK_TX,
	CARL9170_RR_UNRESPONSIVE_DEVICE,
	CARL9170_RR_COMMAND_TIMEOUT,
	CARL9170_RR_TOO_MANY_PHY_ERRORS,
	CARL9170_RR_LOST_RSP,
	CARL9170_RR_INVALID_RSP,
	CARL9170_RR_USER_REQUEST,

	__CARL9170_RR_LAST,
};

enum ar9170_rf_init_mode {
	CARL9170_RFI_NONE,
	CARL9170_RFI_WARM,
	CARL9170_RFI_COLD,
};
	
enum ar9170_ps_off_override_reasons {
	PS_OFF_VIF	= BIT(0),
	PS_OFF_BCN	= BIT(1),
	PS_OFF_ATIM = BIT(2),
	PS_OFF_DATA = BIT(3),
};

enum ar9170_device_state {
	AR9170_UNKNOWN_STATE,
	AR9170_STOPPED,
	AR9170_IDLE,
	AR9170_STARTED,
};

/* Type definition for the ar9170 transmission packets' queue */
typedef struct linked_list_t ar9170_tx_queue;
/* Type definition for the ar9170 receiving packets' queue */
typedef struct linked_list_t ar9170_rx_queue;

struct ar9170_vif {
	unsigned int id;
	struct ieee80211_vif *vif;// FIXME __rcu *vif;
};

struct ar9170_vif_info {
	struct _list_head list;
	bool active;
	unsigned int id;
	struct sk_buff *beacon;
	bool enable_beacon;
};


struct ar9170 {
	
	struct ath_common common;
		
	struct ieee80211_hw *hw;
	
	enum ar9170_device_state state;
	/* TODO remove these please */
	completion_t state_lock;
	mutex_lock_t mutex_lock;
	
	/* interface mode settings */
	struct _list_head vif_list;
	unsigned long vif_bitmap;
	unsigned int vifs;
	struct ar9170_vif vif_priv[AR9170_MAX_VIRTUAL_MAC];

	/* beaconing */
	mutex_lock_t beacon_lock;
	unsigned int global_pretbtt;
	unsigned int global_beacon_int;
	struct ar9170_vif_info *beacon_iter; //FIXME __rcu *beacon_iter;
	unsigned int beacon_enabled;
	bool beacon_ctrl;
	bool beacon_cancel;
	
	/* cryptographic engine */
	U64 usedkeys;
	bool rx_software_decryption;
	bool disable_offload;
	
	/* firmware settings */
	completion_t fw_boot_wait;
	struct {
		const struct ar9170fw_desc_head *desc;
		const struct firmware *fw;
		unsigned int offset;
		unsigned int address;
		unsigned int cmd_bufs;
		unsigned int api_version;
		unsigned int vif_num;
		unsigned int err_counter;
		unsigned int bug_counter;
		U32 beacon_addr;
		unsigned int beacon_max_len;
		bool rx_stream;
		bool tx_stream;
		bool rx_filter;
		bool hw_counters;
		unsigned int mem_blocks;
		unsigned int mem_block_size;
		unsigned int rx_size;
		unsigned int tx_seq_table;
		bool ba_filter;
	} fw;
	
	/* interface configuration combinations */
	struct ieee80211_iface_limit if_comb_limits[1];
	struct ieee80211_iface_combination if_combs[1];
	
	/* qos queue settings */
	struct ieee80211_tx_queue_params edcf[5];
	
	/* statistics */
	unsigned int tx_dropped;
	unsigned int tx_ack_failures;
	unsigned int tx_fcs_errors;
	unsigned int rx_dropped;
	
	/* EEPROM*/
	struct ar9170_eeprom eeprom;
	
	/* internal memory management*/
	unsigned long *mem_bitmap;
	//spinlock_t mem_lock;
	long mem_free_blocks;//atomic_t mem_free_blocks;
	long mem_allocs;//atomic_t mem_allocs;
	
	/* rxstream mpdu merge */
	struct ar9170_rx_head rx_plcp;
	bool rx_has_plcp;
	struct sk_buff *rx_failover;
	int rx_failover_missing;
	
	/* CMD */
	int cmd_seq;
	uint8_t* readbuf;
	uint32_t readlen;
	union {
		le32_t cmd_buf[PAYLOAD_MAX];
		struct ar9170_cmd cmd;
		struct ar9170_rsp rsp;
		};
	completion_t cmd_lock;
	completion_t cmd_async_lock;
	completion_t cmd_wait;	
	completion_t cmd_buf_lock;	
	struct ar9170_send_list* cmd_list;
	
	/* TX */
	completion_t tx_async_lock;
	completion_t tx_buf_lock;
	completion_t tx_atim_wait;
	completion_t tx_data_wait;
	completion_t clear_tx_data_wait_at_next_tbtt;
	completion_t clear_cmd_async_lock_at_next_tbtt;
	completion_t clear_tx_async_lock_at_next_tbtt;
	ar9170_tx_queue* tx_pending_pkts;
	ar9170_tx_queue* tx_pending_atims;
	ar9170_tx_queue* tx_pending_soft_beacon;
	struct ar9170_send_list* tx_list;	
	uint8_t atim_wait_cleared_count;
	
	/* PSM */
	bool erase_awake_nodes_flag;
	
	/* RX */
	ar9170_tx_queue* rx_pending_pkts;
	bool clear_filtering;
	
	/* Filter settings */
	U64 cur_mc_hash;
	U32 cur_filter;
	unsigned int filter_state;
	unsigned int rx_filter_caps;
	bool sniffer_enabled;
		
	/* PHY */
	struct ieee80211_channel *channel;
	unsigned int num_channels;
	int noise[4];
	unsigned int chan_fail;
	unsigned int total_chan_fail;
	U8 heavy_clip;
	U8 ht_settings;
	struct {
		U64 active;	/* usec */
		U64 cca;	/* usec */
		U64 tx_time;	/* usec */
		U64 rx_total;
		U64 rx_overrun;
	} tally;
	struct survey_info *survey;

	/* power calibration data */
	U8 power_5G_leg[4];
	U8 power_2G_cck[4];
	U8 power_2G_ofdm[4];
	U8 power_5G_ht20[8];
	U8 power_5G_ht40[8];
	U8 power_2G_ht20[8];
	U8 power_2G_ht40[8];

#ifdef CONFIG_AR9170_HWRNG
# define AR9170_HWRNG_CACHE_SIZE	AR9170_MAX_CMD_PAYLOAD_LEN
	struct {
		struct hwrng rng;
		bool initialized;
		char name[30 + 1];
		U16 cache[AR9170_HWRNG_CACHE_SIZE / sizeof(U16)];
		unsigned int cache_idx;
	} rng;
#endif /* CONFIG_AR9170_HWRNG */

	/* PSM */
	struct {
		unsigned int	dtim_counter;
		unsigned long	last_beacon;
		unsigned long	last_action;
		unsigned long	last_slept;
		unsigned int	sleep_ms;
		unsigned int	off_override;
		unsigned int	update_mask;
		bool			state;
	} ps;
	
	/* PSM Manager */
	struct {
		LINKED_LIST(wake_neighbors);
		int	psm_state;
		bool create_atims_flag;
		bool send_soft_bcn_flag;
		bool psm_transit_to_sleep;
		bool psm_transit_to_wake;
		U8	last_ATIM_DA[ETH_ALEN];
		U8  last_ATIM_A3[ETH_ALEN];
		
	} ps_mgr;

};
	

/* MAIN */
/* Return the global device structure, pointer and its address. */
struct ar9170* ar9170_get_device();
struct ar9170** ar9170_get_device_pt();

/* USB */
struct ar9170* ar9170_alloc(struct ar9170** ar);
int ar9170_register_device(struct ar9170* ar);
bool ar9170_read_eeprom(struct ar9170* ar);
int ar9170_parse_eeprom( struct ar9170* ar );
bool ar9170_init_device(struct ar9170* ar);
bool ar9170_usb_open(struct ar9170* ar);
bool ar9170_op_start(struct ar9170* ar) /* FIXME - the argument should be of type ieee80211_hw, although not really important... */;
int ar9170_op_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
/* RX */
void ar9170_async_rx( struct ar9170* ar);
void ar9170_schedule_handle_mpdu( struct ar9170* ar, U8 *buf, int len );
void ar9170_handle_mpdu(struct ar9170 *ar, U8 *buf, int len);
void ar9170_handle_command_response(struct ar9170 *ar, void *buf, uint32_t len);
void __ar9170_rx( struct ar9170* ar, uint8_t* buffer, uint32_t len);
void ar9170_rx_untie_cmds( struct ar9170* ar, const uint8_t* buffer, const uint32_t len );
void ar9170_handle_ps(struct ar9170 *ar, struct ar9170_rsp *rsp);
void ar9170_dbg_message(struct ar9170 *ar, const char *buf, U32 len);
void ar9170_op_configure_filter(struct ieee80211_hw *hw, unsigned int changed_flags, unsigned int *new_flags, U64 multicast);
int ar9170_rx_mac_status(struct ar9170 *ar, struct ar9170_rx_head *head, struct ar9170_rx_macstatus *mac, struct ieee80211_rx_status *status);
bool ar9170_ampdu_check(struct ar9170 *ar, U8 *buf, U8 ms);
void ar9170_rx_phy_status(struct ar9170 *ar, struct ar9170_rx_phystatus *phy, struct ieee80211_rx_status *status);
void ar9170_ps_beacon(struct ar9170 *ar, void *data, unsigned int len);
struct sk_buff* ar9170_rx_copy_data(U8 *buf, int len);

/* TX */
bool ar9170_async_tx_soft_beacon(struct ar9170* ar);
ar9170_tx_queue* ar9170_async_tx( struct ar9170* ar, ar9170_tx_queue* tx_queue );
void __ar9170_tx_process_status(struct ar9170 *ar,const uint8_t cookie, const uint8_t info);
void ar9170_tx_process_status(struct ar9170 *ar, const struct ar9170_rsp *cmd);
void ar9170_tx(struct sk_buff* skb);
le32_t ar9170_tx_physet(struct ar9170 *ar, struct ieee80211_tx_info *info, struct ieee80211_tx_rate *txrate);
int ar9170_tx_prepare(struct ar9170* ar, struct sk_buff* skb);
int ar9170_op_tx(struct ieee80211_hw *hw, struct sk_buff *skb);
void ar9170_tx_drop(struct ar9170 *ar, struct sk_buff *skb);
int ar9170_send_beacon_cmd(struct ar9170* ar);
int ar9170_update_beacon(struct ar9170 *ar, bool submit);
void ar9170_tx_rate_tpc_chains(struct ar9170 *ar, struct ieee80211_tx_info *info, struct ieee80211_tx_rate *txrate, unsigned int *phyrate, unsigned int *tpc, unsigned int *chains);
/* RNG */
int ar9170_register_hwrng(struct ar9170 *ar);

struct ieee80211_vif *ar9170_get_main_vif(struct ar9170 *ar);
int ar9170_init_interface(struct ar9170 *ar, struct ieee80211_vif *vif);
int ar9170_op_conf_tx(struct ieee80211_hw *hw,struct ieee80211_vif *vif, U16 queue, const struct ieee80211_tx_queue_params *param);
int ar9170_op_config(struct ieee80211_hw *hw, U32 changed);
int ar9170_ps_update(struct ar9170 *ar);
int ar9170_update_survey(struct ar9170 *ar, bool flush, bool noise);
void ar9170_op_flush(struct ieee80211_hw *hw, bool drop);
void ar9170_op_bss_info_changed(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_bss_conf *bss_conf, U32 changed);

/* Scheduler */
bool ar9170_op_add_pending_pkt(struct ar9170* ar, ar9170_tx_queue** queue_pt, struct sk_buff* skb, bool atomic);
void ar9170_op_scheduler(struct ar9170*);
#endif /* AR9170_H_ */

