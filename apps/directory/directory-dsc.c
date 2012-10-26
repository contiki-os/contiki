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
 *
 */

#include "sys/dsc.h"

/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_ICON_BITMAPS
static unsigned char directoryicon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x43, 0x4c, 0x58, 0x53, 0x60, 0x6f,
  0x00, 0xff, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xff,
  0x00, 0xfe, 0xc2, 0x32, 0x1a, 0xca, 0x06, 0xf6,

  0x40, 0x5f, 0x40, 0x5f, 0x40, 0x5f, 0x40, 0x4f,
  0x00, 0xff, 0x00, 0xff, 0x00, 0xfc, 0x01, 0xf3,
  0x02, 0xfa, 0x02, 0x82, 0x3e, 0xfe, 0xfe, 0xfe,

  0x60, 0x67, 0x50, 0x59, 0x4c, 0x43, 0x7f, 0x00,
  0x07, 0xe7, 0x0f, 0xef, 0x0f, 0x0f, 0xff, 0x00,
  0x8e, 0x06, 0x06, 0x06, 0x8e, 0xfe, 0xfe, 0x00
};
#endif /* CTK_CONF_ICON_BITMAPS */

#if CTK_CONF_ICON_TEXTMAPS
static char directoryicon_textmap[9] = {
  '+', '-', '+',
  '|', 'o', '|',
  '+', '-', '+'
};
#endif /* CTK_CONF_ICON_TEXTMAPS */

#if CTK_CONF_ICONS
static struct ctk_icon directory_icon =
  {CTK_ICON("Directory", directoryicon_bitmap, directoryicon_textmap)};
#endif /* CTK_CONF_ICONS */
/*-----------------------------------------------------------------------------------*/
DSC(directory_dsc,
    "Directory reader",
    "directory.prg",
    directory_process,
    &directory_icon);
/*-----------------------------------------------------------------------------------*/
