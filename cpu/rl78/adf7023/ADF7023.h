/*
 * Copyright (c) 2014, Analog Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Dragos Bogdan <Dragos.Bogdan@Analog.com>
 * Contributors: Ian Martin <martini@redwirellc.com>
 */

#ifndef __ADF7023_H__
#define __ADF7023_H__

/* Status Word */
#define STATUS_SPI_READY  (0x1 << 7)
#define STATUS_IRQ_STATUS (0x1 << 6)
#define STATUS_CMD_READY  (0x1 << 5)
#define STATUS_FW_STATE   (0x1F << 0)

/* FW_STATE Description */
#define FW_STATE_INIT             0x0F
#define FW_STATE_BUSY             0x00
#define FW_STATE_PHY_OFF          0x11
#define FW_STATE_PHY_ON           0x12
#define FW_STATE_PHY_RX           0x13
#define FW_STATE_PHY_TX           0x14
#define FW_STATE_PHY_SLEEP        0x06
#define FW_STATE_GET_RSSI         0x05
#define FW_STATE_IR_CAL           0x07
#define FW_STATE_AES_DECRYPT_INIT 0x08
#define FW_STATE_AES_DECRYPT      0x09
#define FW_STATE_AES_ENCRYPT      0x0A

/* SPI Memory Access Commands */
#define SPI_MEM_WR  0x18 /* Write data to packet RAM sequentially. */
#define SPI_MEM_RD  0x38 /* Read data from packet RAM sequentially. */
#define SPI_MEMR_WR 0x08 /* Write data to packet RAM nonsequentially. */
#define SPI_MEMR_RD 0x28 /* Read data from packet RAM nonsequentially. */
#define SPI_NOP     0xFF /* No operation. */

/* Radio Controller Commands */
#define CMD_SYNC             0xA2 /* This is an optional command. It is not necessary to use it during device initialization */
#define CMD_PHY_OFF          0xB0 /* Performs a transition of the device into the PHY_OFF state. */
#define CMD_PHY_ON           0xB1 /* Performs a transition of the device into the PHY_ON state. */
#define CMD_PHY_RX           0xB2 /* Performs a transition of the device into the PHY_RX state. */
#define CMD_PHY_TX           0xB5 /* Performs a transition of the device into the PHY_TX state. */
#define CMD_PHY_SLEEP        0xBA /* Performs a transition of the device into the PHY_SLEEP state. */
#define CMD_CONFIG_DEV       0xBB /* Configures the radio parameters based on the BBRAM values. */
#define CMD_GET_RSSI         0xBC /* Performs an RSSI measurement. */
#define CMD_BB_CAL           0xBE /* Performs a calibration of the IF filter. */
#define CMD_HW_RESET         0xC8 /* Performs a full hardware reset. The device enters the PHY_SLEEP state. */
#define CMD_RAM_LOAD_INIT    0xBF /* Prepares the program RAM for a firmware module download. */
#define CMD_RAM_LOAD_DONE    0xC7 /* Performs a reset of the communications processor after download of a firmware module to program RAM. */
#define CMD_IR_CAL           0xBD /* Initiates an image rejection calibration routine. */
#define CMD_AES_ENCRYPT      0xD0 /* Performs an AES encryption on the transmit payload data stored in packet RAM. */
#define CMD_AES_DECRYPT      0xD2 /* Performs an AES decryption on the received payload data stored in packet RAM. */
#define CMD_AES_DECRYPT_INIT 0xD1 /* Initializes the internal variables required for AES decryption. */
#define CMD_RS_ENCODE_INIT   0xD1 /* Initializes the internal variables required for the Reed Solomon encoding. */
#define CMD_RS_ENCODE        0xD0 /* Calculates and appends the Reed Solomon check bytes to the transmit payload data stored in packet RAM. */
#define CMD_RS_DECODE        0xD2 /* Performs a Reed Solomon error correction on the received payload data stored in packet RAM. */

