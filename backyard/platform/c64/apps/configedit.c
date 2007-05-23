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
 * $Id: configedit.c,v 1.1 2007/05/23 23:11:24 oliverschmidt Exp $
 *
 */

#include "contiki.h"
#include "contiki-net.h"

#include "ctk/ctk-draw-service.h"
#include "net/packet-service.h"

#include "ctk/ctk.h"
#include "ctk/ctk-draw.h"

#include "program-handler.h"

#include "cfs/cfs.h"

#include <stdio.h>

#include <string.h>

/* TCP/IP configuration window. */
static struct ctk_window window;

#define LABELMAXWIDTH 12

static struct ctk_label cfslabel =
  {CTK_LABEL(0, 1, LABELMAXWIDTH, 1, "Disk driver")};
static char cfs[25];
static struct ctk_textentry cfstextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 1, 16, 1, cfs, 24)};

static struct ctk_label themelabel =
  {CTK_LABEL(0, 3, LABELMAXWIDTH, 1, "CTK theme")};
static char theme[25];
static struct ctk_textentry themetextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 3, 16, 1, theme, 24)};

static struct ctk_label driverlabel =
  {CTK_LABEL(0, 5, LABELMAXWIDTH, 1, "Net driver")};
static char driver[25];
static struct ctk_textentry drivertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 5, 16, 1, driver, 24)};

static struct ctk_label screensaverlabel =
  {CTK_LABEL(0, 7, LABELMAXWIDTH, 1, "Screensaver")};
static char screensaver[25];
static struct ctk_textentry screensavertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 7, 16, 1, screensaver, 24)};


static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 9, LABELMAXWIDTH, 1, "IP address")};
static char ipaddr[25];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 9, 16, 1, ipaddr, 24)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 11, LABELMAXWIDTH, 1, "Netmask")};
static char netmask[25];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 11, 16, 1, netmask, 24)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 13, LABELMAXWIDTH, 1, "Gateway")};
static char gateway[25];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 13, 16, 1, gateway, 24)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 15, LABELMAXWIDTH, 1, "DNS server")};
static char dnsserver[25];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(LABELMAXWIDTH + 1, 15, 16, 1, dnsserver, 24)};

static struct ctk_button savebutton =
  {CTK_BUTTON(0, 17, 12, "Save & close")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(24, 17, 6, "Cancel")};

PROCESS(configedit_process, "Configuration editor");
/*-----------------------------------------------------------------------------------*/
struct ptentry {
  char c;
  char * (* pfunc)(char *str);
};

