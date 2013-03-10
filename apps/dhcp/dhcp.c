#include "contiki-net.h"
#include "ctk/ctk.h"
#include "net/dhcpc.h"



PROCESS(dhcp_process, "DHCP");

AUTOSTART_PROCESSES(&dhcp_process);

static struct ctk_window window;
static struct ctk_button getbutton =
  {CTK_BUTTON(0, 0, 16, "Request address")};
static struct ctk_label statuslabel =
  {CTK_LABEL(0, 1, 16, 1, "")};


static struct ctk_label ipaddrlabel =
  {CTK_LABEL(0, 3, 10, 1, "IP address")};
static char ipaddr[17];
static struct ctk_textentry ipaddrentry =
  {CTK_LABEL(11, 3, 16, 1, ipaddr)};
static struct ctk_label netmasklabel =
  {CTK_LABEL(0, 4, 10, 1, "Netmask")};
static char netmask[17];
static struct ctk_textentry netmaskentry =
  {CTK_LABEL(11, 4, 16, 1, netmask)};
static struct ctk_label gatewaylabel =
  {CTK_LABEL(0, 5, 10, 1, "Gateway")};
static char gateway[17];
static struct ctk_textentry gatewayentry =
  {CTK_LABEL(11, 5, 16, 1, gateway)};
static struct ctk_label dnsserverlabel =
  {CTK_LABEL(0, 6, 10, 1, "DNS server")};
static char dnsserver[17];
static struct ctk_textentry dnsserverentry =
  {CTK_LABEL(11, 6, 16, 1, dnsserver)};

enum {
  SHOWCONFIG
};
/*---------------------------------------------------------------------------*/
static void
set_statustext(char *text)
{
  ctk_label_set_text(&statuslabel, text);
  CTK_WIDGET_REDRAW(&statuslabel);
}
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
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

  addrptr = resolv_getserver();
  if(addrptr != NULL) {
    makeaddr(addrptr, dnsserver);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dhcp_process, ev, data)
{
  PROCESS_BEGIN();
  
  ctk_window_new(&window, 28, 7, "DHCP");
  
  CTK_WIDGET_ADD(&window, &getbutton);
  CTK_WIDGET_ADD(&window, &statuslabel);
  CTK_WIDGET_ADD(&window, &ipaddrlabel);
  CTK_WIDGET_ADD(&window, &ipaddrentry);
  CTK_WIDGET_ADD(&window, &netmasklabel);
  CTK_WIDGET_ADD(&window, &netmaskentry);
  CTK_WIDGET_ADD(&window, &gatewaylabel);
  CTK_WIDGET_ADD(&window, &gatewayentry);
  CTK_WIDGET_ADD(&window, &dnsserverlabel);
  CTK_WIDGET_ADD(&window, &dnsserverentry);
  
  CTK_WIDGET_FOCUS(&window, &getbutton);

  ctk_window_open(&window);
  dhcpc_init(uip_lladdr.addr, sizeof(uip_lladdr.addr));


  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == ctk_signal_widget_activate) {
      if(data == (process_data_t)&getbutton) {
	dhcpc_request();
	set_statustext("Requesting...");
      }
    } else if(ev == tcpip_event) {
      dhcpc_appcall(ev, data);
    } else if(ev == PROCESS_EVENT_EXIT ||
	      ev == ctk_signal_window_close) {
      ctk_window_close(&window);
      process_exit(&dhcp_process);
      LOADER_UNLOAD();
    } else if(ev == SHOWCONFIG) {
      makestrings();
      ctk_window_redraw(&window);  
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
  resolv_conf(&s->dnsaddr);
  set_statustext("Configured.");
  process_post(PROCESS_CURRENT(), SHOWCONFIG, NULL);
}
/*---------------------------------------------------------------------------*/
void
dhcpc_unconfigured(const struct dhcpc_state *s)
{
  set_statustext("Unconfigured.");
  process_post(PROCESS_CURRENT(), SHOWCONFIG, NULL);
}
/*---------------------------------------------------------------------------*/
