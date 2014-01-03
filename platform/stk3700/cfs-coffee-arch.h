/*
 * Copyright (c) 2008, Swedish Institute of Computer Science
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
 *
 */

/**
 * \file
 *	Coffee architecture-dependent header for the DK3750 platform.
 * \author
 * 	Martin Chaplet <m.chaplet@kerlink.fr>
 */

#ifndef CFS_COFFEE_ARCH_H
#define CFS_COFFEE_ARCH_H

#include "contiki-conf.h"
#include "dev/efm32flash.h"

/* Coffee configuration parameters. */
#define COFFEE_SECTOR_SIZE		4096UL
#define COFFEE_PAGE_SIZE		512UL // User-defined (no real page .... can be between 1 and SECTOR_SIZE)

// by default: upper 512ko on Internal flash
#define COFFEE_START			0x80000
//#define COFFEE_SIZE			    (512UL * 1024UL)
#define COFFEE_SIZE             (32UL * 1024UL)

#define COFFEE_NAME_LENGTH		16
#define COFFEE_MAX_OPEN_FILES	6
#define COFFEE_FD_SET_SIZE		8
#define COFFEE_LOG_TABLE_LIMIT	256
#ifdef COFFEE_CONF_DYN_SIZE
#define COFFEE_DYN_SIZE			COFFEE_CONF_DYN_SIZE
#else
#define COFFEE_DYN_SIZE        4*1024
#endif
#define COFFEE_LOG_SIZE			1024

#define COFFEE_IO_SEMANTICS		1
#define COFFEE_APPEND_ONLY		0
#define COFFEE_MICRO_LOGS		0

/* Flash operations. */
#define COFFEE_WRITE(buf, size, offset)				\
		flash_compwrite(COFFEE_START + (offset), (char *)(buf), (size))

#define COFFEE_READ(buf, size, offset)				\
  		flash_compread(COFFEE_START + (offset), (char *)(buf), (size))

#define COFFEE_ERASE(sector)					\
  		flash_erase(COFFEE_START + (sector) * COFFEE_SECTOR_SIZE, COFFEE_SECTOR_SIZE)

/* Coffee types. */
typedef int32_t coffee_page_t;
#define CFS_CONF_OFFSET_TYPE uint32_t

#endif /* !COFFEE_ARCH_H */
