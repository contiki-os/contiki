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
 * This file is part of the Contiki desktop OS for the C64
 *
 * $Id: ctk-hires-theme-springlight.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

#include <conio.h>
#include "ctk-hires-theme.h"

#define COLOR(bg, fg) ((fg << 4) | (bg))

#define BGCOLOR1 0x05
#define BGCOLOR2 0x0d


struct ctk_hires_theme ctk_hires_theme =
  {
    /* Version string. */
    /*    char version[8]; */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

    /* Window borders patterns. */
    /* unsigned char ulcorner[8], */ /* Upper left corner. */
    {0x00,0x3f,0x6a,0x40,0x60,0x40,0x60,0x40},
    
    /* titlebar[8], */            /* Title bar pattern. */
    {0x00,0xff,0xaa,0x00,0x00,0x00,0x00,0x00},
    
    /* urcorner[8],       */       /* Upper right corner. */
    {0x00,0xf0,0x20,0x00,0x00,0x00,0x00,0x00},
    
    /* rightborder[8],     */      /* Right border. */
    {0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02},
    
    /* lrcorner[8],      */        /* Lower right corner. */
    {0x02,0x02,0x02,0x02,0x02,0x06,0xfc,0x00},
    
    /* lowerborder[8], */          /* Lower border. */
    {0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00},
    
    /* llcorner[8],  */            /* Lower left corner. */
    {0x60,0x40,0x40,0x40,0x00,0x00,0x00,0x00},
    
    /* leftborder[8]; */          /* Left border. */
    {0x60,0x40,0x60,0x40,0x60,0x40,0x60,0x40},

    /* Button corner patterns. */
    /*    unsigned char buttonleft[8], */
    {0x0f,0x18,0x10,0x10,0x10,0x10,0x10,0x00},    
    
    /* buttonright[8]; */
    {0x00,0x08,0x08,0x08,0x08,0x08,0x18,0xf0},    
    
      /* Menu border patterns. */
    /*    unsigned char menuleftpattern[8], */
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    
    /* menurightpatterns[8]; */
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    
    /* Window and widget colors. */
    /*    unsigned char windowcolors[6], */
    {COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GRAY1, COLOR_GREEN),
     COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK)},
    
    /* separatorcolors[6], */
    {COLOR(COLOR_GRAY1, COLOR_GRAY2),
     COLOR(COLOR_GRAY1, COLOR_GRAY2),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY1),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY1)},
    
    /* labelcolors[6], */
    {COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK)},
    
    /* buttoncolors[6], */
    {COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GRAY1, COLOR_GREEN),
     COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY2),
     COLOR(COLOR_LIGHTGREEN, COLOR_WHITE)},
    
    /* hyperlinkcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
     COLOR(COLOR_GRAY2, COLOR_LIGHTBLUE),
     COLOR(COLOR_GREEN, COLOR_CYAN),
     COLOR(COLOR_GREEN, COLOR_BLUE),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTGREEN)},
    
    /* textentrycolors[6],  */
    {COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_GREEN, COLOR_BLACK),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK),
     COLOR(COLOR_LIGHTGREEN, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK)},
    
    /* bitmapcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_GRAY1),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK),
     COLOR(COLOR_GRAY3, COLOR_BLACK)},
    
    /* textmapcolors[6], */
    {COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_GRAY2, COLOR_GRAY1),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_WHITE, COLOR_BLACK)},
    
    /* iconcolors[6]; */
    {COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_GREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY1),
     COLOR(COLOR_GRAY1, COLOR_LIGHTGREEN),
     COLOR(COLOR_WHITE, COLOR_BLACK),
     COLOR(COLOR_BLACK, COLOR_YELLOW)},


    /* Button corner colors. */
    /* unsigned char buttonleftcolors[6], */
    {COLOR(COLOR_GRAY1, COLOR_GREEN),
     COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_LIGHTGREEN, COLOR_WHITE),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY2)},
    
    /* buttonrightcolors[6]; */
    {COLOR(COLOR_GRAY1, COLOR_BLACK),
     COLOR(COLOR_GRAY1, COLOR_GREEN),
     COLOR(COLOR_GREEN, COLOR_GRAY1),
     COLOR(COLOR_GREEN, COLOR_LIGHTGREEN),
     COLOR(COLOR_LIGHTGREEN, COLOR_GRAY2),
     COLOR(COLOR_LIGHTGREEN, COLOR_WHITE)},

    
    /* Menu colors. */
    /*    unsigned char menucolor,*/
    COLOR(COLOR_LIGHTGREEN, COLOR_GRAY1),
    
    /*    openmenucolor, */
    COLOR(COLOR_BLACK, COLOR_GREEN),
    
    /* activemenucolor; */
    COLOR(COLOR_BLACK, COLOR_LIGHTGREEN),
    
    /* Border and screen colors. */
    /*    unsigned char bordercolor,*/
    5,
    
    /* screencolor; */
    0,

    /* Pointer sprite 0 color */
    COLOR_WHITE,

    /* Pointer sprite 1 color */
    COLOR_GREEN,


    /* Pointer sprite. */
    /* unsigned char pointer[128]; */
    {0x00, 0x00, 0x00,
    0x40, 0x00, 0x00,
    0x60, 0x00, 0x00,
    0x70, 0x00, 0x00,
    0x78, 0x00, 0x00,
    0x7c, 0x00, 0x00,
    0x70, 0x00, 0x00,
    0x58, 0x00, 0x00,
    0x18, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00,
    
    0xc0, 0x00, 0x00,
    0xe0, 0x00, 0x00,
    0xf0, 0x00, 0x00,
    0xf8, 0x00, 0x00,
    0xfc, 0x00, 0x00,
    0xfe, 0x00, 0x00,
    0xfe, 0x00, 0x00,
    0xfc, 0x00, 0x00,
    0xfc, 0x00, 0x00,
    0x3c, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
     0x00 },
    


    /* Background pattern fill. */
    /*    unsigned char backgroundpattern[8*25]; */
      {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 223, 255, 255, 247, 255, 255, 191, 247, 255, 255, 219, 255, 255, 187,
255, 239, 251, 191, 239, 253, 223, 251, 223, 253, 183, 255, 213, 127, 237, 191,
245, 95, 245, 223, 181, 255, 85, 255, 85, 219, 118, 219, 109, 219, 182, 109, 219, 182, 109, 219, 170, 109, 219, 170, 181, 173, 85, 218, 85, 173, 170, 85, 170, 85, 170, 85, 170, 85, 170, 85, 170, 146, 85, 146, 170, 82, 138, 50, 74, 164, 73,
146, 84, 137, 82, 4, 169, 18, 164, 2, 168, 18, 68, 18, 160, 10, 32, 74, 0, 42, 128, 18, 64, 20, 0, 82, 0, 4, 80, 4, 0, 72, 2, 0, 16, 132, 0, 0, 72, 0, 0, 4, 64, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    /* Background colors. */
    /*    unsigned char backgroundpatterncolors[25]; */
      {COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2)},
      
    

  };
