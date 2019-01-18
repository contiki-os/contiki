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
 * $Id: ctk-draw.c,v 1.1 2007/05/23 23:19:13 oliverschmidt Exp $
 */


#include "ctk/ctk-draw.h"
#include "ctk/ctk.h"

#include "ctk/ctk-draw-service.h"


unsigned char ctk_draw_windowborder_width = 1,
  ctk_draw_windowborder_height = 1,
  ctk_draw_windowtitle_height = 1;

/*---------------------------------------------------------------------------*/
void
ctk_draw_init(void)
{
  const struct ctk_draw_service *i;
  struct service *s;
  
  s = SERVICE_FIND(ctk_draw_service);

  if(s != NULL) {
    i = s->interface;
    
    i->draw_init();
    ctk_draw_windowborder_width = i->windowborder_width;
    ctk_draw_windowborder_height = i->windowborder_height;
    ctk_draw_windowtitle_height = i->windowtitle_height;
  }
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear(unsigned char clipy1, unsigned char clipy2)
{
  SERVICE_CALL(ctk_draw_service, draw_clear(clipy1, clipy2));
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char clipy1,
		      unsigned char clipy2)
{
  SERVICE_CALL(ctk_draw_service, draw_clear_window(window, focus,
						   clipy1, clipy2));
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_window(struct ctk_window *window,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2,
		unsigned char draw_borders)
{
  SERVICE_CALL(ctk_draw_service, draw_window(window, focus,
					     clipy1, clipy2, draw_borders));
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_dialog(struct ctk_window *dialog)
{
  SERVICE_CALL(ctk_draw_service, draw_dialog(dialog));
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_widget(struct ctk_widget *widget,
		unsigned char focus,
		unsigned char clipy1,
		unsigned char clipy2)
{
  SERVICE_CALL(ctk_draw_service, draw_widget(widget, focus,
					     clipy1, clipy2));
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_menus(struct ctk_menus *menus)
{
  SERVICE_CALL(ctk_draw_service, draw_menus(menus));
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_width(void)
{
  struct service *s;

  s = SERVICE_FIND(ctk_draw_service);
  if(s != NULL) {
    return ((struct ctk_draw_service *)s->interface)->width();
  }

  return 40;
}
/*---------------------------------------------------------------------------*/
unsigned char
ctk_draw_height(void)
{
  struct service *s;

  s = SERVICE_FIND(ctk_draw_service);
  if(s != NULL) {
    return ((struct ctk_draw_service *)s->interface)->height();
  }

  return 24;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_xtoc(unsigned short x)
{
  struct service *s;

  s = SERVICE_FIND(ctk_draw_service);
  if(s != NULL) {
    return ((struct ctk_draw_service *)s->interface)->mouse_xtoc(x);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
unsigned short
ctk_mouse_ytoc(unsigned short y)
{
  struct service *s;

  s = SERVICE_FIND(ctk_draw_service);
  if(s != NULL) {
    return ((struct ctk_draw_service *)s->interface)->mouse_ytoc(y);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
ctk_draw_quit(void)
{
  /*  process_post(service.id, EK_EVENT_REQUEST_EXIT, NULL);*/
  /*  process_service_reset(&service);  */
}
/*---------------------------------------------------------------------------*/
