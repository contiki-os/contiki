#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "net/uip-ds6-route.h"
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/netstack.h"
#include "net/rpl/rpl.h"

#include "sicslow-ethernet.h"
#include "cetic-bridge.h"
#include "nvm-config.h"
#include "rio.h"
#include "node-info.h"

#if CONTIKI_TARGET_REDBEE_ECONOTAG
#include "mc1322x.h"
#include "contiki-maca.h"
#endif

#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_prefix_t uip_ds6_prefix_list[];

extern rpl_instance_t instance_table[RPL_MAX_INSTANCES];

int count = 0;
/*---------------------------------------------------------------------------*/
/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"
/* The internal webserver can provide additional information if
 * enough program flash is available.
 */
#define WEBSERVER_CONF_LOADTIME 1
#define WEBSERVER_CONF_FILESTATS 1

#define BUF_USES_STACK 0
/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
	PROCESS_BEGIN();

	httpd_init();

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
		httpd_appcall(data);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#define BUF_SIZE 256
static const char *TOP = "<html><head><title>CETIC WSN</title><link rel=\"stylesheet\" type=\"text/css\" href=\"http://www.cetic.be/squelettes/css/cetic_layout.css\" /><link rel=\"stylesheet\" href=\"http://www.cetic.be/prive/spip_style.css\" type=\"text/css\" /></head>";
static const char *BODY = "<body class=\"page_rubrique\"><div id=\"container\"><div id=\"banner\"><img src=\"http://www.cetic.be/squelettes/css/img/logo.jpg\" style=\"position: absolute; top: 0; left: 0;\"><div id=\"banner_fx\"><div id=\"img_bandeau\"><img src=\"http://www.cetic.be/squelettes/img/banner_home.jpg\"></div><div id=\"barre_nav\"><div class=\"menu-general\"><a href=\"/\">Info</a></div><div class=\"menu-general\"><a href=\"/sensors.html\">Sensors</a></div><div class=\"menu-general\"><a href=\"/rpl.html\">RPL</a></div><div class=\"menu-general\"><a href=\"/network.html\">Network</a></div><div class=\"menu-general\"><a href=\"/config.html\">Config</a></div></div></div></div>\n";
static const char *BOTTOM = "</div></body></html>\n";
#if BUF_USES_STACK
static char *bufptr, *bufend;
#else
static char buf[BUF_SIZE];
static int blen;
#endif
/*---------------------------------------------------------------------------*/
/*OPTIMIZATIONS to gain space : prototypes*/
static void add(char *str,...);
void add_div_home();
void add_network_cases(const uint8_t state);
static void reset_buf();
/*End optimizations*/
/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
	uint16_t a;
	int i, f;
	for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
		a = (addr->u8[i] << 8) + addr->u8[i + 1];
		if(a == 0 && f >= 0) {
			if(f++ == 0) add("::");
		} else {
			if(f > 0) {
				f = -1;
			} else if(i > 0) {
				add(":");
			}
			add("%x", a);
		}
	}
}
#if CONTIKI_TARGET_NATIVE
static void
ipaddr_add_u8(const uint8_t *addr)
{
	uint16_t a;
	int i, f;
	for(i = 0, f = 0; i < 16; i += 2) {
		a = (addr[i] << 8) + addr[i + 1];
		if(a == 0 && f >= 0) {
			if(f++ == 0) add("::");
		} else {
			if(f > 0) {
				f = -1;
			} else if(i > 0) {
				add(":");
			}
			add("%x", a);
		}
	}
}
#endif
static void
lladdr_add(const uip_lladdr_t *addr)
{
	int i;
	for(i = 0; i < sizeof(uip_lladdr_t); i++) {
		if(i > 0) {
			add(":");
		}
		add("%x", addr->addr[i]);
	}
}
static void
ethaddr_add(ethaddr_t *addr)
{
	int i;
	for(i = 0; i < 6; i++) {
		if(i > 0) {
			add(":");
		}

		add("%x", (*addr)[i]);
	}
}
/*---------------------------------------------------------------------------*/

#if CONTIKI_TARGET_REDBEE_ECONOTAG
extern void _start;
//Code
extern void _etext;
//RO data
extern void __data_start;
//Initialised data
extern void _edata;
//Stack
extern void __bss_start;
//Zero initialised data
extern void _bss_end__;
//Heap
extern void _end;
#endif

