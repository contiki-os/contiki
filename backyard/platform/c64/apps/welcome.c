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
 * This file is part of the Contiki desktop environment
 *
 * $Id: welcome.c,v 1.1 2007/05/23 23:11:26 oliverschmidt Exp $
 *
 */

#include "contiki.h"
#include "ctk/ctk.h"
#include <string.h>


static struct ctk_window welcomedialog;
static struct ctk_label welcomelabel1 =
  {CTK_LABEL(1, 1, 30, 1, "Welcome to " CONTIKI_VERSION_STRING "!")};
static struct ctk_label welcomelabel2 =
  {CTK_LABEL(2, 4, 28, 1, "F1     - open menus")};
static struct ctk_label welcomelabel3 =
  {CTK_LABEL(2, 6, 28, 1, "F3     - cycle windows")};
static struct ctk_label welcomelabel4 =
  {CTK_LABEL(2, 8, 28, 1, "F5     - select up")};
static struct ctk_label welcomelabel5 =
  {CTK_LABEL(2, 10, 28, 1, "F7     - select down")};
static struct ctk_label welcomelabel6 =
  {CTK_LABEL(2, 12, 28, 1, "Return - activate selected")};
static struct ctk_label welcomelabel7 =
  {CTK_LABEL(3, 15, 25, 1, "Press any key to continue")};


PROCESS(welcome_process, "Welcome");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(welcome_process, ev, data)
{
  unsigned char width;
  
  PROCESS_BEGIN();
  
  width = ctk_desktop_width(NULL);
  
  if(width > 34) {
    ctk_dialog_new(&welcomedialog, 32, 17);
  } else {
    ctk_dialog_new(&welcomedialog, width - 2, 17);
  }
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel1);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel2);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel3);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel4);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel5);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel6);
  CTK_WIDGET_ADD(&welcomedialog, &welcomelabel7);    
  
  ctk_dialog_open(&welcomedialog);

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			   ev == ctk_signal_keypress);
  ctk_dialog_close();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
