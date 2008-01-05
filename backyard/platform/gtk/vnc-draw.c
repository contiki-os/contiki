/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki VNC client
 *
 * $Id: vnc-draw.c,v 1.1 2008/01/05 21:08:26 oliverschmidt Exp $
 *
 */



#include "vnc-draw.h"

#include "contiki-conf.h"

#ifndef VNC_CONF_VIEWPORT_WIDTH
#define VNC_CONF_VIEWPORT_WIDTH  (32*8)
#endif

#ifndef VNC_CONF_VIEWPORT_HEIGHT
#define VNC_CONF_VIEWPORT_HEIGHT (16*8)
#endif

#ifndef VNC_CONF_REFRESH_ROWS
#define VNC_CONF_REFRESH_ROWS    8
#endif


static unsigned char conv[256];

static unsigned char oratab[8] =
  { 0x80, 0x40, 0x20, 0x10,
    0x08, 0x04, 0x02, 0x01 };
static unsigned char andtab[8] =
  { 0x7f, 0xbf, 0xdf, 0xef,
    0xf7, 0xfb, 0xfd, 0xfe };

static u8_t *bitmapptrtab[VNC_CONF_VIEWPORT_HEIGHT];

static unsigned short viewport_x,
  viewport_y, viewport_w, viewport_h;

u8_t vnc_draw_bitmap[(VNC_CONF_VIEWPORT_WIDTH / 8) *
		     VNC_CONF_VIEWPORT_HEIGHT];



u16_t vnc_draw_x;
u16_t vnc_draw_y;
u8_t *vnc_draw_dataptr;
u8_t *vnc_draw_bitmapptr;
u16_t vnc_draw_datalen;

/*-----------------------------------------------------------------------------------*/
void
vnc_draw_pixel(u16_t x, u8_t y, u8_t c)
{
  u8_t o, a;
  
  vnc_draw_bitmapptr = bitmapptrtab[y] + (x & 0x1f8);
  
  if(c) {
    o = oratab[x & 7];
    *vnc_draw_bitmapptr = *vnc_draw_bitmapptr | o;
  } else {
    a = andtab[x & 7];
    *vnc_draw_bitmapptr = *vnc_draw_bitmapptr & a;
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_draw_pixelline(u16_t x, u16_t y, u8_t *data, u16_t datalen)
{
  u8_t o, a;
  register u8_t *bitmapptr;
  
  vnc_draw_x = x - viewport_x;
  vnc_draw_y = y - viewport_y;

  /*  if(vnc_draw_y & 1) {
    return;
  } else {
    vnc_draw_y /= 2;
    }*/

  
  
  if(vnc_draw_y >= VNC_CONF_VIEWPORT_HEIGHT ||
     vnc_draw_x >= VNC_CONF_VIEWPORT_WIDTH) {
    return;
  }

  vnc_draw_datalen = datalen;
  
  if(vnc_draw_datalen + vnc_draw_x >= VNC_CONF_VIEWPORT_WIDTH) {
    vnc_draw_datalen = VNC_CONF_VIEWPORT_WIDTH - vnc_draw_x;
    if(vnc_draw_datalen == 0) {
      return;
    }
  }
  
  vnc_draw_dataptr = data;

  /*  vnc_draw_bitmapptr = bitmaptab[vnc_draw_y] +
      (vnc_draw_x & 0x1f8);*/

  for(; vnc_draw_datalen > 0; --vnc_draw_datalen) {
    /*    vnc_draw_pixel(vnc_draw_x, vnc_draw_y,
	  conv[*vnc_draw_dataptr]);*/

    bitmapptr = bitmapptrtab[vnc_draw_y] + (vnc_draw_x & 0x1f8);
    
    if(conv[*vnc_draw_dataptr]) {
      o = oratab[vnc_draw_x & 7];
      *bitmapptr = *bitmapptr | o;
    } else {
      a = andtab[vnc_draw_x & 7];
      *bitmapptr = *bitmapptr & a;
    }
    ++vnc_draw_dataptr;
    ++vnc_draw_x;
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_draw_init(void)
{
  unsigned int tmp;
  unsigned int i;
  unsigned short ptr;


  /* Create color conversion table. */
  for(i = 0; i < 256; ++i) {
    if(((i & 0xc0) > 0xc0) ||
       ((i & 0x38) > 0x18) ||
       ((i & 0x07) > 0x03)) {
      conv[i] = 0;
    } else {
      conv[i] = 1;
    }
  }

  memset(vnc_draw_bitmap, 0, sizeof(vnc_draw_bitmap));
  
  for(i = 0; i < VNC_CONF_VIEWPORT_HEIGHT; ++i) {
    bitmapptrtab[i] = (u8_t *)((u8_t *)vnc_draw_bitmap +
			       ((i & 0xfff8)/8) * VNC_CONF_VIEWPORT_WIDTH +
			       (i & 7));
  }

  viewport_x = 0;
  viewport_y = 0;

  viewport_w = VNC_CONF_VIEWPORT_WIDTH;
  viewport_h = VNC_CONF_VIEWPORT_HEIGHT;
  
  return;
}
/*-----------------------------------------------------------------------------------*/
u16_t
vnc_draw_viewport_x(void)
{
  return viewport_x;
}
/*-----------------------------------------------------------------------------------*/
u16_t
vnc_draw_viewport_y(void)
{
  return viewport_y;
}
/*-----------------------------------------------------------------------------------*/
u16_t
vnc_draw_viewport_w(void)
{
  return viewport_w;
}
/*-----------------------------------------------------------------------------------*/
u16_t
vnc_draw_viewport_h(void)
{
  return viewport_h;
}
/*-----------------------------------------------------------------------------------*/
