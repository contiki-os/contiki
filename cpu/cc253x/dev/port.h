/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */


#ifndef PORT_H_
#define PORT_H_

#include "cc253x.h"
#include "sfr-bits.h"
/*---------------------------------------------------------------------------*/
/* Use these to configure your platform's hardware */
#define PORT_FUNC_GPIO(port,pin)     PORT_FUNC_GPIO_X(port,pin)
#define PORT_FUNC_PER(port,pin)      PORT_FUNC_PER_X(port,pin)
#define PORT0_ANALOG_IO(pin)         PORT0_ANALOG_IO_X(pin)
#define PORT0_DIGITAL_IO(pin)        PORT0_DIGITAL_IO_X(pin)
#define PORT_SET(port,pin)           PORT_SET_X(port,pin)
#define PORT_CLEAR(port,pin)         PORT_CLEAR_X(port,pin) PORT_CLEAR_X(port,pin)
#define PORT_TOGGLE(port,pin)        PORT_TOGGLE_X(port,pin) PORT_TOGGLE_X(port,pin)
#define PORT_READ(port,pin)          PORT_READ_X(port,pin)
#define PORT_WRITE(port,pin,v)       PORT_WRITE_X(port,pin,v)
#define PORT_DIR_OUTPUT(port,pin)    PORT_DIR_OUTPUT_X(port,pin)
#define PORT_DIR_INPUT(port,pin)     PORT_DIR_INPUT_X(port,pin)
#define PORT_IRQ_ENABLE(port,pin)    PORT_IRQ_ENABLE_X(port,pin)
#define PORT_IRQ_DISABLE(port,pin)   PORT_IRQ_DISABLE_X(port,pin)
#define PORT_IRQ_ENABLED(port,pin)   PORT_IRQ_ENABLED_X(port,pin)
#define PORT_IRQ_CHECK(port,pin)     PORT_IRQ_CHECK_X(port,pin)
#define PORT_IRQ_EDGE_FALL(port,pin) PORT_IRQ_EDGE_FALL_X(port,pin)
#define PORT_IRQ_EDGE_RISE(port,pin) PORT_IRQ_EDGE_RISE_X(port,pin)
#define PORT_IRQ_FLAG_OFF(port,pin)  PORT_IRQ_FLAG_OFF_X(port,pin)
/*---------------------------------------------------------------------------*/
/* Second Round of Macro Substitutions. Normally, you can stop reading here */
/*---------------------------------------------------------------------------*/
#define PORT_FUNC_GPIO_X(port,pin)     do { P##port##SEL &= ~(1 << pin); } while(0)
#define PORT_FUNC_PER_X(port,pin)      do { P##port##SEL |= 1 << pin; } while(0)
#define PORT0_ANALOG_IO_X(port,pin)    do { APCFG |= 1 << pin; } while(0)
#define PORT0_DIGITAL_IO_X(port,pin)   do { APCFG &= ~(1 << pin); } while(0)
#define PORT_SET_X(port,pin)           do { P##port##_##pin = 1; } while(0)
#define PORT_CLEAR_X(port,pin)         do { P##port##_##pin = 0; } while(0)
#define PORT_TOGGLE_X(port,pin)        do { P##port##_##pin ^= 1; } while(0)
#define PORT_READ_X(port,pin)          (P##port##_##pin)
#define PORT_WRITE_X(port,pin,v)       do { P##port##_##pin = v;} while(0)
#define PORT_DIR_OUTPUT_X(port,pin)    do { P##port##DIR |= 1 << pin; } while(0)
#define PORT_DIR_INPUT_X(port,pin)     do { P##port##DIR &= ~(1 << pin); } while(0)
#define PORT_IRQ_ENABLE_X(port,pin)    do { \
  P##port##IEN |= 1 << pin; \
  PORT##port##_IRQ_ENABLE(); \
} while(0)
#define PORT_IRQ_DISABLE_X(port,pin)   do { \
  P##port##IEN &= ~(1 << pin); \
  PORT##port##_IRQ_DISABLE(); \
} while(0)
#define PORT_IRQ_ENABLED_X(port,pin)   (P##port##IEN & (1 << pin))
#define PORT_IRQ_CHECK_X(port,pin)     (P##port##IFG & (1 << pin))
#define PORT_IRQ_EDGE_FALL_X(port,pin) PORT##port##_IRQ_EDGE_FALL(pin)
#define PORT_IRQ_EDGE_RISE_X(port,pin) PORT##port##_IRQ_EDGE_RISE(pin)
#define PORT_IRQ_FLAG_OFF_X(port,pin)  do { \
  P##port##IFG &= ~(1 << pin); \
  P##port##IF = 0; \
} while(0)
/*---------------------------------------------------------------------------*/
/* To handle SFR diversities
 * - P0IE is in IEN1, which is bit-addressable,
 *   P1IE and P2IE are in IEN2, which is not bit-addressable
 * - Edge detection (rising / falling) config is uniform for all pins in
 *   P0 and P2. For P1, low and high nibble bits are configured separately
 * - Pullup/Pulldown/Tristate is quite different for each port
 *
 * You won't have to invoke these macros directly
 */
