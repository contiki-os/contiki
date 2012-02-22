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
 * $Id: dhcp-client.c,v 1.5 2010/10/31 22:51:17 oliverschmidt Exp $
 *
 */

#include "contiki-net.h"
#include "ctk/ctk.h"
#include "cfs/cfs.h"
#include "net/dhcpc.h"

static struct ctk_window window;

static struct ctk_button requestbutton =
  {CTK_BUTTON(4, 1, 18, "Request IP address")};
static struct ctk_label statuslabel =
  {CTK_LABEL(0, 3, 14, 1, "")};
static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 5, 10, 1, "IP address")};
static char ipaddr[16];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 5, 15, 1, ipaddr, 15)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 7, 10, 1, "Netmask")};
static char netmask[16];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 7, 15, 1, netmask, 15)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 9, 10, 1, "Gateway")};
static char gateway[16];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 9, 15, 1, gateway, 15)};
#if WITH_DNS
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 11, 10, 1, "DNS server")};
static char dnsserver[16];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 11, 15, 1, dnsserver, 15)};
#endif /* WITH_DNS */
static struct ctk_button savebutton =
  {CTK_BUTTON(0, 13, 12, "Save & close")};
static struct ctk_button cancelbutton =
  {CTK_BUTTON(20, 13, 6, "Cancel")};

PROCESS(dhcp_process, "DHCP client");

AUTOSTART_PROCESSES(&dhcp_process);

/*-----------------------------------------------------------------------------------*/
static char *
makebyte(uint8_t byte, char *str)
{
  if(byte >= 100) {
    *str++ = (byte / 100 ) % 10 + '0';
  }
  if(byte >= 10) {
    *str++ = (byte / 10) % 10 + '0';
  }
  *str++ = (byte % 10) + '0';

  return str;
}
/*-----------------------------------------------------------------------------------*/
static void
makeaddr(uip_ipaddr_t *addr, char *str)
{
  str = makebyte(addr->u8[0], str);
  *str++ = '.';
  str = makebyte(addr->u8[1], str);
  *str++ = '.';
  str = makebyte(addr->u8[2], str);
  *str++ = '.';
  str = makebyte(addr->u8[3], str);
  *str++ = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
makestrings(void)
{
  uip_ipaddr_t addr, *addrptr;

  uip_gethostaddr(&addr);
  makeaddr(&addr, ipaddr);
  
  uip_getnetmask(&addr);
  makeaddr(&addr, netmask);
  
  uip_getdraddr(&addr);
  makeaddr(&addr, gateway);

#if WITH_DNS
  addrptr = resolv_getserver();
  if(addrptr != NULL) {
    makeaddr(addrptr, dnsserver);
  }
#endif /* WITH_DNS */
}
/*-----------------------------------------------------------------------------------*/
static void
nullterminate(char *cptr)
{
  /* Find the first space character in the ipaddr and put a zero there
     to end the string. */
  for(; *cptr != ' ' && *cptr != 0; ++cptr);
  *cptr = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
apply_tcpipconfig(void)
{
  int file = cfs_open("contiki.cfg", CFS_READ);
  int size = cfs_read(file, uip_buf, 100);
  cfs_close(file);

  nullterminate(ipaddr);
  uiplib_ipaddrconv(ipaddr, (uip_ipaddr_t *)&uip_buf[0]);

  nullterminate(netmask);
  uiplib_ipaddrconv(netmask, (uip_ipaddr_t *)&uip_buf[4]);

  nullterminate(gateway);
  uiplib_ipaddrconv(gateway, (uip_ipaddr_t *)&uip_buf[8]);
  
#if WITH_DNS
  nullterminate(dnsserver);
  uiplib_ipaddrconv(dnsserver, (uip_ipaddr_t *)&uip_buf[12]);
#endif /* WITH_DNS */

  file = cfs_open("contiki.cfg", CFS_WRITE);
  cfs_write(file, uip_buf, size);
  cfs_close(file);
}
/*-----------------------------------------------------------------------------------*/
static void
set_statustext(char *text)
{
  ctk_label_set_text(&statuslabel, text);
  CTK_WIDGET_REDRAW(&statuslabel);
}
/*-----------------------------------------------------------------------------------*/
static void
app_quit(void)
{
  ctk_window_close(&window);
  process_exit(&dhcp_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(dhcp_process, ev, data)
{
  PROCESS_BEGIN();
  
  ctk_window_new(&window, 29, 14, "DHCP client");
  
  CTK_WIDGET_ADD(&window, &requestbutton);
  CTK_WIDGET_ADD(&window, &statuslabel);  
  CTK_WIDGET_ADD(&window, &ipaddrlabel);  
  CTK_WIDGET_ADD(&window, &ipaddrtextentry);
  CTK_WIDGET_ADD(&window, &netmasklabel);
  CTK_WIDGET_ADD(&window, &netmasktextentry);
  CTK_WIDGET_ADD(&window, &gatewaylabel);
  CTK_WIDGET_ADD(&window, &gatewaytextentry);
#if WITH_DNS
  CTK_WIDGET_ADD(&window, &dnsserverlabel);
  CTK_WIDGET_ADD(&window, &dnsservertextentry);  
#endif /* WITH_DNS */
  CTK_WIDGET_ADD(&window, &savebutton);
  CTK_WIDGET_ADD(&window, &cancelbutton);

  CTK_WIDGET_FOCUS(&window, &requestbutton);  

  ctk_window_open(&window);

  /* Allow resolver to set DNS server address. */
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);

  dhcpc_init(uip_ethaddr.addr, sizeof(uip_ethaddr.addr));

  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_MSG) {
      makestrings();
      ctk_window_redraw(&window);
    } else if(ev == tcpip_event) {
      dhcpc_appcall(ev, data);
    } else if(ev == ctk_signal_button_activate) {   
      if(data == (process_data_t)&requestbutton) {
	dhcpc_request();
	set_statustext("Requesting...");
      }
      if(data == (process_data_t)&savebutton) {
	apply_tcpipconfig();
	app_quit();
      }
      if(data == (process_data_t)&cancelbutton) {
	app_quit();
      }
    } else if(
#if CTK_CONF_WINDOWCLOSE
	      ev == ctk_signal_window_close ||
#endif
	      ev == PROCESS_EVENT_EXIT) {
      app_quit();
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
void
dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
#if WITH_DNS
  resolv_conf(&s->dnsaddr);
#endif /* WITH_DNS */

  set_statustext("Configured.");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
dhcpc_unconfigured(const struct dhcpc_state *s)
{
  static uip_ipaddr_t nulladdr;

  uip_sethostaddr(&nulladdr);
  uip_setnetmask(&nulladdr);
  uip_setdraddr(&nulladdr);
#if WITH_DNS
  resolv_conf(&nulladdr);
#endif /* WITH_DNS */

  set_statustext("Unconfigured.");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
}
/*-----------------------------------------------------------------------------------*/