/* Battery Backup Memory (BBRAM) */
#define BBRAM_REG_INTERRUPT_MASK_0                  0x100
#define BBRAM_REG_INTERRUPT_MASK_1                  0x101
#define BBRAM_REG_NUMBER_OF_WAKEUPS_0               0x102
#define BBRAM_REG_NUMBER_OF_WAKEUPS_1               0x103
#define BBRAM_REG_NUMBER_OF_WAKEUPS_IRQ_THRESHOLD_0 0x104
#define BBRAM_REG_NUMBER_OF_WAKEUPS_IRQ_THRESHOLD_1 0x105
#define BBRAM_REG_RX_DWELL_TIME                     0x106
#define BBRAM_REG_PARMTIME_DIVIDER                  0x107
#define BBRAM_REG_SWM_RSSI_THRESH                   0x108
#define BBRAM_REG_CHANNEL_FREQ_0                    0x109
#define BBRAM_REG_CHANNEL_FREQ_1                    0x10A
#define BBRAM_REG_CHANNEL_FREQ_2                    0x10B
#define BBRAM_REG_RADIO_CFG_0                       0x10C
#define BBRAM_REG_RADIO_CFG_1                       0x10D
#define BBRAM_REG_RADIO_CFG_2                       0x10E
#define BBRAM_REG_RADIO_CFG_3                       0x10F
#define BBRAM_REG_RADIO_CFG_4                       0x110
#define BBRAM_REG_RADIO_CFG_5                       0x111
#define BBRAM_REG_RADIO_CFG_6                       0x112
#define BBRAM_REG_RADIO_CFG_7                       0x113
#define BBRAM_REG_RADIO_CFG_8                       0x114
#define BBRAM_REG_RADIO_CFG_9                       0x115
#define BBRAM_REG_RADIO_CFG_10                      0x116
#define BBRAM_REG_RADIO_CFG_11                      0x117
#define BBRAM_REG_IMAGE_REJECT_CAL_PHASE            0x118
#define BBRAM_REG_IMAGE_REJECT_CAL_AMPLITUDE        0x119
#define BBRAM_REG_MODE_CONTROL                      0x11A
#define BBRAM_REG_PREAMBLE_MATCH                    0x11B
#define BBRAM_REG_SYMBOL_MODE                       0x11C
#define BBRAM_REG_PREAMBLE_LEN                      0x11D
#define BBRAM_REG_CRC_POLY_0                        0x11E
#define BBRAM_REG_CRC_POLY_1                        0x11F
#define BBRAM_REG_SYNC_CONTROL                      0x120
#define BBRAM_REG_SYNC_BYTE_0                       0x121
#define BBRAM_REG_SYNC_BYTE_1                       0x122
#define BBRAM_REG_SYNC_BYTE_2                       0x123
#define BBRAM_REG_TX_BASE_ADR                       0x124
#define BBRAM_REG_RX_BASE_ADR                       0x125
#define BBRAM_REG_PACKET_LENGTH_CONTROL             0x126
#define BBRAM_REG_PACKET_LENGTH_MAX                 0x127
#define BBRAM_REG_STATIC_REG_FIX                    0x128
#define BBRAM_REG_ADDRESS_MATCH_OFFSET              0x129
#define BBRAM_REG_ADDRESS_LENGTH                    0x12A
#define BBRAM_REG_ADDRESS_FILTERING_0               0x12B
#define BBRAM_REG_ADDRESS_FILTERING_1               0x12C
#define BBRAM_REG_ADDRESS_FILTERING_2               0x12D
#define BBRAM_REG_ADDRESS_FILTERING_3               0x12E
#define BBRAM_REG_ADDRESS_FILTERING_4               0x12F
#define BBRAM_REG_ADDRESS_FILTERING_5               0x130
#define BBRAM_REG_ADDRESS_FILTERING_6               0x131
#define BBRAM_REG_ADDRESS_FILTERING_7               0x132
#define BBRAM_REG_ADDRESS_FILTERING_8               0x133
#define BBRAM_REG_ADDRESS_FILTERING_9               0x134
#define BBRAM_REG_ADDRESS_FILTERING_10              0x135
#define BBRAM_REG_ADDRESS_FILTERING_11              0x136
#define BBRAM_REG_ADDRESS_FILTERING_12              0x137
#define BBRAM_REG_RSSI_WAIT_TIME                    0x138
#define BBRAM_REG_TESTMODES                         0x139
#define BBRAM_REG_TRANSITION_CLOCK_DIV              0x13A
#define BBRAM_REG_RESERVED_0                        0x13B
#define BBRAM_REG_RESERVED_1                        0x13C
#define BBRAM_REG_RESERVED_2                        0x13D
#define BBRAM_REG_RX_SYNTH_LOCK_TIME                0x13E
#define BBRAM_REG_TX_SYNTH_LOCK_TIME                0x13F

