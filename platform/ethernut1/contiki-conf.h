#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CCIF
#define CLIF

typedef unsigned short clock_time_t;
#define CLOCK_CONF_SECOND 1000

void clock_delay(unsigned int us2);

void clock_wait(int ms10);

void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);


/*
 * This file is used for setting various compile time settings for the
 * CTK GUI toolkit.
*/

#include "ctk/ctk-vncarch.h"

/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY             CH_F1

/* Defines which key that is to be used for switching the frontmost
   window.  */
#define CTK_CONF_WINDOWSWITCH_KEY     CH_ESC

/* Defines which key that is to be used for switching to the next
   widget.  */
#define CTK_CONF_WIDGETDOWN_KEY       CH_TAB

/* Defines which key that is to be used for switching to the prevoius
   widget.  */
#define CTK_CONF_WIDGETUP_KEY         1

/* Toggles support for icons. */
#define CTK_CONF_ICONS                1 /* 107 bytes */

/* Toggles support for icon bitmaps. */
#define CTK_CONF_ICON_BITMAPS         1

/* Toggles support for icon textmaps. */
#define CTK_CONF_ICON_TEXTMAPS        1

/* Toggles support for windows. */
#define CTK_CONF_WINDOWS              1

/* Toggles support for movable windows. */
#define CTK_CONF_WINDOWMOVE           1 /* 333 bytes */

/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE          1 /* 14 bytes */

/* Toggles support for menus. */
#define CTK_CONF_MENUS                1 /* 1384 bytes */

/* Toggles mouse support (must have support functions in the
architecture specific files to work). */
#define CTK_CONF_MOUSE_SUPPORT        1

/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH            16
/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS         10


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

#define CTK_VNCSERVER_CONF_NUMCONNS 8

#define CTK_VNCSERVER_CONF_MAX_ICONS 8

#define EMAIL_CONF_WIDTH 48
#define EMAIL_CONF_HEIGHT 16

#define IRC_CONF_WIDTH 78
#define IRC_CONF_HEIGHT 20

#define IRC_CONF_SYSTEM_STRING "ethernut"


#define LIBCONIO_CONF_SCREEN_WIDTH  70
#define LIBCONIO_CONF_SCREEN_HEIGHT 40



#define LOG_CONF_ENABLED 0

#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 10

/* COM port to be used for SLIP connection */
#define SLIP_PORT RS232_PORT_0

#define TELNETD_CONF_LINELEN 64
#define TELNETD_CONF_NUMLINES 16


#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 10
#define UIP_CONF_BUFFER_SIZE     100

#define UIP_CONF_TCP_SPLIT       1

#define UIP_CONF_UDP_CONNS       6

#define UIP_CONF_FWCACHE_SIZE    1

#define UIP_CONF_BROADCAST       1


/* The size of the HTML viewing area. */
#define WWW_CONF_WEBPAGE_WIDTH 46
#define WWW_CONF_WEBPAGE_HEIGHT 25

/* The size of the "Back" history. */
#define WWW_CONF_HISTORY_SIZE 8

/* Defines the maximum length of an URL */
#define WWW_CONF_MAX_URLLEN 160

/* The maxiumum number of widgets (i.e., hyperlinks, form elements) on
   a page. */
#define WWW_CONF_MAX_NUMPAGEWIDGETS 30

/* Turns <center> support on or off; must be on for forms to work. */
#define WWW_CONF_RENDERSTATE 1

/* Toggles support for HTML forms. */
#define WWW_CONF_FORMS       1

/* Maximum lengths for various HTML form parameters. */
#define WWW_CONF_MAX_FORMACTIONLEN  80
#define WWW_CONF_MAX_FORMNAMELEN    40
#define WWW_CONF_MAX_INPUTNAMELEN   40
#define WWW_CONF_MAX_INPUTVALUELEN  40

#define WWW_CONF_PAGEVIEW 1

#define HAVE_STDINT_H
#include "avrdef.h"

typedef unsigned short uip_stats_t;


#endif /* __CONTIKI_CONF_H__ */
