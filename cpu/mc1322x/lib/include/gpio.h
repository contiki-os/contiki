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

#ifndef GPIO_H
#define GPIO_H

/* Structure-based GPIO access
   Example usage:

   GPIO->FUNC_SEL0 |= 0x00008000; // set a whole register

   GPIO->FUNC_SEL_08 = 2;	 // set just one pin

   #define MY_PIN GPIO_08
   GPIO->FUNC_SEL.MY_PIN = 2;	 // same, to allow #define for pin names
   GPIO->DATA.MY_PIN = 1;

   gpio_set(GPIO_08);		 // efficiently set or clear a single output bit
   gpio_reset(GPIO_08);
*/

#define _V(x,n,i) uint32_t x##_##i : n;
#define _REP(x,n) \
	_V(x,n,00) _V(x,n,01) _V(x,n,02) _V(x,n,03) _V(x,n,04) _V(x,n,05) _V(x,n,06) _V(x,n,07) \
	_V(x,n,08) _V(x,n,09) _V(x,n,10) _V(x,n,11) _V(x,n,12) _V(x,n,13) _V(x,n,14) _V(x,n,15) \
	_V(x,n,16) _V(x,n,17) _V(x,n,18) _V(x,n,19) _V(x,n,20) _V(x,n,21) _V(x,n,22) _V(x,n,23) \
	_V(x,n,24) _V(x,n,25) _V(x,n,26) _V(x,n,27) _V(x,n,28) _V(x,n,29) _V(x,n,30) _V(x,n,31) \
	_V(x,n,32) _V(x,n,33) _V(x,n,34) _V(x,n,35) _V(x,n,36) _V(x,n,37) _V(x,n,38) _V(x,n,39) \
	_V(x,n,40) _V(x,n,41) _V(x,n,42) _V(x,n,43) _V(x,n,44) _V(x,n,45) _V(x,n,46) _V(x,n,47) \
	_V(x,n,48) _V(x,n,49) _V(x,n,50) _V(x,n,51) _V(x,n,52) _V(x,n,53) _V(x,n,54) _V(x,n,55) \
	_V(x,n,56) _V(x,n,57) _V(x,n,58) _V(x,n,59) _V(x,n,60) _V(x,n,61) _V(x,n,62) _V(x,n,63)

struct GPIO_struct {
#define _IO(x) \
	union { struct { uint32_t x##0; uint32_t x##1; }; \
		struct { _REP(x, 1) };  \
		struct GPIO_##x { _REP(GPIO, 1) } x; };
#define _IO_2bit(x)	\
	union { struct { uint32_t x##0; uint32_t x##1; uint32_t x##2; uint32_t x##3; }; \
		struct { _REP(x, 2) };  \
		struct GPIO_##x { _REP(GPIO, 2) } x; };

	_IO(PAD_DIR);
	_IO(DATA);
	_IO(PAD_PU_EN);
	_IO_2bit(FUNC_SEL);
	_IO(DATA_SEL);
	_IO(PAD_PU_SEL);
	_IO(PAD_HYST_EN);
	_IO(PAD_KEEP);
	_IO(DATA_SET);
	_IO(DATA_RESET);
	_IO(PAD_DIR_SET);
	_IO(PAD_DIR_RESET);
};
#undef _IO
#undef _IO_2bit

/* Build an enum lookup to map GPIO_08 -> 8 */
#undef _V
#define _V(x,n,i) __NUM_GPIO_GPIO_##i,
enum { _REP(0,0) };

/* Macros to set or reset a data pin in the fastest possible way */
#define gpio_set(gpio_xx) __gpio_set(gpio_xx)
#define __gpio_set(gpio_xx)						\
	((__NUM_GPIO_##gpio_xx < 32)					\
	 ? (GPIO->DATA_SET0 = (1 << (__NUM_GPIO_##gpio_xx - 0)))	\
	 : (GPIO->DATA_SET1 = (1 << (__NUM_GPIO_##gpio_xx - 32))))
#define gpio_reset(gpio_xx) __gpio_reset(gpio_xx)
#define __gpio_reset(gpio_xx)						\
	((__NUM_GPIO_##gpio_xx < 32)					\
	 ? (GPIO->DATA_RESET0 = (1 << (__NUM_GPIO_##gpio_xx - 0)))	\
	 : (GPIO->DATA_RESET1 = (1 << (__NUM_GPIO_##gpio_xx - 32))))

#undef _REP
#undef _V

static volatile struct GPIO_struct * const GPIO = (void *) (0x80000000);


/* Old register definitions, for compatibility */
#ifndef REG_NO_COMPAT

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

#endif /* REG_NO_COMPAT */

#endif
