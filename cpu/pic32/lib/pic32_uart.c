/*
 * Contiki PIC32 Port project
 * 
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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

/**
 * \addtogroup pic32 PIC32 Contiki Port
 *
 * @{
 */

/**
 * \file   pic32_uart.c
 * \brief  UART Interface for PIC32MX (pic32mx795f512l)
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */
 
#define __UART_CODE_TEST__ 0

#if __UART_CODE_TEST__
#define __USE_UART__                    1
#define __USE_UART_PORT1A__             1
#define __USE_UART_PORT1B__             1
#define __USE_UART_PORT2A__             1
#define __USE_UART_PORT2B__             1
#define __USE_UART_PORT3A__             1
#define __USE_UART_PORT3B__             1
#endif /* __UART_CODE_TEST__ */

#ifdef __USE_UART__

#include <pic32_uart.h>
#include <pic32_clock.h>
#include <pic32_irq.h>

#include <p32xxxx.h>

#include "contiki.h"

#include "dev/leds.h"

/*---------------------------------------------------------------------------*/
#define UART_PORT_INIT_XA(XX, YY, ZZ)                                                                           \
  int8_t                                                                                                        \
  pic32_uart##XX##A_init(uint32_t baudrate, uint16_t byte_format)                                               \
  {                                                                                                             \
    /* Disable Interrupts: RX, TX, ERR */                                                                       \
    IEC##ZZ##CLR = _IEC##ZZ##_U##XX##AEIE_MASK | _IEC##ZZ##_U##XX##ATXIE_MASK | _IEC##ZZ##_U##XX##ARXIE_MASK;   \
    IFS##ZZ##CLR = _IFS##ZZ##_U##XX##AEIF_MASK | _IFS##ZZ##_U##XX##ATXIF_MASK | _IFS##ZZ##_U##XX##ARXIF_MASK;   \
                                                                                                                \
    /* Clear thant Set Pri and Sub priority */                                                                  \
    IPC##YY##CLR = _IPC##YY##_U##XX##AIP_MASK | _IPC##YY##_U##XX##AIS_MASK;                                     \
    IPC##YY##SET = (6 << _IPC##YY##_U##XX##AIP_POSITION) | (0 << _IPC##YY##_U##XX##AIS_POSITION);               \
                                                                                                                \
    /* Mode Register Reset (this also stops UART) */                                                            \
    U##XX##AMODE = 0;                                                                                           \
                                                                                                                \
    /* Use BRGH = 1: 4 divisor  */                                                                              \
    U##XX##AMODESET = _U##XX##AMODE_BRGH_MASK;                                                                  \
    U##XX##ABRG  = pic32_clock_calculate_brg(4, baudrate);                                                      \
                                                                                                                \
    U##XX##AMODESET = byte_format & 0x07; /* Number of bit, Parity and Stop bits */                             \
                                                                                                                \
    /* Status bits */                                                                                           \
    U##XX##ASTA = 0;                                                                                            \
    U##XX##ASTASET = _U##XX##ASTA_URXEN_MASK | _U##XX##ASTA_UTXEN_MASK; /* Enable RX and TX */                  \
                                                                                                                \
    IEC##ZZ##SET = _IEC##ZZ##_U##XX##ARXIE_MASK;                                                                \
                                                                                                                \
    /* Enable UART port */                                                                                      \
    U##XX##AMODESET = _U##XX##AMODE_UARTEN_MASK;                                                                \
                                                                                                                \
    return UART_NO_ERROR;                                                                                       \
  }
