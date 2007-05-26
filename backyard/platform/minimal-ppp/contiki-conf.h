/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: contiki-conf.h,v 1.1 2007/05/26 07:21:01 oliverschmidt Exp $
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <inttypes.h>

#include "ctk/ctk-vncarch.h"
#define TELNETD_CONF_LINELEN 80
#define TELNETD_CONF_NUMLINES 16

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     420
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP_CHECKSUMS   1

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000

#define LOG_CONF_ENABLED 1

#define CTK_CONF_MAXMENUITEMS	10

/* The size of the HTML viewing area. */
#define WWW_CONF_WEBPAGE_WIDTH 76
#define WWW_CONF_WEBPAGE_HEIGHT 30

/* The size of the "Back" history. */
#define WWW_CONF_HISTORY_SIZE 40

/* Defines the maximum length of an URL */
#define WWW_CONF_MAX_URLLEN 200

/* The maxiumum number of widgets (i.e., hyperlinks, form elements) on
   a page. */
#define WWW_CONF_MAX_NUMPAGEWIDGETS 80

/* Turns <center> support on or off; must be on for forms to work. */
#define WWW_CONF_RENDERSTATE 1

/* Toggles support for HTML forms. */
#define WWW_CONF_FORMS       1

/* Maximum lengths for various HTML form parameters. */
#define WWW_CONF_MAX_FORMACTIONLEN  200
#define WWW_CONF_MAX_FORMNAMELEN    200
#define WWW_CONF_MAX_INPUTNAMELEN   200
#define WWW_CONF_MAX_INPUTVALUELEN  240

#define WWW_CONF_PAGEVIEW 1


/*------------------------------------------------------------------------------*/

#define LIBCONIO_CONF_SCREEN_WIDTH  80
#define LIBCONIO_CONF_SCREEN_HEIGHT 45
/*------------------------------------------------------------------------------*/

#include "ctk-arch.h"

/*
 * This file is used for setting various compile time settings for the
 * CTK GUI toolkit.
*/

/* Toggles mouse support (must have support functions in the
architecture specific files to work). */
#define CTK_CONF_MOUSE_SUPPORT        1

/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY             CH_F1

/* Defines which key that is to be used for switching the frontmost
   window.  */
#define CTK_CONF_WINDOWSWITCH_KEY     CH_F3

/* Defines which key that is to be used for switching to the next
   widget.  */
#define CTK_CONF_WIDGETDOWN_KEY       CH_TAB

/* Defines which key that is to be used for switching to the prevoius
   widget.  */
#define CTK_CONF_WIDGETUP_KEY         CH_F5

/* Toggles support for icons. */
#define CTK_CONF_ICONS                1 /* 107 bytes */

/* Toggles support for icon bitmaps. */
#define CTK_CONF_ICON_BITMAPS         1

/* Toggles support for icon textmaps. */
#define CTK_CONF_ICON_TEXTMAPS        1

/* Toggles support for movable windows. */
#define CTK_CONF_WINDOWMOVE           1 /* 333 bytes */

/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE          1 /* 14 bytes */

/* Toggles support for menus. */
#define CTK_CONF_MENUS                1 /* 1384 bytes */

/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH            16
/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS         10

#define CTK_CONF_WIDGET_FLAGS         1


/*------------------------------------------------------------------------------*/

#define COLOR_BLACK 0
#define COLOR_WHITE 1

#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK

#define WINDOWCOLOR_FOCUS   COLOR_WHITE
#define WINDOWCOLOR         COLOR_WHITE

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_WHITE
#define WIDGETCOLOR_FWIN    COLOR_WHITE
#define WIDGETCOLOR         COLOR_WHITE
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_WHITE

#define MENUCOLOR           COLOR_WHITE
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_WHITE


/*------------------------------------------------------------------------------*/

/* Maximum number of clients to the telnet server */
#define CTK_TERM_CONF_MAX_TELNET_CLIENTS 3

/* Telnet server port */
#define CTK_TERM_CONF_TELNET_PORT 23

/* Serial server output buffer size */
#define CTK_TERM_CONF_SERIAL_BUFFER_SIZE 300

/* Maximum number of clients to the terminal module.
   Should be set to CTK_TERM_CONF_MAX_TELNET_CLIENTS or
   CTK_TERM_CONF_MAX_TELNET_CLIENTS+1 if the serial server is used too
*/
#define CTK_TERM_CONF_MAX_CLIENTS (CTK_TERM_CONF_MAX_TELNET_CLIENTS+1)


/*------------------------------------------------------------------------------*/

#define CTK_VNCSERVER_CONF_NUMCONNS 10

#define CTK_VNCSERVER_CONF_MAX_ICONS 16
/**
 * Some PPP definitions
 */

#define clock_seconds()		time(NULL)

#define	UIP_CONF_LLH_LEN	0

#define IPCP_RETRY_COUNT        5
#define IPCP_TIMEOUT            5
#define xxdebug_printf          printf
#define debug_printf
#define LCP_RETRY_COUNT         10
#define PAP_USERNAME_SIZE       16
#define PAP_PASSWORD_SIZE       16
#define PPP_RX_BUFFER_SIZE      1024


#endif /* __CONTIKI_CONF_H__ */