/* BBRAM_REG_INTERRUPT_MASK_0 - 0x100 */
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_NUM_WAKEUPS     (0x1 << 7)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_SWM_RSSI_DET    (0x1 << 6)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_AES_DONE        (0x1 << 5)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_TX_EOF          (0x1 << 4)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_ADDRESS_MATCH   (0x1 << 3)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT     (0x1 << 2)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_SYNC_DETECT     (0x1 << 1)
#define BBRAM_INTERRUPT_MASK_0_INTERRUPT_PREMABLE_DETECT (0x1 << 0)

/* BBRAM_REG_INTERRUPT_MASK_1 - 0x101*/
#define BBRAM_INTERRUPT_MASK_1_BATTERY_ALARM (0x1 << 7)
#define BBRAM_INTERRUPT_MASK_1_CMD_READY     (0x1 << 6)
#define BBRAM_INTERRUPT_MASK_1_WUC_TIMEOUT   (0x1 << 4)
#define BBRAM_INTERRUPT_MASK_1_SPI_READY     (0x1 << 1)
#define BBRAM_INTERRUPT_MASK_1_CMD_FINISHED  (0x1 << 0)

/* BBRAM_REG_RADIO_CFG_0 - 0x10C */
#define BBRAM_RADIO_CFG_0_DATA_RATE_7_0(x) ((x & 0xFF) << 0)

/* BBRAM_REG_RADIO_CFG_1 - 0x10D */
#define BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8(x) ((x & 0xF) << 4)
#define BBRAM_RADIO_CFG_1_DATA_RATE_11_8(x)      ((x & 0xF) << 0)

/* BBRAM_REG_RADIO_CFG_2 - 0x10E */
#define BBRAM_RADIO_CFG_2_FREQ_DEVIATION_7_0(x) ((x & 0xFF) << 0)

/* BBRAM_REG_RADIO_CFG_6 - 0x112 */
#define BBRAM_RADIO_CFG_6_SYNTH_LUT_CONFIG_0(x) ((x & 0x3F) << 2)
#define BBRAM_RADIO_CFG_6_DISCRIM_PHASE(x)      ((x & 0x3) << 0)

/* BBRAM_REG_RADIO_CFG_7 - 0x113 */
#define BBRAM_RADIO_CFG_7_AGC_LOCK_MODE(x)      ((x & 0x3) << 6)
#define BBRAM_RADIO_CFG_7_SYNTH_LUT_CONTROL(x)  ((x & 0x3) << 4)
#define BBRAM_RADIO_CFG_7_SYNTH_LUT_CONFIG_1(x) ((x & 0xF) << 0)

/* BBRAM_REG_RADIO_CFG_8 - 0x114 */
#define BBRAM_RADIO_CFG_8_PA_SINGLE_DIFF_SEL (0x1 << 7)
#define BBRAM_RADIO_CFG_8_PA_LEVEL(x)        ((x & 0xF) << 3)
#define BBRAM_RADIO_CFG_8_PA_RAMP(x)         ((x & 0x7) << 0)

/* BBRAM_REG_RADIO_CFG_9 - 0x115 */
#define BBRAM_RADIO_CFG_9_IFBW(x)         ((x & 0x3) << 6)
#define BBRAM_RADIO_CFG_9_MOD_SCHEME(x)   ((x & 0x7) << 3)
#define BBRAM_RADIO_CFG_9_DEMOD_SCHEME(x) ((x & 0x7) << 0)

/* BBRAM_REG_RADIO_CFG_10 - 0x116 */
#define BBRAM_RADIO_CFG_10_AFC_POLARITY     (0x0 << 4)
#define BBRAM_RADIO_CFG_10_AFC_SCHEME(x)    ((x & 0x3) << 2)
#define BBRAM_RADIO_CFG_10_AFC_LOCK_MODE(x) ((x & 0x3) << 0)

