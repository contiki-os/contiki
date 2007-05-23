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
 * This file is part of the Contiki desktop environment
 *
 * $Id: iconfield.c,v 1.1 2007/05/23 23:11:27 oliverschmidt Exp $
 *
 */

/*-----------------------------------------------------------------------------------*/
#define LFN 9
static void
loaddirectory(char *ext)
{
  unsigned char i, j;
  unsigned char extlen;

  extlen = strlen(ext);
  
  if(cbm_opendir(LFN, 8) != 0) {
    show_statustext("Cannot open directory");
  } else {
    i = 0;
    while(cbm_readdir(LFN, &dirent) == 0) {
      if(strcmp(&dirent.name[strlen(dirent.name) - extlen],
		ext) == 0) {
	strncpy(filenames[i], dirent.name, 16);
	++i;
	if(i == MAX_NUMFILES) {
	  break;
	}
      }
      cbm_closedir(LFN);
      
      numfiles = i;
      
      j = 0;
      for(i = 0; i < numfiles; ++i) {
	dscs[j] = LOADER_LOAD_DSC(filenames[i]);
	if(dscs[j] != NULL) {
	  ++j;
	}
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
makewindow(struct ctk_window *window)
{
  unsigned char x, y;

  ctk_window_clear(&window);
  CTK_WIDGET_SET_YPOS(&description, height - 3);
  CTK_WIDGET_SET_WIDTH(&description, width);
  CTK_WIDGET_ADD(&window, &description);

  morestart = i;
  
  x = 0; y = 1;
  for(; dscs[i] != NULL; ++i) {

    if(x + strlen(dscs[i]->icon->title) >= width) {
      y += 5;
      x = 0;
      if(y >= height - 2 - 4) {
	morestart = i;
	break;
      }
    }
    CTK_WIDGET_SET_XPOS(dscs[i]->icon, x);
    CTK_WIDGET_SET_YPOS(dscs[i]->icon, y);
    CTK_WIDGET_ADD(&window, dscs[i]->icon);

    x += strlen(dscs[i]->icon->title) + 1;
  }
  CTK_WIDGET_SET_YPOS(&autoexitbutton, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitbutton);
  CTK_WIDGET_SET_YPOS(&autoexitlabel, height - 2);
  CTK_WIDGET_ADD(&window, &autoexitlabel);

  if(i != morestart) {
    CTK_WIDGET_SET_YPOS(&backbutton, height - 1);
    CTK_WIDGET_ADD(&window, &backbutton);
  } else {
    CTK_WIDGET_SET_YPOS(&morebutton, height - 1);
    CTK_WIDGET_ADD(&window, &morebutton);
  }
  CTK_WIDGET_SET_XPOS(&reloadbutton, width - 8);
  CTK_WIDGET_SET_YPOS(&reloadbutton, height - 1);
  CTK_WIDGET_ADD(&window, &reloadbutton);    
}
/*-----------------------------------------------------------------------------------*/
