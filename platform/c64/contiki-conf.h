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

#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#define HAVE_LOGSCR

#include "6502def.h"

#if (WITH_PFS && !CFS_IMPL)
#define cfs_opendir  pfs_opendir
#define cfs_closedir pfs_closedir
#define cfs_readdir  pfs_readdir
#endif

#define CTK_CONF_MENU_KEY         CH_F1
#define CTK_CONF_WINDOWSWITCH_KEY CH_F3
#define CTK_CONF_WIDGETUP_KEY     CH_F5
#define CTK_CONF_WIDGETDOWN_KEY   CH_F7

#if WITH_80COL
#define MOUSE_CONF_XTOC(x) ((x) / 4)
#else
#define MOUSE_CONF_XTOC(x) ((x) / 8)
#endif
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

#if WITH_80COL
#define IRC_CONF_WIDTH  80
#else
#define IRC_CONF_WIDTH  40
#endif
#define IRC_CONF_HEIGHT 24

#ifndef TELNETD_CONF_MAX_IDLE_TIME
#define TELNETD_CONF_MAX_IDLE_TIME 300
#endif

#if WITH_80COL
#define WWW_CONF_WEBPAGE_WIDTH  80
#else
#define WWW_CONF_WEBPAGE_WIDTH  40
#endif
#define WWW_CONF_WEBPAGE_HEIGHT 20
#define WWW_CONF_HISTORY_SIZE    4
#define WWW_CONF_WGET_EXEC(url) exec("wget", url)

#endif /* CONTIKI_CONF_H_ */
