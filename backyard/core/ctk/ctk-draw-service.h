/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ctk-draw-service.h,v 1.1 2007/05/23 23:19:13 oliverschmidt Exp $
 */
#ifndef __CTK_DRAW_SERVICE_H__
#define __CTK_DRAW_SERVICE_H__


#include "ctk/ctk.h"
#include "sys/service.h"

#define ctk_draw_service_name "CTK driver"
SERVICE_INTERFACE(ctk_draw_service, {
  unsigned char windowborder_width;
  unsigned char windowborder_height;
  unsigned char windowtitle_height;
  
  void (* draw_init)(void);
  void (* draw_clear)(unsigned char clipy1, unsigned char clipy2);
  void (* draw_clear_window)(struct ctk_window *window,
			     unsigned char focus,
			     unsigned char clipy1,
			     unsigned char clipy2);
  void (* draw_window)(struct ctk_window *window,
		       unsigned char focus,
		       unsigned char clipy1,
		       unsigned char clipy2,
		       unsigned char draw_borders);
  void (* draw_dialog)(struct ctk_window *dialog);
  void (* draw_widget)(struct ctk_widget *widget,
		       unsigned char focus,
		       unsigned char clipy1,
		       unsigned char clipy2);
  void (* draw_menus)(struct ctk_menus *menus);

  unsigned char (* width)(void);
  unsigned char (* height)(void);

  unsigned short (* mouse_xtoc)(unsigned short);
  unsigned short (* mouse_ytoc)(unsigned short);
});

#endif /* __CTK_DRAW_SERVICE_H__ */