/*-----------------------------------------------------------------------------------*/
static char *
parse(char *str, struct ptentry *t)
{
  struct ptentry *p;

  /* Loop over the parse table entries in t in order to find one that
     matches the first character in str. */
  for(p = t; p->c != 0; ++p) {
    if(*str == p->c) {
      /* Skip rest of the characters up to the first space. */
      while(*str != ' ') {
	++str;
      }

      /* Skip all spaces.*/
      while(*str == ' ') {
	++str;
      }

      /* Call parse table entry function and return. */
      return p->pfunc(str);
    }
  }

  /* Did not find matching entry in parse table. We just call the
     default handler supplied by the caller and return. */
  return p->pfunc(str);
}
/*-----------------------------------------------------------------------------------*/
static char *
skipnewline(char *str)
{
  /* Skip all characters until the newline. */
  while(*str != '\n') {
    ++str;
  }

  /* Return a pointer to the first character after the newline. */
  return str + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
nullterminate(char *str)
{
  char *nt;

  /* Nullterminate string. Start with finding newline character. */
  for(nt = str; *nt != '\r' &&
	*nt != '\n'; ++nt);

  /* Replace newline with a null char. */
  *nt = 0;

  /* Remove trailing spaces. */
  while(nt > str && *(nt - 1) == ' ') {
    *(nt - 1) = 0;
    --nt;
  }
    
  /* Return pointer to null char. */
  return nt;
}
/*-----------------------------------------------------------------------------------*/
static char * __fastcall__
copystr(char *dst, char *src, int len)
{
  char *nt = nullterminate(src);
  strncpy(dst, src, len);
  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
loaddriver(char *str)
{
  return copystr(driver, str, sizeof(driver));
}
/*-----------------------------------------------------------------------------------*/
static char *
loadtheme(char *str)
{
  return copystr(theme, str, sizeof(theme));
}
/*-----------------------------------------------------------------------------------*/
static char *
loadcfs(char *str)
{
  return copystr(cfs, str, sizeof(cfs));
}
/*-----------------------------------------------------------------------------------*/
static char *
loadscreensaver(char *str)
{
  return copystr(screensaver, str, sizeof(screensaver));
}
/*-----------------------------------------------------------------------------------*/
static char *
ipaddrconf(char *str)
{
  return copystr(ipaddr, str, sizeof(ipaddr));
}
/*-----------------------------------------------------------------------------------*/
static char *
netmaskconf(char *str)
{
  return copystr(netmask, str, sizeof(netmask));
}
/*-----------------------------------------------------------------------------------*/
static char *
drconf(char *str)
{
  return copystr(gateway, str, sizeof(gateway));
}
/*-----------------------------------------------------------------------------------*/
static char *
dnsconf(char *str)
{
  return copystr(dnsserver, str, sizeof(dnsserver));
}
/*-----------------------------------------------------------------------------------*/
static struct ptentry initparsetab[] =
  {{'n', loaddriver},
   {'t', loadtheme},
   {'c', loadcfs},
   {'s', loadscreensaver},
   {'i', ipaddrconf},
   {'m', netmaskconf},
   {'r', drconf},
   {'d', dnsconf},
   {'#', skipnewline},

   /* Default action */
   {0, skipnewline}};
static void
initscript(void)
{
  char line[40], *lineptr;
  /*  struct c64_fs_file f;*/
  int f;

  if((f = cfs_open("config.cfg", 0)) == -1) {
    return;
  }
  line[0] = ' ';
  while(line[0] != '.' &&
	line[0] != 0) {
    lineptr = line;
    do {
      if(cfs_read(f, lineptr, 1) != 1) {
	cfs_close(f);
	return;
      }
      ++lineptr;
    } while(*(lineptr - 1) != '\n' &&
	    *(lineptr - 1) != '\r');

    *lineptr = 0;

    if(line[0] != '.' &&
       line[0] != 0) {
      parse(line, initparsetab);
    }
    
  }
  cfs_close(f);
  return;
}
/*-----------------------------------------------------------------------------------*/
static int
makeline(char *line, char c, char *str)
{
  int len;

  len = strlen(str);
  
  line[0] = c;
  line[1] = ' ';
  strcpy(&line[2], str);
  line[2 + len] = '\n';
  line[3 + len] = 0;
  return len + 3;
}
/*-----------------------------------------------------------------------------------*/
static void
savescript(void)
{
  char line[40];
  /*  struct c64_fs_file f;*/
  int f;
  
  f = cfs_open("@:config.cfg", CFS_WRITE);
  if(f == -1) {
    log_message("Could not open config.cfg", "");
    return;
  }
  if(cfs[0] != 0) {
    cfs_write(f, line, makeline(line, 'c', cfs));
  }
  if(theme[0] != 0) {
    cfs_write(f, line, makeline(line, 't', theme));
  }
  if(driver[0] != 0) {
    cfs_write(f, line, makeline(line, 'n', driver));
  }
  if(ipaddr[0] != 0) {
    cfs_write(f, line, makeline(line, 'i', ipaddr));
  }
  if(netmask[0] != 0) {
    cfs_write(f, line, makeline(line, 'm', netmask));
  }
  if(gateway[0] != 0) {
    cfs_write(f, line, makeline(line, 'r', gateway));
  }
  if(dnsserver[0] != 0) {
    cfs_write(f, line, makeline(line, 'd', dnsserver));
  }
  
  if(screensaver[0] != 0) {
    cfs_write(f, line, makeline(line, 's', screensaver));
  }
  
  strcpy(line, ".\n\0\n\n\n");
  cfs_write(f, line, strlen(line));
  
  cfs_close(f);
  
}
/*-----------------------------------------------------------------------------------*/
static void
quit_services(void)
{
  /*  cfs_init_init(NULL);
  ctk_draw_quit();
  
  if(ek_service_find(&packetservice) == EK_ERR_OK) {
    ek_post(packetservice.id, EK_EVENT_REQUEST_EXIT, NULL);
    ek_service_reset(&packetservice);
    }*/
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(configedit_process, ev, data)
{
  PROCESS_BEGIN();
  
  /* Create window. */
  ctk_window_new(&window, 32, 18, "Config editor");
  
  CTK_WIDGET_ADD(&window, &cfslabel);  
  CTK_WIDGET_ADD(&window, &cfstextentry);

  CTK_WIDGET_ADD(&window, &themelabel);  
  CTK_WIDGET_ADD(&window, &themetextentry);
    
  CTK_WIDGET_ADD(&window, &driverlabel);  
  CTK_WIDGET_ADD(&window, &drivertextentry);

  CTK_WIDGET_ADD(&window, &screensaverlabel);  
  CTK_WIDGET_ADD(&window, &screensavertextentry);

  CTK_WIDGET_ADD(&window, &ipaddrlabel);  
  CTK_WIDGET_ADD(&window, &ipaddrtextentry);
  CTK_WIDGET_ADD(&window, &netmasklabel);
  CTK_WIDGET_ADD(&window, &netmasktextentry);
  CTK_WIDGET_ADD(&window, &gatewaylabel);
  CTK_WIDGET_ADD(&window, &gatewaytextentry);
  CTK_WIDGET_ADD(&window, &dnsserverlabel);
  CTK_WIDGET_ADD(&window, &dnsservertextentry);

  CTK_WIDGET_ADD(&window, &savebutton);
  CTK_WIDGET_ADD(&window, &cancelbutton);    
    
  CTK_WIDGET_FOCUS(&window, &cfstextentry);  

  /* Fill the configuration strings with values from the current
     configuration */
  initscript();
    
  ctk_window_open(&window);

  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == ctk_signal_button_activate) {   
      if(data == &savebutton) {
	savescript();
	quit_services();
	ctk_window_close(&window);
	program_handler_load("config.prg", NULL);
	break;
      } else if(data == &cancelbutton) {
	ctk_window_close(&window);
	break;
      }
    } else if(ev == ctk_signal_window_close ||
	      ev == PROCESS_EVENT_EXIT) {
      ctk_window_close(&window);
      break;
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
