/***************************************************************************//**
 *   @file   RDKRL78G14.h
 *   @brief  Header file of the RDKRL78G14 Driver.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: $WCREV$
*******************************************************************************/

#ifndef __RDKRL78G14_H__
#define __RDKRL78G14_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <ior5f104pj.h>     // R5F104PJ definitions
#include <ior5f104pj_ext.h> // Declarations of extended SFR registers
#include <intrinsics.h>     // Declarations of intrinsic functions

/******************************************************************************/
/************************* Operations Definitions *****************************/
/******************************************************************************/

#define NOP	__no_operation();       // No operation
#define EI	__enable_interrupt();   // Enable interrupts
#define DI	__disable_interrupt();  // Disable interrupts

/******************************************************************************/
/**************************** GPIO Definitions ********************************/
/******************************************************************************/

/* External Reset Pin - P130 */
#define EXTRST_PIN		(1 << 0)
#define EXTRST_LOW		P13 &= ~EXTRST_PIN
#define EXTRST_HIGH		P13 |= EXTRST_PIN

/* ST7579 CS Pin - P145 */
#define ST7579_CS_PIN	(1 << 5)
#define ST7579_CS_OUT	PM14 &= ~ST7579_CS_PIN
#define ST7579_CS_LOW	P14 &= ~ST7579_CS_PIN
#define ST7579_CS_HIGH	P14 |=  ST7579_CS_PIN

/* ST7579 RS Pin - P146 */
#define ST7579_RS_PIN	(1 << 6)
#define ST7579_RS_OUT	PM14 &= ~ST7579_RS_PIN
#define ST7579_RS_LOW	P14 &= ~ST7579_RS_PIN
#define ST7579_RS_HIGH	P14 |=  ST7579_RS_PIN

/* PMOD1 CS Pin - P83 */
#define PMOD1_CS_PIN	(1 << 3)
#define PMOD1_CS_OUT	PM8 &= ~PMOD1_CS_PIN
#define PMOD1_CS_LOW	P8 &= ~PMOD1_CS_PIN
#define PMOD1_CS_HIGH	P8 |=  PMOD1_CS_PIN

/* PMOD1 MISO Pin */
#define PMOD1_MISO_PIN	(1 << 1)
#define PMOD1_MISO      ((P7 & PMOD1_MISO_PIN) >> 1)

/* PMOD1 GPIO1 Pin - P47 */
#define PMOD1_GPIO1_PIN       (1 << 7)
#define PMOD1_GPIO1_PIN_OUT   PM4 &= ~PMOD1_GPIO1_PIN;
#define PMOD1_GPIO1_LOW       P4  &= ~PMOD1_GPIO1_PIN;
#define PMOD1_GPIO1_HIGH      P4  |=  PMOD1_GPIO1_PIN;

/* PMOD1 GPIO2 Pin - P46 */
#define PMOD1_GPIO2_PIN       (1 << 6)
#define PMOD1_GPIO2_PIN_OUT   PM4 &= ~PMOD1_GPIO2_PIN;
#define PMOD1_GPIO2_LOW       P4  &= ~PMOD1_GPIO2_PIN;
#define PMOD1_GPIO2_HIGH      P4  |=  PMOD1_GPIO2_PIN;

/* PMOD1 GPIO3 Pin - P110 */
#define PMOD1_GPIO3_PIN       (1 << 0)
#define PMOD1_GPIO3_PIN_OUT   PM11 &= ~PMOD1_GPIO3_PIN;
#define PMOD1_GPIO3_LOW       P11  &= ~PMOD1_GPIO3_PIN;
#define PMOD1_GPIO3_HIGH      P11  |=  PMOD1_GPIO3_PIN;

/* PMOD1 GPIO4 Pin - P111 */
#define PMOD1_GPIO4_PIN       (1 << 1)
#define PMOD1_GPIO4_PIN_OUT   PM11 &= ~PMOD1_GPIO4_PIN;
#define PMOD1_GPIO4_LOW       P11  &= ~PMOD1_GPIO4_PIN;
#define PMOD1_GPIO4_HIGH      P11  |=  PMOD1_GPIO4_PIN;

/* PMOD2 CS Pin - P82 */
#define PMOD2_CS_PIN	(1 << 2)
#define PMOD2_CS_OUT	PM8 &= ~PMOD2_CS_PIN
#define PMOD2_CS_LOW	P8 &= ~PMOD2_CS_PIN
#define PMOD2_CS_HIGH	P8 |=  PMOD2_CS_PIN

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Resets the RDKRL78G14. */
void RDKRL78G14_Reset(void);

/*! Initializes the RDKRL78G14. */
char RDKRL78G14_Init(void);

#endif // __RDKRL78G14_H__
