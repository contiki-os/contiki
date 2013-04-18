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
 * Header with declarations of the RF Core SFR registers. Includes
 * declarations of MAC timer registers.
 */
#ifndef RFCORE_SFR_H_
#define RFCORE_SFR_H_
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR register offsets (MAC Timer)
 * @{
 */
#define RFCORE_SFR_MTCSPCFG     0x40088800 /**< MAC Timer event configuration */
#define RFCORE_SFR_MTCTRL       0x40088804 /**< MAC Timer control register */
#define RFCORE_SFR_MTIRQM       0x40088808 /**< MAC Timer interrupt mask */
#define RFCORE_SFR_MTIRQF       0x4008880C /**< MAC Timer interrupt flags */
#define RFCORE_SFR_MTMSEL       0x40088810 /**< MAC Timer multiplex select */
#define RFCORE_SFR_MTM0         0x40088814 /**< MAC Timer MUX register 0 */
#define RFCORE_SFR_MTM1         0x40088818 /**< MAC Timer MUX register 1 */
#define RFCORE_SFR_MTMOVF2      0x4008881C /**< MAC Timer MUX overflow 2 */
#define RFCORE_SFR_MTMOVF1      0x40088820 /**< MAC Timer MUX overflow 1 */
#define RFCORE_SFR_MTMOVF0      0x40088824 /**< MAC Timer MUX overflow 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR register offsets (RF)
 * @{
 */
#define RFCORE_SFR_RFDATA       0x40088828 /**< TX/RX FIFO data */
#define RFCORE_SFR_RFERRF       0x4008882C /**< RF error interrupt flags */
#define RFCORE_SFR_RFIRQF1      0x40088830 /**< RF interrupt flags */
#define RFCORE_SFR_RFIRQF0      0x40088834 /**< RF interrupt flags */
#define RFCORE_SFR_RFST         0x40088838 /**< RF CSMA-CA/strobe processor */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTCSPCFG register bit masks
 * @{
 */
#define RFCORE_SFR_MTCSPCFG_MACTIMER_EVENMT_CFG 0x00000070 /**< MT_EVENT2 pulse event trigger */
#define RFCORE_SFR_MTCSPCFG_MACTIMER_EVENT1_CFG 0x00000007 /**< MT_EVENT1 pulse event trigger */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTCTRL register bit masks
 * @{
 */
#define RFCORE_SFR_MTCTRL_LATCH_MODE 0x00000008 /**< OVF counter latch mode */
#define RFCORE_SFR_MTCTRL_STATE      0x00000004 /**< State of MAC Timer */
#define RFCORE_SFR_MTCTRL_SYNC       0x00000002 /**< Timer start/stop timing */
#define RFCORE_SFR_MTCTRL_RUN        0x00000001 /**< Timer start/stop */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTIRQM register bit masks
 * @{
 */
#define RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE2M 0x00000020 /**< MACTIMER_OVF_COMPARE2 mask */
#define RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M 0x00000010 /**< MACTIMER_OVF_COMPARE1 mask */
#define RFCORE_SFR_MTIRQM_MACTIMER_OVF_PERM      0x00000008 /**< MACTIMER_OVF_PER mask */
#define RFCORE_SFR_MTIRQM_MACTIMER_COMPARE2M     0x00000004 /**< MACTIMER_COMPARE2 mask */
#define RFCORE_SFR_MTIRQM_MACTIMER_COMPARE1M     0x00000002 /**< MACTIMER_COMPARE1 mask */
#define RFCORE_SFR_MTIRQM_MACTIMER_PERM          0x00000001 /**< MACTIMER_PER mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTIRQF register bit masks
 * @{
 */
#define RFCORE_SFR_MTIRQF_MACTIMER_OVF_COMPARE2F 0x00000020 /**< MACTIMER_OVF_COMPARE2 flag */
#define RFCORE_SFR_MTIRQF_MACTIMER_OVF_COMPARE1F 0x00000010 /**< MACTIMER_OVF_COMPARE1 flag */
#define RFCORE_SFR_MTIRQF_MACTIMER_OVF_PERF      0x00000008 /**< MACTIMER_OVF_PER flag */
#define RFCORE_SFR_MTIRQF_MACTIMER_COMPARE2F     0x00000004 /**< MACTIMER_COMPARE2 flag */
#define RFCORE_SFR_MTIRQF_MACTIMER_COMPARE1F     0x00000002 /**< MACTIMER_COMPARE1 flag */
#define RFCORE_SFR_MTIRQF_MACTIMER_PERF          0x00000001 /**< MACTIMER_PER flag */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTMSEL register bit masks
 * @{
 */
#define RFCORE_SFR_MTMSEL_MTMOVFSEL 0x00000070 /**< MTMOVF register select */
#define RFCORE_SFR_MTMSEL_MTMSEL    0x00000007 /**< MTM register select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTM[0:2] register bit masks
 * @{
 */
#define RFCORE_SFR_MTM0_MTM0       0x000000FF /**< Register[7:0] */
#define RFCORE_SFR_MTM1_MTM1       0x000000FF /**< Register[15:8] */
#define RFCORE_SFR_MTMOVF2_MTMOVF2 0x000000FF /**< Register[23:16] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_MTMOVF[1:0] register bit masks
 * @{
 */
#define RFCORE_SFR_MTMOVF1_MTMOVF1 0x000000FF /**< Register[15:8] */
#define RFCORE_SFR_MTMOVF0_MTMOVF0 0x000000FF /**< Register[7:0] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_RFDATA register bit masks
 * @{
 */
#define RFCORE_SFR_RFDATA_RFD 0x000000FF /**< Read/Write Data from RF FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_RFERRF register bit masks
 * @{
 */
#define RFCORE_SFR_RFERRF_STROBEERR 0x00000040 /**< Strobe error */
#define RFCORE_SFR_RFERRF_TXUNDERF  0x00000020 /**< TX FIFO underflowed */
#define RFCORE_SFR_RFERRF_TXOVERF   0x00000010 /**< TX FIFO overflowed */
#define RFCORE_SFR_RFERRF_RXUNDERF  0x00000008 /**< RX FIFO underflowed */
#define RFCORE_SFR_RFERRF_RXOVERF   0x00000004 /**< RX FIFO overflowed */
#define RFCORE_SFR_RFERRF_RXABO     0x00000002 /**< Frame RX was aborted */
#define RFCORE_SFR_RFERRF_NLOCK     0x00000001 /**< Frequency synthesizer lock error */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_RFIRQF1 register bit masks
 * @{
 */
#define RFCORE_SFR_RFIRQF1_CSP_WAIT   0x00000020 /**< CSP Execution continued */
#define RFCORE_SFR_RFIRQF1_CSP_STOP   0x00000010 /**< CSP has stopped program */
#define RFCORE_SFR_RFIRQF1_CSP_MANINT 0x00000008 /**< CSP Manual interrupt */
#define RFCORE_SFR_RFIRQF1_RFIDLE     0x00000004 /**< IDLE state entered */
#define RFCORE_SFR_RFIRQF1_TXDONE     0x00000002 /**< Complete frame TX finished */
#define RFCORE_SFR_RFIRQF1_TXACKDONE  0x00000001 /**< ACK frame TX finished */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_RFIRQF0 register bit masks
 * @{
 */
#define RFCORE_SFR_RFIRQF0_RXMASKZERO      0x00000080 /**< RXENABLE gone all-zero */
#define RFCORE_SFR_RFIRQF0_RXPKTDONE       0x00000040 /**< Complete frame RX */
#define RFCORE_SFR_RFIRQF0_FRAME_ACCEPTED  0x00000020 /**< Frame has passed frame filter */
#define RFCORE_SFR_RFIRQF0_SRC_MATCH_FOUND 0x00000010 /**< Source match is found */
#define RFCORE_SFR_RFIRQF0_SRC_MATCH_DONE  0x00000008 /**< Source matching is complete */
#define RFCORE_SFR_RFIRQF0_FIFOP           0x00000004 /**< RX FIFO exceeded threshold */
#define RFCORE_SFR_RFIRQF0_SFD             0x00000002 /**< SFD TX or RX */
#define RFCORE_SFR_RFIRQF0_ACT_UNUSED      0x00000001 /**< Reserved */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_SFR_RFST register bit masks
 * @{
 */
#define RFCORE_SFR_RFST_INSTR 0x000000FF /**< Data written to this register */
/** @} */

#endif /* RFCORE_SFR_H_ */
/** @} */
