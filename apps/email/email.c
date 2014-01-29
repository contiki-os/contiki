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
 * This file is part of the Contiki desktop environment for the C64.
 *
 *
 */


#include "contiki.h"
#include "ctk/ctk.h"
#include "smtp.h"
#include "lib/petsciiconv.h"

#include "ctk/ctk-textentry-multiline.h"


#include <string.h>

#ifdef EMAIL_CONF_WIDTH
#define MAIL_WIDTH EMAIL_CONF_WIDTH
#else
#define MAIL_WIDTH 37
#endif

#ifdef EMAIL_CONF_HEIGHT
#define MAIL_HEIGHT EMAIL_CONF_HEIGHT
#else
#define MAIL_HEIGHT 17
#endif

#ifdef EMAIL_CONF_ERASE
#define MAIL_ERASE EMAIL_CONF_ERASE
#else
#define MAIL_ERASE 1
#endif

#if (MAIL_WIDTH - 9) < 39
#define TEXTENTRY_WIDTH (MAIL_WIDTH - 9)
#else
#define TEXTENTRY_WIDTH 39
#endif

static struct ctk_menu menu;
unsigned char menuitem_compose, menuitem_setup, menuitem_quit;

/* The main window. */
static struct ctk_window composewindow;

static struct ctk_separator sep1 =
 {CTK_SEPARATOR(0, MAIL_HEIGHT + 3, MAIL_WIDTH + 1)};
static struct ctk_label statuslabel =
 {CTK_LABEL(7, MAIL_HEIGHT + 4, MAIL_WIDTH - 14, 1, "")};


static struct ctk_label tolabel =
  {CTK_LABEL(0, 0, 3, 1, "To:")};
static char to[40];
static struct ctk_textentry totextentry =
  {CTK_TEXTENTRY(8, 0, TEXTENTRY_WIDTH, 1, to, 39)};

static struct ctk_label cclabel =
  {CTK_LABEL(0, 1, 3, 1, "Cc:")};
static char cc[40];
static struct ctk_textentry cctextentry =
  {CTK_TEXTENTRY(8, 1, TEXTENTRY_WIDTH, 1, cc, 39)};

static struct ctk_label subjectlabel =
  {CTK_LABEL(0, 2, 8, 1, "Subject:")};
static char subject[40];
static struct ctk_textentry subjecttextentry =
  {CTK_TEXTENTRY(8, 2, TEXTENTRY_WIDTH, 1, subject, 39)};

static char mail[MAIL_WIDTH * MAIL_HEIGHT];
struct ctk_textentry mailtextentry =
  {CTK_TEXTENTRY_INPUT(0, 3, MAIL_WIDTH - 1, MAIL_HEIGHT, mail, MAIL_WIDTH - 1, \
		       ctk_textentry_multiline_input)};

static struct ctk_button sendbutton =
  {CTK_BUTTON(0, MAIL_HEIGHT + 4, 4, "Send")};

#if MAIL_ERASE
static struct ctk_button erasebutton =
  {CTK_BUTTON(MAIL_WIDTH - 6, MAIL_HEIGHT + 4, 5, "Erase")};

/* The "Really erase message?" dialog. */
static struct ctk_window erasedialog;
static struct ctk_label erasedialoglabel1 =
  {CTK_LABEL(2, 1, 22, 1, "Really erase message?")};
static struct ctk_label erasedialoglabel2 =
  {CTK_LABEL(0, 2, 26, 1, "All contents will be lost.")};
static struct ctk_button eraseyesbutton =
  {CTK_BUTTON(4, 4, 3, "Yes")};
static struct ctk_button erasenobutton =
  {CTK_BUTTON(18, 4, 2, "No")};
#endif /* MAIL_ERASE */

/* The setup window. */
static struct ctk_window setupwindow;
static struct ctk_label fromaddresslabel =
  {CTK_LABEL(0, 0, 25, 1, "Name and e-mail address")};
static char fromaddress[40];
static struct ctk_textentry fromaddresstextentry =
  {CTK_TEXTENTRY(0, 1, 26, 1, fromaddress, 39)};

static struct ctk_label smtpserverlabel =
  {CTK_LABEL(0, 3, 20, 1, "Outgoing mailserver")};
static char smtpserver[40];
static struct ctk_textentry smtpservertextentry =
  {CTK_TEXTENTRY(0, 4, 26, 1, smtpserver, 39)};

