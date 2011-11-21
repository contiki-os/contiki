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
 * @(#)$Id: 6502def.h,v 1.27 2010/10/22 20:28:58 oliverschmidt Exp $
 */

#ifndef __6502DEF_H__
#define __6502DEF_H__

#include <ctype.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "pfs.h"

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t  s32_t;

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_FASTCALL      __fastcall__

#define CCIF
#define CLIF

#define HAVE_SNPRINTF
#define snprintf(buf, len, ...) sprintf(buf, __VA_ARGS__)

#define CLOCK_CONF_SECOND 2
typedef unsigned short clock_time_t;

typedef unsigned short uip_stats_t;

#define UIP_ARCH_ADD32  1
#define UIP_ARCH_CHKSUM 1

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 1

#define LOADER_CONF_ARCH "lib/unload.h"

#if MTU_SIZE
#define UIP_CONF_BUFFER_SIZE (UIP_LLH_LEN + MTU_SIZE)
#else /* MTU_SIZE */
#define UIP_CONF_BUFFER_SIZE (UIP_LLH_LEN + 1500)
#endif /* MTU_SIZE */

#if CONNECTIONS
#define UIP_CONF_MAX_CONNECTIONS CONNECTIONS
#else /* CONNECTIONS */
#define UIP_CONF_MAX_CONNECTIONS 10
#endif /* CONNECTIONS */

#if WITH_LOGGING
#define LOG_CONF_ENABLED 1
#define UIP_CONF_LOGGING 1
#else /* WITH_LOGGING */
#define LOG_CONF_ENABLED 0
#define UIP_CONF_LOGGING 0
#endif /* WITH_LOGGING */

#if WITH_BOOST
#define UIP_CONF_TCP_SPLIT 1
#else /* WITH_BOOST */
#define UIP_CONF_TCP_SPLIT 0
#endif /* WITH_BOOST */

#if WITH_FORWARDING
#define UIP_CONF_IP_FORWARD 1
#else /* WITH_FORWARDING */
#define UIP_CONF_IP_FORWARD 0
#endif /* WITH_FORWARDING */

#if WITH_CLIENT
#define UIP_CONF_ACTIVE_OPEN 1
#else /* WITH_CLIENT */
#define UIP_CONF_ACTIVE_OPEN 0
#endif /* WITH_CLIENT */

#if WITH_DNS
#define UIP_CONF_UDP 1
#else /* WITH_DNS */
#define UIP_CONF_UDP 0
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

#define CFS_CONF_OFFSET_TYPE off_t

#if WITH_PFS
#define cfs_open     pfs_open
#define cfs_close    pfs_close
#define cfs_read     pfs_read
#define cfs_write    pfs_write
#define cfs_seek     pfs_seek
#define cfs_remove   pfs_remove
#else /* WITH_PFS */
#define CFS_READ     (O_RDONLY)
#define CFS_WRITE    (O_WRONLY | O_CREAT | O_TRUNC)
#define CFS_SEEK_SET (SEEK_SET)
#define CFS_SEEK_CUR (SEEK_CUR)
#define CFS_SEEK_END (SEEK_END)
#define cfs_open     open
#define cfs_close    close
#define cfs_read     read
#define cfs_write    write
#define cfs_seek     lseek
#define cfs_remove   remove
#endif /* WITH_PFS */

#endif /* __6502DEF_H__ */
