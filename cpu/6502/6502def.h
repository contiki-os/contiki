/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * @(#)$Id: 6502def.h,v 1.6 2007/12/23 13:55:38 oliverschmidt Exp $
 */

#ifndef __6502DEF_H__
#define __6502DEF_H__

#include <ctype.h>
#include <conio.h>
#include <stdint.h>

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t  s32_t;

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_FASTCALL      __fastcall__

#define CCIF
#define CLIF

#define CLOCK_CONF_SECOND 2
typedef unsigned short clock_time_t;

typedef unsigned short uip_stats_t;

#define UIP_ARCH_ADD32  1
#define UIP_ARCH_CHKSUM 1

#if MTU_SIZE
#define UIP_CONF_BUFFER_SIZE (UIP_LLH_LEN + MTU_SIZE)
#else /* MTU_SIZE */
#define UIP_CONF_BUFFER_SIZE (UIP_LLH_LEN + 1500)
#endif /* MTU_SIZE */

#if WITH_BOOST
#define UIP_CONF_TCP_SPLIT 1
#else /* WITH_BOOST */
#define UIP_CONF_TCP_SPLIT 0
#endif /* WITH_BOOST */

#if WITH_CLIENT
#define UIP_CONF_ACTIVE_OPEN 1
#else /* WITH_CLIENT */
#define UIP_CONF_ACTIVE_OPEN 0
#endif /* WITH_CLIENT */

#if WITH_DNS
#define UIP_CONF_UDP           1
#define UIP_CONF_UDP_CHECKSUMS 1
#else /* WITH_DNS */
#define UIP_CONF_UDP           0
#define UIP_CONF_UDP_CHECKSUMS 0
#endif /* WITH_DNS */

#define CTK_CONF_WIDGET_FLAGS 0
#define CTK_CONF_WINDOWS      0
#define CTK_CONF_WINDOWMOVE   0
#define CTK_CONF_WINDOWCLOSE  0
#define CTK_CONF_ICONS        0
#define CTK_CONF_MENUS        0
#define CTK_CONF_SCREENSAVER  0

#if WITH_MOUSE
#define CTK_CONF_MOUSE_SUPPORT 1
#else /* WITH_MOUSE */
#define CTK_CONF_MOUSE_SUPPORT 0
#endif /* WITH_MOUSE */

#define ctk_arch_keyavail kbhit
#define ctk_arch_getkey   cgetc
#define ctk_arch_isprint  isprint

#endif /* __6502DEF_H__ */
