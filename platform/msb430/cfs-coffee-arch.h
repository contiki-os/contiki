/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 *	Coffee header for the Tmote Sky platform.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef CFS_COFFEE_ARCH_H
#define CFS_COFFEE_ARCH_H

#include "contiki-conf.h"
#include "dev/sd.h"

/* Coffee configuration parameters. */
#define COFFEE_SECTOR_SIZE		(10*1024*1024UL)
#define COFFEE_PAGE_SIZE		1024UL
#define COFFEE_START			0
#define COFFEE_SIZE			(COFFEE_SECTOR_SIZE * 2)
#define COFFEE_NAME_LENGTH		16
#define COFFEE_MAX_OPEN_FILES		6
#define COFFEE_FD_SET_SIZE		8
#define COFFEE_LOG_TABLE_LIMIT		256
#define COFFEE_DYN_SIZE			32*1024UL
#define COFFEE_LOG_SIZE			8*1024UL
#define COFFEE_MICRO_LOGS		0

/* Flash operations. */
#define COFFEE_WRITE(buf, size, offset)				\
		sd_write(COFFEE_START + (offset), (char *)(buf), (size))

#define COFFEE_READ(buf, size, offset)				\
  		sd_read(COFFEE_START + (offset), (char *)buf, (size))

#define COFFEE_ERASE(sector)					\
		cfs_coffee_arch_erase(sector)

/* Coffee types. */
typedef int16_t coffee_page_t;
typedef sd_offset_t coffee_offset_t;

int cfs_coffee_arch_erase(unsigned);

#endif /* !COFFEE_ARCH_H */
