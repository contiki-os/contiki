/*
 * mac80211 <-> driver interface
 *
 * Copyright 2002-2005, Devicescape Software, Inc.
 * Copyright 2006-2007  Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2010  Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "nl80211.h"
#include "if_ether.h"
#include "cfg80211.h"

#include "string.h"
#include "ieee80211.h"
#include "bitops.h"
#include "compiler.h"

#ifndef MAC80211_H_
#define MAC80211_H_


/**
 * enum mac80211_rate_control_flags - per-rate flags set by the
 *	Rate Control algorithm.
 *
 * These flags are set by the Rate control algorithm for each rate during tx,
 * in the @flags member of struct ieee80211_tx_rate.
 *
 * @IEEE80211_TX_RC_USE_RTS_CTS: Use RTS/CTS exchange for this rate.
 * @IEEE80211_TX_RC_USE_CTS_PROTECT: CTS-to-self protection is required.
 *	This is set if the current BSS requires ERP protection.
 * @IEEE80211_TX_RC_USE_SHORT_PREAMBLE: Use short preamble.
 * @IEEE80211_TX_RC_MCS: HT rate.
 * @IEEE80211_TX_RC_GREEN_FIELD: Indicates whether this rate should be used in
 *	Greenfield mode.
 * @IEEE80211_TX_RC_40_MHZ_WIDTH: Indicates if the Channel Width should be 40 MHz.
 * @IEEE80211_TX_RC_DUP_DATA: The frame should be transmitted on both of the
 *	adjacent 20 MHz channels, if the current channel type is
 *	NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS.
 * @IEEE80211_TX_RC_SHORT_GI: Short Guard interval should be used for this rate.
 */
enum mac80211_rate_control_flags {
	IEEE80211_TX_RC_USE_RTS_CTS		= BIT(0),
	IEEE80211_TX_RC_USE_CTS_PROTECT		= BIT(1),
	IEEE80211_TX_RC_USE_SHORT_PREAMBLE	= BIT(2),

	/* rate index is an MCS rate number instead of an index */
	IEEE80211_TX_RC_MCS			= BIT(3),
	IEEE80211_TX_RC_GREEN_FIELD		= BIT(4),
	IEEE80211_TX_RC_40_MHZ_WIDTH		= BIT(5),
	IEEE80211_TX_RC_DUP_DATA		= BIT(6),
	IEEE80211_TX_RC_SHORT_GI		= BIT(7),
};


/**
 * enum ieee80211_conf_flags - configuration flags
 *
 * Flags to define PHY configuration options
 *
 * @IEEE80211_CONF_MONITOR: there's a monitor interface present -- use this
 *	to determine for example whether to calculate timestamps for packets
 *	or not, do not use instead of filter flags!
 * @IEEE80211_CONF_PS: Enable 802.11 power save mode (managed mode only).
 *	This is the power save mode defined by IEEE 802.11-2007 section 11.2,
 *	meaning that the hardware still wakes up for beacons, is able to
 *	transmit frames and receive the possible acknowledgment frames.
 *	Not to be confused with hardware specific wakeup/sleep states,
 *	driver is responsible for that. See the section "Powersave support"
 *	for more.
 * @IEEE80211_CONF_IDLE: The device is running, but idle; if the flag is set
 *	the driver should be prepared to handle configuration requests but
 *	may turn the device off as much as possible. Typically, this flag will
 *	be set when an interface is set UP but not associated or scanning, but
 *	it can also be unset in that case when monitor interfaces are active.
 * @IEEE80211_CONF_OFFCHANNEL: The device is currently not on its main
 *	operating channel.
 */
enum ieee80211_conf_flags {
	IEEE80211_CONF_MONITOR		= (1<<0),
	IEEE80211_CONF_PS		= (1<<1),
	IEEE80211_CONF_IDLE		= (1<<2),
	IEEE80211_CONF_OFFCHANNEL	= (1<<3),
};



