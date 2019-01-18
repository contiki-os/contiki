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
 * $Id: ctk-80col-theme.h,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */
#ifndef __CTK_80COL_THEME_H__
#define __CTK_80COL_THEME_H__

struct ctk_80col_theme {
  /* Version string. */
  char version[8];
  
  /* Window borders patterns. */
  unsigned char ulcorner[8], /* Upper left corner. */
    titlebar[8],             /* Title bar pattern. */
    urcorner[8],             /* Upper right corner. */
    rightborder[8],          /* Right border. */
    lrcorner[8],             /* Lower right corner. */
    lowerborder[8],          /* Lower border. */
    llcorner[8],             /* Lower left corner. */
    leftborder[8];           /* Left border. */

  /* Button corner patterns. */
  unsigned char buttonleftpattern[8],
    buttonrightpattern[8];

  /* Menu border patterns. */
  unsigned char menuleftpattern[8],
    menurightpattern[8];
  


  /* Window and widget colors. */
  unsigned char windowcolors[6],
    separatorcolors[6],
    labelcolors[6],
    buttoncolors[6],
    hyperlinkcolors[6],
    textentrycolors[6],
    bitmapcolors[6],
    textmapcolors[6],
    iconcolors[6];

  /* Menu colors. */
  unsigned char menucolor,
    openmenucolor,
    activemenucolor;

};

extern struct ctk_80col_theme ctk_80col_theme;
extern struct ctk_80col_theme *ctk_80col_theme_ptr;

#endif /* __CTK_80COL_THEME_H__ */
