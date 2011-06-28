#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_FASTCALL __fastcall
#define CC_CONF_INLINE   __inline


#if _USRDLL
#define CCIF __declspec(dllimport)
#else /* _USRDLL */
#define CCIF __declspec(dllexport)
#endif /* _USRDLL */
#define CLIF __declspec(dllexport)

#ifdef __CYGWIN__
int strcasecmp(const char*, const char*);
char* strdup(const char*);
#else /* __CYGWIN__ */
#define HAVE_SNPRINTF
#define snprintf   _snprintf
#define strcasecmp _stricmp
#define strdup     _strdup
#endif /* __CYGWIN__ */


#define LOG_CONF_ENABLED 1
#include "sys/log.h"
CCIF void debug_printf(char *format, ...);


#define CLOCK_CONF_SECOND 1000
typedef unsigned long clock_time_t;


typedef   signed char    int8_t;
typedef unsigned char   uint8_t;
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
typedef   signed int    int32_t;
typedef unsigned int   uint32_t;

/* These names are deprecated, use C99 names. */
typedef unsigned char   u8_t;
typedef unsigned short u16_t;
typedef unsigned long  u32_t;
typedef          long  s32_t;

typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     420
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT       1
#if UIP_CONF_IPV6
#define UIP_CONF_IP_FORWARD 0
#define UIP_CONF_DS6_NBR_NBU     100
#define UIP_CONF_DS6_DEFRT_NBU   2
#define UIP_CONF_DS6_PREFIX_NBU  5
#define UIP_CONF_DS6_ROUTE_NBU   100
#define UIP_CONF_DS6_ADDR_NBU    10
#define UIP_CONF_DS6_MADDR_NBU   0  //VC++ does not allow zero length arrays
#define UIP_CONF_DS6_AADDR_NBU   0  //inside a struct
#else
#define UIP_CONF_IP_FORWARD      1
#endif
#define UIP_CONF_LOGGING         1
#define UIP_CONF_UDP_CHECKSUMS   1


#include <ctype.h>
#define ctk_arch_isprint isprint

#include "ctk/ctk-console.h"

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

#define CTK_CONF_MENU_KEY         -5  /* F10 */
#define CTK_CONF_WINDOWSWITCH_KEY -6  /* Ctrl-Tab */
#define CTK_CONF_WIDGETUP_KEY     -7  /* Shift-Tab */
#define CTK_CONF_WIDGETDOWN_KEY   '\t'
#define CTK_CONF_WIDGET_FLAGS     0
#define CTK_CONF_SCREENSAVER      0

#ifdef PLATFORM_BUILD
#define CTK_CONF_MOUSE_SUPPORT    1
#define CTK_CONF_WINDOWS          1
#define CTK_CONF_WINDOWMOVE       1
#define CTK_CONF_WINDOWCLOSE      1
#define CTK_CONF_ICONS            1
#define CTK_CONF_ICON_BITMAPS     0
#define CTK_CONF_ICON_TEXTMAPS    1
#define CTK_CONF_MENUS            1
#define CTK_CONF_MENUWIDTH        16
#define CTK_CONF_MAXMENUITEMS     10
#else /* PLATFORM_BUILD */
#define CTK_CONF_MOUSE_SUPPORT    1
#define CTK_CONF_WINDOWS          0
#define CTK_CONF_WINDOWMOVE       0
#define CTK_CONF_WINDOWCLOSE      0
#define CTK_CONF_ICONS            0
#define CTK_CONF_MENUS            0
#endif /* PLATFORM_BUILD */

#define COLOR_BLACK  (0)
#define COLOR_BLUE   (1)
#define COLOR_GRAY   (1 | 2 | 4)
#define COLOR_CYAN   (1 | 2 | 8)
#define COLOR_YELLOW (2 | 4 | 8)
#define COLOR_WHITE  (1 | 2 | 4 | 8)

#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK
#define BACKGROUNDCOLOR     COLOR_BLACK
#define WINDOWCOLOR_FOCUS   COLOR_WHITE  | COLOR_BLUE * 0x10
#define WINDOWCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define DIALOGCOLOR         COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_HLINK   COLOR_CYAN   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FWIN    COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR         COLOR_GRAY   | COLOR_BLUE * 0x10
#define WIDGETCOLOR_DIALOG  COLOR_WHITE  | COLOR_BLUE * 0x10
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW | COLOR_BLUE * 0x10
#define MENUCOLOR           COLOR_WHITE  | COLOR_BLUE * 0x10
#define OPENMENUCOLOR       COLOR_WHITE  | COLOR_BLUE * 0x10
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW | COLOR_BLUE * 0x10


#ifdef PLATFORM_BUILD
#define LOADER_CONF_ARCH "loader/dll-loader.h"
#else /* PLATFORM_BUILD */
#define LOADER_CONF_ARCH "loader/unload.h"
#endif /* PLATFORM_BUILD */

#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 10
#define PROGRAM_HANDLER_CONF_QUIT_MENU   1


#define EMAIL_CONF_WIDTH  76
#define EMAIL_CONF_HEIGHT 30
#ifndef PLATFORM_BUILD
#define EMAIL_CONF_ERASE   0
#endif

#define IRC_CONF_WIDTH         78
#define IRC_CONF_HEIGHT        30
#define IRC_CONF_SYSTEM_STRING "Win32"


#define SHELL_CONF_WITH_PROGRAM_HANDLER 1


#define SHELL_GUI_CONF_XSIZE 78
#define SHELL_GUI_CONF_YSIZE 30


#ifdef PLATFORM_BUILD
#define TELNETD_CONF_GUI 1
#endif /* PLATFORM_BUILD */


#ifdef PLATFORM_BUILD
#define WWW_CONF_WEBPAGE_WIDTH  76
#define WWW_CONF_WEBPAGE_HEIGHT 30
#endif /* PLATFORM_BUILD */

#endif /* __CONTIKI_CONF_H__ */
