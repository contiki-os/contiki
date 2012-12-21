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
 * \file   pic32_spi.c
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

#define __SPI_CODE_TEST__  0

#if __SPI_CODE_TEST__
#define __USE_SPI__             1
#define __USE_SPI_PORT1__       1
#define __USE_SPI_PORT1A__      1
#define __USE_SPI_PORT2A__      1
#define __USE_SPI_PORT3A__      1
#endif /* __SPI_CODE_TEST__ */

#ifdef __USE_SPI__

#include <pic32_spi.h>
#include <pic32_clock.h>
#include <pic32_irq.h>

#include <p32xxxx.h>

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */

#define IS_MASTER(flags)    ((flags) & SPI_MASTER)

/*---------------------------------------------------------------------------*/
#define SPI_PORT(XX, YY)                                    \
  int8_t                                                    \
  pic32_spi##XX##_init(uint32_t baudrate, uint32_t flags)   \
  {                                                         \
                                                            \
    IEC##YY##CLR = _IEC##YY##_SPI##XX##EIE_MASK |           \
                  _IEC##YY##_SPI##XX##TXIE_MASK |           \
                  _IEC##YY##_SPI##XX##RXIE_MASK;            \
                                                            \
    IFS##YY##CLR = _IFS##YY##_SPI##XX##EIF_MASK |           \
                  _IFS##YY##_SPI##XX##TXIF_MASK |           \
                  _IFS##YY##_SPI##XX##RXIF_MASK;            \
                                                            \
    SPI##XX##BRG = pic32_clock_calculate_brg(2, baudrate);  \
                                                            \
    SPI##XX##CON = 0;                                       \
                                                            \
    /* Flag parsing */                                      \
                                                            \
    if(!IS_MASTER(flags)) {                                 \
      return -SPI_ERR_UNIMPLEMENTED;                        \
    }                                                       \
                                                            \
    SPI##XX##CON = flags | _SPI##XX##CON_ON_MASK;           \
                                                            \
    return SPI_NO_ERRORS;                                   \
  }                                                         \
                                                            \
  int8_t                                                    \
  pic32_spi##XX##_close()                                   \
  {                                                         \
    SPI##XX##CONCLR = _SPI##XX##CON_ON_MASK;                \
    return SPI_NO_ERRORS;                                   \
  }                                                         \
                                                            \
  int8_t                                                    \
  pic32_spi##XX##_write(const uint8_t *data, uint32_t len)  \
  {                                                         \
    uint32_t i;                                             \
    uint32_t dummy;                                         \
                                                            \
    for(i = 0; i < len; ++i) {                              \
      SPI##XX##STATCLR = _SPI##XX##STAT_SPIROV_MASK;        \
      while(!SPI##XX##STATbits.SPITBE) {                    \
        ;                                                   \
      }                                                     \
      ASM_DIS_INT; /* fix errata 44 */                      \
      SPI##XX##BUF = data[i];                               \
      ASM_EN_INT;  /* fix errata 44 */                      \
      while(!SPI##XX##STATbits.SPIRBF) {                    \
        ;                                                   \
      }                                                     \
      SPI##XX##STATCLR = _SPI##XX##STAT_SPIROV_MASK;        \
      dummy = SPI##XX##BUF;                                 \
    }                                                       \
                                                            \
    return SPI_NO_ERRORS;                                   \
  }                                                         \
                                                            \
  int8_t                                                    \
  pic32_spi##XX##_read(uint8_t *data, uint32_t len)         \
  {                                                         \
    uint32_t i;                                             \
                                                            \
    for(i = 0; i < len; ++i) {                              \
      SPI##XX##STATCLR = _SPI##XX##STAT_SPIROV_MASK;        \
      while(!SPI##XX##STATbits.SPITBE) {                    \
        ;                                                   \
      }                                                     \
      SPI##XX##BUF = 0;                                     \
      while(!SPI##XX##STATbits.SPIRBF) {                    \
        ;                                                   \
      }                                                     \
      SPI##XX##STATCLR = _SPI##XX##STAT_SPIROV_MASK;        \
      data[i] = SPI##XX##BUF & 0x00FF;                      \
    }                                                       \
                                                            \
    return SPI_NO_ERRORS;                                   \
  }
/*---------------------------------------------------------------------------*/

#ifdef __USE_SPI_PORT1__
SPI_PORT(1, 0)
#endif /* __USE_SPI_PORT1__ */

#ifdef __USE_SPI_PORT1A__
SPI_PORT(1A, 0)
#endif /* __USE_SPI_PORT1A__ */

#ifdef __USE_SPI_PORT2A__
SPI_PORT(2A, 1)
#endif /* __USE_SPI_PORT2A__ */

#ifdef __USE_SPI_PORT3A__
SPI_PORT(3A, 1)
#endif /* __USE_SPI_PORT3A__ */

#endif /* __USE_SPI__ */

/** @} */
