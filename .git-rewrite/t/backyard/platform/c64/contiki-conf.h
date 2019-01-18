#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define LC_CONF_INCLUDE "lib/lc-cc65.h"

/*#pragma charmap(0, 0);
#pragma charmap(1, 1);
#pragma charmap(2, 2);
#pragma charmap(3, 3);
#pragma charmap(4, 4);
#pragma charmap(5, 5);
#pragma charmap(6, 6);
#pragma charmap(7, 7);
#pragma charmap(8, 8);
#pragma charmap(9, 9);
#pragma charmap(10, 10);
#pragma charmap(11, 11);
#pragma charmap(12, 12);
#pragma charmap(13, 13);
#pragma charmap(14, 14);
#pragma charmap(15, 15);
#pragma charmap(16, 16);
#pragma charmap(17, 17);
#pragma charmap(18, 18);
#pragma charmap(19, 19);
#pragma charmap(20, 20);
#pragma charmap(21, 21);
#pragma charmap(22, 22);
#pragma charmap(23, 23);
#pragma charmap(24, 24);
#pragma charmap(25, 25);
#pragma charmap(26, 26);
#pragma charmap(27, 27);
#pragma charmap(28, 28);
#pragma charmap(29, 29);
#pragma charmap(30, 30);
#pragma charmap(31, 31);
#pragma charmap(32, 32);
#pragma charmap(33, 33);
#pragma charmap(34, 34);
#pragma charmap(35, 35);
#pragma charmap(36, 36);
#pragma charmap(37, 37);
#pragma charmap(38, 38);
#pragma charmap(39, 39);
#pragma charmap(40, 40);
#pragma charmap(41, 41);
#pragma charmap(42, 42);
#pragma charmap(43, 43);
#pragma charmap(44, 44);
#pragma charmap(45, 45);
#pragma charmap(46, 46);
#pragma charmap(47, 47);
#pragma charmap(48, 48);
#pragma charmap(49, 49);
#pragma charmap(50, 50);
#pragma charmap(51, 51);
#pragma charmap(52, 52);
#pragma charmap(53, 53);
#pragma charmap(54, 54);
#pragma charmap(55, 55);
#pragma charmap(56, 56);
#pragma charmap(57, 57);
#pragma charmap(58, 58);
#pragma charmap(59, 59);
#pragma charmap(60, 60);
#pragma charmap(61, 61);
#pragma charmap(62, 62);
#pragma charmap(63, 63);
#pragma charmap(64, 64);
#pragma charmap(65, 65);
#pragma charmap(66, 66);
#pragma charmap(67, 67);
#pragma charmap(68, 68);
#pragma charmap(69, 69);
#pragma charmap(70, 70);
#pragma charmap(71, 71);
#pragma charmap(72, 72);
#pragma charmap(73, 73);
#pragma charmap(74, 74);
#pragma charmap(75, 75);
#pragma charmap(76, 76);
#pragma charmap(77, 77);
#pragma charmap(78, 78);
#pragma charmap(79, 79);
#pragma charmap(80, 80);
#pragma charmap(81, 81);
#pragma charmap(82, 82);
#pragma charmap(83, 83);
#pragma charmap(84, 84);
#pragma charmap(85, 85);
#pragma charmap(86, 86);
#pragma charmap(87, 87);
#pragma charmap(88, 88);
#pragma charmap(89, 89);
#pragma charmap(90, 90);
#pragma charmap(91, 91);
#pragma charmap(92, 92);
#pragma charmap(93, 93);
#pragma charmap(94, 94);
#pragma charmap(95, 95);
#pragma charmap(96, 96);
#pragma charmap(97, 97);
#pragma charmap(98, 98);
#pragma charmap(99, 99);
#pragma charmap(100, 100);
#pragma charmap(101, 101);
#pragma charmap(102, 102);
#pragma charmap(103, 103);
#pragma charmap(104, 104);
#pragma charmap(105, 105);
#pragma charmap(106, 106);
#pragma charmap(107, 107);
#pragma charmap(108, 108);
#pragma charmap(109, 109);
#pragma charmap(110, 110);
#pragma charmap(111, 111);
#pragma charmap(112, 112);
#pragma charmap(113, 113);
#pragma charmap(114, 114);
#pragma charmap(115, 115);
#pragma charmap(116, 116);
#pragma charmap(117, 117);
#pragma charmap(118, 118);
#pragma charmap(119, 119);
#pragma charmap(120, 120);
#pragma charmap(121, 121);
#pragma charmap(122, 122);
#pragma charmap(123, 123);
#pragma charmap(124, 124);
#pragma charmap(125, 125);
#pragma charmap(126, 126);
#pragma charmap(127, 127);
#pragma charmap(128, 128);
#pragma charmap(129, 129);
#pragma charmap(130, 130);
#pragma charmap(131, 131);
#pragma charmap(132, 132);
#pragma charmap(133, 133);
#pragma charmap(134, 134);
#pragma charmap(135, 135);
#pragma charmap(136, 136);
#pragma charmap(137, 137);
#pragma charmap(138, 138);
#pragma charmap(139, 139);
#pragma charmap(140, 140);
#pragma charmap(141, 141);
#pragma charmap(142, 142);
#pragma charmap(143, 143);
#pragma charmap(144, 144);
#pragma charmap(145, 145);
#pragma charmap(146, 146);
#pragma charmap(147, 147);
#pragma charmap(148, 148);
#pragma charmap(149, 149);
#pragma charmap(150, 150);
#pragma charmap(151, 151);
#pragma charmap(152, 152);
#pragma charmap(153, 153);
#pragma charmap(154, 154);
#pragma charmap(155, 155);
#pragma charmap(156, 156);
#pragma charmap(157, 157);
#pragma charmap(158, 158);
#pragma charmap(159, 159);
#pragma charmap(160, 160);
#pragma charmap(161, 161);
#pragma charmap(162, 162);
#pragma charmap(163, 163);
#pragma charmap(164, 164);
#pragma charmap(165, 165);
#pragma charmap(166, 166);
#pragma charmap(167, 167);
#pragma charmap(168, 168);
#pragma charmap(169, 169);
#pragma charmap(170, 170);
#pragma charmap(171, 171);
#pragma charmap(172, 172);
#pragma charmap(173, 173);
#pragma charmap(174, 174);
#pragma charmap(175, 175);
#pragma charmap(176, 176);
#pragma charmap(177, 177);
#pragma charmap(178, 178);
#pragma charmap(179, 179);
#pragma charmap(180, 180);
#pragma charmap(181, 181);
#pragma charmap(182, 182);
#pragma charmap(183, 183);
#pragma charmap(184, 184);
#pragma charmap(185, 185);
#pragma charmap(186, 186);
#pragma charmap(187, 187);
#pragma charmap(188, 188);
#pragma charmap(189, 189);
#pragma charmap(190, 190);
#pragma charmap(191, 191);
#pragma charmap(192, 192);
#pragma charmap(193, 193);
#pragma charmap(194, 194);
#pragma charmap(195, 195);
#pragma charmap(196, 196);
#pragma charmap(197, 197);
#pragma charmap(198, 198);
#pragma charmap(199, 199);
#pragma charmap(200, 200);
#pragma charmap(201, 201);
#pragma charmap(202, 202);
#pragma charmap(203, 203);
#pragma charmap(204, 204);
#pragma charmap(205, 205);
#pragma charmap(206, 206);
#pragma charmap(207, 207);
#pragma charmap(208, 208);
#pragma charmap(209, 209);
#pragma charmap(210, 210);
#pragma charmap(211, 211);
#pragma charmap(212, 212);
#pragma charmap(213, 213);
#pragma charmap(214, 214);
#pragma charmap(215, 215);
#pragma charmap(216, 216);
#pragma charmap(217, 217);
#pragma charmap(218, 218);
#pragma charmap(219, 219);
#pragma charmap(220, 220);
#pragma charmap(221, 221);
#pragma charmap(222, 222);
#pragma charmap(223, 223);
#pragma charmap(224, 224);
#pragma charmap(225, 225);
#pragma charmap(226, 226);
#pragma charmap(227, 227);
#pragma charmap(228, 228);
#pragma charmap(229, 229);
#pragma charmap(230, 230);
#pragma charmap(231, 231);
#pragma charmap(232, 232);
#pragma charmap(233, 233);
#pragma charmap(234, 234);
#pragma charmap(235, 235);
#pragma charmap(236, 236);
#pragma charmap(237, 237);
#pragma charmap(238, 238);
#pragma charmap(239, 239);
#pragma charmap(240, 240);
#pragma charmap(241, 241);
#pragma charmap(242, 242);
#pragma charmap(243, 243);
#pragma charmap(244, 244);
#pragma charmap(245, 245);
#pragma charmap(246, 246);
#pragma charmap(247, 247);
#pragma charmap(248, 248);
#pragma charmap(249, 249);
#pragma charmap(250, 250);
#pragma charmap(251, 251);
#pragma charmap(252, 252);
#pragma charmap(253, 253);
#pragma charmap(254, 254);
#pragma charmap(255, 255);*/

