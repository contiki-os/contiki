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
static unsigned char vncicon_bitmap[3*3*8] = {
  0x00, 0x7e, 0x40, 0x73, 0x46, 0x4c, 0x18, 0x13,
  0x00, 0x00, 0xff, 0x81, 0x34, 0xc9, 0x00, 0xb6,
  0x00, 0x7e, 0x02, 0xce, 0x72, 0x32, 0x18, 0x48,

  0x30, 0x27, 0x24, 0x20, 0x37, 0x24, 0x20, 0x33,
  0x00, 0x7b, 0x42, 0x00, 0x7b, 0x42, 0x00, 0x3b,
  0x0c, 0x24, 0x24, 0x04, 0xa4, 0x24, 0x04, 0x4c,

  0x12, 0x19, 0x4c, 0x46, 0x63, 0x40, 0x7c, 0x00,
  0x22, 0x91, 0x00, 0xc4, 0x81, 0xff, 0x00, 0x00,
  0x08, 0x18, 0x32, 0x62, 0xc6, 0x02, 0x3e, 0x00
};
#endif /* CTK_CONF_ICON_BITMAPS */

#if CTK_CONF_ICON_TEXTMAPS
static char vncicon_textmap[9] = {
  'V', 'N', 'C',
  'c', 'l', 'i',
  'e', 'n', 't'
};
#endif /* CTK_CONF_ICON_TEXTMAPS */

#if CTK_CONF_ICONS
static struct ctk_icon vnc_icon =
  {CTK_ICON("VNC viewer", vncicon_bitmap, vncicon_textmap)};
#endif /* CTK_CONF_ICONS */
/*-----------------------------------------------------------------------------------*/
DSC(vnc_dsc,
    "Remote control your PC using Contiki",
    "vnc.prg",
    vnc_process,
    &vnc_icon);
/*-----------------------------------------------------------------------------------*/