/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_index(struct httpd_state *s))
{
	static int i;
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\">");
	add("<h2>Info</h2>");
	add("Version : " CONTIKI_VERSION_STRING "<br />");
	add("Mode : ");
#if CETIC_6LBR_SMARTBRIDGE
	add("SMART BRIGDE");
#endif
#if CETIC_6LBR_TRANSPARENTBRIDGE
	add("TRANSPARENT BRIGDE");
#endif
#if CETIC_6LBR_ROUTER
	add("ROUTER");
#endif
	add("<br />\n");
	i = clock_time()/CLOCK_SECOND;
	add("Uptime : %dh %dm %ds<br />", i/3600, (i/60) % 60, i % 60);
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<br /><h2>WSN</h2>");
	add("MAC: %s<br />RDC: %s (%d Hz)<br />",
			NETSTACK_MAC.name,
			NETSTACK_RDC.name,
			CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
			                NETSTACK_RDC.channel_check_interval()));
	add("Prefix : ");
	ipaddr_add(&cetic_dag->prefix_info.prefix);
	add("/%d", cetic_dag->prefix_info.length);
	add("<br />");
	add("HW address : ");
	lladdr_add(&uip_lladdr);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<br /><h2>Ethernet</h2>");
#if CETIC_6LBR_ROUTER
	add("Address : ");
	ipaddr_add(&eth_ip_addr);
	add("<br />");
	add("Local address : ");
	ipaddr_add(&eth_ip_local_addr);
	add("<br />");
