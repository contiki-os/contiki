/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

/*
 * \file
 *	This file contains a set of configuration for using SDCC as a compiler.
 *
 * \author
 *	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __Z80_DEF_H__
#define __Z80_DEF_H__

#define CC_CONF_FUNCTION_POINTER_ARGS	1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS		        0
#define CC_CONF_UNSIGNED_CHAR_BUGS	0
#define CC_CONF_REGISTER_ARGS		0


/* Generic types. */
typedef   signed char    int8_t;
typedef unsigned char   uint8_t;
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef unsigned char   u8_t;      /* 8 bit type */
typedef unsigned short u16_t;      /* 16 bit type */
typedef unsigned long  u32_t;      /* 32 bit type */
typedef   signed long  s32_t;      /* 32 bit type */
typedef unsigned short uip_stats_t;
typedef   signed long  int32_t;    /* 32 bit type */
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

/* Compiler configurations */
#define CCIF
#define CLIF
#define CC_CONF_CONST_FUNCTION_BUG

/*
 * Enable architecture-depend checksum calculation
 * for uIP configuration.
 * @see uip_arch.h
 * @see uip_arch-asm.S
 */
#define UIP_ARCH_ADD32		1
#define UIP_ARCH_CHKSUM	1
#define UIP_ARCH_IPCHKSUM

#define CC_CONF_ASSIGN_AGGREGATE(dest, src)	\
    memcpy(dest, src, sizeof(*dest))

#define uip_ipaddr_copy(dest, src)		\
    memcpy(dest, src, sizeof(*dest))

#define snprintf(a...)

#endif /* __Z80_DEF_H__ */
