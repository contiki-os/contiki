/*
 * eeprom.h
 *
 * Created: 4/12/2013 7:32:21 PM
 *  Author: Ioannis Glaropoulos
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_


#define AR9170_EEPROM_START		0x1600

#define AR5416_MAX_CHAINS		2
#define AR5416_MODAL_SPURS		5


struct ar9170_eeprom_modal {
	le32_t	antCtrlChain[AR5416_MAX_CHAINS];
	le32_t	antCtrlCommon;
	S8	antennaGainCh[AR5416_MAX_CHAINS];
	U8	switchSettling;
	U8	txRxAttenCh[AR5416_MAX_CHAINS];
	U8	rxTxMarginCh[AR5416_MAX_CHAINS];
	S8	adcDesiredSize;
	S8	pgaDesiredSize;
	U8	xlnaGainCh[AR5416_MAX_CHAINS];
	U8	txEndToXpaOff;
	U8	txEndToRxOn;
	U8	txFrameToXpaOn;
	U8	thresh62;
	S8	noiseFloorThreshCh[AR5416_MAX_CHAINS];
	U8	xpdGain;
	U8	xpd;
	S8	iqCalICh[AR5416_MAX_CHAINS];
	S8	iqCalQCh[AR5416_MAX_CHAINS];
	U8	pdGainOverlap;
	U8	ob;
	U8	db;
	U8	xpaBiasLvl;
	U8	pwrDecreaseFor2Chain;
	U8	pwrDecreaseFor3Chain;
	U8	txFrameToDataStart;
	U8	txFrameToPaOn;
	U8	ht40PowerIncForPdadc;
	U8	bswAtten[AR5416_MAX_CHAINS];
	U8	bswMargin[AR5416_MAX_CHAINS];
	U8	swSettleHt40;
	U8	reserved[22];
	struct spur_channel {
		le16_t spurChan;
		U8	spurRangeLow;
		U8	spurRangeHigh;
	} __attribute__((packed)) spur_channels[AR5416_MODAL_SPURS];
} __attribute__((packed));


#define AR5416_NUM_PD_GAINS		4
#define AR5416_PD_GAIN_ICEPTS		5

struct ar9170_calibration_data_per_freq {
	U8	pwr_pdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
	U8	vpd_pdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
} __attribute__((packed));

#define AR5416_NUM_5G_CAL_PIERS		8
#define AR5416_NUM_2G_CAL_PIERS		4

#define AR5416_NUM_5G_TARGET_PWRS	8
#define AR5416_NUM_2G_CCK_TARGET_PWRS	3
#define AR5416_NUM_2G_OFDM_TARGET_PWRS	4
#define AR5416_MAX_NUM_TGT_PWRS		8

struct ar9170_calibration_target_power_legacy {
	U8	freq;
	U8	power[4];
} __attribute__((packed));

struct ar9170_calibration_target_power_ht {
	U8	freq;
	U8	power[8];
} __attribute__((packed));

#define AR5416_NUM_CTLS			24

struct ar9170_calctl_edges {
	U8	channel;
	#define AR9170_CALCTL_EDGE_FLAGS	0xC0
	U8	power_flags;
} __attribute__((packed));

#define AR5416_NUM_BAND_EDGES		8

struct ar9170_calctl_data {
	struct ar9170_calctl_edges
	control_edges[AR5416_MAX_CHAINS][AR5416_NUM_BAND_EDGES];
} __attribute__((packed));

struct ar9170_eeprom {
	le16_t	length;
	le16_t	checksum;
	le16_t	version;
	U8	operating_flags;
	#define AR9170_OPFLAG_5GHZ		1
	#define AR9170_OPFLAG_2GHZ		2
	U8	misc;
	le16_t	reg_domain[2];
	U8	mac_address[6];
	U8	rx_mask;
	U8	tx_mask;
	le16_t	rf_silent;
	le16_t	bluetooth_options;
	le16_t	device_capabilities;
	le32_t	build_number;
	U8	deviceType;
	U8	reserved[33];

	U8	customer_data[64];

	struct ar9170_eeprom_modal
	modal_header[2];

	U8	cal_freq_pier_5G[AR5416_NUM_5G_CAL_PIERS];
	U8	cal_freq_pier_2G[AR5416_NUM_2G_CAL_PIERS];

	struct ar9170_calibration_data_per_freq
	cal_pier_data_5G[AR5416_MAX_CHAINS][AR5416_NUM_5G_CAL_PIERS],
	cal_pier_data_2G[AR5416_MAX_CHAINS][AR5416_NUM_2G_CAL_PIERS];

	/* power calibration data */
	struct ar9170_calibration_target_power_legacy
	cal_tgt_pwr_5G[AR5416_NUM_5G_TARGET_PWRS];
	struct ar9170_calibration_target_power_ht
	cal_tgt_pwr_5G_ht20[AR5416_NUM_5G_TARGET_PWRS],
	cal_tgt_pwr_5G_ht40[AR5416_NUM_5G_TARGET_PWRS];

	struct ar9170_calibration_target_power_legacy
	cal_tgt_pwr_2G_cck[AR5416_NUM_2G_CCK_TARGET_PWRS],
	cal_tgt_pwr_2G_ofdm[AR5416_NUM_2G_OFDM_TARGET_PWRS];
	struct ar9170_calibration_target_power_ht
	cal_tgt_pwr_2G_ht20[AR5416_NUM_2G_OFDM_TARGET_PWRS],
	cal_tgt_pwr_2G_ht40[AR5416_NUM_2G_OFDM_TARGET_PWRS];

	/* conformance testing limits */
	U8	ctl_index[AR5416_NUM_CTLS];
	struct ar9170_calctl_data
	ctl_data[AR5416_NUM_CTLS];

	U8	pad;
	le16_t	subsystem_id;
} __attribute__((packed));