/* BBRAM_REG_RADIO_CFG_11 - 0x117 */
#define BBRAM_RADIO_CFG_11_AFC_KP(x) ((x & 0xF) << 4)
#define BBRAM_RADIO_CFG_11_AFC_KI(x) ((x & 0xF) << 0)

/* BBRAM_REG_MODE_CONTROL - 0x11A */
#define BBRAM_MODE_CONTROL_SWM_EN                        (0x1 << 7)
#define BBRAM_MODE_CONTROL_BB_CAL                        (0x1 << 6)
#define BBRAM_MODE_CONTROL_SWM_RSSI_QUAL                 (0x1 << 5)
#define BBRAM_MODE_CONTROL_TX_TO_RX_AUTO_TURNAROUND      (0x1 << 4)
#define BBRAM_MODE_CONTROL_RX_TO_TX_AUTO_TURNAROUND      (0x1 << 3)
#define BBRAM_MODE_CONTROL_CUSTOM_TRX_SYNTH_LOCK_TIME_EN (0x1 << 2)
#define BBRAM_MODE_CONTROL_EXT_LNA_EN                    (0x1 << 1)
#define BBRAM_MODE_CONTROL_EXT_PA_EN                     (0x1 << 0)

/* BBRAM_REG_SYMBOL_MODE - 0x11C */
#define BBRAM_SYMBOL_MODE_MANCHESTER_ENC   (0x1 << 6)
#define BBRAM_SYMBOL_MODE_PROG_CRC_EN      (0x1 << 5)
#define BBRAM_SYMBOL_MODE_EIGHT_TEN_ENC    (0x1 << 4)
#define BBRAM_SYMBOL_MODE_DATA_WHITENING   (0x1 << 3)
#define BBRAM_SYMBOL_MODE_SYMBOL_LENGTH(x) ((x & 0x7) << 0)

/* BBRAM_REG_SYNC_CONTROL - 0x120 */
#define BBRAM_SYNC_CONTROL_SYNC_ERROR_TOL(x)   ((x & 0x3) << 6)
#define BBRAM_SYNC_CONTROL_SYNC_WORD_LENGTH(x) ((x & 0x1F) << 0)

/* BBRAM_REG_PACKET_LENGTH_CONTROL - 0x126 */
#define BBRAM_PACKET_LENGTH_CONTROL_DATA_BYTE        (0x1 << 7)
#define BBRAM_PACKET_LENGTH_CONTROL_PACKET_LEN       (0x1 << 6)
#define BBRAM_PACKET_LENGTH_CONTROL_CRC_EN           (0x1 << 5)
#define BBRAM_PACKET_LENGTH_CONTROL_DATA_MODE(x)     ((x & 0x3) << 3)
#define BBRAM_PACKET_LENGTH_CONTROL_LENGTH_OFFSET(x) ((x & 0x7) << 0)

/* BBRAM_REG_TESTMODES - 0x139 */
#define BBRAM_TESTMODES_EXT_PA_LNA_ATB_CONFIG (0x1 << 7)
#define BBRAM_TESTMODES_PER_IRQ_SELF_CLEAR    (0x1 << 3)
#define BBRAM_TESTMODES_PER_ENABLE            (0x1 << 2)
#define BBRAM_TESTMODES_CONTINUOUS_TX         (0x1 << 1)
#define BBRAM_TESTMODES_CONTINUOUS_RX         (0x1 << 0)