/* C compiler configuration. */

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CC_CONF_FASTCALL               fastcall

/* Interface configuration. */

#define CCIF
#define CLIF

/* Clock configuration. */

#include <time.h>

typedef unsigned short clock_time_t;

#define CLOCK_CONF_SECOND CLK_TCK


/*
 * CTK GUI toolkit configuration.
 */

#include "ctk-arch.h"

/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY             CH_F1

/* Defines which key that is to be used for switching the frontmost
   window.  */
#define CTK_CONF_WINDOWSWITCH_KEY     CH_F3

/* Defines which key that is to be used for switching to the prevoius
   widget.  */
#define CTK_CONF_WIDGETUP_KEY         CH_F5

/* Defines which key that is to be used for switching to the next
   widget.  */
#define CTK_CONF_WIDGETDOWN_KEY       CH_F7

/* Toggles mouse support (must have support functions in the
architecture specific files to work). */
#define CTK_CONF_MOUSE_SUPPORT        0

/* Toggles support for icons. */
#define CTK_CONF_ICONS                1

/* Toggles support for icon bitmaps. */
#define CTK_CONF_ICON_BITMAPS         1

/* Toggles support for icon textmaps. */
#define CTK_CONF_ICON_TEXTMAPS        1

/* Toggles support for movable windows. */
#define CTK_CONF_WINDOWMOVE           1