/**
1239  * enum ieee80211_hw_flags - hardware flags
1240  *
1241  * These flags are used to indicate hardware capabilities to
1242  * the stack. Generally, flags here should have their meaning
1243  * done in a way that the simplest hardware doesn't need setting
1244  * any particular flags. There are some exceptions to this rule,
1245  * however, so you are advised to review these flags carefully.
1246  *
1247  * @IEEE80211_HW_HAS_RATE_CONTROL:
1248  *      The hardware or firmware includes rate control, and cannot be
1249  *      controlled by the stack. As such, no rate control algorithm
1250  *      should be instantiated, and the TX rate reported to userspace
1251  *      will be taken from the TX status instead of the rate control
1252  *      algorithm.
1253  *      Note that this requires that the driver implement a number of
1254  *      callbacks so it has the correct information, it needs to have
1255  *      the @set_rts_threshold callback and must look at the BSS config
1256  *      @use_cts_prot for G/N protection, @use_short_slot for slot
1257  *      timing in 2.4 GHz and @use_short_preamble for preambles for
1258  *      CCK frames.
1259  *
1260  * @IEEE80211_HW_RX_INCLUDES_FCS:
1261  *      Indicates that received frames passed to the stack include
1262  *      the FCS at the end.
1263  *
1264  * @IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING:
1265  *      Some wireless LAN chipsets buffer broadcast/multicast frames
1266  *      for power saving stations in the hardware/firmware and others
1267  *      rely on the host system for such buffering. This option is used
1268  *      to configure the IEEE 802.11 upper layer to buffer broadcast and
1269  *      multicast frames when there are power saving stations so that
1270  *      the driver can fetch them with ieee80211_get_buffered_bc().
1271  *
1272  * @IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE:
1273  *      Hardware is not capable of short slot operation on the 2.4 GHz band.
1274  *
1275  * @IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE:
1276  *      Hardware is not capable of receiving frames with short preamble on
1277  *      the 2.4 GHz band.
1278  *
1279  * @IEEE80211_HW_SIGNAL_UNSPEC:
1280  *      Hardware can provide signal values but we don't know its units. We
1281  *      expect values between 0 and @max_signal.
1282  *      If possible please provide dB or dBm instead.
1283  *
1284  * @IEEE80211_HW_SIGNAL_DBM:
1285  *      Hardware gives signal values in dBm, decibel difference from
1286  *      one milliwatt. This is the preferred method since it is standardized
1287  *      between different devices. @max_signal does not need to be set.
1288  *
1289  * @IEEE80211_HW_SPECTRUM_MGMT:
1290  *      Hardware supports spectrum management defined in 802.11h
1291  *      Measurement, Channel Switch, Quieting, TPC
1292  *
1293  * @IEEE80211_HW_AMPDU_AGGREGATION:
1294  *      Hardware supports 11n A-MPDU aggregation.
1295  *
1296  * @IEEE80211_HW_SUPPORTS_PS:
1297  *      Hardware has power save support (i.e. can go to sleep).
1298  *
1299  * @IEEE80211_HW_PS_NULLFUNC_STACK:
1300  *      Hardware requires nullfunc frame handling in stack, implies
1301  *      stack support for dynamic PS.
1302  *
1303  * @IEEE80211_HW_SUPPORTS_DYNAMIC_PS:
1304  *      Hardware has support for dynamic PS.
1305  *
1306  * @IEEE80211_HW_MFP_CAPABLE:
1307  *      Hardware supports management frame protection (MFP, IEEE 802.11w).
1308  *
1309  * @IEEE80211_HW_SUPPORTS_STATIC_SMPS:
1310  *      Hardware supports static spatial multiplexing powersave,
1311  *      ie. can turn off all but one chain even on HT connections
1312  *      that should be using more chains.
1313  *
1314  * @IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS:
1315  *      Hardware supports dynamic spatial multiplexing powersave,
1316  *      ie. can turn off all but one chain and then wake the rest
1317  *      up as required after, for example, rts/cts handshake.
1318  *
1319  * @IEEE80211_HW_SUPPORTS_UAPSD:
1320  *      Hardware supports Unscheduled Automatic Power Save Delivery
1321  *      (U-APSD) in managed mode. The mode is configured with
1322  *      conf_tx() operation.
1323  *
1324  * @IEEE80211_HW_REPORTS_TX_ACK_STATUS:
1325  *      Hardware can provide ack status reports of Tx frames to
1326  *      the stack.
1327  *
1328  * @IEEE80211_HW_CONNECTION_MONITOR:
1329  *      The hardware performs its own connection monitoring, including
1330  *      periodic keep-alives to the AP and probing the AP on beacon loss.
1331  *      When this flag is set, signaling beacon-loss will cause an immediate
1332  *      change to disassociated state.
1333  *
1334  * @IEEE80211_HW_NEED_DTIM_PERIOD:
1335  *      This device needs to know the DTIM period for the BSS before
1336  *      associating.
1337  *
1338  * @IEEE80211_HW_SUPPORTS_PER_STA_GTK: The device's crypto engine supports
1339  *      per-station GTKs as used by IBSS RSN or during fast transition. If
1340  *      the device doesn't support per-station GTKs, but can be asked not
1341  *      to decrypt group addressed frames, then IBSS RSN support is still
1342  *      possible but software crypto will be used. Advertise the wiphy flag
1343  *      only in that case.
1344  *
1345  * @IEEE80211_HW_AP_LINK_PS: When operating in AP mode the device
1346  *      autonomously manages the PS status of connected stations. When
1347  *      this flag is set mac80211 will not trigger PS mode for connected
1348  *      stations based on the PM bit of incoming frames.
1349  *      Use ieee80211_start_ps()/ieee8021_end_ps() to manually configure
1350  *      the PS mode of connected stations.
1351  *
1352  * @IEEE80211_HW_TX_AMPDU_SETUP_IN_HW: The device handles TX A-MPDU session
1353  *      setup strictly in HW. mac80211 should not attempt to do this in
1354  *      software.
1355  *
1356  * @IEEE80211_HW_SCAN_WHILE_IDLE: The device can do hw scan while
1357  *      being idle (i.e. mac80211 doesn't have to go idle-off during the
1358  *      the scan).
1359  *
1360  * @IEEE80211_HW_WANT_MONITOR_VIF: The driver would like to be informed of
1361  *      a virtual monitor interface when monitor interfaces are the only
1362  *      active interfaces.
1363  *
1364  * @IEEE80211_HW_QUEUE_CONTROL: The driver wants to control per-interface
1365  *      queue mapping in order to use different queues (not just one per AC)
1366  *      for different virtual interfaces. See the doc section on HW queue
1367  *      control for more details.
1368  *
1369  * @IEEE80211_HW_P2P_DEV_ADDR_FOR_INTF: Use the P2P Device address for any
1370  *      P2P Interface. This will be honoured even if more than one interface
1371  *      is supported.
1372  *
1373  * @IEEE80211_HW_TEARDOWN_AGGR_ON_BAR_FAIL: On this hardware TX BA session
1374  *      should be tear down once BAR frame will not be acked.
1375  *
1376  */
enum ieee80211_hw_flags {
         IEEE80211_HW_HAS_RATE_CONTROL                   = 1<<0,
         IEEE80211_HW_RX_INCLUDES_FCS                    = 1<<1,
         IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING        = 1<<2,
         IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE          = 1<<3,
         IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE      = 1<<4,
         IEEE80211_HW_SIGNAL_UNSPEC                      = 1<<5,
         IEEE80211_HW_SIGNAL_DBM                         = 1<<6,
         IEEE80211_HW_NEED_DTIM_PERIOD                   = 1<<7,
         IEEE80211_HW_SPECTRUM_MGMT                      = 1<<8,
         IEEE80211_HW_AMPDU_AGGREGATION                  = 1<<9,
         IEEE80211_HW_SUPPORTS_PS                        = 1<<10,
         IEEE80211_HW_PS_NULLFUNC_STACK                  = 1<<11,
         IEEE80211_HW_SUPPORTS_DYNAMIC_PS                = 1<<12,
         IEEE80211_HW_MFP_CAPABLE                        = 1<<13,
         IEEE80211_HW_WANT_MONITOR_VIF                   = 1<<14,
         IEEE80211_HW_SUPPORTS_STATIC_SMPS               = 1<<15,
         IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS              = 1<<16,
         IEEE80211_HW_SUPPORTS_UAPSD                     = 1<<17,
         IEEE80211_HW_REPORTS_TX_ACK_STATUS              = 1<<18,
         IEEE80211_HW_CONNECTION_MONITOR                 = 1<<19,
         IEEE80211_HW_QUEUE_CONTROL                      = 1<<20,
         IEEE80211_HW_SUPPORTS_PER_STA_GTK               = 1<<21,
         IEEE80211_HW_AP_LINK_PS                         = 1<<22,
         IEEE80211_HW_TX_AMPDU_SETUP_IN_HW               = 1<<23,
         IEEE80211_HW_SCAN_WHILE_IDLE                    = 1<<24,
         IEEE80211_HW_P2P_DEV_ADDR_FOR_INTF              = 1<<25,
         IEEE80211_HW_TEARDOWN_AGGR_ON_BAR_FAIL          = 1<<26,
};



