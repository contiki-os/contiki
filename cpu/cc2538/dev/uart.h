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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-uart cc2538 UART
 *
 * Driver for the cc2538 UART controller
 * @{
 *
 * \file
 * Header file for the cc2538 UART driver
 */
#ifndef UART_H_
#define UART_H_

#include "contiki.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name UART base addresses
 * @{
 */
#define UART_0_BASE           0x4000C000
#define UART_1_BASE           0x4000D000

/* Default to UART 0 unless the configuration tells us otherwise */
#ifdef UART_CONF_BASE
#define UART_BASE             UART_CONF_BASE
#else
#define UART_BASE             UART_0_BASE
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Baud rate defines
 *
 * Used in uart_init() to set the values of UART_IBRD and UART_FBRD in order to
 * achieve some standard baud rates. These defines assume that the UART is
 * clocked at 16MHz and that Clock Div is 16 (UART_CTL:HSE clear)
 * @{
 */
#define UART_IBRD_115200              8 /**< IBRD value for baud rate 115200 */
#define UART_FBRD_115200             44 /**< FBRD value for baud rate 115200 */
#define UART_IBRD_230400              4 /**< IBRD value for baud rate 230400 */
#define UART_FBRD_230400             22 /**< FBRD value for baud rate 230400 */
#define UART_IBRD_460800              2 /**< IBRD value for baud rate 460800 */
#define UART_FBRD_460800             11 /**< FBRD value for baud rate 460800 */

#if UART_CONF_BAUD_RATE==115200
#define UART_CONF_IBRD UART_IBRD_115200
#define UART_CONF_FBRD UART_FBRD_115200
#elif UART_CONF_BAUD_RATE==230400
#define UART_CONF_IBRD UART_IBRD_230400
#define UART_CONF_FBRD UART_FBRD_230400
#elif UART_CONF_BAUD_RATE==460800
#define UART_CONF_IBRD UART_IBRD_460800
#define UART_CONF_FBRD UART_FBRD_460800
#else /* Bail out with an error unless the user provided custom values */
#if !(defined UART_CONF_IBRD && defined UART_CONF_FBRD)
#error "UART baud rate misconfigured and custom IBRD/FBRD values not provided"
#error "Check the value of UART_CONF_BAUD_RATE in contiki-conf.h or project-conf.h"
#error "Supported values are 115200, 230400 and 460800. Alternatively, you can"
#error "provide custom values for UART_CONF_IBRD and UART_CONF_FBRD"
#endif
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART Register Offsets
 * @{
 */
#define UART_DR               0x00000000  /**< UART data */
#define UART_RSR              0x00000004  /**< UART RX status and err clear */
#define UART_ECR              0x00000004  /**< UART RX status and err clear */
#define UART_FR               0x00000018  /**< UART flag */
#define UART_ILPR             0x00000020  /**< UART IrDA low-power */
#define UART_IBRD             0x00000024  /**< UART BAUD divisor: integer */
#define UART_FBRD             0x00000028  /**< UART BAUD divisor: fractional */
#define UART_LCRH             0x0000002C  /**< UART line control */
#define UART_CTL              0x00000030  /**< UART control */
#define UART_IFLS             0x00000034  /**< UART interrupt FIFO level */
#define UART_IM               0x00000038  /**< UART interrupt mask */
#define UART_RIS              0x0000003C  /**< UART raw interrupt status */
#define UART_MIS              0x00000040  /**< UART masked interrupt status */
#define UART_ICR              0x00000044  /**< UART interrupt clear */
#define UART_DMACTL           0x00000048  /**< UART DMA control */
#define UART_LCTL             0x00000090  /**< UART LIN control */
#define UART_LSS              0x00000094  /**< UART LIN snap shot */
#define UART_LTIM             0x00000098  /**< UART LIN timer */
#define UART_NINEBITADDR      0x000000A4  /**< UART 9-bit self address */
#define UART_NINEBITAMASK     0x000000A8  /**< UART 9-bit self address mask */
#define UART_PP               0x00000FC0  /**< UART peripheral properties */
#define UART_CC               0x00000FC8  /**< UART clock configuration */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_DR Register Bit-Masks
 * @{
 */
#define UART_DR_OE              0x00000800  /**< UART overrun error */
#define UART_DR_BE              0x00000400  /**< UART break error */
#define UART_DR_PE              0x00000200  /**< UART parity error */
#define UART_DR_FE              0x00000100  /**< UART framing error */
#define UART_DR_DATA            0x000000FF  /**< Data transmitted or received */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_RSR Register Bit-Masks
 * @{
 */
#define UART_RSR_OE             0x00000008  /**< UART overrun error */
#define UART_RSR_BE             0x00000004  /**< UART break error */
#define UART_RSR_PE             0x00000002  /**< UART parity error */
#define UART_RSR_FE             0x00000001  /**< UART framing error */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_ECR Register Bit-Masks
 * @{
 */
#define UART_ECR_DATA           0x000000FF  /**< Error clear */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_FR Register Bit-Masks
 * @{
 */
#define UART_FR_TXFE            0x00000080  /**< UART transmit FIFO empty */
#define UART_FR_RXFF            0x00000040  /**< UART receive FIFO full */
#define UART_FR_TXFF            0x00000020  /**< UART transmit FIFO full */
#define UART_FR_RXFE            0x00000010  /**< UART receive FIFO empty */
#define UART_FR_BUSY            0x00000008  /**< UART busy */
#define UART_FR_CTS             0x00000001  /**< Clear to send */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_ILPR Register Bit-Masks
 * @{
 */
#define UART_ILPR_ILPDVSR       0x000000FF  /**< IrDA low-power divisor */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_IBRD Register Bit-Masks
 * @{
 */
#define UART_IBRD_DIVINT        0x0000FFFF  /**< Integer baud-rate divisor */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_FPRD Register Bit-Masks
 * @{
 */
#define UART_FBRD_DIVFRAC       0x0000003F  /**< Fractional baud-rate divisor */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_LCRH Register Bit-Masks
 * @{
 */
#define UART_LCRH_SPS           0x00000080  /**< UART stick parity select */
#define UART_LCRH_WLEN          0x00000060  /**< UART word length */
#define UART_LCRH_FEN           0x00000010  /**< UART enable FIFOs */
#define UART_LCRH_STP2          0x00000008  /**< UART two stop bits select */
#define UART_LCRH_EPS           0x00000004  /**< UART even parity select */
#define UART_LCRH_PEN           0x00000002  /**< UART parity enable */
#define UART_LCRH_BRK           0x00000001  /**< UART send break */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_LCRH_WLEN Values
 * @{
 */
#define UART_LCRH_WLEN_8        0x00000060
#define UART_LCRH_WLEN_7        0x00000040
#define UART_LCRH_WLEN_6        0x00000020
#define UART_LCRH_WLEN_5        0x00000000
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_CTL Register Bit-Masks
 * @{
 */
#define UART_CTL_RXE            0x00000200  /**< UART receive enable */
#define UART_CTL_TXE            0x00000100  /**< UART transmit enable */
#define UART_CTL_LBE            0x00000080  /**< UART loop back enable */
#define UART_CTL_LIN            0x00000040  /**< LIN mode enable */
#define UART_CTL_HSE            0x00000020  /**< High-speed enable */
#define UART_CTL_EOT            0x00000010  /**< End of transmission */
#define UART_CTL_SMART          0x00000008  /**< ISO 7816 Smart Card support */
#define UART_CTL_SIRLP          0x00000004  /**< UART SIR low-power mode */
#define UART_CTL_SIREN          0x00000002  /**< UART SIR enable */
#define UART_CTL_UARTEN         0x00000001  /**< UART enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_IFLS Register Bit-Masks
 * @{
 */
#define UART_IFLS_RXIFLSEL      0x00000038  /**< UART RX FIFO level select */
#define UART_IFLS_TXIFLSEL      0x00000007  /**< UART TX FIFO level select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_IFLS_RXIFLSEL Possible Values
 * @{
 */
#define UART_IFLS_RXIFLSEL_7_8  0x00000020  /**< UART RX FIFO >= 7/8 full */
#define UART_IFLS_RXIFLSEL_3_4  0x00000018  /**< UART RX FIFO >= 3/4 full */
#define UART_IFLS_RXIFLSEL_1_2  0x00000010  /**< UART RX FIFO >= 1/2 full */
#define UART_IFLS_RXIFLSEL_1_4  0x00000008  /**< UART RX FIFO >= 1/4 full */
#define UART_IFLS_RXIFLSEL_1_8  0x00000000  /**< UART RX FIFO >= 1/8 full */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_IFLS_TXIFLSEL Possible Values
 * @{
 */
#define UART_IFLS_TXIFLSEL_1_8  0x00000004  /**< UART TX FIFO >= 1/8 empty */
#define UART_IFLS_TXIFLSEL_1_4  0x00000003  /**< UART TX FIFO >= 1/4 empty */
#define UART_IFLS_TXIFLSEL_1_2  0x00000002  /**< UART TX FIFO >= 1/2 empty */
#define UART_IFLS_TXIFLSEL_3_4  0x00000001  /**< UART TX FIFO >= 3/4 empty */
#define UART_IFLS_TXIFLSEL_7_8  0x00000000  /**< UART TX FIFO >= 7/8 empty */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_IM Register Bit-Masks
 * @{
 */
#define UART_IM_LME5IM          0x00008000  /**< LIN mode edge 5 intr mask */
#define UART_IM_LME1IM          0x00004000  /**< LIN mode edge 1 intr mask */
#define UART_IM_LMSBIM          0x00002000  /**< LIN mode sync break mask */
#define UART_IM_NINEBITIM       0x00001000  /**< 9-bit mode interrupt mask */
#define UART_IM_OEIM            0x00000400  /**< UART overrun error mask */
#define UART_IM_BEIM            0x00000200  /**< UART break error mask */
#define UART_IM_PEIM            0x00000100  /**< UART parity error mask */
#define UART_IM_FEIM            0x00000080  /**< UART framing error */
#define UART_IM_RTIM            0x00000040  /**< UART receive time-out mask */
#define UART_IM_TXIM            0x00000020  /**< UART transmit intr mask */
#define UART_IM_RXIM            0x00000010  /**< UART receive interrupt mask */
#define UART_IM_CTSIM           0x00000002  /**< UART CTS modem mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_RIS Register Bit-Masks
 * @{
 */
#define UART_RIS_LME5RIS        0x00008000  /**< LIN mode edge 5 raw */
#define UART_RIS_LME1RIS        0x00004000  /**< LIN mode edge 1 raw */
#define UART_RIS_LMSBRIS        0x00002000  /**< LIN mode sync break raw */
#define UART_RIS_NINEBITRIS     0x00001000  /**< 9-bit mode raw intr */
#define UART_RIS_OERIS          0x00000400  /**< UART overrun error raw */
#define UART_RIS_BERIS          0x00000200  /**< UART break error raw */
#define UART_RIS_PERIS          0x00000100  /**< UART parity error raw */
#define UART_RIS_FERIS          0x00000080  /**< UART framing error raw */
#define UART_RIS_RTRIS          0x00000040  /**< UART RX time-out raw */
#define UART_RIS_TXRIS          0x00000020  /**< UART transmit raw */
#define UART_RIS_RXRIS          0x00000010  /**< UART receive raw */
#define UART_RIS_CTSRIS         0x00000002  /**< UART CTS modem */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_RIS Register Bit-Masks
 * @{
 */
#define UART_MIS_LME5MIS        0x00008000  /**< LIN mode edge 5 masked stat */
#define UART_MIS_LME1MIS        0x00004000  /**< LIN mode edge 1 masked stat */
#define UART_MIS_LMSBMIS        0x00002000  /**< LIN mode sync br masked stat */
#define UART_MIS_NINEBITMIS     0x00001000  /**< 9-bit mode masked stat */
#define UART_MIS_OEMIS          0x00000400  /**< UART overrun err masked stat */
#define UART_MIS_BEMIS          0x00000200  /**< UART break err masked stat */
#define UART_MIS_PEMIS          0x00000100  /**< UART parity err masked stat */
#define UART_MIS_FEMIS          0x00000080  /**< UART framing err masked stat */
#define UART_MIS_RTMIS          0x00000040  /**< UART RX time-out masked stat */
#define UART_MIS_TXMIS          0x00000020  /**< UART TX masked intr stat */
#define UART_MIS_RXMIS          0x00000010  /**< UART RX masked intr stat */
#define UART_MIS_CTSMIS         0x00000002  /**< UART CTS modem masked stat */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_ICR Register Bit-Masks
 * @{
 */
#define UART_ICR_LME5IC         0x00008000  /**< LIN mode edge 5 intr clear */
#define UART_ICR_LME1IC         0x00004000  /**< LIN mode edge 1 intr clear */
#define UART_ICR_LMSBIC         0x00002000  /**< LIN mode sync br intr clear */
#define UART_ICR_NINEBITIC      0x00001000  /**< 9-bit mode intr clear */
#define UART_ICR_OEIC           0x00000400  /**< Overrun error intr clear */
#define UART_ICR_BEIC           0x00000200  /**< Break error intr clear */
#define UART_ICR_PEIC           0x00000100  /**< Parity error intr clear */
#define UART_ICR_FEIC           0x00000080  /**< Framing error intr clear */
#define UART_ICR_RTIC           0x00000040  /**< Receive time-out intr clear */
#define UART_ICR_TXIC           0x00000020  /**< Transmit intr clear */
#define UART_ICR_RXIC           0x00000010  /**< Receive intr clear */
#define UART_ICR_CTSIC          0x00000002  /**< UART CTS modem intr clear */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_DMACTL Register Bit-Masks
 * @{
 */
#define UART_DMACTL_DMAERR      0x00000004  /**< DMA on error */
#define UART_DMACTL_TXDMAE      0x00000002  /**< Transmit DMA enable */
#define UART_DMACTL_RXDMAE      0x00000001  /**< Receive DMA enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_LCTL Register Bit-Masks
 * @{
 */
#define UART_LCTL_BLEN          0x00000030  /**< Sync break length */
#define UART_LCTL_MASTER        0x00000001  /**< LIN master enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_LSS Register Bit-Masks
 * @{
 */
#define UART_LSS_TSS            0x0000FFFF  /**< Timer snap shot */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_LTIM Register Bit-Masks
 * @{
 */
#define UART_LTIM_TIMER         0x0000FFFF  /**< Timer value */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_O_NINEBITADDR Register Bit-Masks
 * @{
 */
#define UART_NINEBITADDR_NINEBITEN 0x00008000  /**< Enable 9-bit mode */
#define UART_NINEBITADDR_ADDR   0x000000FF  /**< Self address for 9-bit mode */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_O_NINEBITADDR Register Bit-Masks
 * @{
 */
#define UART_NINEBITAMASK_RANGE 0x0000FF00  /**< Self addr range, 9-bit mode */
#define UART_NINEBITAMASK_MASK  0x000000FF  /**< Self addr mask, 9-bit mode */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_PP Register Bit-Masks
 * @{
 */
#define UART_PP_NB              0x00000002  /**< 9-bit support */
#define UART_PP_SC              0x00000001  /**< Smart card support */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART_CC Register Bit-Masks
 * @{
 */
#define UART_CC_CS              0x00000007  /**< UART BAUD & sys clock source */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART functions
 * @{
 */

/** \brief Initialises the UART controller, configures I/O control
 * and interrupts */
void uart_init(void);

/** \brief Sends a single character down the UART
 * \param b The character to transmit
 */
void uart_write_byte(uint8_t b);

/** \brief Assigns a callback to be called when the UART receives a byte
 * \param input A pointer to the function
 */
void uart_set_input(int (* input)(unsigned char c));

/** @} */

#endif /* UART_H_ */

/**
 * @}
 * @}
 */