/* Modem Configuration Memory (MCR) */
#define MCR_REG_PA_LEVEL_MCR                      0x307
#define MCR_REG_WUC_CONFIG_HIGH                   0x30C
#define MCR_REG_WUC_CONFIG_LOW                    0x30D
#define MCR_REG_WUC_VALUE_HIGH                    0x30E
#define MCR_REG_WUC_VALUE_LOW                     0x30F
#define MCR_REG_WUC_FLAG_RESET                    0x310
#define MCR_REG_WUC_STATUS                        0x311
#define MCR_REG_RSSI_READBACK                     0x312
#define MCR_REG_MAX_AFC_RANGE                     0x315
#define MCR_REG_IMAGE_REJECT_CAL_CONFIG           0x319
#define MCR_REG_CHIP_SHUTDOWN                     0x322
#define MCR_REG_POWERDOWN_RX                      0x324
#define MCR_REG_POWERDOWN_AUX                     0x325
#define MCR_REG_ADC_READBACK_HIGH                 0x327
#define MCR_REG_ADC_READBACK_LOW                  0x328
#define MCR_REG_BATTERY_MONITOR_THRESHOLD_VOLTAGE 0x32D
#define MCR_REG_EXT_UC_CLK_DIVIDE                 0x32E
#define MCR_REG_AGC_CLK_DIVIDE                    0x32F
#define MCR_REG_INTERRUPT_SOURCE_0                0x336
#define MCR_REG_INTERRUPT_SOURCE_1                0x337
#define MCR_REG_CALIBRATION_CONTROL               0x338
#define MCR_REG_CALIBRATION_STATUS                0x339
#define MCR_REG_RXBB_CAL_CALWRD_READBACK          0x345
#define MCR_REG_RXBB_CAL_CALWRD_OVERWRITE         0x346
#define MCR_REG_RCOSC_CAL_READBACK_HIGH           0x34F
#define MCR_REG_RCOSC_CAL_READBACK_LOW            0x350
#define MCR_REG_ADC_CONFIG_LOW                    0x359
#define MCR_REG_ADC_CONFIG_HIGH                   0x35A
#define MCR_REG_AGC_OOK_CONTROL                   0x35B
#define MCR_REG_AGC_CONFIG                        0x35C
#define MCR_REG_AGC_MODE                          0x35D
#define MCR_REG_AGC_LOW_THRESHOLD                 0x35E
#define MCR_REG_AGC_HIGH_THRESHOLD                0x35F
#define MCR_REG_AGC_GAIN_STATUS                   0x360
#define MCR_REG_AGC_ADC_WORD                      0x361
#define MCR_REG_FREQUENCY_ERROR_READBACK          0x372
#define MCR_REG_VCO_BAND_OVRW_VAL                 0x3CB
#define MCR_REG_VCO_AMPL_OVRW_VAL                 0x3CC
#define MCR_REG_VCO_OVRW_EN                       0x3CD
#define MCR_REG_VCO_CAL_CFG                       0x3D0
#define MCR_REG_OSC_CONFIG                        0x3D2
#define MCR_REG_VCO_BAND_READBACK                 0x3DA
#define MCR_REG_VCO_AMPL_READBACK                 0x3DB
#define MCR_REG_ANALOG_TEST_BUS                   0x3F8
#define MCR_REG_RSSI_TSTMUX_SEL                   0x3F9
#define MCR_REG_GPIO_CONFIGURE                    0x3FA
#define MCR_REG_TEST_DAC_GAIN                     0x3FD