/**
 * enum ieee80211_filter_flags - hardware filter flags
 *
 * These flags determine what the filter in hardware should be
 * programmed to let through and what should not be passed to the
 * stack. It is always safe to pass more frames than requested,
 * but this has negative impact on power consumption.
 *
 * @FIF_PROMISC_IN_BSS: promiscuous mode within your BSS,
 *      think of the BSS as your network segment and then this corresponds
 *      to the regular ethernet device promiscuous mode.
 *
 * @FIF_ALLMULTI: pass all multicast frames, this is used if requested
 *      by the user or if the hardware is not capable of filtering by
 *      multicast address.
 *
 * @FIF_FCSFAIL: pass frames with failed FCS (but you need to set the
 *      %RX_FLAG_FAILED_FCS_CRC for them)
 *
 * @FIF_PLCPFAIL: pass frames with failed PLCP CRC (but you need to set
 *      the %RX_FLAG_FAILED_PLCP_CRC for them
 *
 * @FIF_BCN_PRBRESP_PROMISC: This flag is set during scanning to indicate
 *      to the hardware that it should not filter beacons or probe responses
 *      by BSSID. Filtering them can greatly reduce the amount of processing
 *      mac80211 needs to do and the amount of CPU wakeups, so you should
 *      honour this flag if possible.
 *
 * @FIF_CONTROL: pass control frames (except for PS Poll), if PROMISC_IN_BSS
 *      is not set then only those addressed to this station.
 *
 * @FIF_OTHER_BSS: pass frames destined to other BSSes
 *
 * @FIF_PSPOLL: pass PS Poll frames, if PROMISC_IN_BSS is not set then only
 *      those addressed to this station.
 *
 * @FIF_PROBE_REQ: pass probe request frames
 */
enum ieee80211_filter_flags {
         FIF_PROMISC_IN_BSS      = 1<<0,
         FIF_ALLMULTI            = 1<<1,
         FIF_FCSFAIL             = 1<<2,
         FIF_PLCPFAIL            = 1<<3,
         FIF_BCN_PRBRESP_PROMISC = 1<<4,
         FIF_CONTROL             = 1<<5,
         FIF_OTHER_BSS           = 1<<6,
         FIF_PSPOLL              = 1<<7,
         FIF_PROBE_REQ           = 1<<8,
};


