/*
 * Copyright (c) 2012, Kerlink
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* $Id: $ */

#ifndef CC112X_CONST_H
#define CC112X_CONST_H

/*
 * All constants are from the TI CC112X User Guide
 * http://www.ti.com/litv/pdf/swru295c
 *
 * The page numbers below refer to pages in this document.
 */


/* Page 85. */
#define CC112X_POWER_MASK 0x3F
#define CC112X_POWER_MAX  0x3F
#define CC112X_POWER_MIN  0x03
#define CC112X_POWER_OFF  0x00

/* Page 12. */
enum CC112X_header_byte {
  CC112X_READ	= 0x80,
  CC112X_WRITE	= 0x00,
  CC112X_BURST	= 0x40,
};

/* Page 9. */
enum cc112x_status_byte {
  CC112X_CHIP_RDYN_MASK     = 0x80,
  CC112X_STATE_IDLE	        = 0x00,
  CC112X_STATE_RX			= 0x10,
  CC112X_STATE_TX			= 0x20,
  CC112X_STATE_FSTXON       = 0x30,
  CC112X_STATE_CALIBRATE    = 0x40,
  CC112X_STATE_SETTLING	    = 0x50,
  CC112X_STATE_RXFIFO_ERROR	= 0x60,
  CC112X_STATE_TXFIFO_ERROR = 0x70,
  CC112X_STATE_MASK         = 0x70,
};

/* Page 12. */
enum cc112x_register {
	/* Registers */
	CC112X_IOCFG3            = 0x00,
	CC112X_IOCFG2            = 0x01,
	CC112X_IOCFG1            = 0x02,
	CC112X_IOCFG0            = 0x03,
	CC112X_SYNC3             = 0x04,
	CC112X_SYNC2             = 0x05,
	CC112X_SYNC1             = 0x06,
	CC112X_SYNC0             = 0x07,
	CC112X_SYNC_CFG1         = 0x08,
	CC112X_SYNC_CFG0         = 0x09,
	CC112X_DEVIATION_M       = 0x0A,
	CC112X_MODCFG_DEV_E      = 0x0B,
	CC112X_DCFILT_CFG        = 0x0C,
	CC112X_PREAMBLE_CFG1     = 0x0D,
	CC112X_PREAMBLE_CFG0     = 0x0E,
	CC112X_FREQ_IF_CFG       = 0x0F,
	CC112X_IQIC              = 0x10,
	CC112X_CHAN_BW           = 0x11,
	CC112X_MDMCFG1           = 0x12,
	CC112X_MDMCFG0           = 0x13,
	CC112X_DRATE2            = 0x14,
	CC112X_DRATE1            = 0x15,
	CC112X_DRATE0            = 0x16,
	CC112X_AGC_REF           = 0x17,
	CC112X_AGC_CS_THR        = 0x18,
	CC112X_AGC_GAIN_ADJUST   = 0x19,
	CC112X_AGC_CFG3          = 0x1A,
	CC112X_AGC_CFG2          = 0x1B,
	CC112X_AGC_CFG1          = 0x1C,
	CC112X_AGC_CFG0          = 0x1D,
	CC112X_FIFO_CFG          = 0x1E,
	CC112X_DEV_ADDR          = 0x1F,
	CC112X_SETTLING_CFG      = 0x20,
	CC112X_FS_CFG            = 0x21,
	CC112X_WOR_CFG1          = 0x22,
	CC112X_WOR_CFG0          = 0x23,
	CC112X_WOR_EVENT0_MSB    = 0x24,
	CC112X_WOR_EVENT0_LSB    = 0x25,
	CC112X_PKT_CFG2          = 0x26,
	CC112X_PKT_CFG1          = 0x27,
	CC112X_PKT_CFG0          = 0x28,
	CC112X_RFEND_CFG1        = 0x29,
	CC112X_RFEND_CFG0        = 0x2A,
	CC112X_PA_CFG2           = 0x2B,
	CC112X_PA_CFG1           = 0x2C,
	CC112X_PA_CFG0           = 0x2D,
	CC112X_PKT_LEN           = 0x2E,

	/* Extended Memory access */
	CC112X_EXTMEM     = 0x2F,

