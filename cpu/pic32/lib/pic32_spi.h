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
 * \file   pic32_spi.h
 * \brief  SPI interface for PIC32MX (pic32mx795f512l)
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

#ifndef __INCLUDE_PIC32_SPI_H__
#define __INCLUDE_PIC32_SPI_H__

#ifdef __USE_SPI__

#include <p32xxxx.h>

#include <stdint.h>

/* Returned Messages */
#define SPI_NO_ERRORS               0
#define SPI_ERR_BAD_PORT            1
#define SPI_ERR_BAD_ARGS            2
#define SPI_ERR_BUSY                3
#define SPI_ERR_UNIMPLEMENTED       10

/*
 * Configuration Flags 
 * NOTE: this work under the assumption that all the SPI ports have the same 
 * control register specification. See pic32mx family reference manual.
 */

#define SPI_MASTER                  0b00000000000000000000000000100000
#define SPI_CLOCK_IDLE_HIGH         0b00000000000000000000000001000000
#define SPI_SDO_ON_CLOCK_TO_IDLE    0b00000000000000000000000100000000
#define SPI_SDI_ON_CLOCK_END        0b00000000000000000000001000000000

/* Other Flags */
#define SPI_DEFAULT                 (SPI_MASTER | SPI_SDO_ON_CLOCK_TO_IDLE)

#define SPI_DEF(XX)                                               \
  int8_t pic32_spi##XX##_init(uint32_t baudrate, uint32_t flags); \
  int8_t pic32_spi##XX##_close();                                 \
  int8_t pic32_spi##XX##_write(const uint8_t *data, uint32_t len);\
  int8_t pic32_spi##XX##_read(uint8_t *data, uint32_t len);

#ifdef __USE_SPI_PORT1__
SPI_DEF(1)
#endif /* __USE_SPI_PORT1__ */

#ifdef __USE_SPI_PORT1A__
SPI_DEF(1A)
#endif /* __USE_SPI_PORT1A__ */

#ifdef __USE_SPI_PORT2A__
SPI_DEF(2A)
#endif /* __USE_SPI_PORT2A__ */

#ifdef __USE_SPI_PORT3A__
SPI_DEF(3A)
#endif /* __USE_SPI_PORT3A__ */

#endif /* __USE_SPI__ */

#endif /* __INCLUDE_PIC32_SPI_H__ */

/** @} */
