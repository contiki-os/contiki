/*
 * Copyright (c) 2013, Kerlink
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
 * This file is part of the Contiki operating system.
 */

#ifndef __EFM32_FLASH_H__
#define __EFM32_FLASH_H__

/**
 * \addtogroup efm32-devices
 * @{
 */
/**
 * \file
 *         EFM32 flash routines header
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include <dev/flash.h>

#define FLASH_START_ADDRESS 0


void efm32_flash_init(void);

int flash_read(uint32_t address, void * data, uint32_t length);
int flash_write(uint32_t address, const char * data, uint32_t length);
int flash_erase(uint32_t start_address, uint32_t length);

int flash_write_pages(uint32_t address, const char * data, uint32_t length);

int flash_comread(uint32_t address, void * data, uint32_t length);
int flash_compwrite(uint32_t address, const char * data, uint32_t length);

/** @} */

#endif /* __EFM32_FLASH_H__ */

