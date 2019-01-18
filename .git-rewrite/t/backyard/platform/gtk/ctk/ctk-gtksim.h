/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the "ctk" console GUI toolkit for cc65
 *
 * $Id: ctk-gtksim.h,v 1.1 2008/01/05 21:08:26 oliverschmidt Exp $
 *
 */

#ifndef __CTK_GTKSIM_H__
#define __CTK_GTKSIM_H__

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdktypes.h>
#include <gtk/gtk.h>

#define CTK_ARCH_KEY_T guint

unsigned char ctk_arch_keyavail(void);
guint ctk_arch_getkey(void);

#define CH_CURS_RIGHT  GDK_Right
#define CH_CURS_DOWN   GDK_Down
#define CH_CURS_LEFT   GDK_Left
#define CH_CURS_UP     GDK_Up
#define CH_ENTER       GDK_Return
#define CH_TAB         GDK_Tab
#define CH_F1          GDK_F1
#define CH_F2          GDK_F2
#define CH_F3          GDK_F3
#define CH_F4          GDK_F4
#define CH_F5          GDK_F5
#define CH_DEL         GDK_BackSpace
#define CH_ESC         GDK_Escape

void ctk_gtksim_init(void);

void ctk_gtksim_redraw(void);

extern GdkPixmap *ctk_gtksim_pixmap;
extern GtkWidget *ctk_gtksim_drawing_area;

#define CTK_GTKSIM_SCREEN_WIDTH 1024
#define CTK_GTKSIM_SCREEN_HEIGHT 768

#endif /* __CTK_GTKSIM_H__ */