#define AR9170_LED_MODE_POWER_ON		0x0001
#define AR9170_LED_MODE_RESERVED		0x0002
#define AR9170_LED_MODE_DISABLE_STATE		0x0004
#define AR9170_LED_MODE_OFF_IN_PSM		0x0008

/* AR9170_LED_MODE BIT is set */
#define AR9170_LED_MODE_FREQUENCY_S		4
#define AR9170_LED_MODE_FREQUENCY		0x0030
#define AR9170_LED_MODE_FREQUENCY_1HZ		0x0000
#define AR9170_LED_MODE_FREQUENCY_0_5HZ		0x0010
#define AR9170_LED_MODE_FREQUENCY_0_25HZ	0x0020
#define AR9170_LED_MODE_FREQUENCY_0_125HZ	0x0030

/* AR9170_LED_MODE BIT is not set */
#define AR9170_LED_MODE_CONN_STATE_S		4
#define AR9170_LED_MODE_CONN_STATE		0x0030
#define AR9170_LED_MODE_CONN_STATE_FORCE_OFF	0x0000
#define AR9170_LED_MODE_CONN_STATE_FORCE_ON	0x0010
/* Idle off / Active on */
#define AR9170_LED_MODE_CONN_STATE_IOFF_AON	0x0020
/* Idle on / Active off */
#define AR9170_LED_MODE_CONN_STATE_ION_AOFF	0x0010

#define AR9170_LED_MODE_MODE			0x0040
#define AR9170_LED_MODE_RESERVED2		0x0080

#define AR9170_LED_MODE_TON_SCAN_S		8
#define AR9170_LED_MODE_TON_SCAN		0x0f00

#define AR9170_LED_MODE_TOFF_SCAN_S		12
#define AR9170_LED_MODE_TOFF_SCAN		0xf000

struct ar9170_led_mode {
	le16_t led;
};

#endif /* EEPROM_H_ */