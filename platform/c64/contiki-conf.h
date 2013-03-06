/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "6502def.h"

#define CTK_CONF_MENU_KEY         CH_F1
#define CTK_CONF_WINDOWSWITCH_KEY CH_F3
#define CTK_CONF_WIDGETUP_KEY     CH_F5
#define CTK_CONF_WIDGETDOWN_KEY   CH_F7

#define MOUSE_CONF_XTOC(x) ((x) / 8)
#define MOUSE_CONF_YTOC(y) ((y) / 8)

#define BORDERCOLOR       COLOR_BLACK
#define SCREENCOLOR       COLOR_BLACK
#define BACKGROUNDCOLOR   COLOR_BLACK
#define WINDOWCOLOR       COLOR_GRAY3
#define WINDOWCOLOR_FOCUS COLOR_GRAY3
#define WIDGETCOLOR       COLOR_GRAY3
#define WIDGETCOLOR_FOCUS COLOR_YELLOW
#define WIDGETCOLOR_FWIN  COLOR_GRAY3
#define WIDGETCOLOR_HLINK COLOR_CYAN

#define EMAIL_CONF_WIDTH  39
#define EMAIL_CONF_HEIGHT 20
#define EMAIL_CONF_ERASE   0

#define FTP_CONF_WIDTH  18
#define FTP_CONF_HEIGHT 22

#define IRC_CONF_WIDTH  40
#define IRC_CONF_HEIGHT 24

#define WWW_CONF_WEBPAGE_WIDTH      40
#define WWW_CONF_WEBPAGE_HEIGHT     20
#define WWW_CONF_HISTORY_SIZE        4
#define WWW_CONF_MAX_URLLEN         80
#define WWW_CONF_MAX_NUMPAGEWIDGETS 20
#define WWW_CONF_FORMS               1
#define WWW_CONF_MAX_FORMACTIONLEN  20
#define WWW_CONF_MAX_INPUTNAMELEN   20
#define WWW_CONF_MAX_INPUTVALUELEN  20

#endif /* __CONTIKI_CONF_H__ */