/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE          1

/* Toggles support for menus. */
#define CTK_CONF_MENUS                1

/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH            16
/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS         10

/* Toggles support for screen savers. */
#define CTK_CONF_SCREENSAVER          1

/*
 * CTK conio configuration.
 */

#if 0
/* Light gray inverted color scheme: */
#define BORDERCOLOR         COLOR_WHITE
#define SCREENCOLOR         COLOR_WHITE
#define BACKGROUNDCOLOR     COLOR_WHITE

#define WINDOWCOLOR_FOCUS   COLOR_BLACK
#define WINDOWCOLOR         COLOR_GRAY2

#define DIALOGCOLOR         COLOR_RED

#define WIDGETCOLOR_HLINK   COLOR_BLUE
#define WIDGETCOLOR_FWIN    COLOR_BLACK
#define WIDGETCOLOR         COLOR_GRAY1
#define WIDGETCOLOR_DIALOG  COLOR_RED
#define WIDGETCOLOR_FOCUS   COLOR_BLACK

#define MENUCOLOR           COLOR_BLACK
#define OPENMENUCOLOR       COLOR_BLACK
#define ACTIVEMENUITEMCOLOR COLOR_BLACK
#endif /* 0 */

#if 0
/* Blue color scheme: */
#define BORDERCOLOR         COLOR_LIGHTBLUE
#define SCREENCOLOR         COLOR_BLUE
#define BACKGROUNDCOLOR     COLOR_BLUE

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTBLUE
#define WINDOWCOLOR         COLOR_GRAY1

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_CYAN
#define WIDGETCOLOR_FWIN    COLOR_LIGHTBLUE
#define WIDGETCOLOR         COLOR_GRAY1
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_WHITE
#define OPENMENUCOLOR       COLOR_LIGHTBLUE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */

#if 0
/* Black and white monocrome color scheme: */
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
#endif /* 0 */

#if 0
/* Blue monocrome color scheme: */
#define BORDERCOLOR         COLOR_BLUE
#define SCREENCOLOR         COLOR_BLUE
#define BACKGROUNDCOLOR     COLOR_BLUE

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTBLUE
#define WINDOWCOLOR         COLOR_LIGHTBLUE

#define DIALOGCOLOR         COLOR_LIGHTBLUE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_LIGHTBLUE
#define WIDGETCOLOR         COLOR_LIGHTBLUE
#define WIDGETCOLOR_DIALOG  COLOR_LIGHTBLUE
#define WIDGETCOLOR_FOCUS   COLOR_LIGHTBLUE

#define MENUCOLOR           COLOR_LIGHTBLUE
#define OPENMENUCOLOR       COLOR_LIGHTBLUE
#define ACTIVEMENUITEMCOLOR COLOR_LIGHTBLUE
#endif /* 0 */

#if 1
/* Gray color scheme: */
#define BORDERCOLOR         COLOR_GRAY1
#define SCREENCOLOR         COLOR_GRAY1
#define BACKGROUNDCOLOR     COLOR_GRAY1

#define WINDOWCOLOR_FOCUS   COLOR_GRAY3
#define WINDOWCOLOR         COLOR_GRAY2

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_GRAY3
#define WIDGETCOLOR         COLOR_GRAY2
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_GRAY3
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */

