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
 * $Id: vnc-draw.c,v 1.1 2007/05/23 23:11:26 oliverschmidt Exp $
 *
 */



#include "vnc-draw.h"
#include "vnc-draw-asm.h"

#include "vnc-conf.h"


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
    bitmapptrtab[i] = (u8_t *)((u16_t)vnc_draw_bitmap +
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
#if 0
signed short
c64_mouse_x(void)
{
  return 0;
}
/*-----------------------------------------------------------------------------------*/
signed short
c64_mouse_y(void)
{
  return 0;
}
/*-----------------------------------------------------------------------------------*/
u8_t
c64_mouse_buttons(void)
{
  return firebutton;
}
/*-----------------------------------------------------------------------------------*/
void
c64_set_mouse_x(unsigned short x)
{
  joyx = x;
}
/*-----------------------------------------------------------------------------------*/
void
c64_set_mouse_y(unsigned short y)
{
  joyy = y;
}
/*-----------------------------------------------------------------------------------*/
void
c64_set_viewport_x(unsigned short x)
{
  viewport_x = x;
}
/*-----------------------------------------------------------------------------------*/
void
c64_set_viewport_y(unsigned short y)
{
  viewport_y = y;
}
/*-----------------------------------------------------------------------------------*/

#endif /* 0 */
#if 0
#pragma optimize(push, off)
void
c64_scroll_up(unsigned char c)
{
  asm("lda $f7");
  asm("pha");
  asm("lda $f8");
  asm("pha");
  asm("lda $f9");
  asm("pha");
  asm("lda $fa");
  asm("pha");
  asm("lda $fb");
  asm("pha");
  asm("lda $fc");
  asm("pha");
  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");

  asm("lda $01");
  asm("pha");
  asm("lda #$35");
  asm("sta $01");
  
  asm("lda #$80");
  asm("sta $f7");
  asm("lda #$a2");
  asm("sta $f8");
  asm("lda #$00");
  asm("sta $f9");
  asm("lda #$a0");
  asm("sta $fa");

  asm("lda #$80");
  asm("sta $fb");
  asm("lda #$e2");
  asm("sta $fc");
  asm("lda #$00");
  asm("sta $fd");
  asm("lda #$e0");
  asm("sta $fe");

  asm("ldy #0");
  asm("loop:");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("iny");
  asm("bne loop");

  asm("inc $f8");
  asm("inc $fa");
  asm("inc $fc");
  asm("inc $fe");

  asm("lda $fc");
  asm("cmp #$00");
  asm("bne loop");

  asm("ldy #0");
  asm("lda #0");
  asm("sta $fe00,y");
  asm("sta $be00,y");
  asm("sta $fcc0,y");
  asm("sta $bcc0,y");
  asm("iny");
  asm("bne *-13");
  asm("sta $ff00,y");
  asm("sta $bf00,y");
  asm("sta $fdc0,y");
  asm("sta $bdc0,y");
  asm("iny");
  asm("cpy #$40");
  asm("bne *-15");
  

  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");
  asm("pla");
  asm("sta $fc");
  asm("pla");
  asm("sta $fb");
  asm("pla");
  asm("sta $fa");
  asm("pla");
  asm("sta $f9");
  asm("pla");
  asm("sta $f8");
  asm("pla");
  asm("sta $f7");
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
c64_scroll_down(unsigned char c)
{
  asm("lda $f7");
  asm("pha");
  asm("lda $f8");
  asm("pha");
  asm("lda $f9");
  asm("pha");
  asm("lda $fa");
  asm("pha");
  asm("lda $fb");
  asm("pha");
  asm("lda $fc");
  asm("pha");
  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");


  asm("lda $01");
  asm("pha");
  asm("lda #$35");
  asm("sta $01");
  
  asm("lda #$c0");
  asm("sta $fb");
  asm("lda #$fb");
  asm("sta $fc");
  asm("lda #$40");
  asm("sta $fd");
  asm("lda #$fe");
  asm("sta $fe");

  asm("lda #$c0");
  asm("sta $f7");
  asm("lda #$bb");
  asm("sta $f8");
  asm("lda #$40");
  asm("sta $f9");
  asm("lda #$be");
  asm("sta $fa");


  asm("ldy #0");
  asm("loop:");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("iny");
  asm("bne loop");

  asm("dec $f8");
  asm("dec $fa");
  asm("dec $fc");
  asm("dec $fe");


  asm("lda $fe");
  asm("cmp #$df");
  asm("bne loop");

  asm("ldy #0");
  asm("lda #0");
  asm("sta $e000,y");
  asm("sta $a000,y");
  asm("sta $e140,y");
  asm("sta $a140,y");
  asm("iny");
  asm("bne *-13");
  asm("sta $e100,y");
  asm("sta $a100,y");
  asm("sta $e240,y");
  asm("sta $a240,y");
  asm("iny");
  asm("cpy #$40");
  asm("bne *-15");
  
  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");
  asm("pla");
  asm("sta $fc");
  asm("pla");
  asm("sta $fb");
  asm("pla");
  asm("sta $fa");
  asm("pla");
  asm("sta $f9");
  asm("pla");
  asm("sta $f8");
  asm("pla");
  asm("sta $f7");
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
c64_scroll_right(unsigned char c)
{
  asm("lda $f7");
  asm("pha");
  asm("lda $f8");
  asm("pha");
  asm("lda $f9");
  asm("pha");
  asm("lda $fa");
  asm("pha");
  asm("lda $fb");
  asm("pha");
  asm("lda $fc");
  asm("pha");
  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");


  asm("lda $01");
  asm("pha");
  asm("lda #$35");
  asm("sta $01");

  asm("lda #$00");
  asm("sta $f7");
  asm("lda #$a0");
  asm("sta $f8");
  asm("lda #$10");
  asm("sta $f9");
  asm("lda #$a0");
  asm("sta $fa");

  asm("lda #$00");
  asm("sta $fb");
  asm("lda #$e0");
  asm("sta $fc");
  asm("lda #$10");
  asm("sta $fd");
  asm("lda #$e0");
  asm("sta $fe");

  asm("ldx #0");
  
  asm("loop3:");
  
  asm("ldy #$ff");
  asm("loop:");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("dey");
  asm("cpy #$ff");
  asm("bne loop");
  
  asm("inc $f8");
  asm("inc $fa");
  asm("inc $fc");
  asm("inc $fe");

  asm("ldy #$30");
  asm("loop2:");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("dey");
  asm("cpy #$ff");
  asm("bne loop2");

  asm("lda $f7");
  asm("clc");
  asm("adc #$40");
  asm("sta $f7");
  asm("sta $fb");
  asm("bcc :+");
  asm("inc $f8");
  asm("inc $fc");
  asm(":");
  
  asm("lda $f9");
  asm("clc");
  asm("adc #$40");
  asm("sta $f9");
  asm("sta $fd");
  asm("bcc :+");
  asm("inc $fa");
  asm("inc $fe");
  asm(":");
  

  
  asm("inx");
  asm("cpx #24");
  asm("bne loop3");

  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");
  asm("pla");
  asm("sta $fc");
  asm("pla");
  asm("sta $fb");
  asm("pla");
  asm("sta $fa");
  asm("pla");
  asm("sta $f9");
  asm("pla");
  asm("sta $f8");
  asm("pla");
  asm("sta $f7");
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
c64_scroll_left(unsigned char c)
{
  asm("lda $f7");
  asm("pha");
  asm("lda $f8");
  asm("pha");
  asm("lda $f9");
  asm("pha");
  asm("lda $fa");
  asm("pha");
  asm("lda $fb");
  asm("pha");
  asm("lda $fc");
  asm("pha");
  asm("lda $fd");
  asm("pha");
  asm("lda $fe");
  asm("pha");


  asm("lda $01");
  asm("pha");
  asm("lda #$35");
  asm("sta $01");

  asm("lda #$10");
  asm("sta $f7");
  asm("lda #$a0");
  asm("sta $f8");
  asm("lda #$00");
  asm("sta $f9");
  asm("lda #$a0");
  asm("sta $fa");

  asm("lda #$10");
  asm("sta $fb");
  asm("lda #$e0");
  asm("sta $fc");
  asm("lda #$00");
  asm("sta $fd");
  asm("lda #$e0");
  asm("sta $fe");

  asm("ldx #0");
  asm("loop3:");
  asm("ldy #0");
  asm("loop:");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("iny");
  asm("bne loop");
  
  asm("inc $f8");
  asm("inc $fa");
  asm("inc $fc");
  asm("inc $fe");

  asm("ldy #0");
  asm("loop2:");
  asm("lda ($f7),y");
  asm("sta ($f9),y");
  asm("lda ($fb),y");
  asm("sta ($fd),y");
  asm("iny");
  asm("cpy #$30");
  asm("bne loop2");

  asm("lda $f7");
  asm("clc");
  asm("adc #$40");
  asm("sta $f7");
  asm("sta $fb");
  asm("bcc :+");
  asm("inc $f8");
  asm("inc $fc");
  asm(":");
  
  asm("lda $f9");
  asm("clc");
  asm("adc #$40");
  asm("sta $f9");
  asm("sta $fd");
  asm("bcc :+");
  asm("inc $fa");
  asm("inc $fe");
  asm(":");
  

  asm("inx");
  asm("cpx #24");
  asm("bne loop3");

  asm("pla");
  asm("sta $01");
  asm("pla");
  asm("sta $fe");
  asm("pla");
  asm("sta $fd");
  asm("pla");
  asm("sta $fc");
  asm("pla");
  asm("sta $fb");
  asm("pla");
  asm("sta $fa");
  asm("pla");
  asm("sta $f9");
  asm("pla");
  asm("sta $f8");
  asm("pla");
  asm("sta $f7");
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/



#endif /* 0 */
