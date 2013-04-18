/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538-rfcore
 * @{
 *
 * \file
 * Header with declarations of the RF Core XREGs.
 */
#ifndef RFCORE_XREG_H_
#define RFCORE_XREG_H_
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM register offsets
 * @{
 */
#define RFCORE_XREG_FRMFILT0    0x40088600 /**< Frame filtering control */
#define RFCORE_XREG_FRMFILT1    0x40088604 /**< Frame filtering control */
#define RFCORE_XREG_SRCMATCH    0x40088608 /**< Source address matching */
#define RFCORE_XREG_SRCSHORTEN0 0x4008860C /**< Short address matching */
#define RFCORE_XREG_SRCSHORTEN1 0x40088610 /**< Short address matching */
#define RFCORE_XREG_SRCSHORTEN2 0x40088614 /**< Short address matching */
#define RFCORE_XREG_SRCEXTEN0   0x40088618 /**< Extended address matching */
#define RFCORE_XREG_SRCEXTEN1   0x4008861C /**< Extended address matching */
#define RFCORE_XREG_SRCEXTEN2   0x40088620 /**< Extended address matching */
#define RFCORE_XREG_FRMCTRL0    0x40088624 /**< Frame handling */
#define RFCORE_XREG_FRMCTRL1    0x40088628 /**< Frame handling */
#define RFCORE_XREG_RXENABLE    0x4008862C /**< RX enabling */
#define RFCORE_XREG_RXMASKSET   0x40088630 /**< RX enabling */
#define RFCORE_XREG_RXMASKCLR   0x40088634 /**< RX disabling */
#define RFCORE_XREG_FREQTUNE    0x40088638 /**< Crystal oscillator freq tuning */
#define RFCORE_XREG_FREQCTRL    0x4008863C /**< Controls the RF frequency */
#define RFCORE_XREG_TXPOWER     0x40088640 /**< Controls the output power */
#define RFCORE_XREG_TXCTRL      0x40088644 /**< Controls the TX settings */
#define RFCORE_XREG_FSMSTAT0    0x40088648 /**< Radio status register */
#define RFCORE_XREG_FSMSTAT1    0x4008864C /**< Radio status register */
#define RFCORE_XREG_FIFOPCTRL   0x40088650 /**< FIFOP threshold */
#define RFCORE_XREG_FSMCTRL     0x40088654 /**< FSM options */
#define RFCORE_XREG_CCACTRL0    0x40088658 /**< CCA threshold */
#define RFCORE_XREG_CCACTRL1    0x4008865C /**< Other CCA Options */
#define RFCORE_XREG_RSSI        0x40088660 /**< RSSI status register */
#define RFCORE_XREG_RSSISTAT    0x40088664 /**< RSSI valid status register */
#define RFCORE_XREG_RXFIRST     0x40088668 /**< First byte in RX FIFO */
#define RFCORE_XREG_RXFIFOCNT   0x4008866C /**< Number of bytes in RX FIFO */
#define RFCORE_XREG_TXFIFOCNT   0x40088670 /**< Number of bytes in TX FIFO */
#define RFCORE_XREG_RXFIRST_PTR 0x40088674 /**< RX FIFO pointer */
#define RFCORE_XREG_RXLAST_PTR  0x40088678 /**< RX FIFO pointer */
#define RFCORE_XREG_RXP1_PTR    0x4008867C /**< RX FIFO pointer */
#define RFCORE_XREG_RXP2_PTR    0x40088680 /**< RX FIFO pointer */
#define RFCORE_XREG_TXFIRST_PTR 0x40088684 /**< TX FIFO pointer */
#define RFCORE_XREG_TXLAST_PTR  0x40088688 /**< TX FIFO pointer */
#define RFCORE_XREG_RFIRQM0     0x4008868C /**< RF interrupt masks */
#define RFCORE_XREG_RFIRQM1     0x40088690 /**< RF interrupt masks */
#define RFCORE_XREG_RFERRM      0x40088694 /**< RF error interrupt mask */
#define RFCORE_XREG_D18_SPARE_OPAMPMC 0x40088698 /**< Operational amp mode ctrl */
#define RFCORE_XREG_RFRND       0x4008869C /**< Random data */
#define RFCORE_XREG_MDMCTRL0    0x400886A0 /**< Controls modem */
#define RFCORE_XREG_MDMCTRL1    0x400886A4 /**< Controls modem */
#define RFCORE_XREG_FREQEST     0x400886A8 /**< Estimated RF frequency offset */
#define RFCORE_XREG_RXCTRL      0x400886AC /**< Tune receive section */
#define RFCORE_XREG_FSCTRL      0x400886B0 /**< Tune frequency synthesizer */
#define RFCORE_XREG_FSCAL1      0x400886B8 /**< Tune frequency calibration */
#define RFCORE_XREG_FSCAL2      0x400886BC /**< Tune frequency calibration */
#define RFCORE_XREG_FSCAL3      0x400886C0 /**< Tune frequency calibration */
#define RFCORE_XREG_AGCCTRL0    0x400886C4 /**< AGC dynamic range control */
#define RFCORE_XREG_AGCCTRL1    0x400886C8 /**< AGC reference level */
#define RFCORE_XREG_AGCCTRL2    0x400886CC /**< AGC gain override */
#define RFCORE_XREG_AGCCTRL3    0x400886D0 /**< AGC control */
#define RFCORE_XREG_ADCTEST0    0x400886D4 /**< ADC tuning */
#define RFCORE_XREG_ADCTEST1    0x400886D8 /**< ADC tuning */
#define RFCORE_XREG_ADCTEST2    0x400886DC /**< ADC tuning */
#define RFCORE_XREG_MDMTEST0    0x400886E0 /**< Test register for modem */
#define RFCORE_XREG_MDMTEST1    0x400886E4 /**< Test Register for Modem */
#define RFCORE_XREG_DACTEST0    0x400886E8 /**< DAC override value */
#define RFCORE_XREG_DACTEST1    0x400886EC /**< DAC override value */
#define RFCORE_XREG_DACTEST2    0x400886F0 /**< DAC test setting */
#define RFCORE_XREG_ATEST       0x400886F4 /**< Analog test control */
#define RFCORE_XREG_PTEST0      0x400886F8 /**< Override power-down register */
#define RFCORE_XREG_PTEST1      0x400886FC /**< Override power-down register */
#define RFCORE_XREG_CSPPROG0    0x40088700 /**< CSP program */
#define RFCORE_XREG_CSPPROG1    0x40088704 /**< CSP program */
#define RFCORE_XREG_CSPPROG2    0x40088708 /**< CSP program */
#define RFCORE_XREG_CSPPROG3    0x4008870C /**< CSP program */
#define RFCORE_XREG_CSPPROG4    0x40088710 /**< CSP program */
#define RFCORE_XREG_CSPPROG5    0x40088714 /**< CSP program */
#define RFCORE_XREG_CSPPROG6    0x40088718 /**< CSP program */
#define RFCORE_XREG_CSPPROG7    0x4008871C /**< CSP program */
#define RFCORE_XREG_CSPPROG8    0x40088720 /**< CSP program */
#define RFCORE_XREG_CSPPROG9    0x40088724 /**< CSP program */
#define RFCORE_XREG_CSPPROG10   0x40088728 /**< CSP program */
#define RFCORE_XREG_CSPPROG11   0x4008872C /**< CSP program */
#define RFCORE_XREG_CSPPROG12   0x40088730 /**< CSP program */
#define RFCORE_XREG_CSPPROG13   0x40088734 /**< CSP program */
#define RFCORE_XREG_CSPPROG14   0x40088738 /**< CSP program */
#define RFCORE_XREG_CSPPROG15   0x4008873C /**< CSP program */
#define RFCORE_XREG_CSPPROG16   0x40088740 /**< CSP program */
#define RFCORE_XREG_CSPPROG17   0x40088744 /**< CSP program */
#define RFCORE_XREG_CSPPROG18   0x40088748 /**< CSP program */
#define RFCORE_XREG_CSPPROG19   0x4008874C /**< CSP program */
#define RFCORE_XREG_CSPPROG20   0x40088750 /**< CSP program */
#define RFCORE_XREG_CSPPROG21   0x40088754 /**< CSP program */
#define RFCORE_XREG_CSPPROG22   0x40088758 /**< CSP program */
#define RFCORE_XREG_CSPPROG23   0x4008875C /**< CSP program */
#define RFCORE_XREG_CSPCTRL     0x40088780 /**< CSP control bit */
#define RFCORE_XREG_CSPSTAT     0x40088784 /**< CSP status register */
#define RFCORE_XREG_CSPX        0x40088788 /**< CSP X data register */
#define RFCORE_XREG_CSPY        0x4008878C /**< CSP Y data register */
#define RFCORE_XREG_CSPZ        0x40088790 /**< CSP Z data register */
#define RFCORE_XREG_CSPT        0x40088794 /**< CSP T data register */
#define RFCORE_XREG_RFC_DUTY_CYCLE 0x400887A0 /**< RX duty cycle control */
#define RFCORE_XREG_RFC_OBS_CTRL0  0x400887AC /**< RF observation mux control */
#define RFCORE_XREG_RFC_OBS_CTRL1  0x400887B0 /**< RF observation mux control */
#define RFCORE_XREG_RFC_OBS_CTRL2  0x400887B4 /**< RF observation mux control */
#define RFCORE_XREG_TXFILTCFG   0x400887E8 /**< TX filter configuration */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FRMFILT0 register offsets
 * @{
 */
#define RFCORE_XREG_FRMFILT0_MAX_FRAME_VERSION 0x0000000C /**< Frame version filtering */
#define RFCORE_XREG_FRMFILT0_PAN_COORDINATOR 0x00000002 /**< PAN coordinator */
#define RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN 0x00000001 /**< Enables frame filtering */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FRMFILT1 register offsets
 * @{
 */
#define RFCORE_XREG_FRMFILT1_ACCEPT_FT_3_MAC_CMD 0x00000040 /**< MAC command frame filt */
#define RFCORE_XREG_FRMFILT1_ACCEPT_FT_2_ACK 0x00000020 /**< ack frame filt */
#define RFCORE_XREG_FRMFILT1_ACCEPT_FT_1_DATA 0x00000010 /**< data frame filt */
#define RFCORE_XREG_FRMFILT1_ACCEPT_FT_0_BEACON 0x00000008 /**< beacon frame filt */
#define RFCORE_XREG_FRMFILT1_MODIFY_FT_FILTER 0x00000006 /**< Frame type modify */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCMATCH register bit masks
 * @{
 */
#define RFCORE_XREG_SRCMATCH_PEND_DATAREQ_ONLY 0x00000004 /**< AUTOPEND function */
#define RFCORE_XREG_SRCMATCH_AUTOPEND 0x00000002 /**< Automatic acknowledgment */
#define RFCORE_XREG_SRCMATCH_SRC_MATCH_EN 0x00000001 /**< Source address matching enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCSHORTEN0 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCSHORTEN0_SHORT_ADDR_EN 0x000000FF /**< SHORT_ADDR_EN[7:0] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCSHORTEN1 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCSHORTEN1_SHORT_ADDR_EN 0x000000FF /**< SHORT_ADDR_EN[15:8] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCSHORTEN2 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCSHORTEN2_SHORT_ADDR_EN 0x000000FF /**< SHORT_ADDR_EN[23:16] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCEXTEN0 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCEXTEN0_EXT_ADDR_EN 0x000000FF /**< EXT_ADDR_EN[7:0] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCEXTEN1 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCEXTEN1_EXT_ADDR_EN 0x000000FF /**< EXT_ADDR_EN[15:8] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_SRCEXTEN2 register bit masks
 * @{
 */
#define RFCORE_XREG_SRCEXTEN2_EXT_ADDR_EN 0x000000FF /**< EXT_ADDR_EN[23:16] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FRMCTRL0 register bit masks
 * @{
 */
#define RFCORE_XREG_FRMCTRL0_APPEND_DATA_MODE 0x00000080 /**< Append data mode */
#define RFCORE_XREG_FRMCTRL0_AUTOCRC 0x00000040 /**< Auto CRC generation / checking */
#define RFCORE_XREG_FRMCTRL0_AUTOACK 0x00000020 /**< Transmit ACK frame enable */
#define RFCORE_XREG_FRMCTRL0_ENERGY_SCAN 0x00000010 /**< RSSI register content control */
#define RFCORE_XREG_FRMCTRL0_RX_MODE 0x0000000C /**< Set RX modes */
#define RFCORE_XREG_FRMCTRL0_TX_MODE 0x00000003 /**< Set test modes for TX */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FRMCTRL1 register bit masks
 * @{
 */
#define RFCORE_XREG_FRMCTRL1_PENDING_OR 0x00000004 /**< Pending data bit control */
#define RFCORE_XREG_FRMCTRL1_IGNORE_TX_UNDERF 0x00000002 /**< TX underflow ignore */
#define RFCORE_XREG_FRMCTRL1_SET_RXENMASK_ON_TX 0x00000001 /**< RXENABLE control */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXENABLE register bit masks
 * @{
 */
#define RFCORE_XREG_RXENABLE_RXENMASK 0x000000FF /**< Enables the receiver. */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXMASKSET register bit masks
 * @{
 */
#define RFCORE_XREG_RXMASKSET_RXENMASKSET 0x000000FF /**< Write to RXENMASK (OR) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXMASKCLR register bit masks
 * @{
 */
#define RFCORE_XREG_RXMASKCLR_RXENMASKCLR 0x000000FF /**< RXENMASK clear bits */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FREQTUNE register bit masks
 * @{
 */
#define RFCORE_XREG_FREQTUNE_XOSC32M_TUNE 0x0000000F /**< Tune crystal oscillator */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FREQCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_FREQCTRL_FREQ 0x0000007F /**< Frequency control word */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_TXPOWER register bit masks
 * @{
 */
#define RFCORE_XREG_TXPOWER_PA_POWER 0x000000F0 /**< PA power control */
#define RFCORE_XREG_TXPOWER_PA_BIAS 0x0000000F /**< PA bias control */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_TXCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_TXCTRL_DAC_CURR 0x00000070 /**< Change the current in the DAC. */
#define RFCORE_XREG_TXCTRL_DAC_DC 0x0000000C /**< Adjusts the DC level to the TX mixer */
#define RFCORE_XREG_TXCTRL_TXMIX_CURRENT 0x00000003 /**< TX mixer core current */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSMSTAT0 register bit masks
 * @{
 */
#define RFCORE_XREG_FSMSTAT0_CAL_DONE 0x00000080 /**< Calib has been performed */
#define RFCORE_XREG_FSMSTAT0_CAL_RUNNING 0x00000040 /**< Calib status */
#define RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE 0x0000003F /**< FIFO and FFCTRL status */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSMSTAT1 register bit masks
 * @{
 */
#define RFCORE_XREG_FSMSTAT1_FIFO 0x00000080 /**< FIFO status */
#define RFCORE_XREG_FSMSTAT1_FIFOP 0x00000040 /**< FIFOP status */
#define RFCORE_XREG_FSMSTAT1_SFD 0x00000020 /**< SFD was sent/received */
#define RFCORE_XREG_FSMSTAT1_CCA 0x00000010 /**< Clear channel assessment */
#define RFCORE_XREG_FSMSTAT1_SAMPLED_CCA 0x00000008 /**< CCA sample value */
#define RFCORE_XREG_FSMSTAT1_LOCK_STATUS 0x00000004 /**< PLL lock status */
#define RFCORE_XREG_FSMSTAT1_TX_ACTIVE 0x00000002 /**< Status signal - TX states */
#define RFCORE_XREG_FSMSTAT1_RX_ACTIVE 0x00000001 /**< Status signal - RX states */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FIFOPCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_FIFOPCTRL_FIFOP_THR 0x0000007F /**< FIFOP signal threshold */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSMCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_FSMCTRL_SLOTTED_ACK 0x00000002 /**< ACK frame TX timing */
#define RFCORE_XREG_FSMCTRL_RX2RX_TIME_OFF 0x00000001 /**< 12-sym timeout after RX */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CCACTRL0 register bit masks
 * @{
 */
#define RFCORE_XREG_CCACTRL0_CCA_THR 0x000000FF /**< Clear-channel-assessment */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CCACTRL1 register bit masks
 * @{
 */
#define RFCORE_XREG_CCACTRL1_CCA_MODE 0x00000018 /**< CCA mode */
#define RFCORE_XREG_CCACTRL1_CCA_HYST 0x00000007 /**< CCA hysteresis */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RSSI register bit masks
 * @{
 */
#define RFCORE_XREG_RSSI_RSSI_VAL 0x000000FF /**< RSSI estimate */
#define RFCORE_XREG_RSSI_RSSI_VAL_S 0
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RSSISTAT register bit masks
 * @{
 */
#define RFCORE_XREG_RSSISTAT_RSSI_VALID 0x00000001 /**< RSSI value is valid */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXFIRST register bit masks
 * @{
 */
#define RFCORE_XREG_RXFIRST_DATA 0x000000FF /**< First byte of the RX FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXFIFOCNT register bit masks
 * @{
 */
#define RFCORE_XREG_RXFIFOCNT_RXFIFOCNT 0x000000FF /**< Number of bytes in the RX FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_TXFIFOCNT register bit masks
 * @{
 */
#define RFCORE_XREG_TXFIFOCNT_TXFIFOCNT 0x000000FF /**< Number of bytes in the TX FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RX FIFO pointers
 * @{
 */
#define RFCORE_XREG_RXFIRST_PTR_RXFIRST_PTR 0x000000FF /**< Byte 1 */
#define RFCORE_XREG_RXLAST_PTR_RXLAST_PTR 0x000000FF /**< Last byte + 1 */
#define RFCORE_XREG_RXP1_PTR_RXP1_PTR 0x000000FF /**< Frame 1, byte 1 */
#define RFCORE_XREG_RXP2_PTR_RXP2_PTR 0x000000FF /**< Last frame, byte 1 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name TX FIFO pointers
 * @{
 */
#define RFCORE_XREG_TXFIRST_PTR_TXFIRST_PTR 0x000000FF /**< Next byte to be TXd */
#define RFCORE_XREG_TXLAST_PTR_TXLAST_PTR 0x000000FF /**< Last byte + 1 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFIRQM0 register bit masks
 * @{
 */
#define RFCORE_XREG_RFIRQM0_RFIRQM          0x000000FF /**< Interrupt source bit mask */
#define RFCORE_XREG_RFIRQM0_RXMASKZERO      0x00000080 /**< RXENABLE gone all-zero */
#define RFCORE_XREG_RFIRQM0_RXPKTDONE       0x00000040 /**< Complete frame RX */
#define RFCORE_XREG_RFIRQM0_FRAME_ACCEPTED  0x00000020 /**< Frame has passed frame filter */
#define RFCORE_XREG_RFIRQM0_SRC_MATCH_FOUND 0x00000010 /**< Source match is found */
#define RFCORE_XREG_RFIRQM0_SRC_MATCH_DONE  0x00000008 /**< Source matching is complete */
#define RFCORE_XREG_RFIRQM0_FIFOP           0x00000004 /**< RX FIFO exceeded threshold */
#define RFCORE_XREG_RFIRQM0_SFD             0x00000002 /**< SFD TX or RX */
#define RFCORE_XREG_RFIRQM0_ACT_UNUSED      0x00000001 /**< Reserved */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFIRQM1 register bit masks
 * @{
 */
#define RFCORE_XREG_RFIRQM1_RFIRQM     0x0000003F /**< Interrupt source bit mask */
#define RFCORE_XREG_RFIRQM1_CSP_WAIT   0x00000020 /**< CSP Execution continued */
#define RFCORE_XREG_RFIRQM1_CSP_STOP   0x00000010 /**< CSP has stopped program */
#define RFCORE_XREG_RFIRQM1_CSP_MANINT 0x00000008 /**< CSP Manual interrupt */
#define RFCORE_XREG_RFIRQM1_RFIDLE     0x00000004 /**< IDLE state entered */
#define RFCORE_XREG_RFIRQM1_TXDONE     0x00000002 /**< Complete frame TX finished */
#define RFCORE_XREG_RFIRQM1_TXACKDONE  0x00000001 /**< ACK frame TX finished */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFERRM register bit masks
 * @{
 */
#define RFCORE_XREG_RFERRM_RFERRM    0x0000007F /**< RF error interrupt mask */
#define RFCORE_XREG_RFERRM_STROBEERR 0x00000040 /**< Strobe error */
#define RFCORE_XREG_RFERRM_TXUNDERF  0x00000020 /**< TX FIFO underflowed */
#define RFCORE_XREG_RFERRM_TXOVERF   0x00000010 /**< TX FIFO overflowed */
#define RFCORE_XREG_RFERRM_RXUNDERF  0x00000008 /**< RX FIFO underflowed */
#define RFCORE_XREG_RFERRM_RXOVERF   0x00000004 /**< RX FIFO overflowed */
#define RFCORE_XREG_RFERRM_RXABO     0x00000002 /**< Frame RX was aborted */
#define RFCORE_XREG_RFERRM_NLOCK     0x00000001 /**< Frequency synthesizer lock error */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_D18_SPARE_OPAMPMC register bit masks
 * @{
 */
#define RFCORE_XREG_D18_SPARE_OPAMPMC_MODE 0x00000003 /**< Operational amplifier mode */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFRND register bit masks
 * @{
 */
#define RFCORE_XREG_RFRND_QRND  0x00000002 /**< Random bit from the Q channel */
#define RFCORE_XREG_RFRND_IRND  0x00000001 /**< Random bit from the I channel */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_MDMCTRL0 register bit masks
 * @{
 */
#define RFCORE_XREG_MDMCTRL0_DEM_NUM_ZEROS 0x000000C0 /**< Num of zero symbols before sync word */
#define RFCORE_XREG_MDMCTRL0_DEMOD_AVG_MODE 0x00000020 /**< Frequency offset averaging filter */
#define RFCORE_XREG_MDMCTRL0_PREAMBLE_LENGTH 0x0000001E /**< Number of preamble bytes */
#define RFCORE_XREG_MDMCTRL0_TX_FILTER 0x00000001 /**< TX filter type */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_MDMCTRL1 register bit masks
 * @{
 */
#define RFCORE_XREG_MDMCTRL1_CORR_THR_SFD 0x00000020 /**< SFD detection requirements */
#define RFCORE_XREG_MDMCTRL1_CORR_THR 0x0000001F /**< Demodulator correlator threshold */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FREQEST register bit masks
 * @{
 */
#define RFCORE_XREG_FREQEST_FREQEST 0x000000FF
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RXCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_RXCTRL_GBIAS_LNA2_REF 0x00000030 /**<  LNA2/mixer PTAT current output */
#define RFCORE_XREG_RXCTRL_GBIAS_LNA_REF 0x0000000C /**< LNA PTAT current output */
#define RFCORE_XREG_RXCTRL_MIX_CURRENT 0x00000003 /**< Control of the output current */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_FSCTRL_PRE_CURRENT 0x000000C0 /**< Prescaler current setting */
#define RFCORE_XREG_FSCTRL_LODIV_BUF_CURRENT_TX 0x00000030 /**< Adjusts current in mixer and PA adjust */
#define RFCORE_XREG_FSCTRL_LODIV_BUF_CURRENT_RX 0x0000000C /**< Adjusts current in mixer and PA adjust */
#define RFCORE_XREG_FSCTRL_LODIV_CURRENT 0x00000003 /**< Adjusts divider currents */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSCAL1 register bit masks
 * @{
 */
#define RFCORE_XREG_FSCAL1_VCO_CURR_CAL_OE 0x00000080 /**< Override current calibration */
#define RFCORE_XREG_FSCAL1_VCO_CURR_CAL 0x0000007C /**< Calibration result */
#define RFCORE_XREG_FSCAL1_VCO_CURR 0x00000003 /**< Defines current in VCO core */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSCAL2 register bit masks
 * @{
 */
#define RFCORE_XREG_FSCAL2_VCO_CAPARR_OE 0x00000040 /**< Override the calibration result */
#define RFCORE_XREG_FSCAL2_VCO_CAPARR 0x0000003F /**< VCO capacitor array setting */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_FSCAL3 register bit masks
 * @{
 */
#define RFCORE_XREG_FSCAL3_VCO_DAC_EN_OV 0x00000040 /**< VCO DAC Enable */
#define RFCORE_XREG_FSCAL3_VCO_VC_DAC 0x0000003C /**< Varactor control voltage Bit vector */
#define RFCORE_XREG_FSCAL3_VCO_CAPARR_CAL_CTRL 0x00000003 /**< Calibration accuracy setting */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_AGCCTRL0 register bit masks
 * @{
 */
#define RFCORE_XREG_AGCCTRL0_AGC_DR_XTND_EN 0x00000040 /**< AAF attenuation adjustment */
#define RFCORE_XREG_AGCCTRL0_AGC_DR_XTND_THR 0x0000003F /**< Enable extra attenuation in front end */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_AGCCTRL1 register bit masks
 * @{
 */
#define RFCORE_XREG_AGCCTRL1_AGC_REF 0x0000003F /**< Target value for the AGC control loop */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_AGCCTRL2 register bit masks
 * @{
 */
#define RFCORE_XREG_AGCCTRL2_LNA1_CURRENT 0x000000C0 /**< Overrride value for LNA 1 */
#define RFCORE_XREG_AGCCTRL2_LNA2_CURRENT 0x00000038 /**< Overrride value for LNA 2 */
#define RFCORE_XREG_AGCCTRL2_LNA3_CURRENT 0x00000006 /**< Overrride value for LNA 3 */
#define RFCORE_XREG_AGCCTRL2_LNA_CURRENT_OE 0x00000001 /**< AGC LNA override */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_AGCCTRL3 register bit masks
 * @{
 */
#define RFCORE_XREG_AGCCTRL3_AGC_SETTLE_WAIT 0x00000060 /**< AGC analog gain wait */
#define RFCORE_XREG_AGCCTRL3_AGC_WIN_SIZE 0x00000018 /**< AGC accumulate-and-dump window size */
#define RFCORE_XREG_AGCCTRL3_AAF_RP 0x00000006 /**< AGC to AAF control signal override */
#define RFCORE_XREG_AGCCTRL3_AAF_RP_OE 0x00000001 /**< AAF control signal override */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_ADCTEST0 register bit masks
 * @{
 */
#define RFCORE_XREG_ADCTEST0_ADC_VREF_ADJ 0x000000C0 /**< Quantizer threshold control */
#define RFCORE_XREG_ADCTEST0_ADC_QUANT_ADJ 0x00000030 /**< Quantizer threshold control */
#define RFCORE_XREG_ADCTEST0_ADC_GM_ADJ 0x0000000E /**< Gm-control for test and debug */
#define RFCORE_XREG_ADCTEST0_ADC_DAC2_EN 0x00000001 /**< Enables DAC2 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_ADCTEST1 register bit masks
 * @{
 */
#define RFCORE_XREG_ADCTEST1_ADC_TEST_CTRL 0x000000F0 /**< ADC test mode selector */
#define RFCORE_XREG_ADCTEST1_ADC_C2_ADJ 0x0000000C /**< ADC capacitor value adjust */
#define RFCORE_XREG_ADCTEST1_ADC_C3_ADJ 0x00000003 /**< ADC capacitor value adjust */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_ADCTEST2 register bit masks
 * @{
 */
#define RFCORE_XREG_ADCTEST2_ADC_TEST_MODE 0x00000060 /**< ADC data output test mode */
#define RFCORE_XREG_ADCTEST2_AAF_RS 0x00000018 /**< AAF series resistance control */
#define RFCORE_XREG_ADCTEST2_ADC_FF_ADJ 0x00000006 /**< Adjust feed forward */
#define RFCORE_XREG_ADCTEST2_ADC_DAC_ROT 0x00000001 /**< Control of DAC DWA scheme */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_MDMTEST0 register bit masks
 * @{
 */
#define RFCORE_XREG_MDMTEST0_TX_TONE 0x000000F0 /**< Baseband tone TX enable */
#define RFCORE_XREG_MDMTEST0_DC_WIN_SIZE 0x0000000C /**< Controls the numbers of samples */
#define RFCORE_XREG_MDMTEST0_DC_BLOCK_MODE 0x00000003 /**< Mode of operation select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_MDMTEST1 register bit masks
 * @{
 */
#define RFCORE_XREG_MDMTEST1_USEMIRROR_IF 0x00000020 /**< IF frequency select */
#define RFCORE_XREG_MDMTEST1_MOD_IF 0x00000010 /**< Modulation select */
#define RFCORE_XREG_MDMTEST1_RAMP_AMP 0x00000008 /**< Ramping of DAC output enable */
#define RFCORE_XREG_MDMTEST1_RFC_SNIFF_EN 0x00000004 /**< Packet sniffer module enable */
#define RFCORE_XREG_MDMTEST1_MODULATION_MODE 0x00000002 /**< RF-modulation mode */
#define RFCORE_XREG_MDMTEST1_LOOPBACK_EN 0x00000001 /**< Modulated data loopback enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_DACTEST0 register bit masks
 * @{
 */
#define RFCORE_XREG_DACTEST0_DAC_Q 0x400886FF /**< Q-branch DAC override value */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_DACTEST1 register bit masks
 * @{
 */
#define RFCORE_XREG_DACTEST1_DAC_I 0x400886FF /**< I-branch DAC override value */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_DACTEST2 register bit masks
 * @{
 */
#define RFCORE_XREG_DACTEST2_DAC_DEM_EN 0x00000020 /**< Dynamic element matching enable */
#define RFCORE_XREG_DACTEST2_DAC_CASC_CTRL 0x00000018 /**< Adjustment of output stage */
#define RFCORE_XREG_DACTEST2_DAC_SRC 0x00000007 /**< TX DAC data src select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_ATEST register bit masks
 * @{
 */
#define RFCORE_XREG_ATEST_ATEST_CTRL 0x0000003F /**< Controls the analog test mode */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_PTEST0 register bit masks
 * @{
 */
#define RFCORE_XREG_PTEST0_PRE_PD 0x00000080 /**< Prescaler power-down signal */
#define RFCORE_XREG_PTEST0_CHP_PD 0x00000040 /**< Charge pump power-down signal */
#define RFCORE_XREG_PTEST0_ADC_PD 0x00000020 /**< ADC power-down signal When */
#define RFCORE_XREG_PTEST0_DAC_PD 0x00000010 /**< DAC power-down signal When */
#define RFCORE_XREG_PTEST0_LNA_PD 0x0000000C /**< Low-noise amplifier power-down */
#define RFCORE_XREG_PTEST0_TXMIX_PD 0x00000002 /**< Transmit mixer power-down */
#define RFCORE_XREG_PTEST0_AAF_PD 0x00000001 /**< Antialiasing filter power-down */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_PTEST1 register bit masks
 * @{
 */
#define RFCORE_XREG_PTEST1_PD_OVERRIDE 0x00000008 /**< Override module enabling and disabling */
#define RFCORE_XREG_PTEST1_PA_PD 0x00000004 /**< Power amplifier power-down signal */
#define RFCORE_XREG_PTEST1_VCO_PD 0x00000002 /**< VCO power-down signal */
#define RFCORE_XREG_PTEST1_LODIV_PD 0x00000001 /**< LO power-down signal */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPPROG[0:24] register bit masks
 * @{
 */
#define RFCORE_XREG_CSPPROG_CSP_INSTR 0x000000FF /**< Byte N of the CSP program */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPCTRL register bit masks
 * @{
 */
#define RFCORE_XREG_CSPCTRL_MCU_CTRL 0x00000001 /**< CSP MCU control input */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPSTAT register bit masks
 * @{
 */
#define RFCORE_XREG_CSPSTAT_CSP_RUNNING 0x00000020 /**< CSP Running / Idle */
#define RFCORE_XREG_CSPSTAT_CSP_PC 0x0000001F /**< CSP program counter */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPX register bit masks
 * @{
 */
#define RFCORE_XREG_CSPX_CSPX 0x000000FF /**< CSP X data */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPY register bit masks
 * @{
 */
#define RFCORE_XREG_CSPY_CSPY 0x000000FF /**< CSP Y data */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPZ register bit masks
 * @{
 */
#define RFCORE_XREG_CSPZ_CSPZ 0x000000FF /**< CSP Z data */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_CSPT register bit masks
 * @{
 */
#define RFCORE_XREG_CSPT_CSPT 0x000000FF /**< CSP T data */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFC_DUTY_CYCLE register bit masks
 * @{
 */
#define RFCORE_XREG_RFC_DUTY_CYCLE_SWD_EN 0x00000040 /**< Wire debug mode */
#define RFCORE_XREG_RFC_DUTY_CYCLE_DTC_DCCAL_MODE 0x00000030 /**< Periodic DC-recalibration mode */
#define RFCORE_XREG_RFC_DUTY_CYCLE_DUTYCYCLE_CNF 0x0000000E /**< Defines duty cycling */
#define RFCORE_XREG_RFC_DUTY_CYCLE_DUTYCYCLE_EN 0x00000001 /**< Duty cycling mode enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_RFC_OBS_CTRL[0:2] register bit masks
 * @{
 */
#define RFCORE_XREG_RFC_OBS_CTRL0_RFC_OBS_POL0 0x00000040 /**< RFC_OBS_MUX0 XOR bit */
#define RFCORE_XREG_RFC_OBS_CTRL0_RFC_OBS_MUX0 0x0000003F /**< RF Core MUX out control */
#define RFCORE_XREG_RFC_OBS_CTRL1_RFC_OBS_POL1 0x00000040 /**< RFC_OBS_MUX0 XOR bit */
#define RFCORE_XREG_RFC_OBS_CTRL1_RFC_OBS_MUX1 0x0000003F /**< RF Core MUX out control */
#define RFCORE_XREG_RFC_OBS_CTRL2_RFC_OBS_POL2 0x00000040 /**< RFC_OBS_MUX0 XOR bit */
#define RFCORE_XREG_RFC_OBS_CTRL2_RFC_OBS_MUX2 0x0000003F /**< RF Core MUX out control */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_XREG_TXFILTCFG register bit masks
 * @{
 */
#define RFCORE_XREG_TXFILTCFG_FC 0x0000000F /**< Drives signal rfr_txfilt_fc */
/** @} */

#endif /* RFCORE_XREG_H_ */
/** @} */
