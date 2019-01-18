/*
 * Copyright (c) 2002-2004, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment
 *
 * $Id: plasma.c,v 1.1 2007/05/23 23:11:24 oliverschmidt Exp $
 *
 */

#include <stdlib.h>

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "ctk/ctk-mouse.h"
#include "contiki.h"

static unsigned char sinetab1[256] = {
  128, 131, 134, 137, 140, 143, 146, 149, 
  152, 156, 159, 162, 165, 168, 171, 174, 
  176, 179, 182, 185, 188, 191, 193, 196, 
  199, 201, 204, 206, 209, 211, 213, 216, 
  218, 220, 222, 224, 226, 228, 230, 232, 
  234, 236, 237, 239, 240, 242, 243, 245, 
  246, 247, 248, 249, 250, 251, 252, 252, 
  253, 254, 254, 255, 255, 255, 255, 255, 
  255, 255, 255, 255, 255, 255, 254, 254, 
  253, 252, 252, 251, 250, 249, 248, 247, 
  246, 245, 243, 242, 240, 239, 237, 236, 
  234, 232, 230, 228, 226, 224, 222, 220, 
  218, 216, 213, 211, 209, 206, 204, 201, 
  199, 196, 193, 191, 188, 185, 182, 179, 
  176, 174, 171, 168, 165, 162, 159, 156, 
  152, 149, 146, 143, 140, 137, 134, 131, 
  128, 124, 121, 118, 115, 112, 109, 106, 
  103,  99,  96,  93,  90,  87,  84,  81, 
   79,  76,  73,  70,  67,  64,  62,  59, 
   56,  54,  51,  49,  46,  44,  42,  39, 
   37,  35,  33,  31,  29,  27,  25,  23, 
   21,  19,  18,  16,  15,  13,  12,  10, 
    9,   8,   7,   6,   5,   4,   3,   3, 
    2,   1,   1,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   1,   1, 
    2,   3,   3,   4,   5,   6,   7,   8, 
    9,  10,  12,  13,  15,  16,  18,  19, 
   21,  23,  25,  27,  29,  31,  33,  35, 
   37,  39,  42,  44,  46,  49,  51,  54, 
   56,  59,  62,  64,  67,  70,  73,  76, 
   78,  81,  84,  87,  90,  93,  96,  99, 
  103, 106, 109, 112, 115, 118, 121, 124, 
};

static unsigned char sinetab2[256];
static unsigned char sinetab3[256];

static unsigned char colortab[256];

static unsigned char colors[16] =
  {
    0x00, 0x06, 0x0b, 0x04,
    0x0a, 0x0f, 0x07, 0x01,
    0x07, 0x0f, 0x0a, 0x08,
    0x02, 0x09, 0x00, 0x00,
  };

#define XSIZE 25
#define YSIZE 25

#define XADD01 0xfe
#define YADD01 0x05

#define XADD1 0x04
#define YADD1 0x02
#define XADD2 0xfc
#define YADD2 0xf9

#define XADD 0x03
#define YADD 0xfe

#define MOVADD 0xfb

static unsigned char xplasma[XSIZE], yplasma[YSIZE];
static unsigned char xcnt, ycnt;
static unsigned char xcnt01, xcnt02, xcnt1, xcnt2;
static unsigned char ycnt01, ycnt02, ycnt1, ycnt2;

static unsigned char xadd02 = 0x01;
static unsigned char yadd02 = 0xfb;

static unsigned char movcnt;

PROCESS(plasma_process, "Plasma screensaver");
/*---------------------------------------------------------------------------*/
static void
scrninit(void)
{
  unsigned char *ptr, *cptr;
  static int i;
  
  /* Make sine tables */
  for(i = 0; i < 256; ++i) {
    sinetab2[(unsigned char)i] = sinetab1[(unsigned char)i] / 2;
    sinetab3[(unsigned char)i] = sinetab1[(unsigned char)i] / 4;    
  }
    
  /* Make color table */
  for(i = 0; i < 256; ++i) {
    colortab[(unsigned char)i] = colors[(unsigned char)i / 16];
  }
    
  
  /* Fill screen with inverted spaces. */
  cptr = COLOR_RAM;
  for(ptr = (unsigned char *)0x0400;
      ptr != (unsigned char *)0x07e8;
      ++ptr) {
    *ptr = 0xa0;
    *cptr++ = 0x00;
  }
  
  VIC.ctrl1 = 0x1b;  /* $D011 */
  VIC.addr  = 0x17;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  VIC.bordercolor = 0x00; /* $D020 */
  VIC.bgcolor0 = 0x00; /* $D021 */  
  CIA2.pra  = 0x03;  /* $DD00 */

}
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  static unsigned char i, x, y;
  register unsigned char *cptr;
  
  if(ctk_mode_get() == CTK_MODE_SCREENSAVER) {


    xcnt1 = xcnt01;
    xcnt2 = xcnt02;
    
    for(i = 0; i < XSIZE; ++i) {
      xplasma[i] = sinetab1[xcnt1] + sinetab2[xcnt2];
      xcnt1 += XADD1;
      xcnt2 += XADD2;
    }

    ycnt1 = ycnt01;
    ycnt2 = ycnt02;
    
    for(i = 0; i < YSIZE; ++i) {
      yplasma[i] = sinetab1[ycnt1] + sinetab3[ycnt2];
      ycnt1 += YADD1;
      ycnt2 += YADD2;
    }

    xcnt01 += XADD01;
    xcnt02 += xadd02;
    ycnt01 += YADD01;
    ycnt02 += yadd02;

    
    yadd02 = sinetab3[ycnt] / 4;
    xadd02 = sinetab3[xcnt] / 4;

    ycnt += YADD;
    xcnt += XADD;

    movcnt += MOVADD;
    cptr = (unsigned char *)(COLOR_RAM + sinetab1[movcnt]/16);
    for(y = 0; y < YSIZE; ++y) {
      for(x = 0; x < XSIZE; ++x) {
	*cptr = colortab[(xplasma[x] + yplasma[y]) & 0xff];
	++cptr;
      }
      cptr += 40 - XSIZE;
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(plasma_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();
  
  ctk_mode_set(CTK_MODE_SCREENSAVER);
  ctk_mouse_hide();
  
  scrninit();

  PROCESS_WAIT_EVENT_UNTIL(ev == ctk_signal_screensaver_stop ||
			   ev == PROCESS_EVENT_EXIT);
  ctk_draw_init();
  ctk_desktop_redraw(NULL);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
