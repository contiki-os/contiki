/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details. 
 *
 *
 */

#ifndef UART1_H
#define UART1_H

#include <stdint.h>

#define UCON      (0)
/* UCON bits */
#define UCON_SAMP     10
#define UCON_SAMP_8X   0
#define UCON_SAMP_16X  1

#define USTAT     (0x04)
#define UDATA     (0x08)
#define URXCON    (0x0c)
#define UTXCON    (0x10)
#define UCTS      (0x14)
#define UBRCNT    (0x18)

#define UART1_BASE (0x80005000)
#define UART2_BASE (0x8000b000)

#define UART1_UCON       ((volatile uint32_t *) ( UART1_BASE + UCON   ))
#define UART1_USTAT      ((volatile uint32_t *) ( UART1_BASE + USTAT  ))
#define UART1_UDATA      ((volatile uint32_t *) ( UART1_BASE + UDATA  ))
#define UART1_URXCON     ((volatile uint32_t *) ( UART1_BASE + URXCON ))
#define UART1_UTXCON     ((volatile uint32_t *) ( UART1_BASE + UTXCON ))
#define UART1_UCTS       ((volatile uint32_t *) ( UART1_BASE + UCTS   ))
#define UART1_UBRCNT     ((volatile uint32_t *) ( UART1_BASE + UBRCNT ))

#define UART2_UCON       ((volatile uint32_t *) ( UART2_BASE + UCON   ))
#define UART2_USTAT      ((volatile uint32_t *) ( UART2_BASE + USTAT  ))
#define UART2_UDATA      ((volatile uint32_t *) ( UART2_BASE + UDATA  ))
#define UART2_URXCON     ((volatile uint32_t *) ( UART2_BASE + URXCON ))
#define UART2_UTXCON     ((volatile uint32_t *) ( UART2_BASE + UTXCON ))
#define UART2_UCTS       ((volatile uint32_t *) ( UART2_BASE + UCTS   ))
#define UART2_UBRCNT     ((volatile uint32_t *) ( UART2_BASE + UBRCNT ))

extern volatile uint32_t  u1_head, u1_tail;
void uart1_putc(char c);
#define uart1_can_get() (*UART1_URXCON > 0)
uint8_t uart1_getc(void);



#endif
