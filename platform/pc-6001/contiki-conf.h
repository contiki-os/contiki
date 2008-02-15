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
 * $Id: contiki-conf.h,v 1.6 2008/02/15 16:51:06 oliverschmidt Exp $
 *
 */

/*
 * \file
 * 	contiki-conf.h
 * 	A set of configurations of contiki for PC-6001 family. 
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "z80def.h"
#include "sys/cc.h"
#include <ctype.h>
#include <string.h>
#include "ctk/ctk_arch.h"
#include "strcasecmp.h"
#include "log.h"

/* Time type. */
typedef unsigned long clock_time_t;

/* Defines tick counts for a second. */
#define CLOCK_CONF_SECOND		1024

#define rtimer_arch_now()	clock_time()

/* Memory filesystem RAM size. */
#define CFS_RAM_CONF_SIZE		512

/* Logging.. */
#define LOG_CONF_ENABLED		0

#undef MALLOC_TEST

/*---------------------------------------------------------------------------*/
/* screen properties */
#define LIBCONIO_CONF_ATTRIBUTES_ENABLED
#if defined(ARCH_PC6001MK2)
#define LIBCONIO_VRAM_ATTR		0x4000
#define LIBCONIO_VRAM_CHAR		0x4400
#define LIBCONIO_CONF_SCREEN_WIDTH	40
#define LIBCONIO_CONF_SCREEN_HEIGHT	20
#define LIBCONIO_COLOR_NORMAL		0x0f
#define LIBCONIO_COLOR_REVERSED		0x70
#elif (defined(ARCH_PC6001) || defined(ARCH_PC6001A)) && defined(MEMORY_16K)
#define LIBCONIO_VRAM_ATTR		0xc000
#define LIBCONIO_VRAM_CHAR		0xc200
#define LIBCONIO_CONF_SCREEN_WIDTH	32
#define LIBCONIO_CONF_SCREEN_HEIGHT	16
#define LIBCONIO_COLOR_NORMAL		0x20
#define LIBCONIO_COLOR_REVERSED		0x21
#elif (defined(ARCH_PC6001) || defined(ARCH_PC6001A)) && (defined(MEMORY_32K) || defined(MEMORY_ROM))
#define LIBCONIO_VRAM_ATTR		0x8000
#define LIBCONIO_VRAM_CHAR		0x8200
#define LIBCONIO_CONF_SCREEN_WIDTH	32
#define LIBCONIO_CONF_SCREEN_HEIGHT	16
#define LIBCONIO_COLOR_NORMAL		0x20
#define LIBCONIO_COLOR_REVERSED		0x21
#else
#error Specify appropriate ARCH & MEMORY combination
#endif /*  ARCH_PC6001MK2 */

#if 0 /* ctk-conio case */
#define SCREENCOLOR		0x20
#define BORDERCOLOR		0x20
#define WIDGETCOLOR		0x20
#define WIDGETCOLOR_FWIN	0x20
#define BACKGROUNDCOLOR		0x20
#define DIALOGCOLOR		0x20
#define WINDOWCOLOR		0x20
#define WINDOWCOLOR_FOCUS	0x21
#define MENUCOLOR		0x21
#define MENUCOLOR		0x21
#define OPENMENUCOLOR		0x22
#define ACTIVEMENUITEMCOLOR	0x23
#define WIDGETCOLOR_HLINK	0x21
#define WIDGETCOLOR_FOCUS	0x20
#define WIDGETCOLOR_DIALOG	0x22
#endif

/* uIP configuration */
#define UIP_CONF_MAX_CONNECTIONS	4
#define UIP_CONF_MAX_LISTENPORTS	4
#define UIP_CONF_BUFFER_SIZE		400
#define UIP_CONF_BYTE_ORDER		LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT		0
#define UIP_CONF_LOGGING		0

/* uses SLIP */
#define UIP_CONF_UDP			0
#define UIP_CONF_UDP_CHECKSUMS		0
#define UIP_CONF_LLH_LEN		0
#undef UIP_CONF_BROADCAST
#undef RS232_CONF_CALLBACK
/* #define RS232_CONF_CALLBACK             serial_input_byte */
#define slip_arch_init(ubr)             rs232_arch_init(ubr)
#define slip_arch_writeb(c)             rs232_arch_writeb(c)

#ifdef WITH_LOADER_ARCH
//#define AUTOSTART_ENABLE                1
#define LOADER_CONF_ARCH                "loader_arch.h"
#endif /* WITH_LOADER_ARCH */

/*---------------------------------------------------------------------------*/
/* Application specific configurations. */

/* Command shell */
#define SHELL_GUI_CONF_XSIZE		26
#define SHELL_GUI_CONF_YSIZE		10

/* Text editor */
#define EDITOR_CONF_WIDTH		26
#define EDITOR_CONF_HEIGHT		 8

/* Process list */
#define PROCESSLIST_CONF_HEIGHT	12

/* File dialog */
#define FILES_CONF_HEIGHT		 6

/* Shell */
//#define SHELL_CONF_WITH_PROGRAM_HANDLER	 1

/* Telnet */
#define TELNET_CONF_WINDOW_WIDTH        30
#define TELNET_CONF_WINDOW_HEIGHT       13
//#define TELNET_CONF_TEXTAREA_HEIGHT      5 // TELNET_WINDOW_HEIGHT - 8
//#define TELNET_CONF_ENTRY_WIDTH         22 // TELNET_WINDOW_WIDTH - 8

/* Telnetd */
#define TELNETD_CONF_GUI                 0
#define SHELL_CONF_WITH_PROGRAM_HANDLER              0

/* Web server */
#undef WEBSERVER_CONF_LOG_ENABLED
#define HTTPD_CONF_CGI		0
#define HTTPD_CONF_SCRIPT	0
#define HTTPD_CONF_STATISTICS	0

/* unused yet */
#define VNC_CONF_REFRESH_ROWS    8

#define WWW_CONF_WEBPAGE_WIDTH 76
#define WWW_CONF_WEBPAGE_HEIGHT 30
#define WWW_CONF_HISTORY_SIZE 40
#define WWW_CONF_MAX_URLLEN 200
#define WWW_CONF_MAX_NUMPAGEWIDGETS 80
#define WWW_CONF_RENDERSTATE 1
#define WWW_CONF_FORMS       1
#define WWW_CONF_MAX_FORMACTIONLEN  200
#define WWW_CONF_MAX_FORMNAMELEN    200
#define WWW_CONF_MAX_INPUTNAMELEN   200
#define WWW_CONF_MAX_INPUTVALUELEN  240
#define WWW_CONF_PAGEVIEW 1

#endif /* __CONTIKI_CONF_H__ */
