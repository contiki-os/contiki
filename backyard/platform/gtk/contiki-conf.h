#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL



#define CCIF
#define CLIF



typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000

#include <stdint.h>

#include "ctk/ctk-arch.h"

#define CTK_CONF_MOUSE_SUPPORT        1

#define CTK_CONF_MENU_KEY             CH_F1

#define CTK_CONF_WINDOWSWITCH_KEY     CH_F3

#define CTK_CONF_WIDGETDOWN_KEY       CH_TAB

#define CTK_CONF_WIDGETUP_KEY         CH_F5

#define CTK_CONF_ICONS                1

#define CTK_CONF_ICON_BITMAPS         1

#define CTK_CONF_ICON_TEXTMAPS        1

#define CTK_CONF_WINDOWS              1

#define CTK_CONF_WINDOWMOVE           1

#define CTK_CONF_WINDOWCLOSE          1

#define CTK_CONF_MENUS                1

#define CTK_CONF_MENUWIDTH            16
#define CTK_CONF_MAXMENUITEMS         10

#define CTK_CONF_WIDGET_FLAGS         1



#define COLOR_BLACK 0
#define COLOR_WHITE 1

#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK
#define BACKGROUNDCOLOR     COLOR_BLACK

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


#define CTK_VNCSERVER_CONF_NUMCONNS 10

#define CTK_VNCSERVER_CONF_MAX_ICONS 16




#define EMAIL_CONF_WIDTH 76
#define EMAIL_CONF_HEIGHT 25


#define IRC_CONF_WIDTH 78
#define IRC_CONF_HEIGHT 21

#define IRC_CONF_SYSTEM_STRING "GTK simulation"


#define LIBCONIO_CONF_SCREEN_WIDTH  80
#define LIBCONIO_CONF_SCREEN_HEIGHT 45



#define LOG_CONF_ENABLED 0


#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 10



#define SHELL_CONF_WITH_PROGRAM_HANDLER 1



#define SHELL_GUI_CONF_XSIZE 46
#define SHELL_GUI_CONF_YSIZE 22



#define TELNETD_CONF_LINELEN 36
#define TELNETD_CONF_NUMLINES 16


/**
 * The 8-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * char" works for most compilers.
 */
typedef unsigned char u8_t;

/**
 * The 16-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * short" works for most compilers.
 */
typedef unsigned short u16_t;

typedef unsigned long u32_t;
typedef          long s32_t;

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;



#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     420

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

#define UIP_CONF_TCP_SPLIT       1

#define UIP_CONF_LOGGING         1

#define UIP_CONF_UDP_CHECKSUMS   1





#define VNC_CONF_REFRESH_ROWS    8


#define WWW_CONF_WEBPAGE_WIDTH 76
#define WWW_CONF_WEBPAGE_HEIGHT 30

#define WWW_CONF_HISTORY_SIZE 40

#define WWW_CONF_MAX_URLLEN 200

#define WWW_CONF_MAX_NUMPAGEWIDGETS 80

#define WWW_CONF_RENDERSTATE 1

#define WWW_CONF_FORMS       1

#define WWW_CONF_MAX_FORMACTIONLEN  200
#define WWW_CONF_MAX_FORMNAMELEN    200
#define WWW_CONF_MAX_INPUTNAMELEN   200
#define WWW_CONF_MAX_INPUTVALUELEN  240

#define WWW_CONF_PAGEVIEW 1


#endif /* __CONTIKI_CONF_H__ */
