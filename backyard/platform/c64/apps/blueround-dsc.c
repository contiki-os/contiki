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
 * This file is part of the Contiki desktop environment
 *
 * $Id: blueround-dsc.c,v 1.2 2007/09/06 01:36:11 matsutsuka Exp $
 *
 */

#include "sys/dsc.h"

/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_ICON_BITMAPS
static unsigned char blueroundicon_bitmap[3*3*8] = {
  0xfe,0xf8,0xe0,0xc0,0xc0,0x80,0x80,0x00,
  0x00,0x55,0xaa,0x00,0xaa,0x00,0x00,0x00,
  0x7f,0x1f,0x07,0x03,0x03,0x01,0x01,0x00,

  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

  0x00,0x80,0x80,0xc0,0xc0,0xe0,0xf8,0xfe,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0x01,0x03,0x03,0x07,0x1f,0x7f
};
#endif /* CTK_CONF_ICON_BITMAPS */

#if CTK_CONF_ICON_TEXTMAPS
static char blueroundicon_textmap[9] = {
  '(', '=', ')',
  '|', 'o', '|',
  '(', '-', ')'
};
#endif /* CTK_CONF_ICON_TEXTMAPS */

#if CTK_CONF_ICONS
static struct ctk_icon blueround_icon =
  {CTK_ICON("Blueround", blueroundicon_bitmap, blueroundicon_textmap)};
#endif /* CTK_CONF_ICONS */
/*-----------------------------------------------------------------------------------*/
DSC(blueround_dsc,
    "Blueround CTK theme",
    "blueround.prg",
    blueround_init,
    &blueround_icon);
/*-----------------------------------------------------------------------------------*/