/*---------------------------------------------------------------------------*/
#define UART_PORT_INIT_XB(XX, YY, ZZ)                                                                           \
  int8_t                                                                                                        \
  pic32_uart##XX##B_init(uint32_t baudrate, uint16_t byte_format)                                               \
  {                                                                                                             \
    /* Disable Interrupts: RX, TX, ERR */                                                                       \
    IEC##ZZ##CLR = _IEC##ZZ##_U##XX##BEIE_MASK | _IEC##ZZ##_U##XX##BTXIE_MASK | _IEC##ZZ##_U##XX##BRXIE_MASK;   \
    IFS##ZZ##CLR = _IFS##ZZ##_U##XX##BEIF_MASK | _IFS##ZZ##_U##XX##BTXIF_MASK | _IFS##ZZ##_U##XX##BRXIF_MASK;   \
                                                                                                                \
    /* Clear thant Set Pri and Sub priority */                                                                  \
    IPC##YY##CLR = _IPC##YY##_U##XX##BIP_MASK | _IPC##YY##_U##XX##BIS_MASK;                                     \
    IPC##YY##SET = (6 << _IPC##YY##_U##XX##BIP_POSITION) | (0 << _IPC##YY##_U##XX##BIS_POSITION);               \
                                                                                                                \
    /* Mode Register Reset (this also stops UART) */                                                            \
    U##XX##BMODE = 0;                                                                                           \
                                                                                                                \
    /* Use BRGH = 1: 4 divisor  */                                                                              \
    U##XX##BMODESET = _U##XX##BMODE_BRGH_MASK;                                                                  \
    U##XX##BBRG  = pic32_clock_calculate_brg(4, baudrate);                                                      \
                                                                                                                \
    U##XX##BMODESET = byte_format & 0x07; /* Number of bit, Parity and Stop bits */                             \
                                                                                                                \
    /* Status bits */                                                                                           \
    U##XX##BSTA = 0;                                                                                            \
    U##XX##BSTASET = _U##XX##BSTA_URXEN_MASK | _U##XX##BSTA_UTXEN_MASK; /* Enable RX and TX */                  \
                                                                                                                \
    IEC##ZZ##SET = _IEC##ZZ##_U##XX##BRXIE_MASK;                                                                \
                                                                                                                \
    /* Enable UART port */                                                                                      \
    U##XX##BMODESET = _U##XX##BMODE_UARTEN_MASK;                                                                \
                                                                                                                \
    return UART_NO_ERROR;                                                                                       \
  }
/*---------------------------------------------------------------------------*/
#define UART_PORT(XX, YY)                        \
                                                 \
  int8_t                                         \
  pic32_uart##XX##_write(uint8_t data)           \
  {                                              \
    volatile uint8_t wait;                       \
                                                 \
    do {                                         \
      wait = U##XX##STAbits.UTXBF;               \
    } while(wait);                               \
                                                 \
    U##XX##TXREG = data;                         \
                                                 \
    return UART_NO_ERROR;                        \
  }
/*---------------------------------------------------------------------------*/

#ifdef __USE_UART_PORT1A__
UART_PORT(1A, 0)
UART_PORT_INIT_XA(1, 6, 0)
#endif /* __USE_UART_PORT1A__ */

#ifdef __USE_UART_PORT1B__
UART_PORT(1B, 2)
UART_PORT_INIT_XB(1, 12, 2)
#endif /* __USE_UART_PORT1B__ */

#ifdef __USE_UART_PORT2A__
UART_PORT(2A, 1)
UART_PORT_INIT_XA(2, 7, 1)
#endif /* __USE_UART_PORT2A__ */

#ifdef __USE_UART_PORT2B__
UART_PORT(2B, 2)
UART_PORT_INIT_XB(2, 12, 2)
#endif /* __USE_UART_PORT2B__ */

#ifdef __USE_UART_PORT3A__
UART_PORT(3A, 1)
UART_PORT_INIT_XA(3, 8, 1)
#endif /* __USE_UART_PORT3A__ */

#ifdef __USE_UART_PORT3B__
UART_PORT(3B, 2)
UART_PORT_INIT_XB(3, 12, 2)
#endif /* __USE_UART_PORT3B__ */

#endif /* __USE_UART__ */

/** @} */
