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
 * \defgroup cc2538-gpio cc2538 General-Purpose I/O
 *
 * Driver for the cc2538 GPIO controller
 * @{
 *
 * \file
 * Header file with register and macro declarations for the cc2538 GPIO module
 */
#ifndef GPIO_H_
#define GPIO_H_

#include "reg.h"

#include <stdint.h>

/**
 * \brief Type definition for callbacks invoked by the GPIO ISRs
 * \param port The port that triggered the GPIO interrupt. \e port is passed
 *        by its numeric representation (Port A:0, B:1 etc). Defines for
 *        these numeric representations are GPIO_x_NUM
 * \param pin The pin that triggered the interrupt, specified by number
 *        (0, 1, ..., 7)
 *
 * This is the prototype of a function pointer passed to
 * gpio_register_callback(). These callbacks are registered on a port/pin
 * basis. When a GPIO port generates an interrupt, if a callback has been
 * registered for the port/pin combination, the ISR will invoke it. The ISR
 * will pass the port/pin as arguments in that call, so that a developer can
 * re-use the same callback for multiple port/pin combinations
 */
typedef void (* gpio_callback_t)(uint8_t port, uint8_t pin);
/*---------------------------------------------------------------------------*/
/** \name Base addresses for the GPIO register instances
 * @{
 */
#define GPIO_A_BASE             0x400D9000 /**< GPIO_A */
#define GPIO_B_BASE             0x400DA000 /**< GPIO_B */
#define GPIO_C_BASE             0x400DB000 /**< GPIO_C */
#define GPIO_D_BASE             0x400DC000 /**< GPIO_D */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Numeric representation of the four GPIO ports
 * @{
 */
#define GPIO_A_NUM              0 /**< GPIO_A: 0 */
#define GPIO_B_NUM              1 /**< GPIO_B: 1 */
#define GPIO_C_NUM              2 /**< GPIO_C: 2 */
#define GPIO_D_NUM              3 /**< GPIO_D: 3 */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name GPIO Manipulation macros
 * @{
 */
/** \brief Set pins with PIN_MASK of port with PORT_BASE to input.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_SET_INPUT(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_DIR) &= ~PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to output.
* \param PORT_BASE GPIO Port register offset
* \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
*/
#define GPIO_SET_OUTPUT(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_DIR) |= PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE high.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_SET_PIN(PORT_BASE, PIN_MASK) \
  do { REG((PORT_BASE | GPIO_DATA) + (PIN_MASK << 2)) = 0xFF; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE low.
* \param PORT_BASE GPIO Port register offset
* \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
*/
#define GPIO_CLR_PIN(PORT_BASE, PIN_MASK) \
  do { REG((PORT_BASE | GPIO_DATA) + (PIN_MASK << 2)) = 0x00; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to detect edge.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_DETECT_EDGE(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IS) &= ~PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to detect level.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_DETECT_LEVEL(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IS) |= PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to trigger an
 * interrupt on both edges.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_TRIGGER_BOTH_EDGES(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IBE) |= PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to trigger an
 * interrupt on single edge (controlled by GPIO_IEV).
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_TRIGGER_SINGLE_EDGE(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IBE) &= ~PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to trigger an
 * interrupt on rising edge.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_DETECT_RISING(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IEV) |= PIN_MASK; } while(0)

/** \brief Set pins with PIN_MASK of port with PORT_BASE to trigger an
 * interrupt on falling edge.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_DETECT_FALLING(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IEV) &= ~PIN_MASK; } while(0)

/** \brief Enable interrupt triggering for pins with PIN_MASK of port with
 * PORT_BASE.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_ENABLE_INTERRUPT(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IE) |= PIN_MASK; } while(0)

/** \brief Disable interrupt triggering for pins with PIN_MASK of port with
 * PORT_BASE.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_DISABLE_INTERRUPT(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_IE) &= ~PIN_MASK; } while(0)

/** \brief Configure the pin to be under peripheral control with PIN_MASK of
 * port with PORT_BASE.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_PERIPHERAL_CONTROL(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_AFSEL) |= PIN_MASK; } while(0)

/** \brief Configure the pin to be software controlled with PIN_MASK of port
 * with PORT_BASE.
 * \param PORT_BASE GPIO Port register offset
 * \param PIN_MASK Pin number mask. Pin 0: 0x01, Pin 1: 0x02 ... Pin 7: 0x80
 */
#define GPIO_SOFTWARE_CONTROL(PORT_BASE, PIN_MASK) \
  do { REG(PORT_BASE | GPIO_AFSEL) &= ~PIN_MASK; } while(0)

/**
 * \brief Converts a pin number to a pin mask
 * \param The pin number in the range [0..7]
 * \return A pin mask which can be used as the PIN_MASK argument of the macros
 * in this category
 */
#define GPIO_PIN_MASK(PIN) (1 << PIN)

/**
 * \brief Converts a port number to the port base address
 * \param The port number in the range 0 - 3. Likely GPIO_X_NUM.
 * \return The base address for the registers corresponding to that port
 * number.
 */
#define GPIO_PORT_TO_BASE(PORT) (GPIO_A_BASE + (PORT << 12))
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO Register offset declarations
 * @{
 */
#define GPIO_DATA               0x00000000  /**< Data register */
#define GPIO_DIR                0x00000400  /**< Data direction register */
#define GPIO_IS                 0x00000404  /**< Interrupt sense */
#define GPIO_IBE                0x00000408  /**< Interrupt both edges */
#define GPIO_IEV                0x0000040C  /**< Interrupt event */
#define GPIO_IE                 0x00000410  /**< Interrupt mask */
#define GPIO_RIS                0x00000414  /**< Interrupt status - raw */
#define GPIO_MIS                0x00000418  /**< Interrupt status - masked */
#define GPIO_IC                 0x0000041C  /**< Interrupt clear */
#define GPIO_AFSEL              0x00000420  /**< Mode control select */
#define GPIO_GPIOLOCK           0x00000520  /**< GPIO commit unlock */
#define GPIO_GPIOCR             0x00000524  /**< GPIO commit */
#define GPIO_PMUX               0x00000700  /**< PMUX register */
#define GPIO_P_EDGE_CTRL        0x00000704  /**< Port edge control */
#define GPIO_USB_CTRL           0x00000708  /**< USB input power-up edge ctrl */
#define GPIO_PI_IEN             0x00000710  /**< Power-up interrupt enable */
#define GPIO_IRQ_DETECT_ACK     0x00000718  /**< IRQ detect ACK - I/O ports */
#define GPIO_USB_IRQ_ACK        0x0000071C  /**< IRQ detect ACK - USB */
#define GPIO_IRQ_DETECT_UNMASK  0x00000720  /**< IRQ detect ACK - masked */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_DATA register bit masks
 * @{
 */
#define GPIO_DATA_DATA          0x000000FF  /**< Input and output data */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_DIR register bit masks
 * @{
 */
#define GPIO_DIR_DIR            0x000000FF  /**< Pin Input (0) / Output (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IS register bit masks
 * @{
 */
#define GPIO_IS_IS              0x000000FF  /**< Detect Edge (0) / Level (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IBE register bit masks
 * @{
 */
#define GPIO_IBE_IBE            0x000000FF  /**< Both Edges (1) / Single (0) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IEV register bit masks
 * @{
 */
#define GPIO_IEV_IEV            0x000000FF  /**< Rising (1) / Falling (0) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IE register bit masks
 * @{
 */
#define GPIO_IE_IE              0x000000FF  /**< Masked (0) / Not Masked (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_RIS register bit masks
 * @{
 */
#define GPIO_RIS_RIS            0x000000FF  /**< Raw interrupt status */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_MIS register bit masks
 * @{
 */
#define GPIO_MIS_MIS            0x000000FF  /**< Masked interrupt status */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IC register bit masks
 * @{
 */
#define GPIO_IC_IC              0x000000FF  /**< Clear edge detection (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_AFSEL register bit masks
 * @{
 */
#define GPIO_AFSEL_AFSEL        0x000000FF  /**< Software (0) / Peripheral (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_GPIOLOCK register bit masks
 * @{
 */
#define GPIO_GPIOLOCK_LOCK      0xFFFFFFFF  /**< Locked (1) / Unlocked (0) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_GPIOCR register bit masks
 * @{
 */
#define GPIO_GPIOCR_CR          0x000000FF  /**< Allow alternate function (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_PMUX register bit masks
 * @{
 */
#define GPIO_PMUX_CKOEN         0x00000080  /**< Clock out enable */
#define GPIO_PMUX_CKOPIN        0x00000010  /**< Decouple control pin select */
#define GPIO_PMUX_DCEN          0x00000008  /**< Decouple control enable */
#define GPIO_PMUX_DCPIN         0x00000001  /**< Decouple control pin select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_P_EDGE_CTRL register bit masks.
 * \brief Rising (0) / Falling (1)
 * @{
 */
#define GPIO_P_EDGE_CTRL_PDIRC7 0x80000000  /**< Port D bit 7 */
#define GPIO_P_EDGE_CTRL_PDIRC6 0x40000000  /**< Port D bit 6 */
#define GPIO_P_EDGE_CTRL_PDIRC5 0x20000000  /**< Port D bit 5 */
#define GPIO_P_EDGE_CTRL_PDIRC4 0x10000000  /**< Port D bit 4 */
#define GPIO_P_EDGE_CTRL_PDIRC3 0x08000000  /**< Port D bit 3 */
#define GPIO_P_EDGE_CTRL_PDIRC2 0x04000000  /**< Port D bit 2 */
#define GPIO_P_EDGE_CTRL_PDIRC1 0x02000000  /**< Port D bit 1 */
#define GPIO_P_EDGE_CTRL_PDIRC0 0x01000000  /**< Port D bit 0 */
#define GPIO_P_EDGE_CTRL_PCIRC7 0x00800000  /**< Port C bit 7 */
#define GPIO_P_EDGE_CTRL_PCIRC6 0x00400000  /**< Port C bit 6 */
#define GPIO_P_EDGE_CTRL_PCIRC5 0x00200000  /**< Port C bit 5 */
#define GPIO_P_EDGE_CTRL_PCIRC4 0x00100000  /**< Port C bit 4 */
#define GPIO_P_EDGE_CTRL_PCIRC3 0x00080000  /**< Port C bit 3 */
#define GPIO_P_EDGE_CTRL_PCIRC2 0x00040000  /**< Port C bit 2 */
#define GPIO_P_EDGE_CTRL_PCIRC1 0x00020000  /**< Port C bit 1 */
#define GPIO_P_EDGE_CTRL_PCIRC0 0x00010000  /**< Port C bit 0 */
#define GPIO_P_EDGE_CTRL_PBIRC7 0x00008000  /**< Port B bit 7 */
#define GPIO_P_EDGE_CTRL_PBIRC6 0x00004000  /**< Port B bit 6 */
#define GPIO_P_EDGE_CTRL_PBIRC5 0x00002000  /**< Port B bit 5 */
#define GPIO_P_EDGE_CTRL_PBIRC4 0x00001000  /**< Port B bit 4 */
#define GPIO_P_EDGE_CTRL_PBIRC3 0x00000800  /**< Port B bit 3 */
#define GPIO_P_EDGE_CTRL_PBIRC2 0x00000400  /**< Port B bit 2 */
#define GPIO_P_EDGE_CTRL_PBIRC1 0x00000200  /**< Port B bit 1 */
#define GPIO_P_EDGE_CTRL_PBIRC0 0x00000100  /**< Port B bit 0 */
#define GPIO_P_EDGE_CTRL_PAIRC7 0x00000080  /**< Port A bit 7 */
#define GPIO_P_EDGE_CTRL_PAIRC6 0x00000040  /**< Port A bit 6 */
#define GPIO_P_EDGE_CTRL_PAIRC5 0x00000020  /**< Port A bit 5 */
#define GPIO_P_EDGE_CTRL_PAIRC4 0x00000010  /**< Port A bit 4 */
#define GPIO_P_EDGE_CTRL_PAIRC3 0x00000008  /**< Port A bit 3 */
#define GPIO_P_EDGE_CTRL_PAIRC2 0x00000004  /**< Port A bit 2 */
#define GPIO_P_EDGE_CTRL_PAIRC1 0x00000002  /**< Port A bit 1 */
#define GPIO_P_EDGE_CTRL_PAIRC0 0x00000001  /**< Port A bit 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_USB_CTRL register bit masks
 * @{
 */
#define GPIO_USB_CTRL_USB_EDGE_CTL 0x00000001  /**< Rising (0) / Falling (1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_PI_IEN register bit masks.
 * \brief Enabled (1) / Disabled (0)
 * @{
 */
#define GPIO_PI_IEN_PDIEN7      0x80000000  /**< Port D bit 7 */
#define GPIO_PI_IEN_PDIEN6      0x40000000  /**< Port D bit 6 */
#define GPIO_PI_IEN_PDIEN5      0x20000000  /**< Port D bit 5 */
#define GPIO_PI_IEN_PDIEN4      0x10000000  /**< Port D bit 4 */
#define GPIO_PI_IEN_PDIEN3      0x08000000  /**< Port D bit 3 */
#define GPIO_PI_IEN_PDIEN2      0x04000000  /**< Port D bit 2 */
#define GPIO_PI_IEN_PDIEN1      0x02000000  /**< Port D bit 1 */
#define GPIO_PI_IEN_PDIEN0      0x01000000  /**< Port D bit 0 */
#define GPIO_PI_IEN_PCIEN7      0x00800000  /**< Port C bit 7 */
#define GPIO_PI_IEN_PCIEN6      0x00400000  /**< Port C bit 6 */
#define GPIO_PI_IEN_PCIEN5      0x00200000  /**< Port C bit 5 */
#define GPIO_PI_IEN_PCIEN4      0x00100000  /**< Port C bit 4 */
#define GPIO_PI_IEN_PCIEN3      0x00080000  /**< Port C bit 3 */
#define GPIO_PI_IEN_PCIEN2      0x00040000  /**< Port C bit 2 */
#define GPIO_PI_IEN_PCIEN1      0x00020000  /**< Port C bit 1 */
#define GPIO_PI_IEN_PCIEN0      0x00010000  /**< Port C bit 0 */
#define GPIO_PI_IEN_PBIEN7      0x00008000  /**< Port B bit 7 */
#define GPIO_PI_IEN_PBIEN6      0x00004000  /**< Port B bit 6 */
#define GPIO_PI_IEN_PBIEN5      0x00002000  /**< Port B bit 5 */
#define GPIO_PI_IEN_PBIEN4      0x00001000  /**< Port B bit 4 */
#define GPIO_PI_IEN_PBIEN3      0x00000800  /**< Port B bit 3 */
#define GPIO_PI_IEN_PBIEN2      0x00000400  /**< Port B bit 2 */
#define GPIO_PI_IEN_PBIEN1      0x00000200  /**< Port B bit 1 */
#define GPIO_PI_IEN_PBIEN0      0x00000100  /**< Port B bit 0 */
#define GPIO_PI_IEN_PAIEN7      0x00000080  /**< Port A bit 7 */
#define GPIO_PI_IEN_PAIEN6      0x00000040  /**< Port A bit 6 */
#define GPIO_PI_IEN_PAIEN5      0x00000020  /**< Port A bit 5 */
#define GPIO_PI_IEN_PAIEN4      0x00000010  /**< Port A bit 4 */
#define GPIO_PI_IEN_PAIEN3      0x00000008  /**< Port A bit 3 */
#define GPIO_PI_IEN_PAIEN2      0x00000004  /**< Port A bit 2 */
#define GPIO_PI_IEN_PAIEN1      0x00000002  /**< Port A bit 1 */
#define GPIO_PI_IEN_PAIEN0      0x00000001  /**< Port A bit 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IRQ_DETECT_ACK register bit masks
 * \brief Detected (1) / Undetected (0)
 * @{
 */
#define GPIO_IRQ_DETECT_ACK_PDIACK7 0x80000000  /**< Port D bit 7 */
#define GPIO_IRQ_DETECT_ACK_PDIACK6 0x40000000  /**< Port D bit 6 */
#define GPIO_IRQ_DETECT_ACK_PDIACK5 0x20000000  /**< Port D bit 5 */
#define GPIO_IRQ_DETECT_ACK_PDIACK4 0x10000000  /**< Port D bit 4 */
#define GPIO_IRQ_DETECT_ACK_PDIACK3 0x08000000  /**< Port D bit 3 */
#define GPIO_IRQ_DETECT_ACK_PDIACK2 0x04000000  /**< Port D bit 2 */
#define GPIO_IRQ_DETECT_ACK_PDIACK1 0x02000000  /**< Port D bit 1 */
#define GPIO_IRQ_DETECT_ACK_PDIACK0 0x01000000  /**< Port D bit 0 */
#define GPIO_IRQ_DETECT_ACK_PCIACK7 0x00800000  /**< Port C bit 7 */
#define GPIO_IRQ_DETECT_ACK_PCIACK6 0x00400000  /**< Port C bit 6 */
#define GPIO_IRQ_DETECT_ACK_PCIACK5 0x00200000  /**< Port C bit 5 */
#define GPIO_IRQ_DETECT_ACK_PCIACK4 0x00100000  /**< Port C bit 4 */
#define GPIO_IRQ_DETECT_ACK_PCIACK3 0x00080000  /**< Port C bit 3 */
#define GPIO_IRQ_DETECT_ACK_PCIACK2 0x00040000  /**< Port C bit 2 */
#define GPIO_IRQ_DETECT_ACK_PCIACK1 0x00020000  /**< Port C bit 1 */
#define GPIO_IRQ_DETECT_ACK_PCIACK0 0x00010000  /**< Port C bit 0 */
#define GPIO_IRQ_DETECT_ACK_PBIACK7 0x00008000  /**< Port B bit 7 */
#define GPIO_IRQ_DETECT_ACK_PBIACK6 0x00004000  /**< Port B bit 6 */
#define GPIO_IRQ_DETECT_ACK_PBIACK5 0x00002000  /**< Port B bit 5 */
#define GPIO_IRQ_DETECT_ACK_PBIACK4 0x00001000  /**< Port B bit 4 */
#define GPIO_IRQ_DETECT_ACK_PBIACK3 0x00000800  /**< Port B bit 3 */
#define GPIO_IRQ_DETECT_ACK_PBIACK2 0x00000400  /**< Port B bit 2 */
#define GPIO_IRQ_DETECT_ACK_PBIACK1 0x00000200  /**< Port B bit 1 */
#define GPIO_IRQ_DETECT_ACK_PBIACK0 0x00000100  /**< Port B bit 0 */
#define GPIO_IRQ_DETECT_ACK_PAIACK7 0x00000080  /**< Port A bit 7 */
#define GPIO_IRQ_DETECT_ACK_PAIACK6 0x00000040  /**< Port A bit 6 */
#define GPIO_IRQ_DETECT_ACK_PAIACK5 0x00000020  /**< Port A bit 5 */
#define GPIO_IRQ_DETECT_ACK_PAIACK4 0x00000010  /**< Port A bit 4 */
#define GPIO_IRQ_DETECT_ACK_PAIACK3 0x00000008  /**< Port A bit 3 */
#define GPIO_IRQ_DETECT_ACK_PAIACK2 0x00000004  /**< Port A bit 2 */
#define GPIO_IRQ_DETECT_ACK_PAIACK1 0x00000002  /**< Port A bit 1 */
#define GPIO_IRQ_DETECT_ACK_PAIACK0 0x00000001  /**< Port A bit 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_USB_IRQ_ACK register bit masks
 * @{
 */
#define GPIO_USB_IRQ_ACK_USBACK 0x00000001  /**< Detected (1) / Not detected (0) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name GPIO_IRQ_DETECT_UNMASK register bit masks.
 *  \brief Detected (1) / Not detected (0)
 * @{
 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK7 0x80000000  /**< Port D bit 7 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK6 0x40000000  /**< Port D bit 6 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK5 0x20000000  /**< Port D bit 5 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK4 0x10000000  /**< Port D bit 4 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK3 0x08000000  /**< Port D bit 3 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK2 0x04000000  /**< Port D bit 2 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK1 0x02000000  /**< Port D bit 1 */
#define GPIO_IRQ_DETECT_UNMASK_PDIACK0 0x01000000  /**< Port D bit 0 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK7 0x00800000  /**< Port C bit 7 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK6 0x00400000  /**< Port C bit 6 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK5 0x00200000  /**< Port C bit 5 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK4 0x00100000  /**< Port C bit 4 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK3 0x00080000  /**< Port C bit 3 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK2 0x00040000  /**< Port C bit 2 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK1 0x00020000  /**< Port C bit 1 */
#define GPIO_IRQ_DETECT_UNMASK_PCIACK0 0x00010000  /**< Port C bit 0 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK7 0x00008000  /**< Port B bit 7 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK6 0x00004000  /**< Port B bit 6 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK5 0x00002000  /**< Port B bit 5 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK4 0x00001000  /**< Port B bit 4 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK3 0x00000800  /**< Port B bit 3 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK2 0x00000400  /**< Port B bit 2 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK1 0x00000200  /**< Port B bit 1 */
#define GPIO_IRQ_DETECT_UNMASK_PBIACK0 0x00000100  /**< Port B bit 0 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK7 0x00000080  /**< Port A bit 7 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK6 0x00000040  /**< Port A bit 6 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK5 0x00000020  /**< Port A bit 5 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK4 0x00000010  /**< Port A bit 4 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK3 0x00000008  /**< Port A bit 3 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK2 0x00000004  /**< Port A bit 2 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK1 0x00000002  /**< Port A bit 1 */
#define GPIO_IRQ_DETECT_UNMASK_PAIACK0 0x00000001  /**< Port A bit 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \brief Initialise the GPIO module */
void gpio_init();

/**
 * \brief Register GPIO callback
 * \param f Pointer to a function to be called when \a pin of \a port
 *          generates an interrupt
 * \param port Associate \a f with this port. \e port must be specified with
 *        its numeric representation (Port A:0, B:1 etc). Defines for these
 *        numeric representations are GPIO_x_NUM
 * \param pin Associate \a f with this pin, which is specified by number
 *        (0, 1, ..., 7)
 */
void gpio_register_callback(gpio_callback_t f, uint8_t port, uint8_t pin);

#endif /* GPIO_H_ */

/**
 * @}
 * @}
 */
