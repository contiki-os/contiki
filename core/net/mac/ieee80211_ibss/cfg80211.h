/*
 * 802.11 device and configuration interface
 *
 * Copyright 2006-2010  Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "nl80211.h"
#include "if_ether.h"
#include "ethtool.h"
#include "ieee80211.h"
#include "regulatory.h"
#include "bitops.h"
#include "compiler.h"


#ifndef CFG80211_H_
#define CFG80211_H_

#define NETDEV_ALIGN            32 // FIXME to netdevice.h



/**
 * enum wiphy_flags - wiphy capability flags
 *
 * @WIPHY_FLAG_CUSTOM_REGULATORY:  tells us the driver for this device
 *      has its own custom regulatory domain and cannot identify the
 *      ISO / IEC 3166 alpha2 it belongs to. When this is enabled
 *      we will disregard the first regulatory hint (when the
 *      initiator is %REGDOM_SET_BY_CORE).
 * @WIPHY_FLAG_STRICT_REGULATORY: tells us the driver for this device will
 *      ignore regulatory domain settings until it gets its own regulatory
 *      domain via its regulatory_hint() unless the regulatory hint is
 *      from a country IE. After its gets its own regulatory domain it will
 *      only allow further regulatory domain settings to further enhance
 *      compliance. For example if channel 13 and 14 are disabled by this
 *      regulatory domain no user regulatory domain can enable these channels
 *      at a later time. This can be used for devices which do not have
 *      calibration information guaranteed for frequencies or settings
 *      outside of its regulatory domain. If used in combination with
 *      WIPHY_FLAG_CUSTOM_REGULATORY the inspected country IE power settings
 *      will be followed.
 * @WIPHY_FLAG_DISABLE_BEACON_HINTS: enable this if your driver needs to ensure
 *      that passive scan flags and beaconing flags may not be lifted by
 *      cfg80211 due to regulatory beacon hints. For more information on beacon
 *      hints read the documenation for regulatory_hint_found_beacon()
 * @WIPHY_FLAG_NETNS_OK: if not set, do not allow changing the netns of this
 *      wiphy at all
 * @WIPHY_FLAG_PS_ON_BY_DEFAULT: if set to true, powersave will be enabled
 *      by default -- this flag will be set depending on the kernel's default
 *      on wiphy_new(), but can be changed by the driver if it has a good
 *      reason to override the default
 * @WIPHY_FLAG_4ADDR_AP: supports 4addr mode even on AP (with a single station
 *      on a VLAN interface)
 * @WIPHY_FLAG_4ADDR_STATION: supports 4addr mode even as a station
 * @WIPHY_FLAG_CONTROL_PORT_PROTOCOL: This device supports setting the
 *      control port protocol ethertype. The device also honours the
 *      control_port_no_encrypt flag.
 * @WIPHY_FLAG_IBSS_RSN: The device supports IBSS RSN.
 * @WIPHY_FLAG_MESH_AUTH: The device supports mesh authentication by routing
 *      auth frames to userspace. See @NL80211_MESH_SETUP_USERSPACE_AUTH.
 * @WIPHY_FLAG_SUPPORTS_SCHED_SCAN: The device supports scheduled scans.
 * @WIPHY_FLAG_SUPPORTS_FW_ROAM: The device supports roaming feature in the
 *      firmware.
 * @WIPHY_FLAG_AP_UAPSD: The device supports uapsd on AP.
 * @WIPHY_FLAG_SUPPORTS_TDLS: The device supports TDLS (802.11z) operation.
 * @WIPHY_FLAG_TDLS_EXTERNAL_SETUP: The device does not handle TDLS (802.11z)
 *      link setup/discovery operations internally. Setup, discovery and
 *      teardown packets should be sent through the @NL80211_CMD_TDLS_MGMT
 *      command. When this flag is not set, @NL80211_CMD_TDLS_OPER should be
 *      used for asking the driver/firmware to perform a TDLS operation.
 * @WIPHY_FLAG_HAVE_AP_SME: device integrates AP SME
 * @WIPHY_FLAG_REPORTS_OBSS: the device will report beacons from other BSSes
 *      when there are virtual interfaces in AP mode by calling
 *      cfg80211_report_obss_beacon().
 * @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD: When operating as an AP, the device
 *      responds to probe-requests in hardware.
 * @WIPHY_FLAG_OFFCHAN_TX: Device supports direct off-channel TX.
 * @WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL: Device supports remain-on-channel call.
 */
