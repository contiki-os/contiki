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

#include "6502def.h"

#define CTK_CONF_MENU_KEY         CH_ESC
#define CTK_CONF_WINDOWSWITCH_KEY 0x17  /* Ctrl-W */
#define CTK_CONF_WIDGETUP_KEY     0x01  /* Ctrl-A */
#define CTK_CONF_WIDGETDOWN_KEY   '\t'  /* Tab or Ctrl-I */

#define MOUSE_CONF_XTOC(x) ((x) * 2 / 7)
#define MOUSE_CONF_YTOC(y) ((y) / 8)

#define EMAIL_CONF_WIDTH  79
#define EMAIL_CONF_HEIGHT 19
#define EMAIL_CONF_ERASE   0

#define FTP_CONF_WIDTH  38
#define FTP_CONF_HEIGHT 21

#define IRC_CONF_WIDTH  80
#define IRC_CONF_HEIGHT 23

#ifndef TELNETD_CONF_MAX_IDLE_TIME
#define TELNETD_CONF_MAX_IDLE_TIME 300
#endif

#define WWW_CONF_WEBPAGE_HEIGHT 19
#define WWW_CONF_HISTORY_SIZE    4
#define WWW_CONF_WGET_EXEC(url) exec("wget", url)

#endif /* CONTIKI_CONF_H_ */
