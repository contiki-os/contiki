/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * $Id: ctk-mouse.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

#include "ctk/ctk.h"
#include "ctk-mouse.h"
#include "ctk-conf.h"

#if CTK_CONF_MOUSE_SUPPORT

unsigned short ctk_mouse_joyy, ctk_mouse_joyx;
unsigned char ctk_mouse_firebutton;

extern void ctk_mouse_asm_irq(void);

/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
ctk_mouse_init(void)
{
  /* Place mouse pointer at the middle of the screen. */
  ctk_mouse_joyx = 160;
  ctk_mouse_joyy = 100;
  
  /* Setup and start IRQ */
  asm("sei");
  asm("lda #<%v", ctk_mouse_asm_irq);
  asm("sta $0314");
  asm("lda #>%v", ctk_mouse_asm_irq);
  asm("sta $0315");   
  asm("cli");
  
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  if(ctk_mouse_joyx >= 342) {
    ctk_mouse_joyx = 0;
  } else if(ctk_mouse_joyx >= 320) {
    ctk_mouse_joyx = 319;
  }
  return ctk_mouse_joyx;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  return ctk_mouse_joyy;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  return ctk_mouse_firebutton;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_xtoc(unsigned short x)
{
  return x / 8;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_ytoc(unsigned short y)
{
  return y / 8;
}
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
ctk_mouse_hide(void)
{
  /* Turn off sprites 0 and 1 */
  asm("lda #0");
  asm("sta $d015"); 
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
ctk_mouse_show(void)
{  
  /* Turn on sprites 0 and 1 */
  asm("lda #3");
  asm("sta $d015"); 
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#endif /* CTK_CONF_MOUSE_SUPPORT */
