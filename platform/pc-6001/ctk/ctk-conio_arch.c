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
 *
 */

/*
 * \file
 * 	This is architecture-depend contiki toolkit for PC-6001 family.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */
 

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"
#include "contiki-conf.h"
#include "ctk/ctk_arch.h"
#include <stddef.h>

/*---------------------------------------------------------------------------*/
/*
 * Offset constants for assembler
 */
const uint8_t off_widget_x        = offsetof(struct ctk_widget, x);
const uint8_t off_widget_y        = offsetof(struct ctk_widget, y);
const uint8_t off_widget_w        = offsetof(struct ctk_widget, w);
const uint8_t off_widget_h        = offsetof(struct ctk_widget, h);
const uint8_t off_widget_type     = offsetof(struct ctk_widget, type);
const uint8_t off_widget_window   = offsetof(struct ctk_widget, window);

const uint8_t off_widget_label_text      = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_label, text);
const uint8_t off_widget_button_text     = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_button, text);
const uint8_t off_widget_textentry_text  = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_textentry, text);
const uint8_t off_widget_textentry_xpos  = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_textentry, xpos);
const uint8_t off_widget_textentry_ypos  = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_textentry, ypos);
const uint8_t off_widget_textentry_state = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_textentry, state);
#if CTK_CONF_HYPERLINK
const uint8_t off_widget_hyperlink_text = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_hyperlink, text);
#endif /* CTK_CONF_HYPERLINK */

#if CTK_CONF_ICONS
const uint8_t off_widget_icon_title      = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_icon, title);
const uint8_t off_widget_icon_textmap    = offsetof(struct ctk_widget, widget) +
  offsetof(struct ctk_widget_icon, textmap);
#endif /* CTK_CONF_ICONS */

const uint8_t off_window_x        = offsetof(struct ctk_window, x);
const uint8_t off_window_y        = offsetof(struct ctk_window, y);
const uint8_t off_window_h        = offsetof(struct ctk_window, h);
const uint8_t off_window_w        = offsetof(struct ctk_window, w);
const uint8_t off_window_inactive = offsetof(struct ctk_window, inactive);
const uint8_t off_window_active   = offsetof(struct ctk_window, active);
const uint8_t off_window_next     = offsetof(struct ctk_window, next);
const uint8_t off_window_focused  = offsetof(struct ctk_window, focused);

#if CTK_CONF_MENUS
const uint8_t off_menu_title      = offsetof(struct ctk_menu, title);
const uint8_t off_menu_active     = offsetof(struct ctk_menu, active);
const uint8_t off_menu_nitems     = offsetof(struct ctk_menu, nitems);
const uint8_t off_menu_items      = offsetof(struct ctk_menu, items);
const uint8_t off_menu_next       = offsetof(struct ctk_menu, next);
const uint8_t off_menuitem_title  = offsetof(struct ctk_menuitem, title);
const uint8_t size_menuitem       = sizeof(struct ctk_menuitem);
const uint8_t off_menus_open      = offsetof(struct ctk_menus, open);
const uint8_t off_menus_menus     = offsetof(struct ctk_menus, menus);
const uint8_t off_menus_desktopmenu = offsetof(struct ctk_menus, desktopmenu);
#endif

/*---------------------------------------------------------------------------*/
