/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: ctk-arch.h,v 1.1 2006/08/13 16:36:01 oliverschmidt Exp $
 */
#ifndef __CTK_ARCH_H__
#define __CTK_ARCH_H__

#include <ctype.h>

#define CH_ENTER	'\r'
#define CH_DEL		'\b'
#define CH_CURS_UP  	-1
#define CH_CURS_LEFT	-2
#define CH_CURS_RIGHT	-3
#define CH_CURS_DOWN	-4

#define CH_ULCORNER	0xDA
#define CH_URCORNER	0xBF
#define CH_LLCORNER	0xC0
#define CH_LRCORNER	0xD9

#include "ctk/ctk-conio.h"
#include "ctk-console.h"

#undef  ctk_arch_getkey
ctk_arch_key_t ctk_arch_getkey(void);

#undef  ctk_arch_keyavail
unsigned char ctk_arch_keyavail(void);

#define ctk_arch_isprint    isprint
#define cputc		    console_cputc
#define cputs		    console_cputs

#define COLOR_BLACK	(0)
#define COLOR_BLUE	(1)
#define COLOR_GRAY	(1 | 2 | 4)
#define COLOR_CYAN	(1 | 2 | 8)
#define COLOR_YELLOW	(2 | 4 | 8)
#define COLOR_WHITE	(1 | 2 | 4 | 8)

#endif /* __CTK_ARCH_H__ */
