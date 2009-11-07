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
 *	SD driver interface.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef SD_H
#define SD_H

#include "sd-arch.h"

#define SD_DEFAULT_BLOCK_SIZE			512
#define SD_REGISTER_SIZE			16

/* API return codes. */
#define SD_OK					 1

#define SD_INIT_ERROR_NO_CARD			-1
#define SD_INIT_ERROR_ARCH			-2
#define SD_INIT_ERROR_NO_IF_COND		-3
#define SD_INIT_ERROR_NO_BLOCK_SIZE    -4

#define SD_WRITE_ERROR_NO_CMD_RESPONSE		-5
#define SD_WRITE_ERROR_NO_BLOCK_RESPONSE	-6
#define SD_WRITE_ERROR_PROGRAMMING		-7
#define SD_WRITE_ERROR_TOKEN			-8
#define SD_WRITE_ERROR_NO_TOKEN			-9

#define SD_READ_ERROR_NO_CMD_RESPONSE		-10
#define SD_READ_ERROR_INVALID_SIZE		-11
#define SD_READ_ERROR_TOKEN			-12
#define SD_READ_ERROR_NO_TOKEN			-13

/* Type definition. */
typedef uint32_t sd_offset_t;

/* API */
int sd_initialize(void);
int sd_write(sd_offset_t, char *, unsigned);
int sd_read(sd_offset_t, char *, unsigned);
int sd_write_block(sd_offset_t, char *);
int sd_read_block(sd_offset_t, char *);
sd_offset_t sd_get_capacity(void);
char *sd_error_string(int);

#endif /* !SD_H */