struct ADF7023_BBRAM {
  unsigned char interruptMask0;                 /* 0x100 */
  unsigned char interruptMask1;                 /* 0x101 */
  unsigned char numberOfWakeups0;               /* 0x102 */
  unsigned char numberOfWakeups1;               /* 0x103 */
  unsigned char numberOfWakeupsIrqThreshold0;   /* 0x104 */
  unsigned char numberOfWakeupsIrqThreshold1;   /* 0x105 */
  unsigned char rxDwellTime;                    /* 0x106 */
  unsigned char parmtimeDivider;                /* 0x107 */
  unsigned char swmRssiThresh;                  /* 0x108 */
  unsigned char channelFreq0;                   /* 0x109 */
  unsigned char channelFreq1;                   /* 0x10A */
  unsigned char channelFreq2;                   /* 0x10B */
  unsigned char radioCfg0;                      /* 0x10C */
  unsigned char radioCfg1;                      /* 0x10D */
  unsigned char radioCfg2;                      /* 0x10E */
  unsigned char radioCfg3;                      /* 0x10F */
  unsigned char radioCfg4;                      /* 0x110 */
  unsigned char radioCfg5;                      /* 0x111 */
  unsigned char radioCfg6;                      /* 0x112 */
  unsigned char radioCfg7;                      /* 0x113 */
  unsigned char radioCfg8;                      /* 0x114 */
  unsigned char radioCfg9;                      /* 0x115 */
  unsigned char radioCfg10;                     /* 0x116 */
  unsigned char radioCfg11;                     /* 0x117 */
  unsigned char imageRejectCalPhase;            /* 0x118 */
  unsigned char imageRejectCalAmplitude;        /* 0x119 */
  unsigned char modeControl;                    /* 0x11A */
  unsigned char preambleMatch;                  /* 0x11B */
  unsigned char symbolMode;                     /* 0x11C */
  unsigned char preambleLen;                    /* 0x11D */
  unsigned char crcPoly0;                       /* 0x11E */
  unsigned char crcPoly1;                       /* 0x11F */
  unsigned char syncControl;                    /* 0x120 */
  unsigned char syncByte0;                      /* 0x121 */
  unsigned char syncByte1;                      /* 0x122 */
  unsigned char syncByte2;                      /* 0x123 */
  unsigned char txBaseAdr;                      /* 0x124 */
  unsigned char rxBaseAdr;                      /* 0x125 */
  unsigned char packetLengthControl;            /* 0x126 */
  unsigned char packetLengthMax;                /* 0x127 */
  unsigned char staticRegFix;                   /* 0x128 */
  unsigned char addressMatchOffset;             /* 0x129 */
  unsigned char addressLength;                  /* 0x12A */
  unsigned char addressFiltering0;              /* 0x12B */
  unsigned char addressFiltering1;              /* 0x12C */
  unsigned char addressFiltering2;              /* 0x12D */
  unsigned char addressFiltering3;              /* 0x12E */
  unsigned char addressFiltering4;              /* 0x12F */
  unsigned char addressFiltering5;              /* 0x130 */
  unsigned char addressFiltering6;              /* 0x131 */
  unsigned char addressFiltering7;              /* 0x132 */
  unsigned char addressFiltering8;              /* 0x133 */
  unsigned char addressFiltering9;              /* 0x134 */
  unsigned char addressFiltering10;             /* 0x135 */
  unsigned char addressFiltering11;             /* 0x136 */
  unsigned char addressFiltering12;             /* 0x137 */
  unsigned char rssiWaitTime;                   /* 0x138 */
  unsigned char testmodes;                      /* 0x139 */
  unsigned char transitionClockDiv;             /* 0x13A */
  unsigned char reserved0;                      /* 0x13B */
  unsigned char reserved1;                      /* 0x13C */
  unsigned char reserved2;                      /* 0x13D */
  unsigned char rxSynthLockTime;                /* 0x13E */
  unsigned char txSynthLockTime;                /* 0x13F */
};

#define ADF7023_RAM_SIZE 256

#define ADF7023_TX_BASE_ADR 0x10
#define ADF7023_RX_BASE_ADR ((ADF7023_TX_BASE_ADR + ADF7023_RAM_SIZE) / 2)

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initializes the ADF7023. */
char ADF7023_Init(void);

/* Reads the status word of the ADF7023. */
void ADF7023_GetStatus(unsigned char *status);

/* Initiates a command. */
void ADF7023_SetCommand(unsigned char command);

/* Sets a FW state and waits until the device enters in that state. */
void ADF7023_SetFwState(unsigned char fwState);

/* Reads data from the RAM. */
void ADF7023_GetRAM(unsigned long address,
                    unsigned long length,
                    unsigned char *data);

/* Writes data to RAM. */
void ADF7023_SetRAM(unsigned long address,
                    unsigned long length,
                    unsigned char *data);

void ADF7023_SetRAM_And_Verify(unsigned long address, unsigned long length, unsigned char *data);

/* Indicates if an incoming packet is available. */
unsigned char ADF7023_ReceivePacketAvailable(void);

/* Receives one packet. */
void ADF7023_ReceivePacket(unsigned char *packet, unsigned char *payload_length);

/* Transmits one packet. */
void ADF7023_TransmitPacket(unsigned char *packet, unsigned char length);

/* Sets the channel frequency. */
void ADF7023_SetChannelFrequency(unsigned long chFreq);

/* Sets the data rate. */
void ADF7023_SetDataRate(unsigned long dataRate);

/* Sets the frequency deviation. */
void ADF7023_SetFrequencyDeviation(unsigned long freqDev);

#endif /* __ADF7023_H__ */
