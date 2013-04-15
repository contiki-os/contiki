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
/** \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-usb cc2538 USB controller
 *
 * Driver for the cc2538 USB controller.
 *
 * We use the USB core in cpu/cc253x/usb which is known to work on Linux as
 * well as on OS X.
 * @{
 *
 * \file
 * Header file with declarations for the cc2538 USB registers
 */
#ifndef USB_REGS_H_
#define USB_REGS_H_

#include "contiki-conf.h"
/*---------------------------------------------------------------------------*/
/**
 * \name USB Register Offsets
 * @{
 */
#define USB_ADDR                0x40089000  /**< Function address */
#define USB_POW                 0x40089004  /**< Power/Control register */
#define USB_IIF                 0x40089008  /**< IN EPs and EP0 interrupt flags */
#define USB_OIF                 0x40089010  /**< OUT endpoint interrupt flags */
#define USB_CIF                 0x40089018  /**< Common USB interrupt flags */
#define USB_IIE                 0x4008901C  /**< IN EPs and EP0 interrupt mask */
#define USB_OIE                 0x40089024  /**< Out EPs interrupt-enable mask */
#define USB_CIE                 0x4008902C  /**< Common USB interrupt mask */
#define USB_FRML                0x40089030  /**< Current frame number (low byte) */
#define USB_FRMH                0x40089034  /**< Current frame number (high) */
#define USB_INDEX               0x40089038  /**< Current endpoint index register */
#define USB_CTRL                0x4008903C  /**< USB control register */
#define USB_MAXI                0x40089040  /**< MAX packet size for IN EPs{1-5} */
#define USB_CS0_CSIL            0x40089044  /**< EP0 Control and Status or IN EPs
                                                 control and status (low) */
#define USB_CS0                 0x40089044  /**< EP0 Control and Status
                                                 (Alias for USB_CS0_CSIL) */
#define USB_CSIL                0x40089044  /**< IN EPs control and status (low)
                                                 (Alias for USB_CS0_CSIL) */
#define USB_CSIH                0x40089048  /**< IN EPs control and status (high) */
#define USB_MAXO                0x4008904C  /**< MAX packet size for OUT EPs */
#define USB_CSOL                0x40089050  /**< OUT EPs control and status (low) */
#define USB_CSOH                0x40089054  /**< OUT EPs control and status (high) */
#define USB_CNT0_CNTL           0x40089058  /**< Number of RX bytes in EP0 FIFO
                                                 or number of bytes in EP{1-5}
                                                 OUT FIFO (low) */
#define USB_CNT0                0x40089058  /**< Number of RX bytes in EP0 FIFO
                                                 (Alias for USB_CNT0_CNTL) */
#define USB_CNTL                0x40089058  /**< Number of bytes in EP{1-5}
                                                 OUT FIFO (low)
                                                 (Alias for USB_CNT0_CNTL) */
#define USB_CNTH                0x4008905C  /**< Number of bytes in EP{1-5}
                                                 OUT FIFO (high) */
