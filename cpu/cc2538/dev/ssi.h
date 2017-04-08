/*
 * Copyright (c) 2013, University of Michigan.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-spi cc2538 Synchronous Serial Interface
 *
 * Driver for the cc2538 SPI peripheral
 *
 * Register and bitmask definitions based on the Foundation Firmware from
 * Texas Instruments.
 * @{
 *
 * \file
 * Header file for the cc2538 Synchronous Serial Interface
 */

#ifndef SSI_H_
#define SSI_H_

/*---------------------------------------------------------------------------*/
/** \name Number of SSI instances supported by this CPU.
 * @{
 */
#define SSI_INSTANCE_COUNT  2
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Base register memory locations.
 * @{
 */
#define SSI0_BASE               0x40008000 /**< Base address for SSI0 */
#define SSI1_BASE               0x40009000 /**< Base address for SSI1 */
/** Base address of the \c dev instance of the SSI */
#define SSI_BASE(dev)           (SSI0_BASE + (dev) * (SSI1_BASE - SSI0_BASE))
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SSI register offsets
 * @{
 */
#define SSI_CR0                 0x00000000 /**< Control register 0 */
#define SSI_CR1                 0x00000004 /**< Control register 1 */
#define SSI_DR                  0x00000008 /**< Access the TX and RX FIFO */
#define SSI_SR                  0x0000000C /**< Meta information about FIFO */
#define SSI_CPSR                0x00000010 /**< Clock divider */
#define SSI_IM                  0x00000014 /**< Interrupt mask */
#define SSI_RIS                 0x00000018 /**< Raw interrupt status */
#define SSI_MIS                 0x0000001C /**< Masked interrupt status */
#define SSI_ICR                 0x00000020 /**< Interrupt clear register */
#define SSI_DMACTL              0x00000024 /**< DMA control register */
#define SSI_CC                  0x00000FC8 /**< Clock configuration */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SSI Bitmasks and shifts
 * @{
 */
#define SSI_CR0_SCR_M           0x0000FF00 /**< Serial clock rate mask */
#define SSI_CR0_SCR_S           8          /**< Serial clock rate shift */
#define SSI_CR0_SPH_M           0x00000080 /**< Serial clock phase (H) mask */
#define SSI_CR0_SPH_S           7          /**< Serial clock phase (H) shift */
#define SSI_CR0_SPO_M           0x00000040 /**< Serial clock phase (O) mask */
#define SSI_CR0_SPO_S           6          /**< Serial clock phase (O) shift */
#define SSI_CR0_FRF_M           0x00000030 /**< Frame format select mask */
#define SSI_CR0_FRF_S           4          /**< Frame format select shift */
#define SSI_CR0_DSS_M           0x0000000F /**< Data size select mask */
#define SSI_CR0_DSS_S           0          /**< Data size select shift */
#define SSI_CR1_SOD_M           0x00000008 /**< Slave mode output disable mask */
#define SSI_CR1_SOD_S           3          /**< Slave mode output disable shift */
#define SSI_CR1_MS_M            0x00000004 /**< Master and slave select mask */
#define SSI_CR1_MS_S            2          /**< Master and slave select shift */
#define SSI_CR1_SSE_M           0x00000002 /**< Synchronous serial port enable mask */
#define SSI_CR1_SSE_S           1          /**< Synchronous serial port enable shift */
#define SSI_CR1_LBM_M           0x00000001 /**< Loop-back mode mask */
#define SSI_CR1_LBM_S           0          /**< Loop-back mode shift */
#define SSI_DR_DATA_M           0x0000FFFF /**< FIFO data mask */
#define SSI_DR_DATA_S           0          /**< FIFO data shift */
#define SSI_SR_BSY_M            0x00000010 /**< Busy bit mask */
#define SSI_SR_BSY_S            4          /**< Busy bit shift */
#define SSI_SR_RFF_M            0x00000008 /**< Receive FIFO full mask */
#define SSI_SR_RFF_S            3          /**< Receive FIFO full shift */
#define SSI_SR_RNE_M            0x00000004 /**< Receive FIFO not empty mask */
#define SSI_SR_RNE_S            2          /**< Receive FIFO not empty shift */
#define SSI_SR_TNF_M            0x00000002 /**< Transmit FIFO not full mask */
#define SSI_SR_TNF_S            1          /**< Transmit FIFO not full shift */
#define SSI_SR_TFE_M            0x00000001 /**< Transmit FIFO empty mask */
#define SSI_SR_TFE_S            0          /**< Transmit FIFO empty shift */
#define SSI_CPSR_CPSDVSR_M      0x000000FF /**< Clock prescale divisor mask */
#define SSI_CPSR_CPSDVSR_S      0          /**< Clock prescale divisor shift */
#define SSI_IM_TXIM_M           0x00000008 /**< Transmit FIFO interrupt mask mask */
#define SSI_IM_TXIM_S           3          /**< Transmit FIFO interrupt mask shift */
#define SSI_IM_RXIM_M           0x00000004 /**< Receive FIFO interrupt mask mask */
#define SSI_IM_RXIM_S           2          /**< Receive FIFO interrupt mask shift */
#define SSI_IM_RTIM_M           0x00000002 /**< Receive time-out interrupt mask mask */
#define SSI_IM_RTIM_S           1          /**< Receive time-out interrupt mask shift */
#define SSI_IM_RORIM_M          0x00000001 /**< Receive overrun interrupt mask mask */
#define SSI_IM_RORIM_S          0          /**< Receive overrun interrupt mask shift */
#define SSI_RIS_TXRIS_M         0x00000008 /**< SSITXINTR raw state mask */
#define SSI_RIS_TXRIS_S         3          /**< SSITXINTR raw state shift */
#define SSI_RIS_RXRIS_M         0x00000004 /**< SSIRXINTR raw state mask */
#define SSI_RIS_RXRIS_S         2          /**< SSIRXINTR raw state shift */
#define SSI_RIS_RTRIS_M         0x00000002 /**< SSIRTINTR raw state mask */
#define SSI_RIS_RTRIS_S         1          /**< SSIRTINTR raw state shift */
#define SSI_RIS_RORRIS_M        0x00000001 /**< SSIRORINTR raw state mask */
#define SSI_RIS_RORRIS_S        0          /**< SSIRORINTR raw state shift */
#define SSI_MIS_TXMIS_M         0x00000008 /**< SSITXINTR masked state mask */
#define SSI_MIS_TXMIS_S         3          /**< SSITXINTR masked state shift */
#define SSI_MIS_RXMIS_M         0x00000004 /**< SSIRXINTR masked state mask */
#define SSI_MIS_RXMIS_S         2          /**< SSIRXINTR masked state shift */
#define SSI_MIS_RTMIS_M         0x00000002 /**< SSIRTINTR masked state mask */
#define SSI_MIS_RTMIS_S         1          /**< SSIRTINTR masked state shift */
#define SSI_MIS_RORMIS_M        0x00000001 /**< SSIRORINTR masked state mask */
#define SSI_MIS_RORMIS_S        0          /**< SSIRORINTR masked state shift */
#define SSI_ICR_RTIC_M          0x00000002 /**< Receive time-out interrupt clear mask */
#define SSI_ICR_RTIC_S          1          /**< Receive time-out interrupt clear shift */
#define SSI_ICR_RORIC_M         0x00000001 /**< Receive overrun interrupt clear mask */
#define SSI_ICR_RORIC_S         0          /**< Receive overrun interrupt clear shift */
#define SSI_DMACTL_TXDMAE_M     0x00000002 /**< Transmit DMA enable mask */
#define SSI_DMACTL_TXDMAE_S     1          /**< Transmit DMA enable shift */
#define SSI_DMACTL_RXDMAE_M     0x00000001 /**< Receive DMA enable mask */
#define SSI_DMACTL_RXDMAE_S     0          /**< Receive DMA enable shift */
#define SSI_CC_CS_M             0x00000007 /**< Baud and system clock source mask */
#define SSI_CC_CS_S             0          /**< Baud and system clock source shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SSI Register Values
 * @{
 */
#define SSI_CR0_SPH             0x00000080 /**< Serial clock phase (H) */
#define SSI_CR0_SPO             0x00000040 /**< Serial clock phase (O) */
#define SSI_CR0_FRF_MOTOROLA    0x00000000 /**< Motorola frame format */
#define SSI_CR0_FRF_TI          0x00000010 /**< Texas Instruments frame format */
#define SSI_CR0_FRF_MICROWIRE   0x00000020 /**< National Microwire frame format */
#define SSI_CR1_SOD             0x00000008 /**< Slave mode output disable */
#define SSI_CR1_MS              0x00000004 /**< Master and slave select */
#define SSI_CR1_SSE             0x00000002 /**< Synchronous serial port enable */
#define SSI_CR1_LBM             0x00000001 /**< Loop-back mode */
#define SSI_SR_BSY              0x00000010 /**< Busy bit */
#define SSI_SR_RFF              0x00000008 /**< Receive FIFO full */
#define SSI_SR_RNE              0x00000004 /**< Receive FIFO not empty */
#define SSI_SR_TNF              0x00000002 /**< Transmit FIFO not full */
#define SSI_SR_TFE              0x00000001 /**< Transmit FIFO empty */
#define SSI_IM_TXIM             0x00000008 /**< Transmit FIFO interrupt mask */
#define SSI_IM_RXIM             0x00000004 /**< Receive FIFO interrupt mask */
#define SSI_IM_RTIM             0x00000002 /**< Receive time-out interrupt mask */
#define SSI_IM_RORIM            0x00000001 /**< Receive overrun interrupt mask */
#define SSI_RIS_TXRIS           0x00000008 /**< SSITXINTR raw state */
#define SSI_RIS_RXRIS           0x00000004 /**< SSIRXINTR raw state */
#define SSI_RIS_RTRIS           0x00000002 /**< SSIRTINTR raw state */
#define SSI_RIS_RORRIS          0x00000001 /**< SSIRORINTR raw state */
#define SSI_MIS_TXMIS           0x00000008 /**< SSITXINTR masked state */
#define SSI_MIS_RXMIS           0x00000004 /**< SSIRXINTR masked state */
#define SSI_MIS_RTMIS           0x00000002 /**< SSIRTINTR masked state */
#define SSI_MIS_RORMIS          0x00000001 /**< SSIRORINTR masked state */
#define SSI_ICR_RTIC            0x00000002 /**< Receive time-out interrupt clear */
#define SSI_ICR_RORIC           0x00000001 /**< Receive overrun interrupt clear */
#define SSI_DMACTL_TXDMAE       0x00000002 /**< Transmit DMA enable */
#define SSI_DMACTL_RXDMAE       0x00000001 /**< Receive DMA enable */
/** @} */

#endif
/**
 * @}
 * @}
 */