  	/* Extended config registers */
	CC112X_IF_MIX_CFG        = 0x2F00,
	CC112X_FREQOFF_CFG       = 0x2F01,
	CC112X_TOC_CFG           = 0x2F02,
	CC112X_MARC_SPARE        = 0x2F03,
	CC112X_ECG_CFG           = 0x2F04,
	CC112X_SOFT_TX_DATA_CFG  = 0x2F05,
	CC112X_EXT_CTRL          = 0x2F06,
	CC112X_RCCAL_FINE        = 0x2F07,
	CC112X_RCCAL_COARSE      = 0x2F08,
	CC112X_RCCAL_OFFSET      = 0x2F09,
	CC112X_FREQOFF1          = 0x2F0A,
	CC112X_FREQOFF0          = 0x2F0B,
	CC112X_FREQ2             = 0x2F0C,
	CC112X_FREQ1             = 0x2F0D,
	CC112X_FREQ0             = 0x2F0E,
	CC112X_IF_ADC2           = 0x2F0F,
	CC112X_IF_ADC1           = 0x2F10,
	CC112X_IF_ADC0           = 0x2F11,
	CC112X_FS_DIG1           = 0x2F12,
	CC112X_FS_DIG0           = 0x2F13,
	CC112X_FS_CAL3           = 0x2F14,
	CC112X_FS_CAL2           = 0x2F15,
	CC112X_FS_CAL1           = 0x2F16,
	CC112X_FS_CAL0           = 0x2F17,
	CC112X_FS_CHP            = 0x2F18,
	CC112X_FS_DIVTWO         = 0x2F19,
	CC112X_FS_DSM1           = 0x2F1A,
	CC112X_FS_DSM0           = 0x2F1B,
	CC112X_FS_DVC1           = 0x2F1C,
	CC112X_FS_DVC0           = 0x2F1D,
	CC112X_FS_LBI            = 0x2F1E,
	CC112X_FS_PFD            = 0x2F1F,
	CC112X_FS_PRE            = 0x2F20,
	CC112X_FS_REG_DIV_CML    = 0x2F21,
	CC112X_FS_SPARE          = 0x2F22,
	CC112X_FS_VCO4           = 0x2F23,
	CC112X_FS_VCO3           = 0x2F24,
	CC112X_FS_VCO2           = 0x2F25,
	CC112X_FS_VCO1           = 0x2F26,
	CC112X_FS_VCO0           = 0x2F27,
	CC112X_GBIAS6            = 0x2F28,
	CC112X_GBIAS5            = 0x2F29,
	CC112X_GBIAS4            = 0x2F2A,
	CC112X_GBIAS3            = 0x2F2B,
	CC112X_GBIAS2            = 0x2F2C,
	CC112X_GBIAS1            = 0x2F2D,
	CC112X_GBIAS0            = 0x2F2E,
	CC112X_IFAMP             = 0x2F2F,
	CC112X_LNA               = 0x2F30,
	CC112X_RXMIX             = 0x2F31,
	CC112X_XOSC5             = 0x2F32,
	CC112X_XOSC4             = 0x2F33,
	CC112X_XOSC3             = 0x2F34,
	CC112X_XOSC2             = 0x2F35,
	CC112X_XOSC1             = 0x2F36,
	CC112X_XOSC0             = 0x2F37,
	CC112X_ANALOG_SPARE      = 0x2F38,
	CC112X_PA_CFG3           = 0x2F39,
	/*
	 * 0x3A - 0x3E Not Used
	 * 0x3F - 0x40 Reserved
	 * 0x41 - 0x63 Not Used
	 */
	/* Status registers */
	CC112X_WOR_TIME1         = 0x2F64,
	CC112X_WOR_TIME0         = 0x2F65,
	CC112X_WOR_CAPTURE1      = 0x2F66,
	CC112X_WOR_CAPTURE0      = 0x2F67,
	CC112X_BIST              = 0x2F68,
	CC112X_DCFILTOFFSET_I1   = 0x2F69,
	CC112X_DCFILTOFFSET_I0   = 0x2F6A,
	CC112X_DCFILTOFFSET_Q1   = 0x2F6B,
	CC112X_DCFILTOFFSET_Q0   = 0x2F6C,
	CC112X_IQIE_I1           = 0x2F6D,
	CC112X_IQIE_I0           = 0x2F6E,
	CC112X_IQIE_Q1           = 0x2F6F,
	CC112X_IQIE_Q0           = 0x2F70,
	CC112X_RSSI1             = 0x2F71,
	CC112X_RSSI0             = 0x2F72,
	CC112X_MARCSTATE         = 0x2F73,
	CC112X_LQI_VAL           = 0x2F74,
	CC112X_PQT_SYNC_ERR      = 0x2F75,
	CC112X_DEM_STATUS        = 0x2F76,
	CC112X_FREQOFF_EST1      = 0x2F77,
	CC112X_FREQOFF_EST0      = 0x2F78,
	CC112X_AGC_GAIN3         = 0x2F79,
	CC112X_AGC_GAIN2         = 0x2F7A,
	CC112X_AGC_GAIN1         = 0x2F7B,
	CC112X_AGC_GAIN0         = 0x2F7C,
	CC112X_SOFT_RX_DATA_OUT  = 0x2F7D,
	CC112X_SOFT_TX_DATA_IN   = 0x2F7E,
	CC112X_ASK_SOFT_RX_DATA  = 0x2F7F,
	CC112X_RNDGEN            = 0x2F80,
	CC112X_MAGN2             = 0x2F81,
	CC112X_MAGN1             = 0x2F82,
	CC112X_MAGN0             = 0x2F83,
	CC112X_ANG1              = 0x2F84,
	CC112X_ANG0              = 0x2F85,
	CC112X_CHFILT_I2         = 0x2F86,
	CC112X_CHFILT_I1         = 0x2F87,
	CC112X_CHFILT_I0         = 0x2F88,
	CC112X_CHFILT_Q2         = 0x2F89,
	CC112X_CHFILT_Q1         = 0x2F8A,
	CC112X_CHFILT_Q0         = 0x2F8B,
	CC112X_GPIO_STATUS       = 0x2F8C,
	CC112X_FSCAL_CTRL        = 0x2F8D,
	CC112X_PHASE_ADJUST      = 0x2F8E,
	CC112X_PARTNUMBER        = 0x2F8F,
	CC112X_PARTVERSION       = 0x2F90,
	CC112X_SERIAL_STATUS     = 0x2F91,
	CC112X_RX_STATUS         = 0x2F92,
	CC112X_TX_STATUS         = 0x2F93,
	CC112X_MARC_STATUS1      = 0x2F94,
	CC112X_MARC_STATUS0      = 0x2F95,
	CC112X_PA_IFAMP_TEST     = 0x2F96,
	CC112X_FSRF_TEST         = 0x2F97,
	CC112X_PRE_TEST          = 0x2F98,
	CC112X_PRE_OVR           = 0x2F99,
	CC112X_ADC_TEST          = 0x2F9A,
	CC112X_DVC_TEST          = 0x2F9B,
	CC112X_ATEST             = 0x2F9C,
	CC112X_ATEST_LVDS        = 0x2F9D,
	CC112X_ATEST_MODE        = 0x2F9E,
	CC112X_XOSC_TEST1        = 0x2F9F,
	CC112X_XOSC_TEST0        = 0x2FA0,
	/* 0xA1 - 0xD1 : Not Used */

