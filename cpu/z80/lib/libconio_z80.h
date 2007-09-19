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
 * $Id: libconio_z80.h,v 1.1 2007/09/19 12:48:26 matsutsuka Exp $
 *
 */
 /*
  * \file
  * 	Arcitecture-depend libconio module, which supposes
  *     the machine has character VRAM and optional attribute VRAM
  *     on the main memory.
  * \author
  * 	Takahide Matsutsuka <markn@markn.org>
  */

#ifndef __LIBCONIO_ARCH_H__
#define __LIBCONIO_ARCH_H__

#ifndef LIBCONIO_VRAM_CHAR
#error "must specify vram address for characters!"
#endif /*  LIBCONIO_VRAM_CHAR */

#ifdef LIBCONIO_CONF_ATTRIBUTES_ENABLED
#ifndef LIBCONIO_VRAM_ATTR
#error "must specify vram address for attributes!"
#endif /* LIBCONIO_VRAM_ATTR */
#endif /* LIBCONIO_CONF_ATTRIBUTES_ENABLED */

#ifndef LIBCONIO_CONF_SCREEN_WIDTH
#define LIBCONIO_CONF_SCREEN_WIDTH   32
#endif /* LIBCONIO_CONF_SCREEN_WIDTH */

#ifndef LIBCONIO_CONF_SCREEN_HEIGHT
#define LIBCONIO_CONF_SCREEN_HEIGHT  16
#endif /* LIBCONIO_CONF_SCREEN_HEIGHT */

#ifndef LIBCONIO_COLOR_REVERSED
#define LIBCONIO_COLOR_REVERSED 0x21
#endif /* LIBCONIO_COLOR_REVERSED */

#ifndef LIBCONIO_COLOR_NORMAL
#define LIBCONIO_COLOR_NORMAL 0x20
#endif /* LIBCONIO_COLOR_NORMAL */

/*
 * An offset caluclation logic.
 * The default supposes the VRAM is sequential
 */
#ifndef LIBCONIO_VRAM_OFFSET
#define LIBCONIO_VRAM_OFFSET(x, y)  (y) * LIBCONIO_CONF_SCREEN_WIDTH + (x)
#endif /* VRAM_OFFSET */

/*
 * A maximum offset + 1.
 * If offset is equal or more than this maximum, the screen will scroll.
 */
#ifndef LIBCONIO_VRAM_OFFSET_MAX
#define LIBCONIO_VRAM_OFFSET_MAX \
    (LIBCONIO_CONF_SCREEN_HEIGHT * LIBCONIO_CONF_SCREEN_WIDTH)
#endif /* VRAM_MAX */

#endif /* __LIBCONIO_ARCH_H__ */