enum wiphy_flags {
         WIPHY_FLAG_CUSTOM_REGULATORY            = BIT(0),
         WIPHY_FLAG_STRICT_REGULATORY            = BIT(1),
         WIPHY_FLAG_DISABLE_BEACON_HINTS         = BIT(2),
         WIPHY_FLAG_NETNS_OK                     = BIT(3),
         WIPHY_FLAG_PS_ON_BY_DEFAULT             = BIT(4),
         WIPHY_FLAG_4ADDR_AP                     = BIT(5),
         WIPHY_FLAG_4ADDR_STATION                = BIT(6),
         WIPHY_FLAG_CONTROL_PORT_PROTOCOL        = BIT(7),
         WIPHY_FLAG_IBSS_RSN                     = BIT(8),
         WIPHY_FLAG_MESH_AUTH                    = BIT(10),
         WIPHY_FLAG_SUPPORTS_SCHED_SCAN          = BIT(11),
         /* use hole at 12 */
         WIPHY_FLAG_SUPPORTS_FW_ROAM             = BIT(13),
         WIPHY_FLAG_AP_UAPSD                     = BIT(14),
         WIPHY_FLAG_SUPPORTS_TDLS                = BIT(15),
         WIPHY_FLAG_TDLS_EXTERNAL_SETUP          = BIT(16),
         WIPHY_FLAG_HAVE_AP_SME                  = BIT(17),
         WIPHY_FLAG_REPORTS_OBSS                 = BIT(18),
         WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD        = BIT(19),
         WIPHY_FLAG_OFFCHAN_TX                   = BIT(20),
         WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL        = BIT(21),
};


/**
 * enum survey_info_flags - survey information flags
 *
 * @SURVEY_INFO_NOISE_DBM: noise (in dBm) was filled in
 * @SURVEY_INFO_IN_USE: channel is currently being used
 * @SURVEY_INFO_CHANNEL_TIME: channel active time (in ms) was filled in
 * @SURVEY_INFO_CHANNEL_TIME_BUSY: channel busy time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_EXT_BUSY: extension channel busy time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_RX: channel receive time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_TX: channel transmit time was filled in
 *
 * Used by the driver to indicate which info in &struct survey_info
 * it has filled in during the get_survey().
 */
enum survey_info_flags {
	SURVEY_INFO_NOISE_DBM = 1<<0,
	SURVEY_INFO_IN_USE = 1<<1,
	SURVEY_INFO_CHANNEL_TIME = 1<<2,
	SURVEY_INFO_CHANNEL_TIME_BUSY = 1<<3,
	SURVEY_INFO_CHANNEL_TIME_EXT_BUSY = 1<<4,
	SURVEY_INFO_CHANNEL_TIME_RX = 1<<5,
	SURVEY_INFO_CHANNEL_TIME_TX = 1<<6,
};

/**
 * struct survey_info - channel survey response
 *
 * @channel: the channel this survey record reports, mandatory
 * @filled: bitflag of flags from &enum survey_info_flags
 * @noise: channel noise in dBm. This and all following fields are
 *     optional
 * @channel_time: amount of time in ms the radio spent on the channel
 * @channel_time_busy: amount of time the primary channel was sensed busy
 * @channel_time_ext_busy: amount of time the extension channel was sensed busy
 * @channel_time_rx: amount of time the radio spent receiving data
 * @channel_time_tx: amount of time the radio spent transmitting data
 *
 * Used by dump_survey() to report back per-channel survey information.
 *
 * This structure can later be expanded with things like
 * channel duty cycle etc.
 */
struct survey_info {
	struct ieee80211_channel *channel;
	U64 channel_time;
	U64 channel_time_busy;
	U64 channel_time_ext_busy;
	U64 channel_time_rx;
	U64 channel_time_tx;
	U32 filled;
	S8 noise;
};
/**
 * enum cfg80211_signal_type - signal type
 *
 * @CFG80211_SIGNAL_TYPE_NONE: no signal strength information available
 * @CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm)
 * @CFG80211_SIGNAL_TYPE_UNSPEC: signal strength, increasing from 0 through 100
 */
enum cfg80211_signal_type {
	CFG80211_SIGNAL_TYPE_NONE,
	CFG80211_SIGNAL_TYPE_MBM,
	CFG80211_SIGNAL_TYPE_UNSPEC,
};