	/* FIFO registers */
	CC112X_RXFIRST           = 0x2FD2,
	CC112X_TXFIRST           = 0x2FD3,
	CC112X_RXLAST            = 0x2FD4,
	CC112X_TXLAST            = 0x2FD5,
	CC112X_NUM_TXBYTES       = 0x2FD6,
	CC112X_NUM_RXBYTES       = 0x2FD7,
	CC112X_FIFO_NUM_TXBYTES  = 0x2FD8,
	CC112X_FIFO_NUM_RXBYTES  = 0x2FD9,

	/* Strobes */
	CC112X_SRES		= 0x30,
	CC112X_SFSTXON	= 0x31,
	CC112X_SXOFF	= 0x32,
	CC112X_SCAL		= 0x33,
	CC112X_SRX		= 0x34,
	CC112X_STX		= 0x35,
	CC112X_SIDLE    = 0x36,
	CC112X_SWOR		= 0x38,
	CC112X_SPWD		= 0x39,
	CC112X_SFRX		= 0x3A,
	CC112X_SFTX		= 0x3B,
	CC112X_SWORRST	= 0x3C,
	CC112X_SNOP		= 0x3D,

	/* FIFO */
	CC112X_DIRECT_FIFO   = 0x3E,
	CC112X_TXFIFO		 = 0x3F,
	CC112X_RXFIFO		 = 0x3F,
};

/* Page 19. */
enum cc112x_gpiocfg_byte {
  CC112X_GPIO_RXFIFO_THR        = 0,
  CC112X_GPIO_RXFIFO_THR_PKT    = 1,
  CC112X_GPIO_TXFIFO_THR        = 2,
  CC112X_GPIO_TXFIFO_THR_PKT    = 3,
  CC112X_GPIO_RXFIFO_OVERFLOW   = 4,
  CC112X_GPIO_TXFIFO_UNDERFLOW  = 5,
  CC112X_GPIO_PKT_SYNC_RXTX     = 6,
  CC112X_GPIO_CRC_OK            = 7,
  CC112X_GPIO_SERIAL_CLK        = 8,
  CC112X_GPIO_SERIAL_RX         = 9,
  CC112X_GPIO_PQT_REACHED       = 11,
  CC112X_GPIO_PQT_VALID         = 12,
  CC112X_GPIO_RSSI_VALID        = 13,
  CC112X_GPIO_CCA               = 15, // GPIO number dependant
  CC112X_GPIO_PKT_CRC_OK        = 19,
  CC112X_GPIO_MARC_MCU_WAKEUP   = 20,
  CC112X_GPIO_SYNC_LOW0_HIGH1   = 21,
  CC112X_GPIO_HIGHZ             = 48,
  CC112X_GPIO_EXT_CLOCK         = 49,
  CC112X_GPIO_CHIP_RDYn         = 50,
  CC112X_GPIO_XOSC_STABLE       = 59,

  CC112X_GPIO_INV    = 0x40, /* Inverted Output enable */
  CC112X_GPIO_ATRAN  = 0x80, /* Analog Transfer enable */
};


#endif /* CC112X_CONST_H */