/**
 * enum ieee80211_conf_changed - denotes which configuration changed
 *
 * @IEEE80211_CONF_CHANGE_LISTEN_INTERVAL: the listen interval changed
 * @IEEE80211_CONF_CHANGE_MONITOR: the monitor flag changed
 * @IEEE80211_CONF_CHANGE_PS: the PS flag or dynamic PS timeout changed
 * @IEEE80211_CONF_CHANGE_POWER: the TX power changed
 * @IEEE80211_CONF_CHANGE_CHANNEL: the channel/channel_type changed
 * @IEEE80211_CONF_CHANGE_RETRY_LIMITS: retry limits changed
 * @IEEE80211_CONF_CHANGE_IDLE: Idle flag changed
 * @IEEE80211_CONF_CHANGE_SMPS: Spatial multiplexing powersave mode changed
 */
enum ieee80211_conf_changed {
	IEEE80211_CONF_CHANGE_SMPS		= BIT(1),
	IEEE80211_CONF_CHANGE_LISTEN_INTERVAL	= BIT(2),
	IEEE80211_CONF_CHANGE_MONITOR		= BIT(3),
	IEEE80211_CONF_CHANGE_PS		= BIT(4),
	IEEE80211_CONF_CHANGE_POWER		= BIT(5),
	IEEE80211_CONF_CHANGE_CHANNEL		= BIT(6),
	IEEE80211_CONF_CHANGE_RETRY_LIMITS	= BIT(7),
	IEEE80211_CONF_CHANGE_IDLE		= BIT(8),
};
/**
 * enum ieee80211_bss_change - BSS change notification flags
 *
 * These flags are used with the bss_info_changed() callback
 * to indicate which BSS parameter changed.
 *
 * @BSS_CHANGED_ASSOC: association status changed (associated/disassociated),
 *	also implies a change in the AID.
 * @BSS_CHANGED_ERP_CTS_PROT: CTS protection changed
 * @BSS_CHANGED_ERP_PREAMBLE: preamble changed
 * @BSS_CHANGED_ERP_SLOT: slot timing changed
 * @BSS_CHANGED_HT: 802.11n parameters changed
 * @BSS_CHANGED_BASIC_RATES: Basic rateset changed
 * @BSS_CHANGED_BEACON_INT: Beacon interval changed
 * @BSS_CHANGED_BSSID: BSSID changed, for whatever
 *	reason (IBSS and managed mode)
 * @BSS_CHANGED_BEACON: Beacon data changed, retrieve
 *	new beacon (beaconing modes)
 * @BSS_CHANGED_BEACON_ENABLED: Beaconing should be
 *	enabled/disabled (beaconing modes)
 * @BSS_CHANGED_CQM: Connection quality monitor config changed
 * @BSS_CHANGED_IBSS: IBSS join status changed
 * @BSS_CHANGED_ARP_FILTER: Hardware ARP filter address list or state changed.
 * @BSS_CHANGED_QOS: QoS for this association was enabled/disabled. Note
 *	that it is only ever disabled for station mode.
 * @BSS_CHANGED_IDLE: Idle changed for this BSS/interface.
 * @BSS_CHANGED_SSID: SSID changed for this BSS (AP mode)
 * @BSS_CHANGED_AP_PROBE_RESP: Probe Response changed for this BSS (AP mode)
 */
enum ieee80211_bss_change {
	BSS_CHANGED_ASSOC		= 1<<0,
	BSS_CHANGED_ERP_CTS_PROT	= 1<<1,
	BSS_CHANGED_ERP_PREAMBLE	= 1<<2,
	BSS_CHANGED_ERP_SLOT		= 1<<3,
	BSS_CHANGED_HT			= 1<<4,
	BSS_CHANGED_BASIC_RATES		= 1<<5,
	BSS_CHANGED_BEACON_INT		= 1<<6,
	BSS_CHANGED_BSSID		= 1<<7,
	BSS_CHANGED_BEACON		= 1<<8,
	BSS_CHANGED_BEACON_ENABLED	= 1<<9,
	BSS_CHANGED_CQM			= 1<<10,
	BSS_CHANGED_IBSS		= 1<<11,
	BSS_CHANGED_ARP_FILTER		= 1<<12,
	BSS_CHANGED_QOS			= 1<<13,
	BSS_CHANGED_IDLE		= 1<<14,
	BSS_CHANGED_SSID		= 1<<15,
	BSS_CHANGED_AP_PROBE_RESP	= 1<<16,

	/* when adding here, make sure to change ieee80211_reconfig */
};
/**
 * enum ieee80211_ac_numbers - AC numbers as used in mac80211
 * @IEEE80211_AC_VO: voice
 * @IEEE80211_AC_VI: video
 * @IEEE80211_AC_BE: best effort
 * @IEEE80211_AC_BK: background
 */
enum ieee80211_ac_numbers {
	IEEE80211_AC_VO		= 0,
	IEEE80211_AC_VI		= 1,
	IEEE80211_AC_BE		= 2,
	IEEE80211_AC_BK		= 3,
};
#define IEEE80211_NUM_ACS	4


/*
 * The maximum number of IPv4 addresses listed for ARP filtering. If the number
 * of addresses for an interface increase beyond this value, hardware ARP
 * filtering will be disabled.
 */
#define IEEE80211_BSS_ARP_ADDR_LIST_LEN 4




