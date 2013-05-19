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
 * \file   debug-uart.h
 * \brief  Debug output redirection to uart.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifdef __USE_UART__

#include <debug-uart.h>
#include <pic32_uart.h>

#include <dev/serial-line.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define DEBUG_UART(XX, YY)                            \
  void                                                \
  _mon_putc(char c)                                   \
  {                                                   \
    pic32_uart##XX##_write(c);                        \
  }                                                   \
                                                      \
  void                                                \
  dbg_setup_uart(unsigned long ubr)                   \
  {                                                   \
    pic32_uart##XX##_init(ubr, 0);                    \
                                                      \
    PRINTF("Initializing debug uart: %lubps\n", ubr); \
  }                                                   \
                                                      \
  UART_INTERRUPT(XX, YY, pic32_uart##XX##_write);

#ifdef __USE_UART_PORT1A_FOR_DEBUG__
DEBUG_UART(1A, 0);
#elif defined  __USE_UART_PORT1B_FOR_DEBUG__
DEBUG_UART(1B, 2);
#elif defined  __USE_UART_PORT2A_FOR_DEBUG__
DEBUG_UART(2A, 1);
#elif defined  __USE_UART_PORT2B_FOR_DEBUG__
DEBUG_UART(2B, 2);
#elif defined  __USE_UART_PORT3A_FOR_DEBUG__
DEBUG_UART(3A, 1);
#elif defined  __USE_UART_PORT3B_FOR_DEBUG__
DEBUG_UART(3B, 2);
#else
DEBUG_UART(1A);
#endif

#endif /* __USE_UART__*/

/** @} */