#define PORT0_IRQ_ENABLE()  do { P0IE = 1; } while(0)
#define PORT0_IRQ_DISABLE() do { P0IE = 0; } while(0)
#define PORT1_IRQ_ENABLE()  PORT_IRQ_EN_X(1)
#define PORT1_IRQ_DISABLE() PORT_IRQ_DIS_X(1)
#define PORT2_IRQ_ENABLE()  PORT_IRQ_EN_X(2)
#define PORT2_IRQ_DISABLE() PORT_IRQ_DIS_X(2)

#define PORT_IRQ_EN_X(port)  do { IEN2 |= IEN2_P##port##IE; } while(0)
#define PORT_IRQ_DIS_X(port) do { IEN2 &= ~IEN2_P##port##IE; } while(0)
/*---------------------------------------------------------------------------*/
#define PORT0_IRQ_EDGE_FALL(pin) PORT_IRQ_EDGE_F_X(0)
#define PORT0_IRQ_EDGE_RISE(pin) PORT_IRQ_EDGE_R_X(0)
#define PORT1_IRQ_EDGE_FALL(pin) PORT1_##pin##_IRQ_EDGE_F_X()
#define PORT1_IRQ_EDGE_RISE(pin) PORT1_##pin##_IRQ_EDGE_R_X()
#define PORT2_IRQ_EDGE_FALL(pin) PORT_IRQ_EDGE_F_X(2)
#define PORT2_IRQ_EDGE_RISE(pin) PORT_IRQ_EDGE_R_X(2)

/* Ports 0 & 2 */
#define PORT_IRQ_EDGE_F_X(port) do { PICTL |= PICTL_P##port##ICON; } while(0)
#define PORT_IRQ_EDGE_R_X(port) do { PICTL &= ~PICTL_P##port##ICON; } while(0)
/* Port 1 - High Nibble */
#define PORT1_7_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONH; } while(0)
#define PORT1_7_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONH; } while(0)
#define PORT1_6_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONH; } while(0)
#define PORT1_6_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONH; } while(0)
#define PORT1_5_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONH; } while(0)
#define PORT1_5_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONH; } while(0)
#define PORT1_4_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONH; } while(0)
#define PORT1_4_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONH; } while(0)
/* Port 1 - Low Nibble */
#define PORT1_3_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONL; } while(0)
#define PORT1_3_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONL; } while(0)
#define PORT1_2_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONL; } while(0)
#define PORT1_2_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONL; } while(0)
#define PORT1_1_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONL; } while(0)
#define PORT1_1_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONL; } while(0)
#define PORT1_0_IRQ_EDGE_F_X()  do { PICTL |= PICTL_P1ICONL; } while(0)
#define PORT1_0_IRQ_EDGE_R_X()  do { PICTL &= ~PICTL_P1ICONL; } while(0)
/*---------------------------------------------------------------------------*/

#endif /* __PORT_H__ */