#if 0
/* Red color scheme: */
#define BORDERCOLOR         COLOR_BLACK
#define SCREENCOLOR         COLOR_BLACK
#define BACKGROUNDCOLOR     COLOR_BLACK

#define WINDOWCOLOR_FOCUS   COLOR_LIGHTRED
#define WINDOWCOLOR         COLOR_RED

#define DIALOGCOLOR         COLOR_WHITE

#define WIDGETCOLOR_HLINK   COLOR_LIGHTBLUE
#define WIDGETCOLOR_FWIN    COLOR_YELLOW
#define WIDGETCOLOR         COLOR_LIGHTRED
#define WIDGETCOLOR_DIALOG  COLOR_WHITE
#define WIDGETCOLOR_FOCUS   COLOR_YELLOW

#define MENUCOLOR           COLOR_LIGHTRED
#define OPENMENUCOLOR       COLOR_WHITE
#define ACTIVEMENUITEMCOLOR COLOR_YELLOW
#endif /* 0 */

/* CTK VNC server configuration. */

#define CTK_VNCSERVER_CONF_NUMCONNS 1

#define CTK_VNCSERVER_CONF_MAX_ICONS 4

#define CTK_VNCSERVER_CONF_SCREEN (u8_t *)0x0400

/* Email program configuration. */

#define EMAIL_CONF_WIDTH 36
#define EMAIL_CONF_HEIGHT 17

/* IRC program configuration. */

#define IRC_CONF_WIDTH 78
#define IRC_CONF_HEIGHT 21

#define IRC_CONF_SYSTEM_STRING "c64"

/* Libconio configuration. */

#define LIBCONIO_CONF_SCREEN_WIDTH  80
#define LIBCONIO_CONF_SCREEN_HEIGHT 25

/* Logging configuration. */

#define LOG_CONF_ENABLED 1

/* Program handler configuration. */

#define PROGRAM_HANDLER_CONF_MAX_NUMDSCS 10

/* Shell GUI configuration. */

#define SHELL_GUI_CONF_XSIZE 26
#define SHELL_GUI_CONF_YSIZE 12

/* Telnet daemon configuration. */

#define TELNETD_CONF_LINELEN 32
#define TELNETD_CONF_NUMLINES 12

#define TELNETD_CONF_GUI     1

/* uIP configuration. */

#define UIP_CONF_MAX_CONNECTIONS 10
#define UIP_CONF_MAX_LISTENPORTS 10
#define UIP_CONF_BUFFER_SIZE     400
#define UIP_CONF_RECEIVE_WINDOW  320

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

#define UIP_CONF_ARBTAB_SIZE 4

#define UIP_CONF_RESOLV_ENTRIES  4
#define UIP_CONF_PINGADDRCONF    0

#define UIP_CONF_LOGGING         0
#define UIP_CONF_UDP_CONNS       6
#define UIP_CONF_BROADCAST       0

#define UIP_CONF_UDP_CHECKSUMS   0

#define UIP_ARCH_ADD32           1
#define UIP_ARCH_CHKSUM          1

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

/**
 * The 32-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * long" works for most compilers.
 */
typedef unsigned long u32_t;

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;


/* VNC viewer configuration. */

#define VNC_CONF_VIEWPORT_WIDTH  (32*8)
#define VNC_CONF_VIEWPORT_HEIGHT (16*8)

#define VNC_CONF_REFRESH_ROWS    8

/* Web browser configuration. */

#define WWW_CONF_HOMEPAGE "http://contiki.c64.org/"

/* The size of the HTML viewing area. */
#define WWW_CONF_WEBPAGE_WIDTH 36
#define WWW_CONF_WEBPAGE_HEIGHT 17

/* The size of the "Back" history. */
#define WWW_CONF_HISTORY_SIZE 2

/* Defines the maximum length of an URL */
#define WWW_CONF_MAX_URLLEN 55

/* The maxiumum number of widgets (i.e., hyperlinks, form elements) on
   a page. */
#define WWW_CONF_MAX_NUMPAGEWIDGETS 8

/* Turns <center> support on or off; must be on for forms to work. */
#define WWW_CONF_RENDERSTATE 0

/* Toggles support for HTML forms. */
#define WWW_CONF_FORMS       0

/* Maximum lengths for various HTML form parameters. */
#define WWW_CONF_MAX_FORMACTIONLEN  30
#define WWW_CONF_MAX_FORMNAMELEN    20
#define WWW_CONF_MAX_INPUTNAMELEN   20

#define WWW_CONF_MAX_INPUTVALUELEN  (WWW_CONF_WEBPAGE_WIDTH - 1)


#endif /* __CONTIKI_CONF_H__ */