/**
 * enum ieee80211_band - supported frequency bands
 *
 * The bands are assigned this way because the supported
 * bitrates differ in these bands.
 *
 * @IEEE80211_BAND_2GHZ: 2.4GHz ISM band
 * @IEEE80211_BAND_5GHZ: around 5GHz band (4.9-5.7)
 * @IEEE80211_BAND_60GHZ: around 60 GHz band (58.32 - 64.80 GHz)
 * @IEEE80211_NUM_BANDS: number of defined bands
 */
enum ieee80211_band {
	IEEE80211_BAND_2GHZ = NL80211_BAND_2GHZ,
	IEEE80211_BAND_5GHZ = NL80211_BAND_5GHZ,
	IEEE80211_BAND_60GHZ = NL80211_BAND_60GHZ,

	/* keep last */
	IEEE80211_NUM_BANDS
};


/**
 * struct ieee80211_channel - channel definition
 *
 * This structure describes a single channel for use
 * with cfg80211.
 *
 * @center_freq: center frequency in MHz
 * @hw_value: hardware-specific value for the channel
 * @flags: channel flags from &enum ieee80211_channel_flags.
 * @orig_flags: channel flags at registration time, used by regulatory
 *	code to support devices with additional restrictions
 * @band: band this channel belongs to.
 * @max_antenna_gain: maximum antenna gain in dBi
 * @max_power: maximum transmission power (in dBm)
 * @max_reg_power: maximum regulatory transmission power (in dBm)
 * @beacon_found: helper to regulatory code to indicate when a beacon
 *	has been found on this channel. Use regulatory_hint_found_beacon()
 *	to enable this, this is useful only on 5 GHz band.
 * @orig_mag: internal use
 * @orig_mpwr: internal use
 */
struct ieee80211_channel {
	enum ieee80211_band band;
	U16 center_freq;
	U16 hw_value;
	U32 flags;
	int max_antenna_gain;
	int max_power;
	int max_reg_power;
	bool beacon_found;
	U32 orig_flags;
	int orig_mag, orig_mpwr;
};


/**
 * enum ieee80211_rate_flags - rate flags
 *
 * Hardware/specification flags for rates. These are structured
 * in a way that allows using the same bitrate structure for
 * different bands/PHY modes.
 *
 * @IEEE80211_RATE_SHORT_PREAMBLE: Hardware can send with short
 *	preamble on this bitrate; only relevant in 2.4GHz band and
 *	with CCK rates.
 * @IEEE80211_RATE_MANDATORY_A: This bitrate is a mandatory rate
 *	when used with 802.11a (on the 5 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_MANDATORY_B: This bitrate is a mandatory rate
 *	when used with 802.11b (on the 2.4 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_MANDATORY_G: This bitrate is a mandatory rate
 *	when used with 802.11g (on the 2.4 GHz band); filled by the
 *	core code when registering the wiphy.
 * @IEEE80211_RATE_ERP_G: This is an ERP rate in 802.11g mode.
 */
enum ieee80211_rate_flags {
	IEEE80211_RATE_SHORT_PREAMBLE	= 1<<0,
	IEEE80211_RATE_MANDATORY_A	= 1<<1,
	IEEE80211_RATE_MANDATORY_B	= 1<<2,
	IEEE80211_RATE_MANDATORY_G	= 1<<3,
	IEEE80211_RATE_ERP_G		= 1<<4,
};



/**
 * struct ieee80211_rate - bitrate definition
 *
 * This structure describes a bitrate that an 802.11 PHY can
 * operate with. The two values @hw_value and @hw_value_short
 * are only for driver use when pointers to this structure are
 * passed around.
 *
 * @flags: rate-specific flags
 * @bitrate: bitrate in units of 100 Kbps
 * @hw_value: driver/hardware value for this rate
 * @hw_value_short: driver/hardware value for this rate when
 *	short preamble is used
 */
struct ieee80211_rate {
	U32 flags;
	U16 bitrate;
	U16 hw_value, hw_value_short;
};



/**
 * struct ieee80211_sta_ht_cap - STA's HT capabilities
 *
 * This structure describes most essential parameters needed
 * to describe 802.11n HT capabilities for an STA.
 *
 * @ht_supported: is HT supported by the STA
 * @cap: HT capabilities map as described in 802.11n spec
 * @ampdu_factor: Maximum A-MPDU length factor
 * @ampdu_density: Minimum A-MPDU spacing
 * @mcs: Supported MCS rates
 */
