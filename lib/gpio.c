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

#include <mc1322x.h>
#include <stdint.h>

inline void gpio_pad_dir(volatile uint64_t data) 
{
	*GPIO_PAD_DIR0 = (data & 0xffffffff);
	*GPIO_PAD_DIR1 = (data >> 32);
}

inline void gpio_data(volatile uint64_t data) 
{
	*GPIO_DATA0 = (data & 0xffffffff);
	*GPIO_DATA1 = (data >> 32);
}

inline uint64_t gpio_data_get(volatile uint64_t bits) {
	uint64_t rdata = 0;

	rdata = *GPIO_DATA0 & (bits & 0xffffffff);
	rdata |= (*GPIO_DATA1 & (bits >> 32)) << 32;

	return rdata;
}

inline void gpio_pad_pu_en(volatile uint64_t data) 
{
	*GPIO_PAD_PU_EN0 = (data & 0xffffffff);
	*GPIO_PAD_PU_EN1 = (data >> 32);
}

inline void gpio_data_sel(volatile uint64_t data) 
{
	*GPIO_DATA_SEL0 = (data & 0xffffffff);
	*GPIO_DATA_SEL1 = (data >> 32);
}

inline void gpio_pad_pu_sel(volatile uint64_t data) 
{
	*GPIO_PAD_PU_SEL0 = (data & 0xffffffff);
	*GPIO_PAD_PU_SEL1 = (data >> 32);
}

inline void gpio_data_set(volatile uint64_t data) 
{
	*GPIO_DATA_SET0 = (data & 0xffffffff);
	*GPIO_DATA_SET1 = (data >> 32);
}

inline void gpio_data_reset(volatile uint64_t data) 
{
	*GPIO_DATA_RESET0 = (data & 0xffffffff);
	*GPIO_DATA_RESET1 = (data >> 32);
}

inline void gpio_pad_dir_set(volatile uint64_t data) 
{
	*GPIO_PAD_DIR_SET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_SET1 = (data >> 32);
}

inline void gpio_pad_dir_reset(volatile uint64_t data) 
{
	*GPIO_PAD_DIR_RESET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_RESET1 = (data >> 32);
}
