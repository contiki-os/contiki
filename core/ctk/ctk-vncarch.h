/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the "ctk" console GUI toolkit for cc65
 *
 *
 */
#ifndef __CTK_VNCARCH_H__
#define __CTK_VNCARCH_H__

#define CTK_ARCH_KEY_T unsigned char

unsigned char ctk_arch_keyavail(void);
CTK_ARCH_KEY_T ctk_arch_getkey(void);

#define CH_ENTER 0x0d
#define CH_DEL 0x08

#define CH_ESC 0x1b

#define CH_HOME 0x50

#define CH_TAB  0x09

#define CH_CURS_LEFT  0x51
#define CH_CURS_UP    0x52
#define CH_CURS_RIGHT 0x53
#define CH_CURS_DOWN  0x54


#define CH_F1 0xbe
#define CH_F2 0xbf
#define CH_F3 0xc0
#define CH_F4 0xc1
#define CH_F5 0xc2
#define CH_F6 0xc3
#define CH_F7 0xc4



#endif /* __CTK_VNCARCH_H__ */
