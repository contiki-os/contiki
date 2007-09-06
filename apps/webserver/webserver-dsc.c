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
 * $Id: webserver-dsc.c,v 1.3 2007/09/06 01:36:12 matsutsuka Exp $
 *
 */

#include "sys/dsc.h"

/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_ICON_BITMAPS
static unsigned char webservericon_bitmap[3*3*8] = {
  0x00, 0x7f, 0x40, 0x41, 0x44, 0x48, 0x40, 0x50,
  0x00, 0xff, 0x5a, 0x00, 0x00, 0x00, 0x3c, 0x81,
  0x00, 0xfe, 0x02, 0x82, 0x22, 0x12, 0x02, 0x0a,

  0x41, 0x60, 0x42, 0x62, 0x62, 0x42, 0x60, 0x41,
  0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18,
  0x82, 0x06, 0x42, 0x46, 0x46, 0x42, 0x06, 0x82,

  0x50, 0x40, 0x48, 0x44, 0x41, 0x40, 0x7e, 0x00,
  0xc5, 0x34, 0x3c, 0x52, 0x7a, 0x7e, 0xa1, 0xfd,
  0x0a, 0x02, 0x12, 0x22, 0x82, 0x02, 0x7e, 0x00
};
#endif /* CTK_CONF_ICON_BITMAPS */

#if CTK_CONF_ICON_TEXTMAPS
static char webservericon_textmap[9] = {
  '+', '-', '+',
  '|', ')', '|',
  '+', '-', '+'
};
#endif /* CTK_CONF_ICON_TEXTMAPS */

#if CTK_CONF_ICONS
static struct ctk_icon webserver_icon =
  {CTK_ICON("Web server", webservericon_bitmap, webservericon_textmap)};
#endif /* CTK_CONF_ICONS */
/*-----------------------------------------------------------------------------------*/
DSC(webserver_dsc,
    "The Contiki web server",
    "webserver.prg",
    webserver_process,
    &webserver_icon);
/*-----------------------------------------------------------------------------------*/