/*
static struct ctk_label pop3serverlabel =
  {CTK_LABEL(0, 6, 20, 1, "Incoming mailserver")};
static char pop3server[40];
static struct ctk_textentry pop3servertextentry =
  {CTK_TEXTENTRY(0, 7, 26, 1, pop3server, 39)};
*/

/*
static struct ctk_label pop3userlabel =
  {CTK_LABEL(0, 9, 20, 1, "Mailserver username")};
static char pop3user[40];
static struct ctk_textentry pop3usertextentry =
  {CTK_TEXTENTRY(0, 10, 26, 1, pop3user, 39)};
*/

/*
static struct ctk_label pop3passwordlabel =
  {CTK_LABEL(0, 12, 20, 1, "Mailserver password")};
static char pop3password[40];
static struct ctk_textentry pop3passwordtextentry =
  {CTK_TEXTENTRY(0, 13, 26, 1, pop3password, 39)};
*/

static struct ctk_button setupokbutton =
/*  {CTK_BUTTON(24, 15, 2, "Ok")};*/
  {CTK_BUTTON(24, 6, 2, "Ok")};

PROCESS(email_process, "E-mail client");

AUTOSTART_PROCESSES(&email_process);

/*-----------------------------------------------------------------------------------*/
static void
email_quit(void)
{
  ctk_window_close(&setupwindow);
  ctk_window_close(&composewindow);
  ctk_menu_remove(&menu);
  process_exit(&email_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
static void
applyconfig(void)
{
  uip_ipaddr_t addr;
  uip_ipaddr_t *addrptr;
  char *cptr;

  for(cptr = smtpserver; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;
  
  addrptr = &addr;
#if UIP_UDP
  if(uiplib_ipaddrconv(smtpserver, &addr) == 0) {
    if(resolv_lookup(smtpserver, &addrptr) != RESOLV_STATUS_CACHED) {
      resolv_query(smtpserver);
      ctk_label_set_text(&statuslabel, "Resolving host...");
      return;
    }
  }
#else /* UIP_UDP */
  uiplib_ipaddrconv(smtpserver, &addr);
#endif /* UIP_UDP */
  smtp_configure("contiki", addrptr);

  petsciiconv_toascii(fromaddress, sizeof(fromaddress));
}
/*-----------------------------------------------------------------------------------*/
static void
prepare_message(void)
{
  /* Convert fields to ASCII. */
  petsciiconv_toascii(to, sizeof(to));
  petsciiconv_toascii(cc, sizeof(cc));
  petsciiconv_toascii(subject, sizeof(subject));  
  petsciiconv_toascii(mail, sizeof(mail));
}
/*-----------------------------------------------------------------------------------*/
static void
erase_message(void)
{
  CTK_TEXTENTRY_CLEAR(&totextentry);
  CTK_TEXTENTRY_CLEAR(&cctextentry);
  CTK_TEXTENTRY_CLEAR(&subjecttextentry);
  CTK_TEXTENTRY_CLEAR(&mailtextentry);
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(email_process, ev, data)
{
  struct ctk_widget *w;

  PROCESS_BEGIN();
  
#if MAIL_ERASE
  /* Create the "Really erase message?" dialog. */
  ctk_dialog_new(&erasedialog, 26, 6);
  CTK_WIDGET_ADD(&erasedialog, &erasedialoglabel1);
  CTK_WIDGET_ADD(&erasedialog, &erasedialoglabel2);
  CTK_WIDGET_ADD(&erasedialog, &eraseyesbutton);
  CTK_WIDGET_ADD(&erasedialog, &erasenobutton);
  CTK_WIDGET_FOCUS(&erasedialog, &erasenobutton);
#endif /* MAIL_ERASE */
  
  /* Create setup window. */
/*  ctk_window_new(&setupwindow, 28, 16, "E-mail setup");*/
  ctk_window_new(&setupwindow, 28, 7, "E-mail setup");
  
  CTK_WIDGET_ADD(&setupwindow, &fromaddresslabel);
  CTK_WIDGET_ADD(&setupwindow, &fromaddresstextentry);
  CTK_WIDGET_ADD(&setupwindow, &smtpserverlabel);
  CTK_WIDGET_ADD(&setupwindow, &smtpservertextentry);
/*  CTK_WIDGET_ADD(&setupwindow, &pop3serverlabel);*/
/*  CTK_WIDGET_ADD(&setupwindow, &pop3servertextentry);*/
/*  CTK_WIDGET_ADD(&setupwindow, &pop3userlabel);*/
/*  CTK_WIDGET_ADD(&setupwindow, &pop3usertextentry);*/
/*  CTK_WIDGET_ADD(&setupwindow, &pop3passwordlabel);*/
/*  CTK_WIDGET_ADD(&setupwindow, &pop3passwordtextentry);*/
  CTK_WIDGET_ADD(&setupwindow, &setupokbutton);
  
  CTK_WIDGET_FOCUS(&setupwindow, &fromaddresstextentry);
  
  
  /* Create compose window. */
  ctk_window_new(&composewindow, MAIL_WIDTH + 1, MAIL_HEIGHT + 5, "Compose e-mail");
  
  CTK_WIDGET_ADD(&composewindow, &tolabel);
  CTK_WIDGET_ADD(&composewindow, &cclabel);
  CTK_WIDGET_ADD(&composewindow, &subjectlabel);
  
  CTK_WIDGET_ADD(&composewindow, &totextentry);
  CTK_WIDGET_FOCUS(&composewindow, &totextentry);  
  CTK_WIDGET_ADD(&composewindow, &cctextentry);  
  CTK_WIDGET_ADD(&composewindow, &subjecttextentry);
  
  CTK_WIDGET_ADD(&composewindow, &mailtextentry);
  
  CTK_WIDGET_ADD(&composewindow, &sep1);
  CTK_WIDGET_ADD(&composewindow, &statuslabel);
  
  CTK_WIDGET_ADD(&composewindow, &sendbutton);
#if MAIL_ERASE
  CTK_WIDGET_ADD(&composewindow, &erasebutton);
#endif /* MAIL_ERASE */

  erase_message();
  
  /* Create and add the menu */
  ctk_menu_new(&menu, "E-mail");
  menuitem_setup = ctk_menuitem_add(&menu, "Setup");
  menuitem_compose = ctk_menuitem_add(&menu, "Compose");
  menuitem_quit = ctk_menuitem_add(&menu, "Quit");
  ctk_menu_add(&menu);
  
  /* Open setup window */
  ctk_window_open(&setupwindow);

  while(1) {

    PROCESS_WAIT_EVENT();
    
    if(ev == tcpip_event) {
      smtp_appcall(data);      
    } else if(ev == ctk_signal_widget_activate) {
      w = (struct ctk_widget *)data;
      if(w == (struct ctk_widget *)&sendbutton) {
	prepare_message();
	smtp_send(to, cc, fromaddress, subject, mail, MAIL_WIDTH, MAIL_HEIGHT);
	ctk_label_set_text(&statuslabel, "Sending message...");
	CTK_WIDGET_REDRAW(&statuslabel);
#if MAIL_ERASE
      } else if(w == (struct ctk_widget *)&erasebutton) {
	ctk_dialog_open(&erasedialog);      
      } else if(w == (struct ctk_widget *)&eraseyesbutton) {
	erase_message();
	ctk_dialog_close();
      } else if(w == (struct ctk_widget *)&erasenobutton) {
	ctk_dialog_close();
#endif /* MAIL_ERASE */
      } else if(w == (struct ctk_widget *)&setupokbutton) {
	applyconfig();
	ctk_window_close(&setupwindow);
	ctk_window_open(&composewindow);
      }
    } else if(ev == ctk_signal_menu_activate) {
      if((struct ctk_menu *)data == &menu) {
	if(menu.active == menuitem_compose) {
	  ctk_window_open(&composewindow);
	} else if(menu.active == menuitem_setup) {
	  ctk_window_open(&setupwindow);
	} else if(menu.active == menuitem_quit) {
	  email_quit();
	}
      }
#if UIP_UDP
    } else if(ev == resolv_event_found) {
      if(strcmp(data, smtpserver) == 0) {
    if(resolv_lookup(smtpserver, NULL) == RESOLV_STATUS_CACHED) {
	  applyconfig();
	  ctk_label_set_text(&statuslabel, "");
	} else {
	  ctk_label_set_text(&statuslabel, "Host not found");
	}
	CTK_WIDGET_REDRAW(&statuslabel);  
      }
#endif /* UIP_UDP */
    } else if(ev == PROCESS_EVENT_EXIT) {
      email_quit();
    }
  }
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
void
smtp_done(unsigned char error)
{
  if(error == SMTP_ERR_OK) {
    ctk_label_set_text(&statuslabel, "Mail sent");
    erase_message();
    ctk_window_open(&composewindow);
  } else {
    ctk_label_set_text(&statuslabel, "Mail error");
  }
  CTK_WIDGET_REDRAW(&statuslabel);  
}
/*-----------------------------------------------------------------------------------*/
