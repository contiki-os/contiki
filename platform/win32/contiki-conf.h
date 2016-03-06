#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#ifdef __CYGWIN__
#include <sys/types.h>
#endif

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_INLINE        __inline

#define ARCH_DOESNT_NEED_ALIGNED_STRUCTS 1


#if _USRDLL
#define CCIF __declspec(dllimport)
#else /* _USRDLL */
#define CCIF __declspec(dllexport)
#endif /* _USRDLL */
#define CLIF __declspec(dllexport)

#ifdef __CYGWIN__
int strcasecmp(const char*, const char*);
int strncasecmp(const char*, const char*, size_t);
char* strdup(const char*);
#else /* __CYGWIN__ */
#define HAVE_SNPRINTF
#define snprintf    _snprintf
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#define strdup      _strdup
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

#define UIP_CONF_LLH_LEN             14
#define UIP_CONF_BUFFER_SIZE         1514
#define UIP_CONF_TCP_SPLIT           1
#define UIP_CONF_LOGGING             1
#define UIP_CONF_UDP_CHECKSUMS       1
#define UIP_CONF_IP_FORWARD          0
#if NETSTACK_CONF_WITH_IPV6
#define NBR_TABLE_CONF_MAX_NEIGHBORS 100
#define UIP_CONF_DS6_DEFRT_NBU       2
#define UIP_CONF_DS6_PREFIX_NBU      5
#define UIP_CONF_MAX_ROUTES          100
#define UIP_CONF_DS6_ADDR_NBU        10
#define UIP_CONF_DS6_MADDR_NBU       0  //VC++ does not allow zero length arrays
#define UIP_CONF_DS6_AADDR_NBU       0  //inside a struct
#endif

#define RESOLV_CONF_SUPPORTS_MDNS              0
#define RESOLV_CONF_SUPPORTS_RECORD_EXPIRATION 0

#include <ctype.h>
#define ctk_arch_isprint isprint

#include "ctk/ctk-console.h"

#define CH_ULCORNER               '+'
#define CH_URCORNER               '+'
#define CH_LLCORNER               '+'
#define CH_LRCORNER               '+'
#define CH_ENTER                  '\r'
#define CH_DEL                    '\b'
#define CH_CURS_UP                -1
#define CH_CURS_LEFT              -2
#define CH_CURS_RIGHT             -3
#define CH_CURS_DOWN              -4

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

#define CTK_COLOR_BLACK  (0)
#define CTK_COLOR_BLUE   (1)
#define CTK_COLOR_GRAY   (1 | 2 | 4)
#define CTK_COLOR_CYAN   (1 | 2 | 8)
#define CTK_COLOR_YELLOW (2 | 4 | 8)
#define CTK_COLOR_WHITE  (1 | 2 | 4 | 8)

#define COLOR_BG CTK_COLOR_BLUE

#define BORDERCOLOR         CTK_COLOR_BLACK
#define SCREENCOLOR         CTK_COLOR_BLACK
#define BACKGROUNDCOLOR     CTK_COLOR_BLACK
#define WINDOWCOLOR_FOCUS   CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define WINDOWCOLOR         CTK_COLOR_GRAY   | COLOR_BG * 0x10
#define DIALOGCOLOR         CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define WIDGETCOLOR_HLINK   CTK_COLOR_CYAN   | COLOR_BG * 0x10
#define WIDGETCOLOR_FWIN    CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define WIDGETCOLOR         CTK_COLOR_GRAY   | COLOR_BG * 0x10
#define WIDGETCOLOR_DIALOG  CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define WIDGETCOLOR_FOCUS   CTK_COLOR_YELLOW | COLOR_BG * 0x10
#define MENUCOLOR           CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define OPENMENUCOLOR       CTK_COLOR_WHITE  | COLOR_BG * 0x10
#define ACTIVEMENUITEMCOLOR CTK_COLOR_YELLOW | COLOR_BG * 0x10


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


#define TELNETD_CONF_MAX_IDLE_TIME 300
#ifdef PLATFORM_BUILD
#define TELNETD_CONF_GUI             1
#endif /* PLATFORM_BUILD */


#ifdef PLATFORM_BUILD
#define WWW_CONF_WEBPAGE_WIDTH  76
#define WWW_CONF_WEBPAGE_HEIGHT 30
#else /* PLATFORM_BUILD */
#define WWW_CONF_WGET_EXEC(url) execlp("wget.win32", "wget.win32", \
                                       "192.168.0.2", url, (char *)NULL)
#endif /* PLATFORM_BUILD */

#endif /* CONTIKI_CONF_H_ */
