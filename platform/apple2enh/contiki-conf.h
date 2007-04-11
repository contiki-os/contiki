#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__



#include "6502-conf.h"



#define LOG_CONF_ENABLED 1
#include "sys/log.h"
void debug_printf(char *format, ...);
void error_exit(char *message);



#define CLOCK_CONF_SECOND 2
typedef unsigned short clock_time_t;



#define CH_ULCORNER	          0xDA
#define CH_URCORNER	          0xBF
#define CH_LLCORNER	          0xC0
#define CH_LRCORNER	          0xD9
#define CH_ENTER	          '\r'
#define CH_DEL		          '\b'
#define CH_CURS_UP  	          -1
#define CH_CURS_LEFT	          -2
#define CH_CURS_RIGHT	          -3
#define CH_CURS_DOWN	          -4

#define CTK_CONF_MENU_KEY         CH_ESC
#define CTK_CONF_WINDOWSWITCH_KEY 0x17	/* Ctrl-W */
#define CTK_CONF_WIDGETDOWN_KEY   '\t'	/* Tab or Ctrl-I */
#define CTK_CONF_WIDGETUP_KEY     0x01	/* Ctrl-A */
#define CTK_CONF_MOUSE_SUPPORT    0
#define CTK_CONF_ICONS            1
#define CTK_CONF_ICON_BITMAPS     0
#define CTK_CONF_ICON_TEXTMAPS    1
#define CTK_CONF_WINDOWMOVE       1
#define CTK_CONF_WINDOWCLOSE      1
#define CTK_CONF_MENUS            1
#define CTK_CONF_MENUWIDTH        16
#define CTK_CONF_MAXMENUITEMS     10
#define CTK_CONF_WIDGET_FLAGS     0
#define CTK_CONF_SCREENSAVER      0

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



#define EMAIL_CONF_WIDTH 48
#define EMAIL_CONF_HEIGHT 16



#define IRC_CONF_WIDTH 78
#define IRC_CONF_HEIGHT 20
#define IRC_CONF_SYSTEM_STRING "Apple2"



//#define LOADER_CONF_ARCH "loader/???-loader.h"

#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 6
#define PROGRAM_HANDLER_CONF_QUIT_MENU   1



#define SHELL_GUI_CONF_XSIZE 38
#define SHELL_GUI_CONF_YSIZE 12



#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 10
#define UIP_CONF_BUFFER_SIZE     (1024 - 2)
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP_CHECKSUMS   1



#define WWW_CONF_WEBPAGE_WIDTH      78
#define WWW_CONF_WEBPAGE_HEIGHT     16
#define WWW_CONF_HISTORY_SIZE       4
#define WWW_CONF_MAX_URLLEN         78
#define WWW_CONF_MAX_NUMPAGEWIDGETS 26
#define WWW_CONF_RENDERSTATE        1
#define WWW_CONF_FORMS              1
#define WWW_CONF_MAX_FORMACTIONLEN  30
#define WWW_CONF_MAX_FORMNAMELEN    20
#define WWW_CONF_MAX_INPUTNAMELEN   20
#define WWW_CONF_MAX_INPUTVALUELEN  (WWW_CONF_WEBPAGE_WIDTH - 1)

#endif /* __CONTIKI_CONF_H__ */