/**
 * struct ieee80211_bss_conf - holds the BSS's changing parameters
 *
 * This structure keeps information about a BSS (and an association
 * to that BSS) that can change during the lifetime of the BSS.
 *
 * @assoc: association status
 * @ibss_joined: indicates whether this station is part of an IBSS
 *	or not
 * @aid: association ID number, valid only when @assoc is true
 * @use_cts_prot: use CTS protection
 * @use_short_preamble: use 802.11b short preamble;
 *	if the hardware cannot handle this it must set the
 *	IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE hardware flag
 * @use_short_slot: use short slot time (only relevant for ERP);
 *	if the hardware cannot handle this it must set the
 *	IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE hardware flag
 * @dtim_period: num of beacons before the next DTIM, for beaconing,
 *	valid in station mode only while @assoc is true and if also
 *	requested by %IEEE80211_HW_NEED_DTIM_PERIOD (cf. also hw conf
 *	@ps_dtim_period)
 * @sync_tsf: last beacon's/probe response's TSF timestamp (could be old
 *	as it may have been received during scanning long ago)
 * @sync_device_ts: the device timestamp corresponding to the sync_tsf,
 *	the driver/device can use this to calculate synchronisation
 * @beacon_int: beacon interval
 * @assoc_capability: capabilities taken from assoc resp
 * @basic_rates: bitmap of basic rates, each bit stands for an
 *	index into the rate table configured by the driver in
 *	the current band.
 * @mcast_rate: per-band multicast rate index + 1 (0: disabled)
 * @bssid: The BSSID for this BSS
 * @enable_beacon: whether beaconing should be enabled or not
 * @channel_type: Channel type for this BSS -- the hardware might be
 *	configured for HT40+ while this BSS only uses no-HT, for
 *	example.
 * @ht_operation_mode: HT operation mode like in &struct ieee80211_ht_operation.
 *	This field is only valid when the channel type is one of the HT types.
 * @cqm_rssi_thold: Connection quality monitor RSSI threshold, a zero value
 *	implies disabled
 * @cqm_rssi_hyst: Connection quality monitor RSSI hysteresis
 * @arp_addr_list: List of IPv4 addresses for hardware ARP filtering. The
 *	may filter ARP queries targeted for other addresses than listed here.
 *	The driver must allow ARP queries targeted for all address listed here
 *	to pass through. An empty list implies no ARP queries need to pass.
 * @arp_addr_cnt: Number of addresses currently on the list.
 * @arp_filter_enabled: Enable ARP filtering - if enabled, the hardware may
 *	filter ARP queries based on the @arp_addr_list, if disabled, the
 *	hardware must not perform any ARP filtering. Note, that the filter will
 *	be enabled also in promiscuous mode.
 * @qos: This is a QoS-enabled BSS.
 * @idle: This interface is idle. There's also a global idle flag in the
 *	hardware config which may be more appropriate depending on what
 *	your driver/device needs to do.
 * @ssid: The SSID of the current vif. Only valid in AP-mode.
 * @ssid_len: Length of SSID given in @ssid.
 * @hidden_ssid: The SSID of the current vif is hidden. Only valid in AP-mode.
 */
struct ieee80211_bss_conf {
	const U8 *bssid;
	/* association related data */
	bool assoc, ibss_joined;
	U16 aid;
	/* erp related data */
	bool use_cts_prot;
	bool use_short_preamble;
	bool use_short_slot;
	bool enable_beacon;
	U8 dtim_period;
	U8 bcn_ctrl_period;
	U16 beacon_int;
	U16 assoc_capability;
	U64 sync_tsf;
	U32 sync_device_ts;
	U32 basic_rates;
	int mcast_rate[IEEE80211_NUM_BANDS];
	U16 ht_operation_mode;
	S32 cqm_rssi_thold;
	U32 cqm_rssi_hyst;
	enum nl80211_channel_type channel_type;
	be32_t arp_addr_list[IEEE80211_BSS_ARP_ADDR_LIST_LEN];
	U8 arp_addr_cnt;
	bool arp_filter_enabled;
	bool qos;
	bool idle;
	U8 ssid[IEEE80211_MAX_SSID_LEN];
	size_t ssid_len;
	bool hidden_ssid;
	U16 atim_window;
	U16 soft_beacon_int;
};




/**
 * enum ieee80211_smps_mode - spatial multiplexing power save mode
 *
 * @IEEE80211_SMPS_AUTOMATIC: automatic
 * @IEEE80211_SMPS_OFF: off
 * @IEEE80211_SMPS_STATIC: static
 * @IEEE80211_SMPS_DYNAMIC: dynamic
 * @IEEE80211_SMPS_NUM_MODES: internal, don't use
 */
enum ieee80211_smps_mode {
	IEEE80211_SMPS_AUTOMATIC,
	IEEE80211_SMPS_OFF,
	IEEE80211_SMPS_STATIC,
	IEEE80211_SMPS_DYNAMIC,

	/* keep last */
	IEEE80211_SMPS_NUM_MODES,
};


/**
 * struct ieee80211_tx_queue_params - transmit queue configuration
 *
 * The information provided in this structure is required for QoS
 * transmit queue configuration. Cf. IEEE 802.11 7.3.2.29.
 *
 * @aifs: arbitration interframe space [0..255]
 * @cw_min: minimum contention window [a value of the form
 *	2^n-1 in the range 1..32767]
 * @cw_max: maximum contention window [like @cw_min]
 * @txop: maximum burst time in units of 32 usecs, 0 meaning disabled
 * @uapsd: is U-APSD mode enabled for the queue
 */
