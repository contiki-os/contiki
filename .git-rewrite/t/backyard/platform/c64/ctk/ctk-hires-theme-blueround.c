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
 * $Id: ctk-hires-theme-blueround.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

#include <conio.h>
#include "ctk-hires-theme.h"

#define COLOR(bg, fg) ((fg << 4) | (bg))

#define BGCOLOR1 0x06
#define BGCOLOR2 0x00
#define BGCOLOR3 0x06
#define BGCOLOR4 0x00

struct ctk_hires_theme ctk_hires_theme =
  {
    /* Version string. */
    /*    char version[8]; */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

    /* Window borders patterns. */
    /* unsigned char ulcorner[8], */ /* Upper left corner. */
    {0xfe,0xf8,0xe0,0xc0,0xc0,0x80,0x80,0x00},
    
    /* titlebar[8], */            /* Title bar pattern. */
    {0x00,0x55,0xaa,0x00,0xaa,0x00,0x00,0x00},
    
    /* urcorner[8],       */       /* Upper right corner. */
    {0x7f,0x1f,0x07,0x03,0x03,0x01,0x01,0x00},
    
    /* rightborder[8],     */      /* Right border. */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    
    /* lrcorner[8],      */        /* Lower right corner. */
    {0x00,0x01,0x01,0x03,0x03,0x07,0x1f,0x7f},
    
    /* lowerborder[8], */          /* Lower border. */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    
    /* llcorner[8],  */            /* Lower left corner. */
    {0x00,0x80,0x80,0xc0,0xc0,0xe0,0xf8,0xfe},
    
    /* leftborder[8]; */          /* Left border. */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

    /* Button corner patterns. */
    /*    unsigned char buttonleft[8], */
    {0x03,0x0f,0x1f,0x1f,0x1f,0x1f,0x0f,0x03},
    
    /* buttonright[8]; */
    {0xc0,0xf0,0xf8,0xf8,0xf8,0xf8,0xf0,0xc0},
    
      /* Menu border patterns. */
    /*    unsigned char menuleftpattern[8], */
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    
    /* menurightpatterns[8]; */
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    
    /* Window and widget colors. */
    /*    unsigned char windowcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLACK),
     COLOR(COLOR_BLACK, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},
    
    /* separatorcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},
    
    /* labelcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},
    
    /* buttoncolors[6], */
    {COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_CYAN),
     COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_CYAN)},
    
    /* hyperlinkcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_CYAN),
     COLOR(COLOR_BLUE, COLOR_WHITE),
     COLOR(COLOR_LIGHTBLUE, COLOR_CYAN),
     COLOR(COLOR_LIGHTBLUE, COLOR_WHITE)},
    
    /* textentrycolors[6],  */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLACK, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLACK, COLOR_CYAN),
     COLOR(COLOR_WHITE, COLOR_BLUE),
     COLOR(COLOR_WHITE, COLOR_BLACK)},
    
    /* bitmapcolors[6], */
    {COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_CYAN),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_CYAN)},
    
    /* textmapcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},
    
    /* iconcolors[6]; */
    {COLOR(COLOR_BLUE, COLOR_BLACK),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTGREEN, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLACK),
     COLOR(COLOR_LIGHTBLUE, COLOR_CYAN)},


    /* Button corner colors. */
    /* unsigned char buttonleftcolors[6], */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},
    
    /* buttonrightcolors[6]; */
    {COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLACK, COLOR_BLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
     COLOR(COLOR_LIGHTBLUE, COLOR_BLUE)},

    
    /* Menu colors. */
    /*    unsigned char menucolor,*/
    COLOR(COLOR_BLUE, COLOR_LIGHTBLUE),
    
    /*    openmenucolor, */
    COLOR(COLOR_LIGHTBLUE, COLOR_BLUE),
    
    /* activemenucolor; */
    COLOR(COLOR_CYAN, COLOR_BLUE),
    
    /* Border and screen colors. */
    /*    unsigned char bordercolor,*/
    0,
    
    /* screencolor; */
    0,

    /* Pointer sprite 0 color */
    /*unsigned char pointermaskcolor,*/
    3,

    /* Pointer sprite 1 color */
    /*     pointercolor; */
    6,

    
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
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
     0x00,0x00,0x10,0x38,0x10,0x00,0x00,0x00,
     0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,
     0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00,
     0x00,0x3c,0x7e,0x7e,0x7e,0x7e,0x3c,0x00,
     0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c,
     0x7e,0xff,0xff,0xff,0xff,0xff,0xff,0x7e,
     
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
     0x00,0x00,0x10,0x38,0x10,0x00,0x00,0x00,
     0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,
     0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00,
     0x00,0x3c,0x7e,0x7e,0x7e,0x7e,0x3c,0x00,
     0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c,
     0x7e,0xff,0xff,0xff,0xff,0xff,0xff,0x7e,

     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
     0x00,0x00,0x10,0x38,0x10,0x00,0x00,0x00,
     0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,
     0x00,0x18,0x3c,0x7e,0x7e,0x3c,0x18,0x00,
     0x00,0x3c,0x7e,0x7e,0x7e,0x7e,0x3c,0x00,
     0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c,
     0x7e,0xff,0xff,0xff,0xff,0xff,0xff,0x7e,

     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    },

    /* Background colors. */
    /*    unsigned char backgroundpatterncolors[25]; */
      {
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),
       COLOR(BGCOLOR1,BGCOLOR2),COLOR(BGCOLOR1,BGCOLOR2),

       COLOR(BGCOLOR2,BGCOLOR3),COLOR(BGCOLOR2,BGCOLOR3),
       COLOR(BGCOLOR2,BGCOLOR3),COLOR(BGCOLOR2,BGCOLOR3),
       COLOR(BGCOLOR2,BGCOLOR3),COLOR(BGCOLOR2,BGCOLOR3),
       COLOR(BGCOLOR2,BGCOLOR3),COLOR(BGCOLOR2,BGCOLOR3),

       COLOR(BGCOLOR3,BGCOLOR4),COLOR(BGCOLOR3,BGCOLOR4),
       COLOR(BGCOLOR3,BGCOLOR4),COLOR(BGCOLOR3,BGCOLOR4),
       COLOR(BGCOLOR3,BGCOLOR4),COLOR(BGCOLOR3,BGCOLOR4),
       COLOR(BGCOLOR3,BGCOLOR4),COLOR(BGCOLOR3,BGCOLOR4),

      COLOR(BGCOLOR4,BGCOLOR4)},
      
    

  };
