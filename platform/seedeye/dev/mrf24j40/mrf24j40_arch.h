/*
 * Contiki SeedEye Platform project
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
 * \addtogroup mrf24j40 MRF24J40 Driver
 *
 * @{
 */

/** 
 * \file   mrf24j40_arch.h
 * \brief  MRF24J40 Specific Arch Conf
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifndef __MRF24J40_ARCH_H__
#define __MRF24J40_ARCH_H__

#include "p32xxxx.h"

#include <stdint.h>

#include "dev/radio.h"

/* Pin Mapping */
#define MRF24J40_RESETn                     PORTGbits.RG15
#define MRF24J40_INT                        PORTAbits.RA15
#define MRF24J40_CSn                        PORTFbits.RF12
#define MRF24J40_WAKE                       PORTGbits.RG12

/* Pin Tri-States */
#define MRF24J40_TRIS_RESETn                TRISGbits.TRISG1
#define MRF24J40_TRIS_INT                   TRISAbits.TRISA15
#define MRF24J40_TRIS_CSn                   TRISFbits.TRISF12
#define MRF24J40_TRIS_WAKE                  TRISGbits.TRISG12

/* RESET low/high */
#define MRF24J40_HARDRESET_LOW()            MRF24J40_RESETn = 0
#define MRF24J40_HARDRESET_HIGH()           MRF24J40_RESETn = 1
#define MRF24J40_CSn_LOW()                  MRF24J40_CSn = 0
#define MRF24J40_CSn_HIGH()                 MRF24J40_CSn = 1

/* Spi port Mapping */
#ifdef __USE_MRF24J40_SPI_PORT_1__
#define MRF24J40_SPI_PORT_INIT  pic32_spi1_init
#define MRF24J40_SPI_PORT_WRITE pic32_spi1_write
#define MRF24J40_SPI_PORT_READ  pic32_spi1_read
#elif defined __USE_MRF24J40_SPI_PORT_1A__
#define MRF24J40_SPI_PORT_INIT  pic32_spi1A_init
#define MRF24J40_SPI_PORT_WRITE pic32_spi1A_write
#define MRF24J40_SPI_PORT_READ  pic32_spi1A_read
#elif defined __USE_MRF24J40_SPI_PORT_2A__
#define MRF24J40_SPI_PORT_INIT  pic32_spi2A_init
#define MRF24J40_SPI_PORT_WRITE pic32_spi2A_write
#define MRF24J40_SPI_PORT_READ  pic32_spi2A_read
#elif defined __USE_MRF24J40_SPI_PORT_3A__
#define MRF24J40_SPI_PORT_INIT  pic32_spi3A_init
#define MRF24J40_SPI_PORT_WRITE pic32_spi3A_write
#define MRF24J40_SPI_PORT_READ  pic32_spi3A_read
#else
#define MRF24J40_SPI_PORT_INIT  pic32_spi3A_init
#define MRF24J40_SPI_PORT_WRITE pic32_spi3A_write
#define MRF24J40_SPI_PORT_READ  pic32_spi3A_read
#endif

/* IRC Configuration */
#define MRF24J40_ISR()                      ISR(_EXTERNAL_4_VECTOR)
#define MRF24J40_INTERRUPT_FLAG_SET()       IFS0SET = _IFS0_INT4IF_MASK
#define MRF24J40_INTERRUPT_FLAG_CLR()       IFS0CLR = _IFS0_INT4IF_MASK
#define MRF24J40_INTERRUPT_ENABLE_SET()     IEC0SET = _IEC0_INT4IE_MASK
#define MRF24J40_INTERRUPT_ENABLE_CLR()     IEC0CLR = _IEC0_INT4IE_MASK
#define MRF24J40_INTERRUPT_ENABLE_STAT()    IEC0bits.INT4IE

#define MRF24J40_PINDIRECTION_INIT()        \
do {                                        \
    MRF24J40_TRIS_RESETn = 0;               \
    MRF24J40_TRIS_INT = 1;                  \
    MRF24J40_TRIS_CSn = 0;                  \
    MRF24J40_TRIS_WAKE = 0;                 \
} while(0)

#define MRF24J40_INTERRUPT_INIT(p, s)                                       \
do {                                                                        \
    MRF24J40_INTERRUPT_ENABLE_CLR();                                        \
    MRF24J40_INTERRUPT_FLAG_CLR();                                          \
    INTCONCLR = _INTCON_INT4EP_MASK;                                        \
    IPC4CLR = _IPC4_INT4IP_MASK | _IPC4_INT4IS_MASK;                        \
    IPC4SET = (p << _IPC4_INT4IP_POSITION) | (s << _IPC4_INT4IS_POSITION);  \
    MRF24J40_INTERRUPT_ENABLE_SET();                                        \
} while(0)

#endif /* __MRF24J40_ARCH_H__ */

/** @} */
