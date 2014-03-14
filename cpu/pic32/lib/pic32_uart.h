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
 * \file   pic32_uart.h
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

#ifndef INCLUDE_PIC32_UART_H_
#define INCLUDE_PIC32_UART_H_

#ifdef __USE_UART__

#include <pic32_irq.h>

#include <p32xxxx.h>

#include <stdint.h>

#include "dev/leds.h"

#define BAUD2UBR(x)                x

/* Returned Messages */
#define UART_NO_ERROR              0
#define UART_ERR_NO_DATA           1
#define UART_ERR_OVERFLOW          2

#define UART_PORT_DEF(XX)                                                \
  int8_t pic32_uart##XX##_init(uint32_t baudrate, uint16_t byte_format); \
  int8_t pic32_uart##XX##_write(uint8_t data);

#define UART_INTERRUPT(XX, YY, CALLBACK)                                 \
  ISR(_UART_##XX##_VECTOR)                                               \
  {                                                                      \
    volatile uint8_t byte;                                               \
    if(IFS##YY##bits.U##XX##RXIF) {                                      \
      if((U##XX##STAbits.PERR == 0) && (U##XX##STAbits.FERR == 0)) {     \
        CALLBACK(U##XX##RXREG);                                          \
      } else {                                                           \
        byte = U##XX##RXREG; /* NULL READ */                             \
      }                                                                  \
      IFS##YY##CLR = _IFS##YY##_U##XX##RXIF_MASK;                        \
    }                                                                    \
    if(U##XX##STAbits.OERR) {                                            \
      U##XX##STACLR = _U##XX##STA_OERR_MASK;                             \
    }                                                                    \
  }

#ifdef __USE_UART_PORT1A__
UART_PORT_DEF(1A)
#endif /* __USE_UART_PORT1A__ */

#ifdef __USE_UART_PORT1B__
UART_PORT_DEF(1B)
#endif /* __USE_UART_PORT1B__ */

#ifdef __USE_UART_PORT2A__
UART_PORT_DEF(2A)
#endif /* __USE_UART_PORT2A__ */

#ifdef __USE_UART_PORT2B__
UART_PORT_DEF(2B)
#endif /* __USE_UART_PORT2B__ */

#ifdef __USE_UART_PORT3A__
UART_PORT_DEF(3A)
#endif /* __USE_UART_PORT3A__ */

#ifdef __USE_UART_PORT3B__
UART_PORT_DEF(3B)
#endif /* __USE_UART_PORT3B__ */

#endif /* __USE_UART__ */

#endif /* INCLUDE_PIC32_UART_H_ */

/** @} */