struct ieee80211_tx_queue_params {
	U16 txop;
	U16 cw_min;
	U16 cw_max;
	U8 aifs;
	bool uapsd;
};



/**
 * struct ieee80211_conf - configuration of the device
 *
 * This struct indicates how the driver shall configure the hardware.
 *
 * @flags: configuration flags defined above
 *
 * @listen_interval: listen interval in units of beacon interval
 * @max_sleep_period: the maximum number of beacon intervals to sleep for
 *	before checking the beacon for a TIM bit (managed mode only); this
 *	value will be only achievable between DTIM frames, the hardware
 *	needs to check for the multicast traffic bit in DTIM beacons.
 *	This variable is valid only when the CONF_PS flag is set.
 * @ps_dtim_period: The DTIM period of the AP we're connected to, for use
 *	in power saving. Power saving will not be enabled until a beacon
 *	has been received and the DTIM period is known.
 * @dynamic_ps_timeout: The dynamic powersave timeout (in ms), see the
 *	powersave documentation below. This variable is valid only when
 *	the CONF_PS flag is set.
 *
 * @power_level: requested transmit power (in dBm)
 *
 * @channel: the channel to tune to
 * @channel_type: the channel (HT) type
 *
 * @long_frame_max_tx_count: Maximum number of transmissions for a "long" frame
 *    (a frame not RTS protected), called "dot11LongRetryLimit" in 802.11,
 *    but actually means the number of transmissions not the number of retries
 * @short_frame_max_tx_count: Maximum number of transmissions for a "short"
 *    frame, called "dot11ShortRetryLimit" in 802.11, but actually means the
 *    number of transmissions not the number of retries
 *
 * @smps_mode: spatial multiplexing powersave mode; note that
 *	%IEEE80211_SMPS_STATIC is used when the device is not
 *	configured for an HT channel
 */
struct ieee80211_conf {
	U32 flags;
	int power_level, dynamic_ps_timeout;
	int max_sleep_period;

	U16 listen_interval;
	U8 ps_dtim_period;

	U8 long_frame_max_tx_count, short_frame_max_tx_count;

	struct ieee80211_channel *channel;
	enum nl80211_channel_type channel_type;
	enum ieee80211_smps_mode smps_mode;
};
/**
 * struct ieee80211_hw - hardware information and state
 *
 * This structure contains the configuration and hardware
 * information for an 802.11 PHY.
 *
 * @wiphy: This points to the &struct wiphy allocated for this
 *	802.11 PHY. You must fill in the @perm_addr and @dev
 *	members of this structure using SET_IEEE80211_DEV()
 *	and SET_IEEE80211_PERM_ADDR(). Additionally, all supported
 *	bands (with channels, bitrates) are registered here.
 *
 * @conf: &struct ieee80211_conf, device configuration, don't use.
 *
 * @priv: pointer to private area that was allocated for driver use
 *	along with this structure.
 *
 * @flags: hardware flags, see &enum ieee80211_hw_flags.
 *
 * @extra_tx_headroom: headroom to reserve in each transmit skb
 *	for use by the driver (e.g. for transmit headers.)
 *
 * @channel_change_time: time (in microseconds) it takes to change channels.
 *
 * @max_signal: Maximum value for signal (rssi) in RX information, used
 *     only when @IEEE80211_HW_SIGNAL_UNSPEC or @IEEE80211_HW_SIGNAL_DB
 *
 * @max_listen_interval: max listen interval in units of beacon interval
 *     that HW supports
 *
 * @queues: number of available hardware transmit queues for
 *	data packets. WMM/QoS requires at least four, these
 *	queues need to have configurable access parameters.
 *
 * @rate_control_algorithm: rate control algorithm for this hardware.
 *	If unset (NULL), the default algorithm will be used. Must be
 *	set before calling ieee80211_register_hw().
 *
 * @vif_data_size: size (in bytes) of the drv_priv data area
 *	within &struct ieee80211_vif.
 * @sta_data_size: size (in bytes) of the drv_priv data area
 *	within &struct ieee80211_sta.
 *
 * @max_rates: maximum number of alternate rate retry stages the hw
 *	can handle.
 * @max_report_rates: maximum number of alternate rate retry stages
 *	the hw can report back.
 * @max_rate_tries: maximum number of tries for each stage
 *
 * @napi_weight: weight used for NAPI polling.  You must specify an
 *	appropriate value here if a napi_poll operation is provided
 *	by your driver.
 *
 * @max_rx_aggregation_subframes: maximum buffer size (number of
 *	sub-frames) to be used for A-MPDU block ack receiver
 *	aggregation.
 *	This is only relevant if the device has restrictions on the
 *	number of subframes, if it relies on mac80211 to do reordering
 *	it shouldn't be set.
 *
 * @max_tx_aggregation_subframes: maximum number of subframes in an
 *	aggregate an HT driver will transmit, used by the peer as a
 *	hint to size its reorder buffer.
 *
 * @offchannel_tx_hw_queue: HW queue ID to use for offchannel TX
 *	(if %IEEE80211_HW_QUEUE_CONTROL is set)
 *
 * @radiotap_mcs_details: lists which MCS information can the HW
 *	reports, by default it is set to _MCS, _GI and _BW but doesn't
 *	include _FMT. Use %IEEE80211_RADIOTAP_MCS_HAVE_* values, only
 *	adding _BW is supported today.
 *
 * @netdev_features: netdev features to be set in each netdev created
 *	from this HW. Note only HW checksum features are currently
 *	compatible with mac80211. Other feature bits will be rejected.
 */