struct ieee80211_sta_ht_cap {
	U16 cap; /* use IEEE80211_HT_CAP_ */
	bool ht_supported;
	U8 ampdu_factor;
	U8 ampdu_density;
	struct ieee80211_mcs_info mcs;
};


/**
 * struct ieee80211_sta_vht_cap - STA's VHT capabilities
 *
 * This structure describes most essential parameters needed
 * to describe 802.11ac VHT capabilities for an STA.
 *
 * @vht_supported: is VHT supported by the STA
 * @cap: VHT capabilities map as described in 802.11ac spec
 * @vht_mcs: Supported VHT MCS rates
 */
struct ieee80211_sta_vht_cap {
	bool vht_supported;
	U32 cap; /* use IEEE80211_VHT_CAP_ */
	struct ieee80211_vht_mcs_info vht_mcs;
};


/**
 * struct ieee80211_supported_band - frequency band definition
 *
 * This structure describes a frequency band a wiphy
 * is able to operate in.
 *
 * @channels: Array of channels the hardware can operate in
 *	in this band.
 * @band: the band this structure represents
 * @n_channels: Number of channels in @channels
 * @bitrates: Array of bitrates the hardware can operate with
 *	in this band. Must be sorted to give a valid "supported
 *	rates" IE, i.e. CCK rates first, then OFDM.
 * @n_bitrates: Number of bitrates in @bitrates
 * @ht_cap: HT capabilities in this band
 */
struct ieee80211_supported_band {
	struct ieee80211_channel *channels;
	struct ieee80211_rate *bitrates;
	enum ieee80211_band band;
	int n_channels;
	int n_bitrates;
	struct ieee80211_sta_ht_cap ht_cap;
	struct ieee80211_sta_vht_cap vht_cap;
};



/**
 * struct ieee80211_iface_limit - limit on certain interface types
 * @max: maximum number of interfaces of these types
 * @types: interface types (bits)
 */
struct ieee80211_iface_limit {
	U16 max;
	U16 types;
};

/**
 * struct ieee80211_iface_combination - possible interface combination
 * @limits: limits for the given interface types
 * @n_limits: number of limitations
 * @num_different_channels: can use up to this many different channels
 * @max_interfaces: maximum number of interfaces in total allowed in this
 *	group
 * @beacon_int_infra_match: In this combination, the beacon intervals
 *	between infrastructure and AP types must match. This is required
 *	only in special cases.
 *
 * These examples can be expressed as follows:
 *
 * Allow #STA <= 1, #AP <= 1, matching BI, channels = 1, 2 total:
 *
 *  struct ieee80211_iface_limit limits1[] = {
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_STATION), },
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_AP}, },
 *  };
 *  struct ieee80211_iface_combination combination1 = {
 *	.limits = limits1,
 *	.n_limits = ARRAY_SIZE(limits1),
 *	.max_interfaces = 2,
 *	.beacon_int_infra_match = true,
 *  };
 *
 *
 * Allow #{AP, P2P-GO} <= 8, channels = 1, 8 total:
 *
 *  struct ieee80211_iface_limit limits2[] = {
 *	{ .max = 8, .types = BIT(NL80211_IFTYPE_AP) |
 *			     BIT(NL80211_IFTYPE_P2P_GO), },
 *  };
 *  struct ieee80211_iface_combination combination2 = {
 *	.limits = limits2,
 *	.n_limits = ARRAY_SIZE(limits2),
 *	.max_interfaces = 8,
 *	.num_different_channels = 1,
 *  };
 *
 *
 * Allow #STA <= 1, #{P2P-client,P2P-GO} <= 3 on two channels, 4 total.
 * This allows for an infrastructure connection and three P2P connections.
 *
 *  struct ieee80211_iface_limit limits3[] = {
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_STATION), },
 *	{ .max = 3, .types = BIT(NL80211_IFTYPE_P2P_GO) |
 *			     BIT(NL80211_IFTYPE_P2P_CLIENT), },
 *  };
 *  struct ieee80211_iface_combination combination3 = {
 *	.limits = limits3,
 *	.n_limits = ARRAY_SIZE(limits3),
 *	.max_interfaces = 4,
 *	.num_different_channels = 2,
 *  };
 */
struct ieee80211_iface_combination {
	const struct ieee80211_iface_limit *limits;
	U32 num_different_channels;
	U16 max_interfaces;
	U8 n_limits;
	bool beacon_int_infra_match;
};

