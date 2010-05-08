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
 * $Id$
 */

#ifndef GPIO_H
#define GPIO_H

#define GPIO_PAD_DIR0      ((volatile uint32_t *) 0x80000000)
#define GPIO_PAD_DIR1      ((volatile uint32_t *) 0x80000004)
#define GPIO_DATA0         ((volatile uint32_t *) 0x80000008)
#define GPIO_DATA1         ((volatile uint32_t *) 0x8000000c)
#define GPIO_PAD_PU_EN0    ((volatile uint32_t *) 0x80000010)
#define GPIO_PAD_PU_EN1    ((volatile uint32_t *) 0x80000014)
#define GPIO_FUNC_SEL0     ((volatile uint32_t *) 0x80000018) /* GPIO 15 -  0;  2 bit blocks */
#define GPIO_FUNC_SEL1     ((volatile uint32_t *) 0x8000001c) /* GPIO 16 - 31;  2 bit blocks */
#define GPIO_FUNC_SEL2     ((volatile uint32_t *) 0x80000020) /* GPIO 32 - 47;  2 bit blocks */
#define GPIO_FUNC_SEL3     ((volatile uint32_t *) 0x80000024) /* GPIO 48 - 63;  2 bit blocks */
#define GPIO_DATA_SEL0     ((volatile uint32_t *) 0x80000028) 
#define GPIO_DATA_SEL1     ((volatile uint32_t *) 0x8000002c) 
#define GPIO_PAD_PU_SEL0   ((volatile uint32_t *) 0x80000030)
#define GPIO_PAD_PU_SEL1   ((volatile uint32_t *) 0x80000034)

#define GPIO_DATA_SET0      ((volatile uint32_t *) 0x80000048)
#define GPIO_DATA_SET1      ((volatile uint32_t *) 0x8000004c)
#define GPIO_DATA_RESET0    ((volatile uint32_t *) 0x80000050)
#define GPIO_DATA_RESET1    ((volatile uint32_t *) 0x80000054)
#define GPIO_PAD_DIR_SET0   ((volatile uint32_t *) 0x80000058)
#define GPIO_PAD_DIR_SET1   ((volatile uint32_t *) 0x8000005c)
#define GPIO_PAD_DIR_RESET0 ((volatile uint32_t *) 0x80000060)
#define GPIO_PAD_DIR_RESET1 ((volatile uint32_t *) 0x80000064)

inline void gpio_pad_dir(volatile uint64_t data);
inline void gpio_data(volatile uint64_t data); 
inline uint64_t gpio_data_get(volatile uint64_t bits);
inline void gpio_pad_pu_en(volatile uint64_t data); 
inline void gpio_data_sel(volatile uint64_t data); 
inline void gpio_data_pu_sel(volatile uint64_t data); 
inline void gpio_data_set(volatile uint64_t data); 
inline void gpio_data_reset(volatile uint64_t data); 
inline void gpio_pad_dir_set(volatile uint64_t data); 
inline void gpio_pad_dir_reset(volatile uint64_t data); 

/* select pullup or pulldown for GPIO 0-31 (b=0-31) */
#define	gpio_sel0_pullup(b) (set_bit(*GPIO_PAD_PU_SEL0,b))
#define	gpio_sel0_pulldown(b) (clear_bit(*GPIO_PAD_PU_SEL0,b))

/* select pullup or pulldown for GPIO 32-63 (b=32-63) */
#define	gpio_sel1_pullup(b) (set_bit(*GPIO_PAD_PU_SEL1,b-32))
#define	gpio_sel1_pulldown(b) (clear_bit(*GPIO_PAD_PU_SEL1,b-32))

/* enable/disable pullup for GPIO 0-31 (b=0-31) */
#define gpio_pu0_enable(b) (set_bit(*GPIO_PAD_PU_EN0,b))
#define gpio_pu0_disable(b) (clear_bit(*GPIO_PAD_PU_EN0,b))

/* enable/disable pullup for GPIO 32-63 (b=32-63) */
#define gpio_pu1_enable(b) (set_bit(*GPIO_PAD_PU_EN1,b-32))
#define gpio_pu1_disable(b) (clear_bit(*GPIO_PAD_PU_EN1,b-32))

#endif
