/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-udma cc2538 micro-DMA
 *
 * Driver for the cc2538 uDMA controller
 * @{
 *
 * \file
 * Header file with register, macro and function declarations for the cc2538
 * micro-DMA controller module
 */
#ifndef UDMA_H_
#define UDMA_H_

#include "contiki-conf.h"

/*
 * Enable all uDMA channels unless a conf file tells us to do otherwise.
 * Using all 31 channels will consume a lot of RAM for the channel control
 * data structure. Thus it's wise to set this define to the number of the
 * highest channel in use
 */
#ifndef UDMA_CONF_MAX_CHANNEL
#define UDMA_CONF_MAX_CHANNEL   31
#endif
/*---------------------------------------------------------------------------*/
/**
 * \name uDMA Register offset declarations
 * @{
 */
#define UDMA_STAT               0x400FF000  /**< DMA status */
#define UDMA_CFG                0x400FF004  /**< DMA configuration */
#define UDMA_CTLBASE            0x400FF008  /**< DMA channel control base pointer */
#define UDMA_ALTBASE            0x400FF00C  /**< DMA alternate channel control base pointer */
#define UDMA_WAITSTAT           0x400FF010  /**< DMA channel wait-on-request status */
#define UDMA_SWREQ              0x400FF014  /**< DMA channel software request */
#define UDMA_USEBURSTSET        0x400FF018  /**< DMA channel useburst set */
#define UDMA_USEBURSTCLR        0x400FF01C  /**< DMA channel useburst clear */
#define UDMA_REQMASKSET         0x400FF020  /**< DMA channel request mask set */
#define UDMA_REQMASKCLR         0x400FF024  /**< DMA channel request mask clear */
#define UDMA_ENASET             0x400FF028  /**< DMA channel enable set */
#define UDMA_ENACLR             0x400FF02C  /**< DMA channel enable clear */
#define UDMA_ALTSET             0x400FF030  /**< DMA channel primary alternate set */
#define UDMA_ALTCLR             0x400FF034  /**< DMA channel primary alternate clear */
#define UDMA_PRIOSET            0x400FF038  /**< DMA channel priority set */
#define UDMA_PRIOCLR            0x400FF03C  /**< DMA channel priority clear */
#define UDMA_ERRCLR             0x400FF04C  /**< DMA bus error clear */
#define UDMA_CHASGN             0x400FF500  /**< DMA channel assignment */
#define UDMA_CHIS               0x400FF504  /**< DMA channel interrupt status */
#define UDMA_CHMAP0             0x400FF510  /**< DMA channel map select 0 */
#define UDMA_CHMAP1             0x400FF514  /**< DMA channel map select 1 */
#define UDMA_CHMAP2             0x400FF518  /**< DMA channel map select 2 */
#define UDMA_CHMAP3             0x400FF51C  /**< DMA channel map select 3 */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_STAT register bit masks
 * @{
 */
#define UDMA_STAT_DMACHANS      0x001F0000  /**< Available uDMA channels minus 1 */
#define UDMA_STAT_STATE         0x000000F0  /**< Control state machine status */
#define UDMA_STAT_MASTEN        0x00000001  /**< Master enable status */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CFG register bit masks
 * @{
 */
#define UDMA_CFG_MASTEN         0x00000001  /**< Controller master enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CTLBASE register bit masks
 * @{
 */
#define UDMA_CTLBASE_ADDR       0xFFFFFC00  /**< Channel control base address */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ALTBASE register bit masks
 * @{
 */
#define UDMA_ALTBASE_ADDR       0xFFFFFFFF  /**< Alternate channel address pointer */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_WAITSTAT register bit masks
 * @{
 */
#define UDMA_WAITSTAT_WAITREQ   0xFFFFFFFF  /**< Channel [n] wait status */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_SWREQ register bit masks
 * @{
 */
#define UDMA_SWREQ_SWREQ        0xFFFFFFFF  /**< Channel [n] software request */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_USEBURSTSET register bit masks
 * @{
 */
#define UDMA_USEBURSTSET_SET    0xFFFFFFFF  /**< Channel [n] useburst set */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_USEBURSTCLR register bit masks
 * @{
 */
#define UDMA_USEBURSTCLR_CLR    0xFFFFFFFF  /**< Channel [n] useburst clear */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_REQMASKSET register bit masks
 * @{
 */
#define UDMA_REQMASKSET_SET     0xFFFFFFFF  /**< Channel [n] request mask set */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_REQMASKCLR register bit masks
 * @{
 */
#define UDMA_REQMASKCLR_CLR     0xFFFFFFFF  /**< Channel [n] request mask clear */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ENASET register bit masks
 * @{
 */
#define UDMA_ENASET_SET         0xFFFFFFFF  /**< Channel [n] enable set */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ENACLR register bit masks
 * @{
 */
#define UDMA_ENACLR_CLR         0xFFFFFFFF  /**< Channel [n] enable clear */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ALTSET register bit masks
 * @{
 */
#define UDMA_ALTSET_SET         0xFFFFFFFF  /**< Channel [n] alternate set */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ALTCLR register bit masks
 * @{
 */
#define UDMA_ALTCLR_CLR         0xFFFFFFFF  /**< Channel [n] alternate clear */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_PRIOSET register bit masks
 * @{
 */
#define UDMA_PRIOSET_SET        0xFFFFFFFF  /**< Channel [n] priority set */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_PRIOCLR register bit masks
 * @{
 */
#define UDMA_PRIOCLR_CLR        0xFFFFFFFF  /**< Channel [n] priority clear */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_ERRCLR register bit masks
 * @{
 */
#define UDMA_ERRCLR_ERRCLR      0x00000001  /**< uDMA bus error status */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CHASGN register bit masks
 * @{
 */
#define UDMA_CHASGN_CHASGN      0xFFFFFFFF  /**< Channel [n] assignment select */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CHIS register bit masks
 * @{
 */
#define UDMA_CHIS_CHIS          0xFFFFFFFF  /**< Channel [n] interrupt status */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CHMAP0 register bit masks
 * @{
 */
#define UDMA_CHMAP0_CH7SEL      0xF0000000  /**< uDMA channel 7 source select */
#define UDMA_CHMAP0_CH6SEL      0x0F000000  /**< uDMA channel 6 source select */
#define UDMA_CHMAP0_CH5SEL      0x00F00000  /**< uDMA channel 5 source select */
#define UDMA_CHMAP0_CH4SEL      0x000F0000  /**< uDMA channel 4 source select */
#define UDMA_CHMAP0_CH3SEL      0x0000F000  /**< uDMA channel 3 source select */
#define UDMA_CHMAP0_CH2SEL      0x00000F00  /**< uDMA channel 2 source select */
#define UDMA_CHMAP0_CH1SEL      0x000000F0  /**< uDMA channel 1 source select */
#define UDMA_CHMAP0_CH0SEL      0x0000000F  /**< uDMA channel 0 source select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UDMA_CHMAP1 register bit masks
 * @{
 */
#define UDMA_CHMAP1_CH15SEL     0xF0000000  /**< uDMA channel 15 source select */
#define UDMA_CHMAP1_CH14SEL     0x0F000000  /**< uDMA channel 14 source select */
#define UDMA_CHMAP1_CH13SEL     0x00F00000  /**< uDMA channel 13 source select */
#define UDMA_CHMAP1_CH12SEL     0x000F0000  /**< uDMA channel 12 source select */
#define UDMA_CHMAP1_CH11SEL     0x0000F000  /**< uDMA channel 11 source select */
#define UDMA_CHMAP1_CH10SEL     0x00000F00  /**< uDMA channel 10 source select */
#define UDMA_CHMAP1_CH9SEL      0x000000F0  /**< uDMA channel 9 source select */
#define UDMA_CHMAP1_CH8SEL      0x0000000F  /**< uDMA channel 8 source select */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CHMAP2 register bit masks
 * @{
 */
#define UDMA_CHMAP2_CH23SEL     0xF0000000  /**< uDMA channel 23 source select */
#define UDMA_CHMAP2_CH22SEL     0x0F000000  /**< uDMA channel 22 source select */
#define UDMA_CHMAP2_CH21SEL     0x00F00000  /**< uDMA channel 21 source select */
#define UDMA_CHMAP2_CH20SEL     0x000F0000  /**< uDMA channel 20 source select */
#define UDMA_CHMAP2_CH19SEL     0x0000F000  /**< uDMA channel 19 source select */
#define UDMA_CHMAP2_CH18SEL     0x00000F00  /**< uDMA channel 18 source select */
#define UDMA_CHMAP2_CH17SEL     0x000000F0  /**< uDMA channel 17 source select */
#define UDMA_CHMAP2_CH16SEL     0x0000000F  /**< uDMA channel 16 source select */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UDMA_CHMAP3 register bit masks
 * @{
 */
#define UDMA_CHMAP3_CH31SEL     0xF0000000  /**< uDMA channel 31 source select */
#define UDMA_CHMAP3_CH30SEL     0x0F000000  /**< uDMA channel 30 source select */
#define UDMA_CHMAP3_CH29SEL     0x00F00000  /**< uDMA channel 29 source select */
#define UDMA_CHMAP3_CH28SEL     0x000F0000  /**< uDMA channel 28 source select */
#define UDMA_CHMAP3_CH27SEL     0x0000F000  /**< uDMA channel 27 source select */
#define UDMA_CHMAP3_CH26SEL     0x00000F00  /**< uDMA channel 26 source select */
#define UDMA_CHMAP3_CH25SEL     0x000000F0  /**< uDMA channel 25 source select */
#define UDMA_CHMAP3_CH24SEL     0x0000000F  /**< uDMA channel 24 source select */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name uDMA Channel encoding assignments
 * @{
 */
/* Channel 0 */
#define UDMA_CH0_RESERVED0          0x00
#define UDMA_CH0_RESERVED1          0x01
#define UDMA_CH0_RESERVED2          0x02
#define UDMA_CH0_RESERVED3          0x03
#define UDMA_CH0_USB                0x04

/* Channel 1 */
#define UDMA_CH1_RESERVED0          0x00
#define UDMA_CH1_RESERVED1          0x01
#define UDMA_CH1_RESERVED2          0x02
#define UDMA_CH1_RESERVED3          0x03
#define UDMA_CH1_ADC                0x04

/* Channel 2 */
#define UDMA_CH2_RESERVED0          0x00
#define UDMA_CH2_TIMER3A            0x01
#define UDMA_CH2_RESERVED2          0x02
#define UDMA_CH2_RESERVED3          0x03
#define UDMA_CH2_FLASH              0x04

/* Channel 3 */
#define UDMA_CH3_RESERVED0          0x00
#define UDMA_CH3_TIMER3B            0x01
#define UDMA_CH3_RESERVED2          0x02
#define UDMA_CH3_RESERVED3          0x03
#define UDMA_CH3_RFCORETRG1         0x04

/* Channel 4 */
#define UDMA_CH4_RESERVED0          0x00
#define UDMA_CH4_TIMER2A            0x01
#define UDMA_CH4_RESERVED2          0x02
#define UDMA_CH4_RESERVED3          0x03
#define UDMA_CH4_RFCORETRG2         0x04

/* Channel 5 */
#define UDMA_CH5_RESERVED0          0x00
#define UDMA_CH5_TIMER2B            0x01
#define UDMA_CH5_RESERVED2          0x02
#define UDMA_CH5_RESERVED3          0x03
#define UDMA_CH5_RESERVED4          0x04

/* Channel 6 */
#define UDMA_CH6_RESERVED0          0x00
#define UDMA_CH6_TIMER2A            0x01
#define UDMA_CH6_RESERVED2          0x02
#define UDMA_CH6_RESERVED3          0x03
#define UDMA_CH6_RESERVED4          0x04

/* Channel 7 */
#define UDMA_CH7_RESERVED0          0x00
#define UDMA_CH7_TIMER2B            0x01
#define UDMA_CH7_RESERVED2          0x02
#define UDMA_CH7_RESERVED3          0x03
#define UDMA_CH7_RESERVED4          0x04

/* Channel 8 */
#define UDMA_CH8_UART0RX            0x00
#define UDMA_CH8_UART1RX            0x01
#define UDMA_CH8_RESERVED2          0x02
#define UDMA_CH8_RESERVED3          0x03
#define UDMA_CH8_RESERVED4          0x04

/* Channel 9 */
#define UDMA_CH9_UART0TX            0x00
#define UDMA_CH9_UART1TX            0x01
#define UDMA_CH9_RESERVED2          0x02
#define UDMA_CH9_RESERVED3          0x03
#define UDMA_CH9_RESERVED4          0x04

/* Channel 10 */
#define UDMA_CH10_SSI0RX            0x00
#define UDMA_CH10_SSI1RX            0x01
#define UDMA_CH10_RESERVED2         0x02
#define UDMA_CH10_RESERVED3         0x03
#define UDMA_CH10_RESERVED4         0x04

/* Channel 11 */
#define UDMA_CH11_SSI0TX            0x00
#define UDMA_CH11_SSI1TX            0x01
#define UDMA_CH11_RESERVED2         0x02
#define UDMA_CH11_RESERVED3         0x03
#define UDMA_CH11_RESERVED4         0x04

/* Channel 12 */
#define UDMA_CH12_RESERVED0         0x00
#define UDMA_CH12_RESERVED1         0x01
#define UDMA_CH12_RESERVED2         0x02
#define UDMA_CH12_RESERVED3         0x03
#define UDMA_CH12_RESERVED4         0x04

/* Channel 13 */
#define UDMA_CH13_RESERVED0         0x00
#define UDMA_CH13_RESERVED1         0x01
#define UDMA_CH13_RESERVED2         0x02
#define UDMA_CH13_RESERVED3         0x03
#define UDMA_CH13_RESERVED4         0x04

/* Channel 14 */
#define UDMA_CH14_ADC0              0x00
#define UDMA_CH14_TIMER2A           0x01
#define UDMA_CH14_RESERVED2         0x02
#define UDMA_CH14_RESERVED3         0x03
#define UDMA_CH14_RESERVED4         0x04

/* Channel 15 */
#define UDMA_CH15_ADC1              0x00
#define UDMA_CH15_TIMER2B           0x01
#define UDMA_CH15_RESERVED2         0x02
#define UDMA_CH15_RESERVED3         0x03
#define UDMA_CH15_RESERVED4         0x04

/* Channel 16 */
#define UDMA_CH16_ADC2              0x00
#define UDMA_CH16_RESERVED1         0x01
#define UDMA_CH16_RESERVED2         0x02
#define UDMA_CH16_RESERVED3         0x03
#define UDMA_CH16_RESERVED4         0x04

/* Channel 17 */
#define UDMA_CH17_ADC3              0x00
#define UDMA_CH17_RESERVED1         0x01
#define UDMA_CH17_RESERVED2         0x02
#define UDMA_CH17_RESERVED3         0x03
#define UDMA_CH17_RESERVED4         0x04

/* Channel 18 */
#define UDMA_CH18_TIMER0A           0x00
#define UDMA_CH18_TIMER1A           0x01
#define UDMA_CH18_RESERVED2         0x02
#define UDMA_CH18_RESERVED3         0x03
#define UDMA_CH18_RESERVED4         0x04

/* Channel 19 */
#define UDMA_CH19_TIMER0B           0x00
#define UDMA_CH19_TIMER1B           0x01
#define UDMA_CH19_RESERVED2         0x02
#define UDMA_CH19_RESERVED3         0x03
#define UDMA_CH19_RESERVED4         0x04

/* Channel 20 */
#define UDMA_CH20_TIMER1A           0x00
#define UDMA_CH20_RESERVED1         0x01
#define UDMA_CH20_RESERVED2         0x02
#define UDMA_CH20_RESERVED3         0x03
#define UDMA_CH20_RESERVED4         0x04

/* Channel 21 */
#define UDMA_CH21_TIMER1B           0x00
#define UDMA_CH21_RESERVED1         0x01
#define UDMA_CH21_RESERVED2         0x02
#define UDMA_CH21_RESERVED3         0x03
#define UDMA_CH21_RESERVED4         0x04

/* Channel 22 */
#define UDMA_CH22_UART1RX           0x00
#define UDMA_CH22_RESERVED1         0x01
#define UDMA_CH22_RESERVED2         0x02
#define UDMA_CH22_RESERVED3         0x03
#define UDMA_CH22_RESERVED4         0x04

/* Channel 23 */
#define UDMA_CH23_UART1TX           0x00
#define UDMA_CH23_RESERVED1         0x01
#define UDMA_CH23_RESERVED2         0x02
#define UDMA_CH23_RESERVED3         0x03
#define UDMA_CH23_RESERVED4         0x04

/* Channel 24 */
#define UDMA_CH24_SSI1RX            0x00
#define UDMA_CH24_ADC4              0x01
#define UDMA_CH24_RESERVED2         0x02
#define UDMA_CH24_RESERVED3         0x03
#define UDMA_CH24_RESERVED4         0x04

/* Channel 25 */
#define UDMA_CH25_SSI1TX            0x00
#define UDMA_CH25_ADC5              0x01
#define UDMA_CH25_RESERVED2         0x02
#define UDMA_CH25_RESERVED3         0x03
#define UDMA_CH25_RESERVED4         0x04

/* Channel 26 */
#define UDMA_CH26_RESERVED0         0x00
#define UDMA_CH26_ADC6              0x01
#define UDMA_CH26_RESERVED2         0x02
#define UDMA_CH26_RESERVED3         0x03
#define UDMA_CH26_RESERVED4         0x04

/* Channel 27 */
#define UDMA_CH27_RESERVED0         0x00
#define UDMA_CH27_ADC7              0x01
#define UDMA_CH27_RESERVED2         0x02
#define UDMA_CH27_RESERVED3         0x03
#define UDMA_CH27_RESERVED4         0x04

/* Channel 28 */
#define UDMA_CH28_RESERVED0         0x00
#define UDMA_CH28_RESERVED1         0x01
#define UDMA_CH28_RESERVED2         0x02
#define UDMA_CH28_RESERVED3         0x03
#define UDMA_CH28_RESERVED4         0x04

/* Channel 29 */
#define UDMA_CH29_RESERVED0         0x00
#define UDMA_CH29_RESERVED1         0x01
#define UDMA_CH29_RESERVED2         0x02
#define UDMA_CH29_RESERVED3         0x03
#define UDMA_CH29_RFCORET2TRG1      0x04

/* Channel 30 */
#define UDMA_CH30_SW                0x00
#define UDMA_CH30_RESERVED1         0x01
#define UDMA_CH30_RESERVED2         0x02
#define UDMA_CH30_RESERVED3         0x03
#define UDMA_CH30_RFCORET2TRG2      0x04

/* Channel 31 */
#define UDMA_CH31_RESERVED0         0x00
#define UDMA_CH31_RESERVED1         0x01
#define UDMA_CH31_RESERVED2         0x02
#define UDMA_CH31_RESERVED3         0x03
#define UDMA_CH31_RESERVED4         0x04
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Values to ORd together as the ctrl argument of
 * udma_set_channel_control_word()
 * @{
 */
#define UDMA_CHCTL_DSTINC_NONE  0xC0000000  /**< Dst address no increment */
#define UDMA_CHCTL_DSTINC_32    0x80000000  /**< Dst address increment 32 bit */
#define UDMA_CHCTL_DSTINC_16    0x40000000  /**< Dst address increment 16 bit */
#define UDMA_CHCTL_DSTINC_8     0x00000000  /**< Dst address increment 8 bit */

#define UDMA_CHCTL_DSTSIZE_32   0x20000000  /**< Destination size 32 bit */
#define UDMA_CHCTL_DSTSIZE_16   0x10000000  /**< Destination size 16 bit */
#define UDMA_CHCTL_DSTSIZE_8    0x00000000  /**< Destination size 8 bit */

#define UDMA_CHCTL_SRCINC_NONE  0x0C000000  /**< Source address no increment */
#define UDMA_CHCTL_SRCINC_32    0x08000000  /**< Source address increment 32 bit */
#define UDMA_CHCTL_SRCINC_16    0x04000000  /**< Source address increment 16 bit */
#define UDMA_CHCTL_SRCINC_8     0x00000000  /**< Source address increment 8 bit */

#define UDMA_CHCTL_SRCSIZE_32   0x02000000  /**< Source size 32 bit */
#define UDMA_CHCTL_SRCSIZE_16   0x01000000  /**< Source size 16 bit */
#define UDMA_CHCTL_SRCSIZE_8    0x00000000  /**< Source size 8 bit */

#define UDMA_CHCTL_ARBSIZE_1    0x00000000  /**< Arbitration size 1 Transfer */
#define UDMA_CHCTL_ARBSIZE_2    0x00004000  /**< Arbitration size 2 Transfers */
#define UDMA_CHCTL_ARBSIZE_4    0x00008000  /**< Arbitration size 4 Transfers */
#define UDMA_CHCTL_ARBSIZE_8    0x0000C000  /**< Arbitration size 8 Transfers */
#define UDMA_CHCTL_ARBSIZE_16   0x00010000  /**< Arbitration size 16 Transfers */
#define UDMA_CHCTL_ARBSIZE_32   0x00014000  /**< Arbitration size 32 Transfers */
#define UDMA_CHCTL_ARBSIZE_64   0x00018000  /**< Arbitration size 64 Transfers */
#define UDMA_CHCTL_ARBSIZE_128  0x0001C000  /**< Arbitration size 128 Transfers */
#define UDMA_CHCTL_ARBSIZE_256  0x00020000  /**< Arbitration size 256 Transfers */
#define UDMA_CHCTL_ARBSIZE_512  0x00024000  /**< Arbitration size 512 Transfers */
#define UDMA_CHCTL_ARBSIZE_1024 0x00028000  /**< Arbitration size 1024 Transfers */

#define UDMA_CHCTL_XFERMODE_STOP     0x00000000  /**< Stop */
#define UDMA_CHCTL_XFERMODE_BASIC    0x00000001  /**< Basic */
#define UDMA_CHCTL_XFERMODE_AUTO     0x00000002  /**< Auto-Request */
#define UDMA_CHCTL_XFERMODE_PINGPONG 0x00000003  /**< Ping-Pong */
#define UDMA_CHCTL_XFERMODE_MEM_SG   0x00000004  /**< Memory Scatter-Gather */
#define UDMA_CHCTL_XFERMODE_MEM_SGA  0x00000005  /**< Memory Scatter-Gather Alt */
#define UDMA_CHCTL_XFERMODE_PER_SG   0x00000006  /**< Peripheral Scatter-Gather */
#define UDMA_CHCTL_XFERMODE_PER_SGA  0x00000007  /**< Peripheral Scatter-Gather Alt */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \brief Initialise the uDMA driver
 *
 * Prepares the channel control structure and enables the controller
 */
void udma_init(void);

/**
 * \brief Sets the channels source address
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 * \param src_end The source's end address
 */
void udma_set_channel_src(uint8_t channel, uint32_t src_end);

/**
 * \brief Sets the channel's destination address
 * \param dst_end The destination's end address
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_set_channel_dst(uint8_t channel, uint32_t dst_end);

/**
 * \brief Configure the channel's control word
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 * \param ctrl The value of the control word
 *
 * The value of the control word is generated by ORing the values defined as
 * UDMA_CHCTL_xyz
 *
 * For example, to configure a channel with 8 bit source and destination size,
 * 0 source increment and 8 bit destination increment, one would need to pass
 * UDMA_CHCTL_DSTINC_8 | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_SRCSIZE_8 |
 * UDMA_CHCTL_DSTSIZE_8
 *
 * Macros defined as 0 can be omitted.
 */
void udma_set_channel_control_word(uint8_t channel, uint32_t ctrl);

/**
 * \brief Choose an encoding for a uDMA channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 * \param enc A value in [0 , 4]
 *
 * Possible values for the \e encoding param are defined as UDMA_CHnn_xyz
 */
void udma_set_channel_assignment(uint8_t channel, uint8_t enc);

/**
 * \brief Enables a uDMA channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_enable(uint8_t channel);

/**
 * \brief Disables a uDMA channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_disable(uint8_t channel);

/**
 * \brief Use the alternate control data structure for a channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 *
 * \note Currently, the driver only reserves memory space for primary contrl
 * data structures
 */
void udma_channel_use_alternate(uint8_t channel);

/**
 * \brief Use the primary control data structure for a channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_use_primary(uint8_t channel);

/**
 * \brief Set a uDMA channel to high priority
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_prio_set_high(uint8_t channel);

/**
 * \brief Set a uDMA channel to default priority
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_prio_set_default(uint8_t channel);

/**
 * \brief Configure a channel to only use burst transfers
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 *
 * \note The uDMA controller may under certain conditions automatically disable
 * burst mode, in which case this function will need to be called again to
 * re-enable them
 */
void udma_channel_use_burst(uint8_t channel);

/**
 * \brief Configure a channel to use single as well as burst requests
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_use_single(uint8_t channel);

/**
 * \brief Disable peripheral triggers for a uDMA channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 *
 * Calling this function will result in the uDMA controller not acknowledging
 * peripheral-generated transfer triggers. Afterwards, the channel may be used
 * with software triggers
 */
void udma_channel_mask_set(uint8_t channel);

/**
 * \brief Enable peripheral triggers for a uDMA channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_mask_clr(uint8_t channel);

/**
 * \brief Generate a software trigger to start a transfer
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 */
void udma_channel_sw_request(uint8_t channel);

/**
 * \brief Retrieve the current mode for a channel
 * \param channel The channel as a value in [0 , UDMA_CONF_MAX_CHANNEL]
 * \return The channel's current mode
 *
 * The return value will be one of the UDMA_CHCTL_XFERMODE_xyz defines. This
 * function is useful to determine whether a uDMA transfer has completed, in
 * which case the return value will be UDMA_CHCTL_XFERMODE_STOP
 */
uint8_t udma_channel_get_mode(uint8_t channel);

/**
 * \brief Calculate the value of the xfersize field in the control structure
 * \param len The number of items to be transferred
 * \return The value to be written to the control structure to achieve the
 * desired transfer size
 *
 * If we want to transfer \e len items, we will normally do something like
 * udma_set_channel_control_word(OTHER_FLAGS | udma_xfer_size(len))
 */
#define udma_xfer_size(len) ((len - 1) << 4)

#endif /* UDMA_H_ */

/**
 * @}
 * @}
 */
