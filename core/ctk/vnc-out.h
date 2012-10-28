/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 *
 */

#ifndef __VNC_OUT_H__
#define __VNC_OUT_H__


void vnc_out_init(void);
void vnc_out_new(struct vnc_server_state *vs);

void vnc_out_send_blank(struct vnc_server_state *vs);
void vnc_out_send_screen(struct vnc_server_state *vs);
void vnc_out_send_update(struct vnc_server_state *vs);

void vnc_out_key_event(struct vnc_server_state *vs);
void vnc_out_pointer_event(struct vnc_server_state *vs);

void vnc_out_acked(struct vnc_server_state *vs);

void vnc_out_poll(struct vnc_server_state *vs);


void vnc_out_update_screen(uint8_t x, uint8_t y, uint8_t c, uint8_t color);
char vnc_out_getkey(void);
char vnc_out_keyavail(void);

void vnc_out_update_area(struct vnc_server_state *vs,
			 uint8_t x, uint8_t y, uint8_t w, uint8_t h);

#include "ctk/ctk.h"

unsigned char vnc_out_add_icon(struct ctk_icon *icon);

#if 1
#define VNC_OUT_BACKGROUNDCOLOR 0
#define VNC_OUT_WINDOWCOLOR    1
#define VNC_OUT_SEPARATORCOLOR 7 /*(VNC_OUT_WINDOWCOLOR + 6)*/
#define VNC_OUT_LABELCOLOR     13 /*(VNC_OUT_SEPARATORCOLOR + 6)*/
#define VNC_OUT_BUTTONCOLOR    19 /*(VNC_OUT_LABELCOLOR + 6)*/
#define VNC_OUT_HYPERLINKCOLOR 25 /*(VNC_OUT_BUTTONCOLOR + 6)*/
#define VNC_OUT_TEXTENTRYCOLOR 31 /*(VNC_OUT_HYPERLINKCOLOR + 6)*/
#define VNC_OUT_ICONCOLOR      37 /*(VNC_OUT_TEXTENTRYCOLOR + 6)*/
#define VNC_OUT_MENUCOLOR      43 /*(VNC_OUT_ICONCOLOR + 6)*/
#define VNC_OUT_OPENMENUCOLOR  44/*(VNC_OUT_MENUCOLOR + 1)*/
#define VNC_OUT_ACTIVEMENUCOLOR 45 /*(VNC_OUT_OPENMENUCOLOR + 1) */
#else
#define VNC_OUT_BACKGROUNDCOLOR 0
#define VNC_OUT_WINDOWCOLOR    1
#define VNC_OUT_SEPARATORCOLOR (VNC_OUT_WINDOWCOLOR + 6)
#define VNC_OUT_LABELCOLOR     (VNC_OUT_SEPARATORCOLOR + 6)
#define VNC_OUT_BUTTONCOLOR    (VNC_OUT_LABELCOLOR + 6)
#define VNC_OUT_HYPERLINKCOLOR (VNC_OUT_BUTTONCOLOR + 6)
#define VNC_OUT_TEXTENTRYCOLOR (VNC_OUT_HYPERLINKCOLOR + 6)
#define VNC_OUT_ICONCOLOR      (VNC_OUT_TEXTENTRYCOLOR + 6)
#define VNC_OUT_MENUCOLOR      (VNC_OUT_ICONCOLOR + 6)
#define VNC_OUT_OPENMENUCOLOR  (VNC_OUT_MENUCOLOR + 1)
#define VNC_OUT_ACTIVEMENUCOLOR (VNC_OUT_OPENMENUCOLOR + 1)

#endif

#endif /* __VNC_OUT_H__ */