#endif
	add("HW address : ");
	ethaddr_add(&eth_mac_addr);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if CONTIKI_TARGET_REDBEE_ECONOTAG
	add("<br /><h2>Memory</h2>");

	add("Global : %d (%d %%)<br /><br />", &_end - &_start, (100 * (&_end - &_start)) / (96*1024) );

	add("Code : %d<br />", &_etext - &_start );
	add("Initialised data : %d<br /><br />", &_edata - &_etext );
	add("Data : %d<br />", &_bss_end__ - &__bss_start );
	add("Stack : %d<br />", &__bss_start - &_edata );
	add("Heap : %d<br />", &_end - &_bss_end__ );
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
#if CETIC_NODE_INFO
static
PT_THREAD(generate_sensors(struct httpd_state *s))
{
	static int i;
	static node_info_t *node;
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\">");
	add("<table class=\"spip\"><theader><tr class=\"row_first\"><td>Node</td><td>Type</td><td>Status</td><td>Last seen</td></tr></theader><tbody>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
		if(node_info_table[i].isused) {
			add("<tr><td><a href=http://[");
			ipaddr_add(&node_info_table[i].ipaddr);
			add("]/>");
			SEND_STRING(&s->sout, buf); //TODO: why tunslip6 needs an output here, wpcapslip does not
			reset_buf();
			ipaddr_add(&node_info_table[i].ipaddr);
			add("</a></td>");

			if ( node_info_table[i].ipaddr.u8[8] == 0x02 && node_info_table[i].ipaddr.u8[9] == 0x12 &&
					node_info_table[i].ipaddr.u8[10] == 0x74 ) {
				add("<td><a href=http://[");
				ipaddr_add(&node_info_table[i].ipaddr);
				add("]/status.shtml>Crossbow</a></td>");
			} else if ( node_info_table[i].ipaddr.u8[8] == 0x02 && node_info_table[i].ipaddr.u8[9] == 0x50 &&
					node_info_table[i].ipaddr.u8[10] == 0xC2 && node_info_table[i].ipaddr.u8[11] == 0xA8 &&
					(node_info_table[i].ipaddr.u8[12] & 0XF0) == 0xC0 ) {
				add("<td>Redwire</td>");
			} else if ( (node_info_table[i].ipaddr.u8[8] & 0x02 ) == 0 ) {
				add("<td>User defined</td>");
			} else {
				add("<td></td>");
			}
			add("<td>%s</td>", node_info_table[i].my_info);
			add("<td>%d</td>", (clock_time() - node_info_table[i].last_lookup)/CLOCK_SECOND);
			add("</tr>");
			SEND_STRING(&s->sout, buf);
			reset_buf();
		}
	}
	add("</tbody></table><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<center><img src=\"http://chart.googleapis.com/chart?cht=gv&chls=1&chl=digraph{_%x;", (uip_lladdr.addr[6]<<8) + uip_lladdr.addr[7]);
	for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
		if(node_info_table[i].isused) {
			int l = strlen(node_info_table[i].my_info);
			if ( l > 4 ) {
				add("_%04hx->_%s;", (node_info_table[i].ipaddr.u8[14]<<8)+node_info_table[i].ipaddr.u8[15], &node_info_table[i].my_info[l-4]);
			}
		}
	}
	add("}\"alt=\"\" /></center>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}
#endif
/*---------------------------------------------------------------------------*/

static
PT_THREAD(generate_rpl(struct httpd_state *s))
{
	static int i;
	static int j;

#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();

	add_div_home();
	add("<div id=\"left_home\">");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	for(i = 0; i < RPL_MAX_INSTANCES; ++i) {
		if ( instance_table[i].used ) {
			add("<h2>Instance %d</h2>", instance_table[i].instance_id);
			for(j = 0; j < RPL_MAX_DAG_PER_INSTANCE; ++j) {
				if ( instance_table[i].dag_table[j].used ) {
					add("<h3>DODAG %d</h3>", j);
					add("DODAG ID : ");
					ipaddr_add(&instance_table[i].dag_table[j].dag_id);
					add("<br />Version : %d", instance_table[i].dag_table[j].version);
					add("<br />Grounded : %s", instance_table[i].dag_table[j].grounded ? "Yes" : "No");
					add("<br />Preference : %d", instance_table[i].dag_table[j].preference);
					add("<br />");
					add("<br />Joined : %s", instance_table[i].dag_table[j].joined ? "Yes" : "No");
					add("<br />Rank : %d", instance_table[i].dag_table[j].rank);
					add("<br />");
					SEND_STRING(&s->sout, buf);
					reset_buf();
				}
			}
		}
	}
	add("<br /><h3>Actions</h3>");
	add("<a href=\"rpl-gr\">Trigger global repair</a><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/

static
PT_THREAD(generate_network(struct httpd_state *s))
{
	static int i;
	static uip_ds6_route_t *r;
	static uip_ds6_defrt_t *dr;

#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();

	add_div_home();
	add("<div id=\"left_home\">");
	add("<br /><h2>Addresses</h2><pre>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	for (i=0;i<UIP_DS6_ADDR_NB;i++) {
		if (uip_ds6_if.addr_list[i].isused) {
			ipaddr_add(&uip_ds6_if.addr_list[i].ipaddr);
			char flag;
			if ( uip_ds6_if.addr_list[i].state == ADDR_TENTATIVE ) flag ='T';
			else if ( uip_ds6_if.addr_list[i].state == ADDR_PREFERRED ) flag ='P';
			else flag = '?';
			add( " %c", flag);
			if ( uip_ds6_if.addr_list[i].type == ADDR_MANUAL ) flag ='M';
			else if ( uip_ds6_if.addr_list[i].type == ADDR_DHCP ) flag ='D';
			else if ( uip_ds6_if.addr_list[i].type == ADDR_AUTOCONF ) flag = 'A';
			else flag = '?';
			add( " %c", flag);
            if ( ! uip_ds6_if.addr_list[i].isinfinite ) {
            	add(" %u s", stimer_remaining(&uip_ds6_if.addr_list[i].vlifetime));
            }
			add("\n");
			SEND_STRING(&s->sout, buf);
			reset_buf();
		}
	}

	add("</pre><h2>Prefixes</h2><pre>");
	for (i=0;i<UIP_DS6_PREFIX_NB;i++) {
		if (uip_ds6_prefix_list[i].isused) {
			ipaddr_add(&uip_ds6_prefix_list[i].ipaddr);
			add(" ");
#if UIP_CONF_ROUTER
			if (uip_ds6_prefix_list[i].advertise) add("*");
#else
			if (uip_ds6_prefix_list[i].isinfinite) add("I");
#endif
			add("\n");
		}
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("</pre><h2>Neighbors</h2><pre>");
	for(i = 0; i < UIP_DS6_NBR_NB; i++) {
		if(uip_ds6_nbr_cache[i].isused) {
			add("[<a href=\"nbr_rm?%d\">X</a>] ", i);
			ipaddr_add(&uip_ds6_nbr_cache[i].ipaddr);
			add(" ");
			lladdr_add(&uip_ds6_nbr_cache[i].lladdr);
			add(" ");
			add_network_cases(uip_ds6_nbr_cache[i].state);
			add("\n");
			SEND_STRING(&s->sout, buf);
			reset_buf();
		}
	}
	add("</pre><h2>Routes</h2><pre>");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	for(r = uip_ds6_route_list_head(), i=0; r != NULL; r = list_item_next(r), ++i) {
		add("[<a href=\"route_rm?%d\">X</a>] ", i);
		ipaddr_add(&r->ipaddr);
		add("/%u (via ", r->length);
		ipaddr_add(&r->nexthop);
		if(1 || (r->state.lifetime < 600)) {
			add(") %lu s\n", r->state.lifetime);
		} else {
			add(")\n");
		}
		SEND_STRING(&s->sout, buf);
		reset_buf();
	}

	add("</pre><h2>Default Routers</h2><pre>");

	for(dr = uip_ds6_defrt_list_head(); dr != NULL; dr = list_item_next(r)) {
		ipaddr_add(&dr->ipaddr);
		if ( ! dr->isinfinite ) {
			add(" %u s", stimer_remaining(&dr->lifetime));
		}
		add("\n");
		SEND_STRING(&s->sout, buf);
		reset_buf();
	}

#if UIP_CONF_DS6_ROUTE_INFORMATION
	add("</pre><h2>Route info</h2><pre>");
	for (i=0;i<UIP_DS6_ROUTE_INFO_NB;i++) {
		if (uip_ds6_route_info_list[i].isused) {
			ipaddr_add(&uip_ds6_route_info_list[i].ipaddr);
			add("/%u (%x) %us\n", uip_ds6_route_info_list[i].length, uip_ds6_route_info_list[i].flags, uip_ds6_route_info_list[i].lifetime);
		}
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

	add("</pre><h2>HW Prefixes cache</h2><pre>");
	for (i=0;i<prefixCounter;i++) {
		add("%02x:%02x:%02x\n", prefixBuffer[i][0], prefixBuffer[i][1], prefixBuffer[i][2]);
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("</pre><br />");

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
#if WEBSERVER_EDITABLE_CONFIG
static
PT_THREAD(generate_config(struct httpd_state *s))
{
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\"><form action=\"config\" method=\"get\">");
	add("<h2>WSN Network</h2>");
	add("<h3>WSN configuration</h3>");
	add("Channel : <input type=\"text\" name=\"channel\" value=\"%d\" /><br />", nvm_data.channel);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<h3>IP configuration</h3>");
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("Network configuration : <br /><input type=\"radio\" name=\"wait_ra\" value=\"1\" %s> autoconfiguration<br />",
			(nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) != 0 ? "checked" : "");
	add("<input type=\"radio\" name=\"wait_ra\" value=\"0\" %s>static<br />",
			(nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) == 0 ? "checked" : "");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("Prefix : <input type=\"text\" name=\"wsn_pre\" value=\"");
	ipaddr_add_u8(&nvm_data.wsn_net_prefix);
	add("\" /><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("Default router : <input type=\"text\" name=\"eth_dft\" value=\"");
	ipaddr_add_u8(&nvm_data.eth_dft_router);
	add("\" /><br />");
#else
	add("Prefix : <input type=\"text\" name=\"wsn_pre\" value=\"");
	ipaddr_add_u8(&nvm_data.wsn_net_prefix);
	add("\" /><br />");
#endif
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("Address : <br />");
	add("<input type=\"radio\" name=\"wsn_auto\" value=\"1\" %s>autoconfiguration<br />",
			(nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) != 0 ? "checked" : "");
	add("<input type=\"radio\" name=\"wsn_auto\" value=\"0\" %s>manual ",
			(nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) == 0 ? "checked" : "");
	add("<input type=\"text\" name=\"wsn_addr\" value=\"");
	ipaddr_add_u8(&nvm_data.wsn_ip_addr);
	add("\" /><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if CETIC_6LBR_ROUTER
	add("<br /><h2>Eth Network</h2>");
	add("<h3>IP configuration</h3>");
	add("Prefix : <input type=\"text\" name=\"eth_pre\" value=\"");
	ipaddr_add_u8(&nvm_data.eth_net_prefix);
	add("\" /><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("Address : <br />");
	add("<input type=\"radio\" name=\"eth_manual\" value=\"0\" %s>autoconfiguration<br />",
			(nvm_data.mode & CETIC_MODE_ETH_MANUAL) == 0 ? "checked" : "");
	add("<input type=\"radio\" name=\"eth_manual\" value=\"1\" %s>manual ",
			(nvm_data.mode & CETIC_MODE_ETH_MANUAL) != 0 ? "checked" : "");
	add("<input type=\"text\" name=\"eth_addr\" value=\"");
	ipaddr_add_u8(&nvm_data.eth_ip_addr);
	add("\" /><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("Default router : <input type=\"text\" name=\"eth_dft\" value=\"");
	ipaddr_add_u8(&nvm_data.eth_dft_router);
	add("\" /><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	add("RA Daemon : ");
	add("<input type=\"radio\" name=\"ra_daemon\" value=\"1\" %s>active (Lifetime : %d)",
			(nvm_data.mode & CETIC_MODE_ROUTER_SEND_CONFIG) != 0 ? "checked" : "",
			UIP_CONF_ROUTER_LIFETIME);
	add("<input type=\"radio\" name=\"ra_daemon\" value=\"0\" %s>inactive",
			(nvm_data.mode & CETIC_MODE_ROUTER_SEND_CONFIG) == 0 ? "checked" : "");
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("<br /><h3>Packet filtering</h3>");
	add("RPL filtering : <br />");
	add( "<input type=\"radio\" name=\"rpl_filter\" value=\"1\" %s>enabled ",
			(nvm_data.mode & CETIC_MODE_FILTER_RPL_MASK) != 0 ? "checked" : "");
	add( "<input type=\"radio\" name=\"rpl_filter\" value=\"1\" %s>disabled ",
			(nvm_data.mode & CETIC_MODE_FILTER_RPL_MASK) == 0 ? "checked" : "");
#else
	add("<br /><h3>Packet filtering</h3>");
	add("Address rewrite : ");
	add( "<input type=\"radio\" name=\"rewrite\" value=\"1\" %s>enabled ",
		(nvm_data.mode & CETIC_MODE_REWRITE_ADDR_MASK) != 0 ? "checked" : "");
	add( "<input type=\"radio\" name=\"rewrite\" value=\"1\" %s>disabled ",
		(nvm_data.mode & CETIC_MODE_REWRITE_ADDR_MASK) == 0 ? "checked" : "");
#endif
	add("<br /><input type=\"submit\" value=\"Submit\"/></form>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_reboot(struct httpd_state *s))
{
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\">");
	add("Restarting BR...<br />");
	add("</div>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	SEND_STRING(&s->sout, BOTTOM);

	process_post(&cetic_bridge_process, 0, NULL);

	PSOCK_END(&s->sout);
}
#else
static
PT_THREAD(generate_config(struct httpd_state *s))
{
	static int i;
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\">");
	add("<h2>WSN Network</h2>");
	add("<h3>WSN configuration</h3>");
	add("Channel : %d<br />", nvm_data.channel);
#if CONTIKI_TARGET_REDBEE_ECONOTAG
	add("PanID : 0x%x<br />", *MACA_MACPANID);
#endif
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

    add("<h3>IP configuration</h3>");
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("Network configuration : ");
	if ((nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) != 0 ) {
		add(" autoconfiguration (<a href=\"no-ra\">static</a>)<br />");
	} else {
		add(" static (<a href=\"ra\">autoconfiguration</a>)<br />");
		add("Prefix : ");
		ipaddr_add(&wsn_net_prefix);
		add("<br />");
	}
#else
	add("Prefix : ");
	ipaddr_add(&wsn_net_prefix);
	add("<br />");
#endif
	add("Address : ");
	if ((nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) != 0 ) {
		add("autoconfiguration");
	} else {
		ipaddr_add(&wsn_ip_addr);
	}
	add("<br />");
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("Default router : ");
	ipaddr_add(&eth_dft_router);
	add("<br />");
#endif
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if CETIC_6LBR_ROUTER
	add("<br /><h2>Eth Network</h2>");
	add("<h3>IP configuration</h3>");
	add("Prefix : ");
	ipaddr_add(&eth_net_prefix);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("Address : ");
	ipaddr_add(&eth_ip_addr);
	add("<br />");
	add("Default router : ");
	ipaddr_add(&eth_dft_router);
	add("<br />");
	add("RA Daemon : ");
	if ((nvm_data.mode & CETIC_MODE_ROUTER_SEND_CONFIG) != 0 ) {
		add("active (Lifetime : %d)", UIP_CONF_ROUTER_LIFETIME);
	} else {
		add("inactive");
	}
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("<br /><h3>Packet filtering</h3>");
	add("RPL filtering : ");
	if ((nvm_data.mode & CETIC_MODE_FILTER_RPL_MASK) != 0 ) {
		add( "enabled (<a href=\"/no-filter-rpl\">disable</a>)<br />");
	} else {
		add( "disabled (<a href=\"/filter-rpl\">enable</a>)<br />");
	}
#endif
#if CETIC_6LBR_ROUTER
	add("<br /><h3>Packet filtering</h3>");
	add("Address rewrite : ");
	if ((nvm_data.mode & CETIC_MODE_REWRITE_ADDR_MASK) != 0 ) {
		add( "enabled (<a href=\"/no-rewrite\">disable</a>)<br />");
	} else {
		add( "disabled (<a href=\"/rewrite\">enable</a>)<br />");
	}
#endif
	add("<br />\n");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}
#endif
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_404(struct httpd_state *s))
{
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
	add_div_home();
	add("<div id=\"left_home\">");
	add("404 : Page not found<br />");
	add("</div>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	SEND_STRING(&s->sout, BOTTOM);
	PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

#if WEBSERVER_EDITABLE_CONFIG
int
update_config(const char * name)
{
	const char * ptr = name;
	char * next;
	uint8_t reboot_needed = 0;
	uint8_t do_update = 1;
    uip_ipaddr_t loc_fipaddr;

	while (*ptr) {
		const char *  param = ptr;
		next = index(ptr, '=');
		if (!next) break;
		*next=0;
		ptr = next + 1;
		const char * value = ptr;
		next = index(ptr, '&');
		if (next) {
			*next=0;
			ptr = next + 1;
		} else {
			ptr += strlen(ptr);
		}

		PRINTF("Got param: '%s' = '%s'\n", param, value);
		if ( strcmp(param, "wait_ra") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_WAIT_RA_MASK;
				reboot_needed = 1;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_WAIT_RA_MASK;
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "channel") == 0) {
			nvm_data.channel = atoi(value);
			reboot_needed = 1;
		} else if ( strcmp(param, "wsn_pre") == 0) {
		    if(uiplib_ipaddrconv(value, &loc_fipaddr)) {
		    	memcpy(&nvm_data.wsn_net_prefix, &loc_fipaddr.u8, sizeof(nvm_data.wsn_net_prefix));
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "wsn_auto") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_WSN_AUTOCONF;
				reboot_needed = 1;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_WSN_AUTOCONF;
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "wsn_addr") == 0) {
		    if(uiplib_ipaddrconv(value, &loc_fipaddr)) {
		    	memcpy(&nvm_data.wsn_ip_addr, &loc_fipaddr.u8, sizeof(nvm_data.wsn_ip_addr));
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "eth_pre") == 0) {
		    if(uiplib_ipaddrconv(value, &loc_fipaddr)) {
		    	memcpy(&nvm_data.eth_net_prefix, &loc_fipaddr.u8, sizeof(nvm_data.eth_net_prefix));
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "eth_manual") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_ETH_MANUAL;
				reboot_needed = 1;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_ETH_MANUAL;
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "eth_addr") == 0) {
		    if(uiplib_ipaddrconv(value, &loc_fipaddr)) {
		    	memcpy(&nvm_data.eth_ip_addr, &loc_fipaddr.u8, sizeof(nvm_data.eth_ip_addr));
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "eth_dft") == 0) {
		    if(uiplib_ipaddrconv(value, &loc_fipaddr)) {
		    	memcpy(&nvm_data.eth_dft_router, &loc_fipaddr.u8, sizeof(nvm_data.eth_dft_router));
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "ra_daemon") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_ROUTER_SEND_CONFIG;
				reboot_needed = 1;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_ROUTER_SEND_CONFIG;
				reboot_needed = 1;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "rpl_filter") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_FILTER_RPL_MASK;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_FILTER_RPL_MASK;
		    } else {
		    	do_update = 0;
		    }
		} else if ( strcmp(param, "rewrite") == 0) {
			if (strcmp(value, "0") == 0) {
				nvm_data.mode &= ~CETIC_MODE_REWRITE_ADDR_MASK;
			} else if (strcmp(value, "1") == 0) {
				nvm_data.mode |= CETIC_MODE_REWRITE_ADDR_MASK;
		    } else {
		    	do_update = 0;
		    }
		}
	}
	if ( do_update ) {
		store_nvm_config();
	}
	return ! reboot_needed;
}
#endif

/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
	static uip_ds6_route_t *r;
	static int i;

	if (strcmp(name, "index.html") == 0 || strcmp(name, "") == 0) {
		return generate_index;
#if CETIC_NODE_INFO
	} else if (strcmp(name, "sensors.html") == 0) {
			return generate_sensors;
#endif
	} else if (strcmp(name, "rpl.html") == 0) {
		return generate_rpl;
	} else if (strcmp(name, "network.html") == 0) {
		return generate_network;
	} else if (strcmp(name, "config.html") == 0) {
		return generate_config;
	} else if (strcmp(name, "rpl-gr") == 0) {
		rpl_repair_root(RPL_DEFAULT_INSTANCE);
		return generate_rpl;
	} else if (memcmp(name, "route_rm?", 9) == 0) {
		i = atoi(name+9);
		for(r = uip_ds6_route_list_head(); r != NULL; r = list_item_next(r), --i) {
			if ( i == 0 ) {
				uip_ds6_route_rm(r);
				break;
			}
		}
		return generate_network;
	} else if (memcmp(name, "nbr_rm?", 7) == 0) {
		uip_ds6_nbr_cache[atoi(name+7)].isused = 0;
		return generate_network;
#if !WEBSERVER_EDITABLE_CONFIG
	} else if (strcmp(name, "rewrite") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_REWRITE_ADDR_MASK) | CETIC_MODE_REWRITE_ADDR_MASK;
		store_nvm_config();
		return generate_config;
	} else if (strcmp(name, "no-rewrite") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_REWRITE_ADDR_MASK);
		store_nvm_config();
		return generate_config;
	} else if (strcmp(name, "filter-rpl") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_FILTER_RPL_MASK) | CETIC_MODE_FILTER_RPL_MASK;
		store_nvm_config();
		return generate_config;
	} else if (strcmp(name, "no-filter-rpl") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_FILTER_RPL_MASK);
		store_nvm_config();
		return generate_config;
	} else if (strcmp(name, "ra") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_WAIT_RA_MASK) | CETIC_MODE_WAIT_RA_MASK;
		store_nvm_config();
		return generate_config;
	} else if (strcmp(name, "no-ra") == 0) {
		nvm_data.mode = (nvm_data.mode & ~CETIC_MODE_WAIT_RA_MASK);
		store_nvm_config();
		return generate_config;
#else
	} else if (memcmp(name, "config?", 7) == 0) {
		if ( update_config(name+7) ) {
			return generate_config;
		} else {
			return generate_reboot;
		}
#endif
	} else {
		return generate_404;
	}
}
/*---------------------------------------------------------------------------*/
void
add_div_home()
{
    add("<div id=\"intro_home\"><h1>CETIC Wireless Sensor Network</h1></div>");
}
/*---------------------------------------------------------------------------*/
void
add_network_cases(const uint8_t state)
{
    switch (state) {
    case NBR_INCOMPLETE: add("INCOMPLETE");break;
	case NBR_REACHABLE: add("REACHABLE");break;
	case NBR_STALE: add("STALE");break;
	case NBR_DELAY: add("DELAY");break;
	case NBR_PROBE: add("NBR_PROBE");break;
	}
}
/*---------------------------------------------------------------------------*/
/*Macro redefined : RESET_BUF()*/
/*---------------------------------------------------------------------------*/
static void
reset_buf()
{
#if BUF_USES_STACK
    bufptr = buf;
    bufend = bufptr + sizeof(buf);
#else
    blen = 0;
#endif
}
/*---------------------------------------------------------------------------*/
/*Macro redefined : ADD()*/
/*---------------------------------------------------------------------------*/
static void
add(char *str, ...)
{
    va_list arg; 
    va_start (arg, str);
    //ADD(str, arg); //TODO : bug while formating
#if BUF_USES_STACK
    bufptr += vsnprintf(bufptr, bufend - bufptr, str, arg);
#else
    blen += vsnprintf(&buf[blen], sizeof(buf) - blen, str, arg);
#endif
    va_end (arg);
}
/*---------------------------------------------------------------------------*/