struct ieee80211_hw {
	struct ieee80211_conf conf;
	struct wiphy *wiphy;
	const char *rate_control_algorithm;
	void *priv;
	U32 flags;
	unsigned int extra_tx_headroom;
	int channel_change_time;
	int vif_data_size;
	int sta_data_size;
	int napi_weight;
	U16 queues;
	U16 max_listen_interval;
	S8 max_signal;
	U8 max_rates;
	U8 max_report_rates;
	U8 max_rate_tries;
	U8 max_rx_aggregation_subframes;
	U8 max_tx_aggregation_subframes;
	U8 offchannel_tx_hw_queue;
	U8 radiotap_mcs_details;
	//netdev_features_t netdev_features; FIXME
};

/**
 * SET_IEEE80211_PERM_ADDR - set the permanent MAC address for 802.11 hardware
 *
 * @hw: the &struct ieee80211_hw to set the MAC address for
 * @addr: the address to set
 */
inline void SET_IEEE80211_PERM_ADDR(struct ieee80211_hw *hw, U8 *addr) 
{
	memcpy(hw->wiphy->perm_addr, addr, ETH_ALEN);
}




/**
 * struct ieee80211_vif - per-interface data
 *
 * Data in this structure is continually present for driver
 * use during the life of a virtual interface.
 *
 * @type: type of this virtual interface
 * @bss_conf: BSS configuration for this interface, either our own
 *	or the BSS we're associated to
 * @addr: address of this interface
 * @p2p: indicates whether this AP or STA interface is a p2p
 *	interface, i.e. a GO or p2p-sta respectively
 * @driver_flags: flags/capabilities the driver has for this interface,
 *	these need to be set (or cleared) when the interface is added
 *	or, if supported by the driver, the interface type is changed
 *	at runtime, mac80211 will never touch this field
 * @hw_queue: hardware queue for each AC
 * @cab_queue: content-after-beacon (DTIM beacon really) queue, AP mode only
 * @drv_priv: data area for driver use, will always be aligned to
 *	sizeof(void *).
 */
struct ieee80211_vif {
	enum nl80211_iftype type;
	struct ieee80211_bss_conf bss_conf;
	U8 addr[ETH_ALEN];
	bool p2p;

	U8 cab_queue;
	U8 hw_queue[IEEE80211_NUM_ACS];

	U32 driver_flags;
	
	bool prepared; // Added by John

	/* must be last */
	U8 drv_priv[0] __attribute__((aligned(sizeof(void *))));
};

/**
 * struct ieee80211_tx_rate - rate selection/status
 *
 * @idx: rate index to attempt to send with
 * @flags: rate control flags (&enum mac80211_rate_control_flags)
 * @count: number of tries in this rate before going to the next rate
 *
 * A value of -1 for @idx indicates an invalid rate and, if used
 * in an array of retry rates, that no more rates should be tried.
 *
 * When used for transmit status reporting, the driver should
 * always report the rate along with the flags it used.
 *
 * &struct ieee80211_tx_info contains an array of these structs
 * in the control information, and it will be filled by the rate
 * control algorithm according to what should be sent. For example,
 * if this array contains, in the format { <idx>, <count> } the
 * information
 *    { 3, 2 }, { 2, 2 }, { 1, 4 }, { -1, 0 }, { -1, 0 }
 * then this means that the frame should be transmitted
 * up to twice at rate 3, up to twice at rate 2, and up to four
 * times at rate 1 if it doesn't get acknowledged. Say it gets
 * acknowledged by the peer after the fifth attempt, the status
 * information should then contain
 *   { 3, 2 }, { 2, 2 }, { 1, 1 }, { -1, 0 } ...
 * since it was transmitted twice at rate 3, twice at rate 2
 * and once at rate 1 after which we received an acknowledgement.
 */
struct ieee80211_tx_rate {
	S8 idx;
	U8 count;
	U8 flags;
} __attribute__((packed));





/* maximum number of rate stages */
#define IEEE80211_TX_MAX_RATES	4

// XXX Modified - adjusted to our needs
struct ieee80211_tx_info {
	
	uint32_t flags;
	uint8_t band;
	
	U8 hw_queue;
	
	U16 ack_frame_id;
	
	union {
		struct {
			union {
				/* rate control */
				struct {
					struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];
					S8 rts_cts_rate_idx;
				};
				/* only needed before rate control */
				unsigned long jiffies;
			};
			/* NB: vif can be NULL for injected frames */
		/*	
			struct ieee80211_vif *vif;
			struct ieee80211_key_conf *hw_key;
			struct ieee80211_sta *sta;
		*/
		} control;
		struct {
		struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];
		int ack_signal;
		U8 ampdu_ack_len;
		U8 ampdu_len;
		U8 antenna;
		/* 21 bytes free */
		} status;	
	};		
};



