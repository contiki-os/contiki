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
  * 	Arcitecture-depend libconio module, which supposes
  *     the machine has character VRAM and optional attribute VRAM
  *     on the main memory.
  * \author
  * 	Takahide Matsutsuka <markn@markn.org>
  */

#include "contiki.h"
#include "sys/log.h"
#include "libconio.h"
#include "libconio_z80.h"

/*---------------------------------------------------------------------------*/
static void scroll() {
  unsigned char y;
  uint16_t src, dst;
  for (y = 0; y < LIBCONIO_CONF_SCREEN_HEIGHT - 1; y++) {
    dst = LIBCONIO_VRAM_OFFSET(0, y);
    src = LIBCONIO_VRAM_OFFSET(0, y + 1);
    memcpy(LIBCONIO_VRAM_CHAR + dst,
	   LIBCONIO_VRAM_CHAR + src,
	   LIBCONIO_CONF_SCREEN_WIDTH);
#ifdef LIBCONIO_CONF_ATTRIBUTES_ENABLED
    memcpy(LIBCONIO_VRAM_ATTR + dst,
	   LIBCONIO_VRAM_ATTR + src,
	   LIBCONIO_CONF_SCREEN_WIDTH);
#endif /* LIBCONIO_CONF_ATTRIBUTES_ENABLED */
  }
  dst = LIBCONIO_VRAM_OFFSET(0, LIBCONIO_CONF_SCREEN_HEIGHT - 1);
  memset(LIBCONIO_VRAM_CHAR + dst, ' ',
	 LIBCONIO_CONF_SCREEN_WIDTH);
#ifdef LIBCONIO_CONF_ATTRIBUTES_ENABLED
  memset(LIBCONIO_VRAM_ATTR + dst, LIBCONIO_COLOR_NORMAL,
	 LIBCONIO_CONF_SCREEN_WIDTH);
#endif /* LIBCONIO_CONF_ATTRIBUTES_ENABLED */

  gotoxy(0, LIBCONIO_CONF_SCREEN_HEIGHT - 1);
}
/*---------------------------------------------------------------------------*/
/* make sure that the position is inside screen */
static void adjust(unsigned char *x, unsigned char *y) {
  if (*x > LIBCONIO_CONF_SCREEN_WIDTH) {
    *y += *x / LIBCONIO_CONF_SCREEN_WIDTH;
    *x = *x % LIBCONIO_CONF_SCREEN_WIDTH;
    gotoxy(*x, *y);
  }
}
/*---------------------------------------------------------------------------*/
void ctk_arch_draw_char(char c,
			unsigned char xpos,
			unsigned char ypos,
			unsigned char reversed,
			unsigned char color) {
  uint16_t off;
  adjust(&xpos, &ypos);

  off = LIBCONIO_VRAM_OFFSET(xpos, ypos);
  if (off >= LIBCONIO_VRAM_OFFSET_MAX) {
    scroll();
    off = LIBCONIO_VRAM_OFFSET(0, LIBCONIO_CONF_SCREEN_HEIGHT - 1);
  }
  *(char *)(LIBCONIO_VRAM_CHAR + off) = c;
#ifdef LIBCONIO_CONF_ATTRIBUTES_ENABLED
  *(char *)(LIBCONIO_VRAM_ATTR + off) = reversed ?
    LIBCONIO_COLOR_REVERSED : LIBCONIO_COLOR_NORMAL;
#endif /* LIBCONIO_CONF_ATTRIBUTES_ENABLED */
}
/*---------------------------------------------------------------------------*/
