/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: ctk-mouse.c,v 1.3 2009/10/18 09:33:08 oliverschmidt Exp $
 */

#include <mouse.h>
#include <stdlib.h>
#include <modload.h>

#include "cfs/cfs.h"
#include "ctk.h"
#include "ctk-mouse.h"

#if CTK_CONF_MOUSE_SUPPORT

static struct mouse_pos pos;
static uint8_t okay;

/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_init(void)
{
  struct mod_ctrl module_control = {cfs_read};

  module_control.callerdata = cfs_open(mouse_stddrv, CFS_READ);
  okay = module_control.callerdata >= 0;
  if(okay) {
    okay = mod_load(&module_control) == MLOAD_OK;
    if(okay) {
      okay = mouse_install(&mouse_def_callbacks, module_control.module) == MOUSE_ERR_OK;
      if(okay) {
	atexit((void (*)(void))mouse_uninstall);
      } else {
	mod_free(module_control.module);
      }
    }
    cfs_close(module_control.callerdata);
  }
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  if(okay) {
    mouse_pos(&pos);
  }
  return pos.x;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  if(okay) {
    mouse_pos(&pos);
  }
  return pos.y;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  if(okay) {
    return mouse_buttons();
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_xtoc(unsigned short x)
{
  return MOUSE_CONF_XTOC(x);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_ytoc(unsigned short y)
{
  return MOUSE_CONF_YTOC(y);
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_hide(void)
{
  if(okay) {
    mouse_hide();
  }
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_show(void)
{
  if(okay) {
    mouse_show();
  }
}
/*-----------------------------------------------------------------------------------*/
#endif /* CTK_CONF_MOUSE_SUPPORT */