/**
 * enum mac80211_rx_flags - receive flags
 *
 * These flags are used with the @flag member of &struct ieee80211_rx_status.
 * @RX_FLAG_MMIC_ERROR: Michael MIC error was reported on this frame.
 *	Use together with %RX_FLAG_MMIC_STRIPPED.
 * @RX_FLAG_DECRYPTED: This frame was decrypted in hardware.
 * @RX_FLAG_MMIC_STRIPPED: the Michael MIC is stripped off this frame,
 *	verification has been done by the hardware.
 * @RX_FLAG_IV_STRIPPED: The IV/ICV are stripped from this frame.
 *	If this flag is set, the stack cannot do any replay detection
 *	hence the driver or hardware will have to do that.
 * @RX_FLAG_FAILED_FCS_CRC: Set this flag if the FCS check failed on
 *	the frame.
 * @RX_FLAG_FAILED_PLCP_CRC: Set this flag if the PCLP check failed on
 *	the frame.
 * @RX_FLAG_MACTIME_MPDU: The timestamp passed in the RX status (@mactime
 *	field) is valid and contains the time the first symbol of the MPDU
 *	was received. This is useful in monitor mode and for proper IBSS
 *	merging.
 * @RX_FLAG_SHORTPRE: Short preamble was used for this frame
 * @RX_FLAG_HT: HT MCS was used and rate_idx is MCS index
 * @RX_FLAG_40MHZ: HT40 (40 MHz) was used
 * @RX_FLAG_SHORT_GI: Short guard interval was used
 * @RX_FLAG_NO_SIGNAL_VAL: The signal strength value is not present.
 *	Valid only for data frames (mainly A-MPDU)
 * @RX_FLAG_HT_GF: This frame was received in a HT-greenfield transmission, if
 *	the driver fills this value it should add %IEEE80211_RADIOTAP_MCS_HAVE_FMT
 *	to hw.radiotap_mcs_details to advertise that fact
 */
enum mac80211_rx_flags {
	RX_FLAG_MMIC_ERROR	= 1<<0,
	RX_FLAG_DECRYPTED	= 1<<1,
	RX_FLAG_MMIC_STRIPPED	= 1<<3,
	RX_FLAG_IV_STRIPPED	= 1<<4,
	RX_FLAG_FAILED_FCS_CRC	= 1<<5,
	RX_FLAG_FAILED_PLCP_CRC = 1<<6,
	RX_FLAG_MACTIME_MPDU	= 1<<7,
	RX_FLAG_SHORTPRE	= 1<<8,
	RX_FLAG_HT		= 1<<9,
	RX_FLAG_40MHZ		= 1<<10,
	RX_FLAG_SHORT_GI	= 1<<11,
	RX_FLAG_NO_SIGNAL_VAL	= 1<<12,
	RX_FLAG_HT_GF		= 1<<13,
};


/**
 * struct ieee80211_rx_status - receive status
 *
 * The low-level driver should provide this information (the subset
 * supported by hardware) to the 802.11 code with each received
 * frame, in the skb's control buffer (cb).
 *
 * @mactime: value in microseconds of the 64-bit Time Synchronization Function
 * 	(TSF) timer when the first data symbol (MPDU) arrived at the hardware.
 * @device_timestamp: arbitrary timestamp for the device, mac80211 doesn't use
 *	it but can store it and pass it back to the driver for synchronisation
 * @band: the active band when this frame was received
 * @freq: frequency the radio was tuned to when receiving this frame, in MHz
 * @signal: signal strength when receiving this frame, either in dBm, in dB or
 *	unspecified depending on the hardware capabilities flags
 *	@IEEE80211_HW_SIGNAL_*
 * @antenna: antenna used
 * @rate_idx: index of data rate into band's supported rates or MCS index if
 *	HT rates are use (RX_FLAG_HT)
 * @flag: %RX_FLAG_*
 * @rx_flags: internal RX flags for mac80211
 */
struct ieee80211_rx_status {
	U64 mactime;
	U32 device_timestamp;
	U16 flag;
	U16 freq;
	U8 rate_idx;
	U8 rx_flags;
	U8 band;
	U8 antenna;
	S8 signal;
};


/**
 * struct ieee80211_sta - station table entry
 *
 * A station table entry represents a station we are possibly
 * communicating with. Since stations are RCU-managed in
 * mac80211, any ieee80211_sta pointer you get access to must
 * either be protected by rcu_read_lock() explicitly or implicitly,
 * or you must take good care to not use such a pointer after a
 * call to your sta_remove callback that removed it.
 *
 * @addr: MAC address
 * @aid: AID we assigned to the station if we're an AP
 * @supp_rates: Bitmap of supported rates (per band)
 * @ht_cap: HT capabilities of this STA; restricted to our own TX capabilities
 * @wme: indicates whether the STA supports WME. Only valid during AP-mode.
 * @drv_priv: data area for driver use, will always be aligned to
 *	sizeof(void *), size is determined in hw information.
 * @uapsd_queues: bitmap of queues configured for uapsd. Only valid
 *	if wme is supported.
 * @max_sp: max Service Period. Only valid if wme is supported.
 */
struct ieee80211_sta {
	U32 supp_rates[IEEE80211_NUM_BANDS];
	U8 addr[ETH_ALEN];
	U16 aid;
	struct ieee80211_sta_ht_cap ht_cap;
	bool wme;
	U8 uapsd_queues;
	U8 max_sp;

	/* must be last */
	U8 drv_priv[0] __attribute__((__aligned__(sizeof(void *))));
};
#endif /* MAC80211_H_ */