struct mac_address {
	U8 addr[ETH_ALEN];
};

struct ieee80211_txrx_stypes {
	U16 tx, rx;
};

/**
 * struct wiphy_wowlan_support - WoWLAN support data
 * @flags: see &enum wiphy_wowlan_support_flags
 * @n_patterns: number of supported wakeup patterns
 *	(see nl80211.h for the pattern definition)
 * @pattern_max_len: maximum length of each pattern
 * @pattern_min_len: minimum length of each pattern
 */
struct wiphy_wowlan_support {
	U32 flags;
	int n_patterns;
	int pattern_max_len;
	int pattern_min_len;
};



/**
 * struct wiphy - wireless hardware description
 * @reg_notifier: the driver's regulatory notification callback,
 *	note that if your driver uses wiphy_apply_custom_regulatory()
 *	the reg_notifier's request can be passed as NULL
 * @regd: the driver's regulatory domain, if one was requested via
 * 	the regulatory_hint() API. This can be used by the driver
 *	on the reg_notifier() if it chooses to ignore future
 *	regulatory domain changes caused by other drivers.
 * @signal_type: signal type reported in &struct cfg80211_bss.
 * @cipher_suites: supported cipher suites
 * @n_cipher_suites: number of supported cipher suites
 * @retry_short: Retry limit for short frames (dot11ShortRetryLimit)
 * @retry_long: Retry limit for long frames (dot11LongRetryLimit)
 * @frag_threshold: Fragmentation threshold (dot11FragmentationThreshold);
 *	-1 = fragmentation disabled, only odd values >= 256 used
 * @rts_threshold: RTS threshold (dot11RTSThreshold); -1 = RTS/CTS disabled
 * @_net: the network namespace this wiphy currently lives in
 * @perm_addr: permanent MAC address of this device
 * @addr_mask: If the device supports multiple MAC addresses by masking,
 *	set this to a mask with variable bits set to 1, e.g. if the last
 *	four bits are variable then set it to 00:...:00:0f. The actual
 *	variable bits shall be determined by the interfaces added, with
 *	interfaces not matching the mask being rejected to be brought up.
 * @n_addresses: number of addresses in @addresses.
 * @addresses: If the device has more than one address, set this pointer
 *	to a list of addresses (6 bytes each). The first one will be used
 *	by default for perm_addr. In this case, the mask should be set to
 *	all-zeroes. In this case it is assumed that the device can handle
 *	the same number of arbitrary MAC addresses.
 * @registered: protects ->resume and ->suspend sysfs callbacks against
 *	unregister hardware
 * @debugfsdir: debugfs directory used for this wiphy, will be renamed
 *	automatically on wiphy renames
 * @dev: (virtual) struct device for this wiphy
 * @registered: helps synchronize suspend/resume with wiphy unregister
 * @wext: wireless extension handlers
 * @priv: driver private data (sized according to wiphy_new() parameter)
 * @interface_modes: bitmask of interfaces types valid for this wiphy,
 *	must be set by driver
 * @iface_combinations: Valid interface combinations array, should not
 *	list single interface types.
 * @n_iface_combinations: number of entries in @iface_combinations array.
 * @software_iftypes: bitmask of software interface types, these are not
 *	subject to any restrictions since they are purely managed in SW.
 * @flags: wiphy flags, see &enum wiphy_flags
 * @features: features advertised to nl80211, see &enum nl80211_feature_flags.
 * @bss_priv_size: each BSS struct has private data allocated with it,
 *	this variable determines its size
 * @max_scan_ssids: maximum number of SSIDs the device can scan for in
 *	any given scan
 * @max_sched_scan_ssids: maximum number of SSIDs the device can scan
 *	for in any given scheduled scan
 * @max_match_sets: maximum number of match sets the device can handle
 *	when performing a scheduled scan, 0 if filtering is not
 *	supported.
 * @max_scan_ie_len: maximum length of user-controlled IEs device can
 *	add to probe request frames transmitted during a scan, must not
 *	include fixed IEs like supported rates
 * @max_sched_scan_ie_len: same as max_scan_ie_len, but for scheduled
 *	scans
 * @coverage_class: current coverage class
 * @fw_version: firmware version for ethtool reporting
 * @hw_version: hardware version for ethtool reporting
 * @max_num_pmkids: maximum number of PMKIDs supported by device
 * @privid: a pointer that drivers can use to identify if an arbitrary
 *	wiphy is theirs, e.g. in global notifiers
 * @bands: information about bands/channels supported by this device
 *
 * @mgmt_stypes: bitmasks of frame subtypes that can be subscribed to or
 *	transmitted through nl80211, points to an array indexed by interface
 *	type
 *
 * @available_antennas_tx: bitmap of antennas which are available to be
 *	configured as TX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_rx is set.
 *
 * @available_antennas_rx: bitmap of antennas which are available to be
 *	configured as RX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_tx is set.
 *
 * @probe_resp_offload:
 *	 Bitmap of supported protocols for probe response offloading.
 *	 See &enum nl80211_probe_resp_offload_support_attr. Only valid
 *	 when the wiphy flag @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD is set.
 *
 * @max_remain_on_channel_duration: Maximum time a remain-on-channel operation
 *	may request, if implemented.
 *
 * @wowlan: WoWLAN support information
 *
 * @ap_sme_capa: AP SME capabilities, flags from &enum nl80211_ap_sme_features.
 * @ht_capa_mod_mask:  Specify what ht_cap values can be over-ridden.
 *	If null, then none can be over-ridden.
 */
