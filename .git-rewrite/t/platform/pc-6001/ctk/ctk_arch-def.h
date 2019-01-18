/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ctk_arch-def.h,v 1.5 2007/12/14 23:49:14 oliverschmidt Exp $
 *
 */

/*
 * \file
 * 	ctk_arch-def.h
 * 	Definitions of CTK for PC-6001 family, which consist of the following:
 * 	- Configuraton for ctk main library.
 * 	- Configuraton for ctk_arch library.
 * 	This file is used in assembler file with precompilation, so
 *      NEVER include other than directives and comments.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __CTK_ARCH_DEF_H__
#define __CTK_ARCH_DEF_H__

/* characters */
#if defined(ARCH_PC6001) || defined(ARCH_PC6001MK2)
#define CH_VERTLINE			0x16
#define CH_HOLILINE			0x17
#define CH_ULCORNER			0x18
#define CH_URCORNER			0x19
#define CH_LLCORNER			0x1a
#define CH_LRCORNER			0x1b
#elif defined(ARCH_PC6001A)
#define CH_VERTLINE			'|'
#define CH_HOLILINE			'-'
#define CH_ULCORNER			'/'
#define CH_URCORNER			'\\'
#define CH_LLCORNER			'\\'
#define CH_LRCORNER			'/'
#else
#error invalid ARCH
#endif
#define CH_SPACE			0x20

/* screen properties */
#if defined(ARCH_PC6001MK2)
#define VRAM_ATTR			0x4000
#define VRAM_CHAR			0x4400
#define SCREEN_WIDTH			40
#define SCREEN_HEIGHT			20
#define COLOR_NORMAL			0x0f
#define COLOR_REVERSED			0x70
#elif (defined(ARCH_PC6001) || defined(ARCH_PC6001A)) && defined(MEMORY_16K)
#define VRAM_ATTR			0xc000
#define VRAM_CHAR			0xc200
#define SCREEN_WIDTH			32
#define SCREEN_HEIGHT			16
#define COLOR_NORMAL			0x20
#define COLOR_REVERSED			0x21
#elif (defined(ARCH_PC6001) || defined(ARCH_PC6001A)) && (defined(MEMORY_32K) || defined(MEMORY_ROM))
#define VRAM_ATTR			0x8000
#define VRAM_CHAR			0x8200
#define SCREEN_WIDTH			32
#define SCREEN_HEIGHT			16
#define COLOR_NORMAL			0x20
#define COLOR_REVERSED			0x21
#else
#error Specify appropriate ARCH & MEMORY combination
#endif /*  ARCH_PC6001MK2 */

/* key code */
#define CH_F1				-16//0xf0
#define CH_F2				-15//0xf1
#define CH_F3				-14//0xf2
#define CH_F4				-13//0xf3
#define CH_F5				-12//0xf4
#define CH_TAB				0x09

#define CH_CURS_UP			0x1e
#define CH_CURS_DOWN			0x1f
#define CH_CURS_LEFT			0x1d
#define CH_CURS_RIGHT			0x1c
#define CH_ENTER			0x0d
#define CH_ESC				0x1b
#define CH_STOP				0x03
#define CH_DEL				0x08

/* Contiki toolkit options */

#if defined(CTK_CLIENT) || defined(CTK_SERVER)
/* If network is available, turn all options off by the memory reason */
#define CTK_CONF_ICONS			0
#define CTK_CONF_ICON_TEXTMAPS		0
#define CTK_CONF_WINDOWS		1
#define CTK_CONF_WINDOWCLOSE		0
#define CTK_CONF_WINDOWMOVE		0
#define CTK_CONF_MENUS			0
#define CTK_CONF_HYPERLINK		0
#elif defined(CTK_NOICON)
/* In the case of loader arch, we omit the icons by the memory reason */
#define CTK_CONF_ICONS			0
#define CTK_CONF_ICON_TEXTMAPS		0
#define CTK_CONF_WINDOWS		1
#define CTK_CONF_WINDOWCLOSE		1
#define CTK_CONF_WINDOWMOVE		1
#define CTK_CONF_MENUS			1
#define CTK_CONF_HYPERLINK		1
#else /* CTK_MINIMUM */
/* Toggles support for desktop icons. */
#define CTK_CONF_ICONS			1
/* Define if text icon is used. */
#define CTK_CONF_ICON_TEXTMAPS		1
/* Toggles support for windows. */
#define CTK_CONF_WINDOWS		1
/* Toggles support for closable windows. */
#define CTK_CONF_WINDOWCLOSE		1
/* Toggles support for movable windows. */
#define CTK_CONF_WINDOWMOVE		1
/* Toggles support for menus. */
#define CTK_CONF_MENUS			1
/* Defines if hyperlink is supported. */
#define CTK_CONF_HYPERLINK		1
#endif /* CTK_MINIMUM */

/* Define if bitmap icon is used. */
#define CTK_CONF_ICON_BITMAPS		0
/* Defines the default width of a menu. */
#define CTK_CONF_MENUWIDTH		16
/* Defines if screen saver is supported. */
#define CTK_CONF_SCREENSAVER		0
/* Defines if mouse is supported. */
#define CTK_CONF_MOUSE_SUPPORT		0

/* The maximum number of menu items in each menu. */
#define CTK_CONF_MAXMENUITEMS		4

/* Key used to switch the frontmost window.  */
#define CTK_CONF_WINDOWSWITCH_KEY	CH_F3
/* Key used to move down a widget.  */
#define CTK_CONF_WIDGETDOWN_KEY		CH_TAB
/* Key used to move up a widget.  */
#define CTK_CONF_WIDGETUP_KEY		CH_F5
/* Defines which key that is to be used for activating the menus */
#define CTK_CONF_MENU_KEY		CH_F1

#ifdef LIBCONIO_CONF_EXPORT
#define LIBCONIO_EXPORT LIBCONIO_CONF_EXPORT
#else /* LIBCONIO_CONF_EXPORT */
#define LIBCONIO_EXPORT 0
#endif /* LIBCONIO_CONF_EXPORT */

/* Imported symbols from ctk.h */

#define _CTK_FOCUS_NONE		0
#define _CTK_FOCUS_WIDGET	1
#define _CTK_FOCUS_WINDOW	2
#define _CTK_FOCUS_DIALOG	4

#define _CTK_WIDGET_SEPARATOR	1
#define _CTK_WIDGET_LABEL	2
#define _CTK_WIDGET_BUTTON	3
#define _CTK_WIDGET_HYPERLINK	4
#define _CTK_WIDGET_TEXTENTRY	5
#define _CTK_WIDGET_BITMAP	6
#define _CTK_WIDGET_ICON	7

#define _CTK_TEXTENTRY_NORMAL	0
#define _CTK_TEXTENTRY_EDIT	1

#endif /* __CTK_ARCH_DEF_H__ */
