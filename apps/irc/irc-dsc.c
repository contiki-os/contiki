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
static unsigned char irc_icon_bitmap[3*3*8] = {
  0x00, 0x79, 0x43, 0x73, 0x47, 0x77, 0x47, 0x6f,
  0x00, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xf8, 0xfb,
  0x00, 0x16, 0x02, 0x00, 0x02, 0x00, 0x00, 0xc2,

  0x48, 0x4c, 0x5f, 0x5f, 0x1f, 0x3f, 0x3f, 0x03,
  0x79, 0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xfe, 0xfc,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x77, 0x47, 0x70, 0x43, 0x79, 0x41, 0x7c, 0x00,
  0xfc, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xf7, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x84, 0xf0, 0x00
};
#endif /* CTK_CONF_ICON_BITMAPS */

#if CTK_CONF_ICON_TEXTMAPS
static char irc_icon_textmap[9] = {
  'I', 'R', 'C',
  '-', '-', '-',
  'I', 'R', 'C'
};
#endif /* CTK_CONF_ICON_TEXTMAPS */

#if CTK_CONF_ICONS
static struct ctk_icon irc_icon =
  {CTK_ICON("IRC client", irc_icon_bitmap, irc_icon_textmap)};
#endif /* CTK_CONF_ICONS */
/*-----------------------------------------------------------------------------------*/
DSC(irc_dsc,
    "Internet Relay Chat client",
    "irc.prg",
    irc_process,
    &irc_icon);
/*-----------------------------------------------------------------------------------*/