struct wiphy {
	/* assign these fields before you register the wiphy */

#define WIPHY_COMPAT_PAD_SIZE	2048
	U8 padding[WIPHY_COMPAT_PAD_SIZE];

	/* permanent MAC address(es) */
	U8 perm_addr[ETH_ALEN];
	U8 addr_mask[ETH_ALEN];

	struct mac_address *addresses;

	const struct ieee80211_txrx_stypes *mgmt_stypes;

	const struct ieee80211_iface_combination *iface_combinations;
	int n_iface_combinations;
	U16 software_iftypes;

	U16 n_addresses;

	/* Supported interface modes, OR together BIT(NL80211_IFTYPE_...) */
	U16 interface_modes;

	U32 flags, features;

	U32 ap_sme_capa;

	enum cfg80211_signal_type signal_type;

	int bss_priv_size;
	U8 max_scan_ssids;
	U8 max_sched_scan_ssids;
	U8 max_match_sets;
	U16 max_scan_ie_len;
	U16 max_sched_scan_ie_len;

	int n_cipher_suites;
	const U32 *cipher_suites;

	U8 retry_short;
	U8 retry_long;
	U32 frag_threshold;
	U32 rts_threshold;
	U8 coverage_class;

	char fw_version[ETHTOOL_BUSINFO_LEN];
	U32 hw_version;

#ifdef CONFIG_PM
	struct wiphy_wowlan_support wowlan;
#endif

	U16 max_remain_on_channel_duration;

	U8 max_num_pmkids;

	U32 available_antennas_tx;
	U32 available_antennas_rx;

	/*
	 * Bitmap of supported protocols for probe response offloading
	 * see &enum nl80211_probe_resp_offload_support_attr. Only valid
	 * when the wiphy flag @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD is set.
	 */
	U32 probe_resp_offload;

	/* If multiple wiphys are registered and you're handed e.g.
	 * a regular netdev with assigned ieee80211_ptr, you won't
	 * know whether it points to a wiphy your driver has registered
	 * or not. Assign this to something global to your driver to
	 * help determine whether you own this wiphy or not. */
	const void *privid;

	struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];

	/* Lets us get back the wiphy on the callback */
	//FIXME XXX int (*reg_notifier)(struct wiphy *wiphy,struct regulatory_request *request);

	/* fields below are read-only, assigned by cfg80211 */

	const struct ieee80211_regdomain *regd;

	/* the item in /sys/class/ieee80211/ points to this,
	 * you need use set_wiphy_dev() (see below) */
	// FIXMEstruct device dev;

	/* protects ->resume, ->suspend sysfs callbacks against unregister hw */
	bool registered;

	/* dir in debugfs: ieee80211/<wiphyname> */
	// FIXME struct dentry *debugfsdir;

	const struct ieee80211_ht_cap *ht_capa_mod_mask;

#ifdef CONFIG_NET_NS
	/* the network namespace this phy lives in currently */
	struct net *_net;
#endif

#ifdef CONFIG_CFG80211_WEXT
	const struct iw_handler_def *wext;
#endif

	char priv[0] __attribute__((__aligned__(NETDEV_ALIGN)));
};



















#endif /* CFG80211_H_ */