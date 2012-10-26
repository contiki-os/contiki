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
 *
 */

#include "contiki-net.h"
#include "ctk/ctk.h"


/* TCP/IP configuration window. */
static struct ctk_window tcpipwindow;

static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 1, 10, 1, "IP address")};
static char ipaddr[17];
static struct ctk_textentry ipaddrtextentry =
  {CTK_TEXTENTRY(11, 1, 16, 1, ipaddr, 16)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 3, 10, 1, "Netmask")};
static char netmask[17];
static struct ctk_textentry netmasktextentry =
  {CTK_TEXTENTRY(11, 3, 16, 1, netmask, 16)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 5, 10, 1, "Gateway")};
static char gateway[17];
static struct ctk_textentry gatewaytextentry =
  {CTK_TEXTENTRY(11, 5, 16, 1, gateway, 16)};
#if UIP_UDP
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 7, 10, 1, "DNS server")};
static char dnsserver[17];
static struct ctk_textentry dnsservertextentry =
  {CTK_TEXTENTRY(11, 7, 16, 1, dnsserver, 16)};
#endif /* UIP_UDP */
static struct ctk_button tcpipclosebutton =
  {CTK_BUTTON(0, 9, 2, "Ok")};

PROCESS(netconf_process, "Network configurator");

AUTOSTART_PROCESSES(&netconf_process);

static void makestrings(void);

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

#if UIP_UDP
  addrptr = resolv_getserver();
  if(addrptr != NULL) {
    makeaddr(addrptr, dnsserver);
  }
#endif /* UIP_UDP */
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
  uip_ipaddr_t addr;

  nullterminate(ipaddr);
  if(uiplib_ipaddrconv(ipaddr, &addr)) {
    uip_sethostaddr(&addr);
  }
  
  nullterminate(netmask);
  if(uiplib_ipaddrconv(netmask, &addr)) {
    uip_setnetmask(&addr);
  }

  nullterminate(gateway);
  if(uiplib_ipaddrconv(gateway, &addr)) {
    uip_setdraddr(&addr);
  }
  
#if UIP_UDP
  nullterminate(dnsserver);
  if(uiplib_ipaddrconv(dnsserver, &addr)) {
    resolv_conf(&addr);
  }
#endif /* UIP_UDP */
}
/*-----------------------------------------------------------------------------------*/
static void
netconf_quit(void)
{
  process_exit(&netconf_process);
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(netconf_process, ev, data)
{

  PROCESS_BEGIN();
  
  /* Create TCP/IP configuration window. */
  ctk_window_new(&tcpipwindow, 30, 10, "TCP/IP config");
  
  CTK_WIDGET_ADD(&tcpipwindow, &ipaddrlabel);  
  CTK_WIDGET_ADD(&tcpipwindow, &ipaddrtextentry);
  CTK_WIDGET_ADD(&tcpipwindow, &netmasklabel);
  CTK_WIDGET_ADD(&tcpipwindow, &netmasktextentry);
  CTK_WIDGET_ADD(&tcpipwindow, &gatewaylabel);
  CTK_WIDGET_ADD(&tcpipwindow, &gatewaytextentry);
#if UIP_UDP
  CTK_WIDGET_ADD(&tcpipwindow, &dnsserverlabel);
  CTK_WIDGET_ADD(&tcpipwindow, &dnsservertextentry);  
#endif /* UIP_UDP */
  CTK_WIDGET_ADD(&tcpipwindow, &tcpipclosebutton);

  CTK_WIDGET_FOCUS(&tcpipwindow, &ipaddrtextentry);  
  
  /* Fill the configuration strings with values from the current
     configuration */
  makestrings();
    
  ctk_window_open(&tcpipwindow);

  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == ctk_signal_button_activate) {   
      if(data == (process_data_t)&tcpipclosebutton) {
	apply_tcpipconfig();
	ctk_window_close(&tcpipwindow);
	netconf_quit();
      }
    } else if(
#if CTK_CONF_WINDOWCLOSE
	      ev == ctk_signal_window_close ||
#endif
	      ev == PROCESS_EVENT_EXIT) {
      ctk_window_close(&tcpipwindow);
      netconf_quit();
    }
  }

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