#define USB_F0                  0x40089080  /**< Endpoint-0 FIFO */
#define USB_F1                  0x40089088  /**< Endpoint-1 FIFO */
#define USB_F2                  0x40089090  /**< Endpoint-2 FIFO */
#define USB_F3                  0x40089098  /**< Endpoint-3 FIFO */
#define USB_F4                  0x400890A0  /**< Endpoint-4 FIFO */
#define USB_F5                  0x400890A8  /**< Endpoint-5 FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_ADDR Register Bit-Masks
 * @{
 */
#define USB_ADDR_UPDATE         0x00000080  /**< 1 while address updating */
#define USB_ADDR_USBADDR        0x0000007F  /**< Device address */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_POW Register Bit-Masks
 * @{
 */
#define USB_POW_ISO_WAIT_SOF    0x00000080  /**< 1 until SOF received - ISO only */
#define USB_POW_RST             0x00000008  /**< 1 During reset signaling */
#define USB_POW_RESUME          0x00000004  /**< Remote wakeup resume signalling */
#define USB_POW_SUSPEND         0x00000002  /**< Suspend mode entered */
#define USB_POW_SUSPEND_EN      0x00000001  /**< Suspend enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_IIF Register Bit-Masks
 * @{
 */
#define USB_IIF_INEP5IF         0x00000020  /**< IN EP5 Interrupt flag */
#define USB_IIF_INEP4IF         0x00000010  /**< IN EP4 Interrupt flag */
#define USB_IIF_INEP3IF         0x00000008  /**< IN EP3 Interrupt flag */
#define USB_IIF_INEP2IF         0x00000004  /**< IN EP2 Interrupt flag */
#define USB_IIF_INEP1IF         0x00000002  /**< IN EP1 Interrupt flag */
#define USB_IIF_EP0IF           0x00000001  /**< EP0 Interrupt flag */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_OIF Register Bit-Masks
 * @{
 */
#define USB_OIF_OUTEP5IF        0x00000020  /**< OUT EP5 Interrupt flag */
#define USB_OIF_OUTEP4IF        0x00000010  /**< OUT EP4 Interrupt flag */
#define USB_OIF_OUTEP3IF        0x00000008  /**< OUT EP3 Interrupt flag */
#define USB_OIF_OUTEP2IF        0x00000004  /**< OUT EP2 Interrupt flag */
#define USB_OIF_OUTEP1IF        0x00000002  /**< OUT EP1 Interrupt flag */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CIF Register Bit-Masks
 * @{
 */
#define USB_CIF_SOFIF           0x00000008  /**< Start-of-frame interrupt flag */
#define USB_CIF_RSTIF           0x00000004  /**< Reset interrupt flag */
#define USB_CIF_RESUMEIF        0x00000002  /**< Resume interrupt flag */
#define USB_CIF_SUSPENDIF       0x00000001  /**< Suspend interrupt flag */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_IIE Register Bit-Masks
 * @{
 */
#define USB_IIE_INEP5IE         0x00000020  /**< IN EP5 interrupt enable */
#define USB_IIE_INEP4IE         0x00000010  /**< IN EP4 interrupt enable */
#define USB_IIE_INEP3IE         0x00000008  /**< IN EP3 interrupt enable */
#define USB_IIE_INEP2IE         0x00000004  /**< IN EP2 interrupt enable */
#define USB_IIE_INEP1IE         0x00000002  /**< IN EP1 interrupt enable */
#define USB_IIE_EP0IE           0x00000001  /**< EP0 interrupt enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_OIE Register Bit-Masks
 * @{
 */
#define USB_OIE_OUTEP5IE        0x00000020  /**< OUT EP5 interrupt enable */
#define USB_OIE_OUTEP4IE        0x00000010  /**< OUT EP4 interrupt enable */
#define USB_OIE_OUTEP3IE        0x00000008  /**< OUT EP3 interrupt enable */
#define USB_OIE_OUTEP2IE        0x00000004  /**< OUT EP2 interrupt enable */
#define USB_OIE_OUTEP1IE        0x00000002  /**< OUT EP1 interrupt enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CIE Register Bit-Masks
 * @{
 */
#define USB_CIE_SOFIE           0x00000008  /**< Start-of-frame interrupt enable */
#define USB_CIE_RSTIE           0x00000004  /**< Reset interrupt enable */
#define USB_CIE_RESUMEIE        0x00000002  /**< Resume interrupt enable */
#define USB_CIE_SUSPENDIE       0x00000001  /**< Suspend interrupt enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_FRML Register Bit-Masks
 * @{
 */
#define USB_FRML_FRAME          0x000000FF  /**< Low byte of 11-bit frame number */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_FRMH Register Bit-Masks
 * @{
 */
#define USB_FRMH_FRAME          0x00000007  /**< 3 MSBs of 11-bit frame number */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_INDEX Register Bit-Masks
 * @{
 */
#define USB_INDEX_USBINDEX      0x0000000F  /**< Endpoint selected */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CTRL Register Bit-Masks
 * @{
 */
#define USB_CTRL_PLL_LOCKED     0x00000080  /**< PLL locked status */
#define USB_CTRL_PLL_EN         0x00000002  /**< 48-MHz USB PLL enable */
#define USB_CTRL_USB_EN         0x00000001  /**< USB enable */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_MAXI Register Bit-Masks
 * @{
 */
#define USB_MAXI_USBMAXI        0x000000FF  /**< Maximum packet size */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CS0_CSIL Register Bit-Masks
 * @{
 */
/** Listed as reserved in the UG, is this right? */
#define USB_CS0_CLR_SETUP_END                       0x00000080

/** Deassert OUTPKT_RDY bit of this register or reset the data toggle to 0 */
#define USB_CS0_CSIL_CLR_OUTPKT_RDY_or_CLR_DATA_TOG 0x00000040
#define USB_CS0_CLR_OUTPKT_RDY                      0x00000040
#define USB_CSIL_CLR_DATA_TOG                       0x00000040

/**
 * Set this bit to 1 to terminate the current transaction or
 * is set when a STALL handshake has been sent
 */
#define USB_CS0_CSIL_SEND_STALL_or_SENT_STALL       0x00000020
#define USB_CS0_SEND_STALL                          0x00000020
#define USB_CSIL_SENT_STALL                         0x00000020

/**
 * Is set if the control transfer ends due to a premature end-of-control
 * transfer or set to 1 to make the USB controller reply with a STALL handshake
 * when receiving IN tokens
 */
#define USB_CS0_CSIL_SETUP_END_or_SEND_STALL        0x00000010
#define USB_CS0_SETUP_END                           0x00000010
#define USB_CSIL_SEND_STALL                         0x00000010

/**
 * Signal the end of a data transfer or set to 1 to flush next packet that
 * is ready to transfer from the IN FIFO
 */
#define USB_CS0_CSIL_DATA_END_or_FLUSH_PACKET       0x00000008
#define USB_CS0_DATA_END                            0x00000008
#define USB_CSIL_FLUSH_PACKET                       0x00000008

/**
 * Set when a STALL handshake is sent or set if an IN token is received when
 * INPKT_RDY = 0, and a zero-length data packet is transmitted in response to
 * the IN token. In bulk/interrupt mode, this bit is set when a NAK is returned
 * in response to an IN token
 */
#define USB_CS0_CSIL_SENT_STALL_or_UNDERRUN         0x00000004
#define USB_CS0_SENT_STALL                          0x00000004
#define USB_CSIL_UNDERRUN                           0x00000004

/**
 * Data packet has been loaded into the EP0 FIFO or at least one packet in the
 * IN FIFO
 */
#define USB_CS0_CSIL_INPKT_RDY_or_PKT_PRESENT       0x00000002
#define USB_CS0_INPKT_RDY                           0x00000002
#define USB_CSIL_PKT_PRESENT                        0x00000002

/** Data packet received or data packet has been loaded into the IN FIFO */
#define USB_CS0_CSIL_OUTPKT_RDY_or_INPKT_RDY        0x00000001
#define USB_CS0_OUTPKT_RDY                          0x00000001
#define USB_CSIL_INPKT_RDY                          0x00000001

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CSIH Register Bit-Masks
 * @{
 */
#define USB_CSIH_AUTISET        0x00000080  /**< Auto-assert INPKT_RDY */
#define USB_CSIH_ISO            0x00000040  /**< Selects IN endpoint type */
#define USB_CSIH_FORCE_DATA_TOG 0x00000008  /**< IN EP force data toggle switch */
#define USB_CSIH_IN_DBL_BUF     0x00000001  /**< Double buffering enable (IN FIFO) */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_MAXO Register Bit-Masks
 * @{
 */
#define USB_MAXO_USBMAXO        0x000000FF  /**< Maximum packet size */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CSOL Register Bit-Masks
 * @{
 */
#define USB_CSOL_CLR_DATA_TOG   0x00000080  /**< Setting resets data toggle to 0 */
#define USB_CSOL_SENT_STALL     0x00000040  /**< STALL handshake sent */
#define USB_CSOL_SEND_STALL     0x00000020  /**< Reply with STALL to OUT tokens */
#define USB_CSOL_FLUSH_PACKET   0x00000010  /**< Flush next packet read from OUT FIFO */
#define USB_CSOL_DATA_ERROR     0x00000008  /**< CRC or bit-stuff error in RX packet */
#define USB_CSOL_OVERRUN        0x00000004  /**< OUT packet can not be loaded into OUT FIFO */
#define USB_CSOL_FIFO_FULL      0x00000002  /**< OUT FIFO full */
#define USB_CSOL_OUTPKT_RDY     0x00000001  /**< OUT packet read in OUT FIFO */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CSOH Register Bit-Masks
 * @{
 */
#define USB_CSOH_AUTOCLEAR      0x00000080  /**< Auto-clear OUTPKT_RDY */
#define USB_CSOH_ISO            0x00000040  /**< Selects OUT endpoint type */
#define USB_CSOH_OUT_DBL_BUF    0x00000001  /**< Double buffering enable (OUT FIFO) */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CNT0_CNTL Register Bit-Masks
 * @{
 */
#define USB_CNT0_CNTL_USBCNT0    0x0000003F /**< Number of RX bytes in EP0 FIFO */
#define USB_CNT0_USBCNT0         0x0000003F

#define USB_CNT0_CNTL_USBCNT_5_0 0x0000003F /**< 6 LSBs of the number of RX
                                                 bytes in EP1-5 OUT FIFO */
#define USB_CNTL_USBCNT_5_0      0x0000003F
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_CNTH Register Bit-Masks
 * @{
 */
#define USB_CNTH_USBCNT         0x00000007  /**< 3 MSBs of RX byte number */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB_F[0-5] Register Bit-Masks
 * @{
 */
#define USB_F0_USBF0            0x000000FF  /**< Endpoint 0 FIFO mask */
#define USB_F1_USBF1            0x000000FF  /**< Endpoint 1 FIFO mask */
#define USB_F2_USBF2            0x000000FF  /**< Endpoint 2 FIFO mask */
#define USB_F3_USBF3            0x000000FF  /**< Endpoint 3 FIFO mask */
#define USB_F4_USBF4            0x000000FF  /**< Endpoint 4 FIFO mask */
#define USB_F5_USBF5            0x000000FF  /**< Endpoint 5 FIFO mask */
/** @} */

#endif /* USB_REGS_H_ */

/**
 * @}
 * @}
 */
